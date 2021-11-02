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
#ifdef __APPLE__
#include <TargetConditionals.h>
#import <CoreVideo/CoreVideo.h>
#endif
#include "math_util.c"

static void IJK_GLES2_printProgramInfo(GLuint program)
{
    if (!program)
        return;

    GLint info_len = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &info_len);
    if (!info_len) {
        ALOGE("[GLES2][Program] empty info\n");
        return;
    }

    char    buf_stack[32];
    char   *buf_heap = NULL;
    char   *buf      = buf_stack;
    GLsizei buf_len  = sizeof(buf_stack) - 1;
    if (info_len > sizeof(buf_stack)) {
        buf_heap = (char*) malloc(info_len + 1);
        if (buf_heap) {
            buf     = buf_heap;
            buf_len = info_len;
        }
    }

    glGetProgramInfoLog(program, buf_len, NULL, buf);
    ALOGE("[GLES2][Program] error %s\n", buf);

    if (buf_heap)
        free(buf_heap);
}

void IJK_GLES2_Renderer_reset(IJK_GLES2_Renderer *renderer)
{
    if (!renderer)
        return;

    if (renderer->vertex_shader)
        glDeleteShader(renderer->vertex_shader);
    if (renderer->fragment_shader)
        glDeleteShader(renderer->fragment_shader);
    if (renderer->program)
        glDeleteProgram(renderer->program);

    renderer->vertex_shader   = 0;
    renderer->fragment_shader = 0;
    renderer->program         = 0;

    for (int i = 0; i < IJK_GLES2_MAX_PLANE; ++i) {
        if (renderer->plane_textures[i]) {
            glDeleteTextures(1, &renderer->plane_textures[i]);
            renderer->plane_textures[i] = 0;
        }
    }
}

void IJK_GLES2_Renderer_free(IJK_GLES2_Renderer *renderer)
{
    if (!renderer)
        return;

    if (renderer->func_destroy)
        renderer->func_destroy(renderer);

#if 0
    if (renderer->vertex_shader)    ALOGW("[GLES2] renderer: vertex_shader not deleted.\n");
    if (renderer->fragment_shader)  ALOGW("[GLES2] renderer: fragment_shader not deleted.\n");
    if (renderer->program)          ALOGW("[GLES2] renderer: program not deleted.\n");

    for (int i = 0; i < IJK_GLES2_MAX_PLANE; ++i) {
        if (renderer->plane_textures[i])
            ALOGW("[GLES2] renderer: plane texture[%d] not deleted.\n", i);
    }
#endif

    free(renderer);
}

void IJK_GLES2_Renderer_freeP(IJK_GLES2_Renderer **renderer)
{
    if (!renderer || !*renderer)
        return;

    IJK_GLES2_Renderer_free(*renderer);
    *renderer = NULL;
}

IJK_GLES2_Renderer *IJK_GLES2_Renderer_create_base(const char *fragment_shader_source)
{
    assert(fragment_shader_source);

    IJK_GLES2_Renderer *renderer = (IJK_GLES2_Renderer *)calloc(1, sizeof(IJK_GLES2_Renderer));
    if (!renderer)
        goto fail;

    renderer->vertex_shader = IJK_GLES2_loadShader(GL_VERTEX_SHADER, IJK_GLES2_getVertexShader_default());
    if (!renderer->vertex_shader)
        goto fail;

    renderer->fragment_shader = IJK_GLES2_loadShader(GL_FRAGMENT_SHADER, fragment_shader_source);
    if (!renderer->fragment_shader)
        goto fail;

    renderer->program = glCreateProgram();                          IJK_GLES2_checkError("glCreateProgram");
    if (!renderer->program)
        goto fail;

    glAttachShader(renderer->program, renderer->vertex_shader);     IJK_GLES2_checkError("glAttachShader(vertex)");
    glAttachShader(renderer->program, renderer->fragment_shader);   IJK_GLES2_checkError("glAttachShader(fragment)");
    glLinkProgram(renderer->program);                               IJK_GLES2_checkError("glLinkProgram");
    GLint link_status = GL_FALSE;
    glGetProgramiv(renderer->program, GL_LINK_STATUS, &link_status);
    if (!link_status)
        goto fail;


    renderer->av4_position = glGetAttribLocation(renderer->program, "av4_Position");                IJK_GLES2_checkError_TRACE("glGetAttribLocation(av4_Position)");
    renderer->av2_texcoord = glGetAttribLocation(renderer->program, "av2_Texcoord");                IJK_GLES2_checkError_TRACE("glGetAttribLocation(av2_Texcoord)");
    renderer->um4_mvp      = glGetUniformLocation(renderer->program, "um4_ModelViewProjection");    IJK_GLES2_checkError_TRACE("glGetUniformLocation(um4_ModelViewProjection)");

    return renderer;

fail:

    if (renderer && renderer->program)
        IJK_GLES2_printProgramInfo(renderer->program);

    IJK_GLES2_Renderer_free(renderer);
    return NULL;
}

