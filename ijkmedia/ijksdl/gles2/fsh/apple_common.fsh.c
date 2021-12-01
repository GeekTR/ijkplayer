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

#include "ijksdl/gles2/internal.h"

//macOS use sampler2DRect,need texture dimensions

#if TARGET_OS_OSX

//for bgrx texture
static const char g_shader_rect_bgrx_1[] = IJK_GLES_STRING(
    varying vec2 vv2_Texcoord;
    uniform vec3 um3_rgbAdjustment;
    
    uniform sampler2DRect us2_Sampler0;
    uniform vec2 textureDimension0;
    
    uniform int isSubtitle;

    vec3 rgb_adjust(vec3 rgb,vec3 rgbAdjustment) {
        //C 是对比度值，B 是亮度值，S 是饱和度
        float B = rgbAdjustment.x;
        float S = rgbAdjustment.y;
        float C = rgbAdjustment.z;

        rgb = (rgb - 0.5) * C + 0.5;
        rgb = rgb + (0.75 * B - 0.5) / 2.5 - 0.1;
        vec3 intensity = vec3(dot(rgb, vec3(0.299, 0.587, 0.114)));
        return intensity + S * (rgb - intensity);
    }
                                                      
    void main()
    {
        if (isSubtitle == 1) {
            vec2 recTexCoord0 = vv2_Texcoord * textureDimension0;
            gl_FragColor = texture2DRect(us2_Sampler0, recTexCoord0);
        } else {
            vec3 rgb;
            
            vec2 recTexCoord0 = vv2_Texcoord * textureDimension0;
            rgb = texture2DRect(us2_Sampler0, recTexCoord0).rgb;
            rgb = rgb_adjust(rgb,um3_rgbAdjustment);

            gl_FragColor = vec4(rgb, 1.0);
        }
    }
);

//for rgbx texture
static const char g_shader_rect_rgbx_1[] = IJK_GLES_STRING(
    varying vec2 vv2_Texcoord;
    uniform vec3 um3_rgbAdjustment;
    
    uniform sampler2DRect us2_Sampler0;
    uniform vec2 textureDimension0;
    
    uniform int isSubtitle;

    vec3 rgb_adjust(vec3 rgb,vec3 rgbAdjustment) {
        //C 是对比度值，B 是亮度值，S 是饱和度
        float B = rgbAdjustment.x;
        float S = rgbAdjustment.y;
        float C = rgbAdjustment.z;

        rgb = (rgb - 0.5) * C + 0.5;
        rgb = rgb + (0.75 * B - 0.5) / 2.5 - 0.1;
        vec3 intensity = vec3(dot(rgb, vec3(0.299, 0.587, 0.114)));
        return intensity + S * (rgb - intensity);
    }
                                                      
    void main()
    {
        if (isSubtitle == 1) {
            vec2 recTexCoord0 = vv2_Texcoord * textureDimension0;
            gl_FragColor = texture2DRect(us2_Sampler0, recTexCoord0);
        } else {
            vec3 rgb;
            
            vec2 recTexCoord0 = vv2_Texcoord * textureDimension0;
            rgb = texture2DRect(us2_Sampler0, recTexCoord0).rgb;
            rgb = rgb_adjust(rgb,um3_rgbAdjustment);

            gl_FragColor = vec4(rgb, 1.0);
        }
    }
);

//for xrgb texture
static const char g_shader_rect_xrgb_1[] = IJK_GLES_STRING(
    varying vec2 vv2_Texcoord;
    uniform vec3 um3_rgbAdjustment;
    
    uniform sampler2DRect us2_Sampler0;
    uniform vec2 textureDimension0;
    
    uniform int isSubtitle;

    vec3 rgb_adjust(vec3 rgb,vec3 rgbAdjustment) {
        //C 是对比度值，B 是亮度值，S 是饱和度
        float B = rgbAdjustment.x;
        float S = rgbAdjustment.y;
        float C = rgbAdjustment.z;

        rgb = (rgb - 0.5) * C + 0.5;
        rgb = rgb + (0.75 * B - 0.5) / 2.5 - 0.1;
        vec3 intensity = vec3(dot(rgb, vec3(0.299, 0.587, 0.114)));
        return intensity + S * (rgb - intensity);
    }
                                                      
    void main()
    {
        if (isSubtitle == 1) {
            vec2 recTexCoord0 = vv2_Texcoord * textureDimension0;
            gl_FragColor = texture2DRect(us2_Sampler0, recTexCoord0);
        } else {
            vec3 rgb;
            
            vec2 recTexCoord0 = vv2_Texcoord * textureDimension0;
            //bgra -> argb
            //argb -> bgra
            rgb = texture2DRect(us2_Sampler0, recTexCoord0).gra;
            rgb = rgb_adjust(rgb,um3_rgbAdjustment);

            gl_FragColor = vec4(rgb, 1.0);
        }
    }
);

