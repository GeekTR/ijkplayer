/*
 * Copyright (c) 2016 Bilibili
 * copyright (c) 2016 Zhang Rui <bbcallen@gmail.com>
 *
 * This file is part of ijkPlayer.
 *
 * ijkPlayer is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * ijkPlayer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with ijkPlayer; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "internal.h"
#if TARGET_OS_OSX
#import <OpenGL/OpenGL.h>
#import <OpenGL/gl3.h>
#else
#if __OBJC__
#import <OpenGLES/ES3/gl.h>
#import <OpenGLES/EAGL.h>
//https://github.com/lechium/iOS14Headers/blob/c0b7e9f0049c6b2571358584eed67c841140624f/System/Library/Frameworks/OpenGLES.framework/EAGLContext.h
@interface EAGLContext ()
- (BOOL)texImageIOSurface:(IOSurfaceRef)arg1 target:(unsigned long long)arg2 internalFormat:(unsigned long long)arg3 width:(unsigned)arg4 height:(unsigned)arg5 format:(unsigned long long)arg6 type:(unsigned long long)arg7 plane:(unsigned)arg8 invert:(BOOL)arg9;
//begin ios 11
- (BOOL)texImageIOSurface:(IOSurfaceRef)arg1 target:(unsigned long long)arg2 internalFormat:(unsigned long long)arg3 width:(unsigned)arg4 height:(unsigned)arg5 format:(unsigned long long)arg6 type:(unsigned long long)arg7 plane:(unsigned)arg8;
@end

#endif
#endif

#import <CoreVideo/CoreVideo.h>
#include "ijksdl_vout_overlay_videotoolbox.h"
#include "ijksdl_vout_overlay_ffmpeg.h"
#include "renderer_pixfmt.h"
#import "ijk_vout_common.h"

#if TARGET_OS_OSX
#define GL_TEXTURE_TARGET GL_TEXTURE_RECTANGLE
#else
#define GL_TEXTURE_TARGET GL_TEXTURE_2D
#endif

typedef struct _Frame_Size
{
    int w;
    int h;
}Frame_Size;

typedef struct IJK_GLES2_Renderer_Opaque
{
    GLint                 isSubtitle;
    int samples;
#if TARGET_OS_OSX
    GLint                 textureDimension[3];
    Frame_Size            frameSize[3];
#endif
} IJK_GLES2_Renderer_Opaque;

static GLboolean use(IJK_GLES2_Renderer *renderer)
{
    ALOGI("use common vtb render\n");
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    //glEnable(GL_TEXTURE_TARGET);
    glUseProgram(renderer->program);            IJK_GLES2_checkError_TRACE("glUseProgram");
    IJK_GLES2_Renderer_Opaque * opaque = renderer->opaque;
    assert(opaque->samples);
    
    if (0 == renderer->plane_textures[0])
        glGenTextures(opaque->samples, renderer->plane_textures);
    return GL_TRUE;
}

static GLsizei getBufferWidth(IJK_GLES2_Renderer *renderer, SDL_VoutOverlay *overlay)
{
    if (!overlay)
        return 0;

    if (overlay->format == SDL_FCC__VTB || overlay->format == SDL_FCC__FFVTB) {
        return overlay->pitches[0];
    } else {
        assert(0);
    }
}

static GLboolean upload_texture_use_IOSurface(CVPixelBufferRef pixel_buffer,IJK_GLES2_Renderer *renderer)
{
    uint32_t cvpixfmt = CVPixelBufferGetPixelFormatType(pixel_buffer);
    struct vt_format *f = vt_get_gl_format(cvpixfmt);
    if (!f) {
        ALOGE("CVPixelBuffer has unsupported format type\n");
        return GL_FALSE;
    }

    const bool planar = CVPixelBufferIsPlanar(pixel_buffer);
    const int planes  = (int)CVPixelBufferGetPlaneCount(pixel_buffer);
    assert(planar && planes == f->planes || f->planes == 1);
    
    GLenum gl_target = GL_TEXTURE_TARGET;
    
    IOSurfaceRef surface = CVPixelBufferGetIOSurface(pixel_buffer);
    
    if (!surface) {
        printf("CVPixelBuffer has no IOSurface\n");
        return GL_FALSE;
    }
    
    for (int i = 0; i < f->planes; i++) {
        GLfloat w = (GLfloat)CVPixelBufferGetWidthOfPlane(pixel_buffer, i);
        GLfloat h = (GLfloat)CVPixelBufferGetHeightOfPlane(pixel_buffer, i);

        //设置采样器位置，保证了每个uniform采样器对应着正确的纹理单元
        glUniform1i(renderer->us2_sampler[i], i);
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(gl_target, renderer->plane_textures[i]);
        struct vt_gl_plane_format plane_format = f->gl[i];
#if TARGET_OS_IOS
        if (![[EAGLContext currentContext] texImageIOSurface:surface target:gl_target internalFormat:plane_format.gl_internal_format width:w height:h format:plane_format.gl_format type:plane_format.gl_type plane:i invert:NO]) {
            ALOGE("creating IOSurface texture for plane %d failed.\n",i);
            return GL_FALSE;
        }
//        
//        //(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid* pixels)
//        glTexImage2D(gl_target, 0, plane_format.gl_internal_format, w, h, 0, plane_format.gl_format, plane_format.gl_type, CVPixelBufferGetBaseAddressOfPlane(pixel_buffer,i));
        
#else
        Frame_Size size = renderer->opaque->frameSize[i];
        if (size.w != w || size.h != h) {
            glUniform2f(renderer->opaque->textureDimension[i], w, h);
            size.w = w;
            size.h = h;
            renderer->opaque->frameSize[i] = size;
        }
        
        CGLError err = CGLTexImageIOSurface2D(CGLGetCurrentContext(),
                                              gl_target,
                                              plane_format.gl_internal_format,
                                              w,
                                              h,
                                              plane_format.gl_format,
                                              plane_format.gl_type,
                                              surface,
                                              i);
        if (err != kCGLNoError) {
            ALOGE("creating IOSurface texture for plane %d failed: %s\n",
                   i, CGLErrorString(err));
            return GL_FALSE;
        }
#endif
        {
            glTexParameteri(gl_target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(gl_target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameterf(gl_target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameterf(gl_target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
    }
    return GL_TRUE;
}

static GLboolean upload_Texture(IJK_GLES2_Renderer *renderer, void *picture)
{
    if (!picture) {
        return GL_FALSE;
    }
    CVPixelBufferRef pixel_buffer = (CVPixelBufferRef)picture;
    
    IJK_GLES2_Renderer_Opaque *opaque = renderer->opaque;
    if (!opaque) {
        return GL_FALSE;
    }
    
    if (renderer->colorMatrix != YUV_2_RGB_Color_Matrix_None) {
        glUniformMatrix3fv(renderer->um3_color_conversion, 1, GL_FALSE, IJK_GLES2_getColorMatrix(renderer->colorMatrix));
    }
    
    if (renderer->fullRangeUM != -1) {
        glUniform1i(renderer->fullRangeUM, renderer->isFullRange);
    }
    
    if (renderer->transferFunUM != -1) {
        glUniform1i(renderer->transferFunUM, renderer->transferFun);
    }
    
    if (renderer->hdrAnimationUM != -1) {
        glUniform1f(renderer->hdrAnimationUM, renderer->hdrAnimationPercentage);
    }
    
    IJK_GLES2_checkError_TRACE("transferFunUM");
    CVPixelBufferRetain(pixel_buffer);
    GLboolean uploaded = upload_texture_use_IOSurface(pixel_buffer, renderer);
    CVPixelBufferRelease(pixel_buffer);
    return uploaded;
}

static void * getVideoImage(IJK_GLES2_Renderer *renderer, SDL_VoutOverlay *overlay)
{
    assert(0);
    //why call me?
    return NULL;
}

static GLvoid destroy(IJK_GLES2_Renderer *renderer)
{
    if (!renderer || !renderer->opaque)
        return;
    free(renderer->opaque);
    renderer->opaque = nil;
}

static GLvoid useSubtitle(IJK_GLES2_Renderer *renderer,GLboolean subtitle)
{
    glUniform1i(renderer->opaque->isSubtitle, (GLint)subtitle);
}

static GLvoid updateHDRAnimation(IJK_GLES2_Renderer *renderer,float per)
{
    renderer->hdrAnimationPercentage = per;
}

static GLboolean uploadSubtitle(IJK_GLES2_Renderer *renderer,void *subtitle)
{
    if (!subtitle) {
        return GL_FALSE;
    }
        
    IJK_GLES2_Renderer_Opaque *opaque = renderer->opaque;
    if (!opaque) {
        return GL_FALSE;
    }
    
    CVPixelBufferRef cvPixelRef = (CVPixelBufferRef)subtitle;
    CVPixelBufferRetain(cvPixelRef);
    GLboolean uploaded = upload_texture_use_IOSurface(cvPixelRef, renderer);
    CVPixelBufferRelease(cvPixelRef);
    return uploaded;
}

IJK_GLES2_Renderer *ijk_create_common_gl_Renderer(CVPixelBufferRef videoPicture, int openglVer)
{
    Uint32 cv_format = CVPixelBufferGetPixelFormatType(videoPicture);
    CFStringRef colorMatrixStr = CVBufferGetAttachment(videoPicture, kCVImageBufferYCbCrMatrixKey, NULL);
    CFStringRef transferFuntion = CVBufferGetAttachment(videoPicture, kCVImageBufferTransferFunctionKey, NULL);
    
    IJK_SHADER_TYPE shaderType;
    if (cv_format == kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange || cv_format == kCVPixelFormatType_420YpCbCr8BiPlanarFullRange) {
        ALOGI("create render yuv420sp\n");
        shaderType = YUV_2P_SDR_SHADER;
    } else if (cv_format == kCVPixelFormatType_32BGRA) {
        ALOGI("create render bgrx\n");
        shaderType = BGRX_SHADER;
    } else if (cv_format == kCVPixelFormatType_32ARGB) {
        ALOGI("create render xrgb\n");
        shaderType = XRGB_SHADER;
    } else if (cv_format == kCVPixelFormatType_420YpCbCr8Planar ||
               cv_format == kCVPixelFormatType_420YpCbCr8PlanarFullRange) {
        ALOGI("create render yuv420p\n");
        shaderType = YUV_3P_SHADER;
    }
    #if TARGET_OS_OSX
    else if (cv_format == kCVPixelFormatType_422YpCbCr8) {
        ALOGI("create render uyvy\n");
        shaderType = UYVY_SHADER;
    } else if (cv_format == kCVPixelFormatType_422YpCbCr8_yuvs || cv_format == kCVPixelFormatType_422YpCbCr8FullRange) {
        ALOGI("create render yuyv\n");
        shaderType = YUYV_SHADER;
    }
    #endif
    else if (cv_format == kCVPixelFormatType_444YpCbCr10BiPlanarVideoRange ||
             cv_format == kCVPixelFormatType_444YpCbCr10BiPlanarFullRange ||
             cv_format == kCVPixelFormatType_422YpCbCr10BiPlanarVideoRange ||
             cv_format == kCVPixelFormatType_422YpCbCr10BiPlanarFullRange ||
             cv_format == kCVPixelFormatType_420YpCbCr10BiPlanarVideoRange ||
             cv_format == kCVPixelFormatType_420YpCbCr10BiPlanarFullRange
             ) {
        if (colorMatrixStr != nil &&
            CFStringCompare(colorMatrixStr, kCVImageBufferYCbCrMatrix_ITU_R_2020, 0) == kCFCompareEqualTo) {
            shaderType = YUV_2P_HDR_SHADER;
        } else {
            shaderType = YUV_2P_SDR_SHADER;
        }
    } else {
        ALOGE("create render failed,unknown format:%4s\n",(char *)&cv_format);
        return NULL;
    }
    
    YUV_2_RGB_Color_Matrix colorMatrixType = YUV_2_RGB_Color_Matrix_None;
    if (shaderType != BGRX_SHADER && shaderType != XRGB_SHADER) {
        if (colorMatrixStr != nil) {
            if (CFStringCompare(colorMatrixStr, kCVImageBufferYCbCrMatrix_ITU_R_709_2, 0) == kCFCompareEqualTo) {
                colorMatrixType = YUV_2_RGB_Color_Matrix_BT709;
            } else if (CFStringCompare(colorMatrixStr, kCVImageBufferYCbCrMatrix_ITU_R_601_4, 0) == kCFCompareEqualTo) {
                colorMatrixType = YUV_2_RGB_Color_Matrix_BT601;
            } else if (CFStringCompare(colorMatrixStr, kCVImageBufferYCbCrMatrix_ITU_R_2020, 0) == kCFCompareEqualTo) {
                colorMatrixType = YUV_2_RGB_Color_Matrix_BT2020;
            } else {
                colorMatrixType = YUV_2_RGB_Color_Matrix_BT709;
            }
        } else {
            colorMatrixType = YUV_2_RGB_Color_Matrix_BT709;
        }
    }
    int fullRange = 0;
    //full color range
    if (kCVPixelFormatType_420YpCbCr8BiPlanarFullRange == cv_format ||
        kCVPixelFormatType_420YpCbCr8PlanarFullRange == cv_format ||
        kCVPixelFormatType_422YpCbCr8FullRange == cv_format ||
        kCVPixelFormatType_420YpCbCr10BiPlanarFullRange == cv_format ||
        kCVPixelFormatType_422YpCbCr10BiPlanarFullRange == cv_format ||
        kCVPixelFormatType_444YpCbCr10BiPlanarFullRange == cv_format) {
        fullRange = 1;
    }
    IJK_Color_Transfer_Function tf;
    if (transferFuntion) {
        if (CFStringCompare(transferFuntion, IJK_TransferFunction_ITU_R_2100_HLG, 0) == kCFCompareEqualTo) {
            tf = IJK_Color_Transfer_Function_HLG;
        } else if (CFStringCompare(transferFuntion, IJK_TransferFunction_SMPTE_ST_2084_PQ, 0) == kCFCompareEqualTo || CFStringCompare(transferFuntion, IJK_TransferFunction_SMPTE_ST_428_1, 0) == kCFCompareEqualTo) {
            tf = IJK_Color_Transfer_Function_PQ;
        } else {
            tf = IJK_Color_Transfer_Function_LINEAR;
        }
    } else {
        tf = IJK_Color_Transfer_Function_LINEAR;
    }
        
    char shader_buffer[4096] = { '\0' };
    
    ijk_get_apple_common_fragment_shader(shaderType,shader_buffer,openglVer);
    IJK_GLES2_Renderer *renderer = IJK_GLES2_Renderer_create_base(shader_buffer,openglVer);

    if (!renderer)
        goto fail;
    
    renderer->colorMatrix = colorMatrixType;
    renderer->isFullRange = fullRange;
    renderer->transferFun = tf;
    
    const int samples = IJK_Sample_Count_For_Shader(shaderType);
    assert(samples);
    
    for (int i = 0; i < samples; i++) {
        char name[20] = "us2_Sampler";
        name[strlen(name)] = (char)i + '0';
        renderer->us2_sampler[i] = glGetUniformLocation(renderer->program, name); IJK_GLES2_checkError_TRACE("glGetUniformLocation(us2_Sampler)");
    }
    
    //yuv to rgb
    renderer->um3_color_conversion = glGetUniformLocation(renderer->program, "um3_ColorConversion");
    renderer->fullRangeUM   = glGetUniformLocation(renderer->program, "isFullRange");
    renderer->transferFunUM = glGetUniformLocation(renderer->program, "transferFun");
    renderer->hdrAnimationUM = glGetUniformLocation(renderer->program, "hdrPercentage");
    renderer->um3_rgb_adjustment = glGetUniformLocation(renderer->program, "um3_rgbAdjustment"); IJK_GLES2_checkError_TRACE("glGetUniformLocation(um3_rgb_adjustmentVector)");
    
    renderer->func_use            = use;
    renderer->func_getBufferWidth = getBufferWidth;
    renderer->func_uploadTexture  = upload_Texture;
    renderer->func_getVideoImage  = getVideoImage;
    renderer->func_destroy        = destroy;
    renderer->func_useSubtitle    = useSubtitle;
    renderer->func_uploadSubtitle = uploadSubtitle;
    renderer->func_updateHDRAnimation = updateHDRAnimation;
    renderer->opaque = calloc(1, sizeof(IJK_GLES2_Renderer_Opaque));
    if (!renderer->opaque)
        goto fail;
    bzero(renderer->opaque, sizeof(IJK_GLES2_Renderer_Opaque));
    renderer->opaque->samples = samples;
    renderer->opaque->isSubtitle = glGetUniformLocation(renderer->program, "isSubtitle");
    
#if TARGET_OS_OSX
    for (int i = 0; i < samples; i++) {
        char name[20] = "textureDimension";
        name[strlen(name)] = (char)i + '0';
        GLint textureDimension = glGetUniformLocation(renderer->program, name);
        assert(textureDimension >= 0);
        renderer->opaque->textureDimension[i] = textureDimension;
    }
#endif
    renderer->format = cv_format;
    return renderer;
fail:
    IJK_GLES2_Renderer_free(renderer);
    return NULL;
}