IJK_GLES2_Renderer *IJK_GLES2_Renderer_create(SDL_VoutOverlay *overlay)
{
    if (!overlay)
        return NULL;

    IJK_GLES2_printString("Version", GL_VERSION);
    IJK_GLES2_printString("Vendor", GL_VENDOR);
    IJK_GLES2_printString("Renderer", GL_RENDERER);
    IJK_GLES2_printString("Extensions", GL_EXTENSIONS);

    IJK_GLES2_Renderer *renderer = NULL;
    
    switch (overlay->format)
    {
        case SDL_FCC__VTB:
            if (overlay->ff_format == kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange || overlay->ff_format == kCVPixelFormatType_420YpCbCr8BiPlanarFullRange) {
                renderer = IJK_GL_Renderer_create_yuv420sp_vtb(overlay);
            } else if (overlay->ff_format == kCVPixelFormatType_32BGRA) {
                renderer = IJK_GL_Renderer_create_rgbx();
            } else if (overlay->ff_format == kCVPixelFormatType_24RGB) {
                renderer = IJK_GL_Renderer_create_rgbx();
            } else if (overlay->ff_format == kCVPixelFormatType_32ARGB) {
                renderer = IJK_GL_Renderer_create_xrgb();
            }
#if TARGET_OS_OSX
            else if (overlay->ff_format == kCVPixelFormatType_422YpCbCr8) {
                renderer = IJK_GL_Renderer_create_uyvy();
            }
#endif
            else {
                ALOGE("unknown pixformat!");
                assert(0);
            }
            break;
        case SDL_FCC_I420:
            renderer = IJK_GLES2_Renderer_create_yuv420p();
            break;
        case SDL_FCC_NV12:
            renderer = IJK_GL_Renderer_create_yuv420sp();
            break;
        case SDL_FCC_RGB565:
        case SDL_FCC_BGR565:
        case SDL_FCC_BGR24:
        case SDL_FCC_RGB24:
        case SDL_FCC_RGBA:
        case SDL_FCC_RGB0:
        case SDL_FCC_BGRA:
        case SDL_FCC_BGR0:
            renderer = IJK_GL_Renderer_create_rgbx();
            break;
        case SDL_FCC_ARGB:
        case SDL_FCC_0RGB:
            renderer = IJK_GL_Renderer_create_xrgb();
            break;
        case SDL_FCC_YV12:      renderer = IJK_GLES2_Renderer_create_yuv420p();
            break;
#if TARGET_OS_IPHONE
        case SDL_FCC_I444P10LE: renderer = IJK_GLES2_Renderer_create_yuv444p10le();
            break;
#endif
        default:
            assert(0);
            break;
    }

    if (renderer) {
        renderer->format = overlay->format;
    }
    return renderer;
}

GLboolean IJK_GLES2_Renderer_isValid(IJK_GLES2_Renderer *renderer)
{
    return renderer && renderer->program ? GL_TRUE : GL_FALSE;
}

GLboolean IJK_GLES2_Renderer_isFormat(IJK_GLES2_Renderer *renderer, int format)
{
    if (!IJK_GLES2_Renderer_isValid(renderer))
        return GL_FALSE;

    return renderer->format == format ? GL_TRUE : GL_FALSE;
}

/*
 * Per-Context routine
 */
GLboolean IJK_GLES2_Renderer_setupGLES()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);       IJK_GLES2_checkError_TRACE("glClearColor");
    glEnable(GL_CULL_FACE);                     IJK_GLES2_checkError_TRACE("glEnable(GL_CULL_FACE)");
    glCullFace(GL_BACK);                        IJK_GLES2_checkError_TRACE("glCullFace");
    glDisable(GL_DEPTH_TEST);

    return GL_TRUE;
}