//for 420sp
static const char g_shader_rect_2[] = IJK_GLES_STRING(
    varying vec2 vv2_Texcoord;
    uniform mat3 um3_ColorConversion;
    uniform vec3 um3_rgbAdjustment;
    
    uniform sampler2DRect us2_Sampler0;
    uniform sampler2DRect us2_Sampler1;
    
    uniform vec2 textureDimension0;
    uniform vec2 textureDimension1;
    
    uniform int isSubtitle;
    uniform int isFullRange;

    vec3 rgb_adjust(vec3 rgb,vec3 rgbAdjustment) {
        //C 是对比度值，B 是亮度值，S 是饱和度
        float B = rgbAdjustment.x;
        float S = rgbAdjustment.y;
        float C = rgbAdjustment.z;

        rgb = (rgb - 0.5) * C + 0.5;
        rgb = rgb + (0.75 * B - 0.5) / 2.5 - 0.1;
        vec3 intensity = vec3(dot(rgb, vec3(0.299, 0.587, 0.114)));
        return intensity + S * (rgb - intensity);
    }
                                                      
    void main()
    {
        if (isSubtitle == 1) {
            vec2 recTexCoord0 = vv2_Texcoord * textureDimension0;
            gl_FragColor = texture2DRect(us2_Sampler0, recTexCoord0);
        } else {
            vec3 yuv;
            vec3 rgb;
            
            vec2 recTexCoord0 = vv2_Texcoord * textureDimension0;
            vec2 recTexCoord1 = vv2_Texcoord * textureDimension1;
            
            yuv.x = texture2DRect(us2_Sampler0, recTexCoord0).r;
            if (isFullRange == 1) {
                yuv.x = yuv.x - (16.0 / 255.0);
            }
            yuv.yz = texture2DRect(us2_Sampler1, recTexCoord1).ra - vec2(0.5, 0.5);
            
            rgb = um3_ColorConversion * yuv;
            
            rgb = rgb_adjust(rgb,um3_rgbAdjustment);

            gl_FragColor = vec4(rgb, 1.0);
        }
    }
);

//for yuv420p
static const char g_shader_rect_3[] = IJK_GLES_STRING(
    varying vec2 vv2_Texcoord;
    uniform mat3 um3_ColorConversion;
    uniform vec3 um3_rgbAdjustment;
                                                   
    uniform sampler2DRect us2_Sampler0;
    uniform sampler2DRect us2_Sampler1;
    uniform sampler2DRect us2_Sampler2;
                                                   
    uniform vec2 textureDimension0;
    uniform vec2 textureDimension1;
    uniform vec2 textureDimension2;
                                                   
    uniform int isSubtitle;
    uniform int isFullRange;

    vec3 rgb_adjust(vec3 rgb,vec3 rgbAdjustment) {
        //C 是对比度值，B 是亮度值，S 是饱和度
        float B = rgbAdjustment.x;
        float S = rgbAdjustment.y;
        float C = rgbAdjustment.z;

        rgb = (rgb - 0.5) * C + 0.5;
        rgb = rgb + (0.75 * B - 0.5) / 2.5 - 0.1;
        vec3 intensity = vec3(dot(rgb, vec3(0.299, 0.587, 0.114)));
        return intensity + S * (rgb - intensity);
    }
                                                      
    void main()
    {
        if (isSubtitle == 1) {
            vec2 recTexCoord0 = vv2_Texcoord * textureDimension0;
            gl_FragColor = texture2DRect(us2_Sampler0, recTexCoord0);
        } else {
            vec3 yuv;
            vec3 rgb;

            vec2 recTexCoord0 = vv2_Texcoord * textureDimension0;
            vec2 recTexCoord1 = vv2_Texcoord * textureDimension1;
            vec2 recTexCoord2 = vv2_Texcoord * textureDimension2;

            yuv.x = texture2DRect(us2_Sampler0, recTexCoord0).r;
            if (isFullRange == 1) {
                yuv.x = yuv.x - (16.0 / 255.0);
            }
            yuv.y = texture2DRect(us2_Sampler1, recTexCoord1).r - 0.5;
            yuv.z = texture2DRect(us2_Sampler2, recTexCoord2).r - 0.5;

            rgb = um3_ColorConversion * yuv;

            rgb = rgb_adjust(rgb,um3_rgbAdjustment);

            gl_FragColor = vec4(rgb, 1.0);
        }
    }
);

const char *IJK_GL_getAppleCommonFragmentShader(IJK_SHADER_TYPE type)
{
    //for rgbx
    switch (type) {
        case BGRX_SHADER:
        {
            return g_shader_rect_bgrx_1;
        }
        case XRGB_SHADER:
        {
            return g_shader_rect_xrgb_1;
        }
        case YUV_2P_SHADER:
        {
            return g_shader_rect_2;
        }
        case YUV_3P_SHADER:
        {
            return g_shader_rect_3;
        }
        case UYVY_SHADER:
        {
            return g_shader_rect_rgbx_1;
        }
        case NONE_SHADER:
        {
            assert(0);
            return "";
        }
    }
}

#else

static const char g_shader[] = IJK_GLES_STRING(
    precision highp float;
    varying   highp vec2 vv2_Texcoord;
    uniform         mat3 um3_ColorConversion;
    uniform   lowp  sampler2D us2_SamplerX;
    uniform   lowp  sampler2D us2_SamplerY;

    void main()
    {
        mediump vec3 yuv;
        lowp    vec3 rgb;

        yuv.x  = (texture2D(us2_SamplerX,  vv2_Texcoord).r  - (16.0 / 255.0));
        yuv.yz = (texture2D(us2_SamplerY,  vv2_Texcoord).rg - vec2(0.5, 0.5));
        rgb = um3_ColorConversion * yuv;
        gl_FragColor = vec4(rgb, 1);
    }
);

const char *IJK_GL_getAppleCommonFragmentShader(IJK_SHADER_TYPE type)
{
#warning todo
    return "";
}

#endif
