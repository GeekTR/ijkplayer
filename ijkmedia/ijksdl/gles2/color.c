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

//https://github.com/lemenkov/libyuv/blob/6900494d90ae095d44405cd4cc3f346971fa69c9/source/row_common.cc#L2

//Full Range YUV to RGB reference
const GLfloat *IJK_GLES2_getColorMatrix_bt2020(void)
{
    // BT.2020, which is the standard for HDR.
    static const GLfloat g_bt2020[] = {
        1.164384, 1.164384 , 1.164384,
        0.0     , -0.187326, 2.14177,
        1.67867 , -0.65042 , 0.0
    };
    return g_bt2020;
}

const GLfloat *IJK_GLES2_getColorMatrix_bt709(void)
{
    // BT.709, which is the standard for HDTV.
    static const GLfloat g_bt709[] = {
        1.164,  1.164,  1.164,
        0.0,   -0.213,  2.112,
        1.793, -0.533,  0.0,
    };
    return g_bt709;
}

const GLfloat *IJK_GLES2_getColorMatrix_bt601(void)
{
    // BT.601, which is the standard for HDTV.
    static const GLfloat g_bt601[] = {
        1.164,  1.164, 1.164,
        0.0,   -0.391, 2.018,
        1.596, -0.813, 0.0,
    };
    return g_bt601;
}