static void IJK_GLES2_Renderer_Vertices_reset(IJK_GLES2_Renderer *renderer)
{
/*
 OpenGL 世界坐标系：
 取值范围：[-1.0,1.0]
       Y^
        |
        |
 -------o-------> X
        |
        |
*/
    //默认占满整个世界
    
    //左下
    renderer->vertices[0] = -1.0f;
    renderer->vertices[1] = -1.0f;
    //右下
    renderer->vertices[2] =  1.0f;
    renderer->vertices[3] = -1.0f;
    //左上
    renderer->vertices[4] = -1.0f;
    renderer->vertices[5] =  1.0f;
    //右上
    renderer->vertices[6] =  1.0f;
    renderer->vertices[7] =  1.0f;
}

static void IJK_GLES2_Renderer_Vertices_apply(IJK_GLES2_Renderer *renderer)
{
    switch (renderer->gravity) {
        case IJK_GLES2_GRAVITY_RESIZE_ASPECT:
            break;
        case IJK_GLES2_GRAVITY_RESIZE_ASPECT_FILL:
            break;
        case IJK_GLES2_GRAVITY_RESIZE:
            IJK_GLES2_Renderer_Vertices_reset(renderer);
            return;
        default:
            ALOGE("[GLES2] unknown gravity %d\n", renderer->gravity);
            IJK_GLES2_Renderer_Vertices_reset(renderer);
            return;
    }

    if (renderer->layer_width <= 0 ||
        renderer->layer_height <= 0 ||
        renderer->frame_width <= 0 ||
        renderer->frame_height <= 0)
    {
        ALOGE("[GLES2] invalid width/height for gravity aspect\n");
        IJK_GLES2_Renderer_Vertices_reset(renderer);
        return;
    }

    float width     = renderer->frame_width;
    float height    = renderer->frame_height;

    if (renderer->frame_sar_num > 0 && renderer->frame_sar_den > 0) {
        width = width * renderer->frame_sar_num / renderer->frame_sar_den;
    }

    const float dW  = (float)renderer->layer_width	/ width;
    const float dH  = (float)renderer->layer_height / height;
    float dd        = 1.0f;
    float nW        = 1.0f;
    float nH        = 1.0f;

    switch (renderer->gravity) {
        case IJK_GLES2_GRAVITY_RESIZE_ASPECT_FILL:  dd = FFMAX(dW, dH); break;
        case IJK_GLES2_GRAVITY_RESIZE_ASPECT:       dd = FFMIN(dW, dH); break;
    }

    nW = (width  * dd / (float)renderer->layer_width);
    nH = (height * dd / (float)renderer->layer_height);

    renderer->vertices[0] = - nW;
    renderer->vertices[1] = - nH;
    renderer->vertices[2] =   nW;
    renderer->vertices[3] = - nH;
    renderer->vertices[4] = - nW;
    renderer->vertices[5] =   nH;
    renderer->vertices[6] =   nW;
    renderer->vertices[7] =   nH;
}

static void IJK_GLES2_Renderer_Vertices_reloadVertex(IJK_GLES2_Renderer *renderer)
{
    glVertexAttribPointer(renderer->av4_position, 2, GL_FLOAT, GL_FALSE, 0, renderer->vertices);    IJK_GLES2_checkError_TRACE("glVertexAttribPointer(av2_texcoord)");
    glEnableVertexAttribArray(renderer->av4_position);                                      IJK_GLES2_checkError_TRACE("glEnableVertexAttribArray(av2_texcoord)");
}

#define IJK_GLES2_GRAVITY_MIN                   (0)
#define IJK_GLES2_GRAVITY_RESIZE                (0) // Stretch to fill layer bounds.
#define IJK_GLES2_GRAVITY_RESIZE_ASPECT         (1) // Preserve aspect ratio; fit within layer bounds.
#define IJK_GLES2_GRAVITY_RESIZE_ASPECT_FILL    (2) // Preserve aspect ratio; fill layer bounds.
#define IJK_GLES2_GRAVITY_MAX                   (2)

GLboolean IJK_GLES2_Renderer_setGravity(IJK_GLES2_Renderer *renderer, int gravity, GLsizei layer_width, GLsizei layer_height)
{
    if (renderer->gravity != gravity && gravity >= IJK_GLES2_GRAVITY_MIN && gravity <= IJK_GLES2_GRAVITY_MAX)
        renderer->vertices_changed = 1;
    else if (renderer->layer_width != layer_width)
        renderer->vertices_changed = 1;
    else if (renderer->layer_height != layer_height)
        renderer->vertices_changed = 1;
    else
        return GL_TRUE;

    renderer->gravity      = gravity;
    renderer->layer_width  = layer_width;
    renderer->layer_height = layer_height;
    return GL_TRUE;
}

void IJK_GLES2_Renderer_updateRotate(IJK_GLES2_Renderer *renderer,int type,int degrees)
{
    renderer->rotate_type = type;
    renderer->rotate_degrees = degrees;
}

static void IJK_GLES2_Renderer_TexCoords_cropRight(IJK_GLES2_Renderer *renderer, GLfloat cropRight)
{
    ALOGE("IJK_GLES2_Renderer_TexCoords_cropRight:%g\n",cropRight);
    
/*
 OpenGL 纹理坐标系：
 取值范围：[0.0,1.0]
   Y ^
     |
     |
     o-------> X
*/
    //默认将纹理贴满画布
    //左上
    renderer->texcoords[0] = 0.0f;
    renderer->texcoords[1] = 1.0f;
    //右上
    renderer->texcoords[2] = 1.0f - cropRight;
    renderer->texcoords[3] = 1.0f;
    //左下(圆点)
    renderer->texcoords[4] = 0.0f;
    renderer->texcoords[5] = 0.0f;
    //右下
    renderer->texcoords[6] = 1.0f - cropRight;
    renderer->texcoords[7] = 0.0f;
}

static void IJK_GLES2_Renderer_TexCoords_reset(IJK_GLES2_Renderer *renderer)
{
    IJK_GLES2_Renderer_TexCoords_cropRight(renderer, 0.0f);
}

static void IJK_GLES2_Renderer_TexCoords_reloadVertex(IJK_GLES2_Renderer *renderer)
{
    glVertexAttribPointer(renderer->av2_texcoord, 2, GL_FLOAT, GL_FALSE, 0, renderer->texcoords);
    IJK_GLES2_checkError_TRACE("glVertexAttribPointer(av2_texcoord)");
    glEnableVertexAttribArray(renderer->av2_texcoord);
    IJK_GLES2_checkError_TRACE("glEnableVertexAttribArray(av2_texcoord)");
}

/*
 * Per-Renderer routine
 */
GLboolean IJK_GLES2_Renderer_use(IJK_GLES2_Renderer *renderer)
{
    if (!renderer)
        return GL_FALSE;

    assert(renderer->func_use);
    if (!renderer->func_use(renderer))
        return GL_FALSE;

    IJK_GLES_Matrix modelViewProj;
    IJK_GLES2_loadOrtho(&modelViewProj, -1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
    glUniformMatrix4fv(renderer->um4_mvp, 1, GL_FALSE, modelViewProj.m);                    IJK_GLES2_checkError_TRACE("glUniformMatrix4fv(um4_mvp)");

    IJK_GLES2_Renderer_TexCoords_reset(renderer);
    IJK_GLES2_Renderer_TexCoords_reloadVertex(renderer);

    IJK_GLES2_Renderer_Vertices_reset(renderer);
    IJK_GLES2_Renderer_Vertices_reloadVertex(renderer);

    return GL_TRUE;
}

/*
 * Per-Frame routine
 */
GLboolean IJK_GLES2_Renderer_renderOverlay(IJK_GLES2_Renderer *renderer, SDL_VoutOverlay *overlay)
{
    if (!renderer || !renderer->func_uploadTexture)
        return GL_FALSE;

    glClear(GL_COLOR_BUFFER_BIT);               IJK_GLES2_checkError_TRACE("glClear");

    GLsizei visible_width  = renderer->frame_width;
    GLsizei visible_height = renderer->frame_height;
    if (overlay) {
        visible_width  = overlay->w;
        visible_height = overlay->h;
        if (renderer->frame_width   != visible_width    ||
            renderer->frame_height  != visible_height   ||
            renderer->frame_sar_num != overlay->sar_num ||
            renderer->frame_sar_den != overlay->sar_den) {

            renderer->frame_width   = visible_width;
            renderer->frame_height  = visible_height;
            renderer->frame_sar_num = overlay->sar_num;
            renderer->frame_sar_den = overlay->sar_den;

            renderer->vertices_changed = 1;
        }
        
        if (renderer->func_useSubtitle) {
            renderer->func_useSubtitle(renderer, GL_FALSE);
        }
        renderer->last_buffer_width = renderer->func_getBufferWidth(renderer, overlay);

        if (!renderer->func_uploadTexture(renderer, overlay))
            return GL_FALSE;
    } else {
        // NULL overlay means force reload vertice
        renderer->vertices_changed = 1;
    }

    GLsizei buffer_width = renderer->last_buffer_width;
    if (renderer->vertices_changed ||
        (buffer_width > 0 &&
         buffer_width > visible_width &&
         buffer_width != renderer->buffer_width &&
         visible_width != renderer->visible_width)){

        renderer->vertices_changed = 0;

        IJK_GLES2_Renderer_Vertices_apply(renderer);
        IJK_GLES2_Renderer_Vertices_reloadVertex(renderer);

        renderer->buffer_width  = buffer_width;
        renderer->visible_width = visible_width;

        GLsizei padding_pixels     = buffer_width - visible_width;
        GLfloat padding_normalized = ((GLfloat)padding_pixels) / buffer_width;

        IJK_GLES2_Renderer_TexCoords_reset(renderer);
        IJK_GLES2_Renderer_TexCoords_cropRight(renderer, padding_normalized);
        IJK_GLES2_Renderer_TexCoords_reloadVertex(renderer);
    }
    
    IJK_GLES2_Renderer_Vertices_reloadVertex(renderer);
    
    //for debug rotation
    //static int degrees = 0;
    //degrees++;
    
    ijk_float3_vector rotate_v3 = { 0.0 };
    //rotate x
    if (renderer->rotate_type == 1) {
        rotate_v3.x = 1.0;
    }
    //rotate y
    else if (renderer->rotate_type == 2) {
        rotate_v3.y = 1.0;
    }
    //rotate z
    else if (renderer->rotate_type == 3) {
        rotate_v3.z = 1.0;
    }
    
    float radians = radians_from_degrees(renderer->rotate_degrees);
    ijk_float4x4_matrix rotation_matrix = matrix4x4_rotation(radians, rotate_v3);
    
    IJK_GLES_Matrix modelViewProj;
    IJK_GLES2_loadOrtho(&modelViewProj, -1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
    ijk_float4x4_matrix proj_matrix = ijk_make_matrix_fromArr(modelViewProj.m);
    
    ijk_float4x4_matrix r_matrix;
    ijk_matrix_multiply(&proj_matrix,&rotation_matrix,&r_matrix);
    
    glUniformMatrix4fv(renderer->um4_mvp, 1, GL_FALSE, (GLfloat*)(&r_matrix.e));                    IJK_GLES2_checkError_TRACE("glUniformMatrix4fv(um4_mvp)");
    
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);      IJK_GLES2_checkError_TRACE("glDrawArrays");

    return GL_TRUE;
}

GLboolean IJK_GLES2_Renderer_renderSubtitle(IJK_GLES2_Renderer *renderer, SDL_VoutOverlay *overlay, void *subtitle)
{
    if (subtitle && renderer->func_uploadSubtitle) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        if (renderer->func_useSubtitle) {
            renderer->func_useSubtitle(renderer, GL_TRUE);
        }

        IJK_Subtile_Size labelSize = {0};
        renderer->func_uploadSubtitle(renderer,subtitle,&labelSize);
        
        GLfloat vertices[8] = {0.0f};
        
        float labelWidth = labelSize.w;
        float labelHeight = labelSize.h;
        float picWidth = overlay->w;
        float picHeight = overlay->h;
        
        float ratiox = labelWidth / picWidth;
        float ratioy = labelHeight / picHeight;
        float leftX  = 0 - ratiox;
        float rightX = 0 + ratiox;
        //2 * 0.2 - 1
        float bottomY = 2 * 0.05 - 1;
        //bottomY + 2 * ratioy
        float topY = bottomY + 2 * ratioy;
        
        //左下
        vertices[0] = leftX;
        vertices[1] = bottomY;
        //右下
        vertices[2] = rightX;
        vertices[3] = bottomY;
        //左上
        vertices[4] = leftX;
        vertices[5] = topY;
        //右上
        vertices[6] = rightX;
        vertices[7] = topY;
        
        glVertexAttribPointer(renderer->av4_position, 2, GL_FLOAT, GL_FALSE, 0, vertices);    IJK_GLES2_checkError_TRACE("glVertexAttribPointer(av2_texcoord)");
        glEnableVertexAttribArray(renderer->av4_position);                                      IJK_GLES2_checkError_TRACE("glEnableVertexAttribArray(av2_texcoord)");
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);      IJK_GLES2_checkError_TRACE("glDrawArrays");
    }
    return GL_TRUE;
}
