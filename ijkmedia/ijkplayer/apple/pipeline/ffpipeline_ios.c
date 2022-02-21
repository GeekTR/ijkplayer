/*
 * ffpipeline_ios.c
 *
 * Copyright (c) 2014 Zhou Quan <zhouqicy@gmail.com>
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

#include "ffpipeline_ios.h"
#include "ffpipenode_ios_videotoolbox_vdec.h"
#include "ffpipenode_ffplay_vdec.h"
#include "ff_ffplay.h"
#import "ijksdl/apple/ijksdl_aout_ios_audiounit.h"

struct IJKFF_Pipeline_Opaque {
    FFPlayer    *ffp;
};

static void func_destroy(IJKFF_Pipeline *pipeline)
{
    
}

static int func_has_another_video_decoder(IJKFF_Pipeline *pipeline, FFPlayer *ffp)
{
    return 1;
}

static IJKFF_Pipenode *func_open_another_video_decoder(IJKFF_Pipeline *pipeline, FFPlayer *ffp)
{
    IJKFF_Pipenode* node = ffp->node_vdec ?: ffp->node_vdec_2;
    IJKFF_Pipenode* node2 = NULL;
    
    if (node->vdec_type == FFP_PROPV_DECODER_AVCODEC) {
        node2 = ffpipenode_create_video_decoder_from_ios_videotoolbox(ffp);
        if (node2) {
            node2->vdec_type = FFP_PROPV_DECODER_VIDEOTOOLBOX;
        }
    } else if (node->vdec_type == FFP_PROPV_DECODER_VIDEOTOOLBOX) {
        node2 = ffpipenode_create_video_decoder_from_ffplay(ffp);
        if (node2) {
            node2->vdec_type = FFP_PROPV_DECODER_AVCODEC;
        }
    }
    return node2;
}

static IJKFF_Pipenode *func_open_video_decoder(IJKFF_Pipeline *pipeline, FFPlayer *ffp)
{
    IJKFF_Pipenode* node = NULL;
    if (ffp->videotoolbox) {
        node = ffpipenode_create_video_decoder_from_ios_videotoolbox(ffp);
        if (!node)
            ALOGE("vtb fail!!! switch to ffmpeg decode!!!! \n");
    }
    if (node == NULL) {
        node = ffpipenode_create_video_decoder_from_ffplay(ffp);
        node->vdec_type = FFP_PROPV_DECODER_AVCODEC;
    } else {
        node->vdec_type = FFP_PROPV_DECODER_VIDEOTOOLBOX;
    }
    ffp_notify_msg2(ffp, FFP_MSG_VIDEO_DECODER_OPEN, node->vdec_type == FFP_PROPV_DECODER_VIDEOTOOLBOX);
    node->is_using = 1;
    return node;
}

static SDL_Aout *func_open_audio_output(IJKFF_Pipeline *pipeline, FFPlayer *ffp)
{
    return SDL_AoutIos_CreateForAudioUnit();
}

static SDL_Class g_pipeline_class = {
    .name = "ffpipeline_ios",
};

IJKFF_Pipeline *ffpipeline_create_from_ios(FFPlayer *ffp)
{
    IJKFF_Pipeline *pipeline = ffpipeline_alloc(&g_pipeline_class, sizeof(IJKFF_Pipeline_Opaque));
    if (!pipeline)
        return pipeline;

    IJKFF_Pipeline_Opaque *opaque             = pipeline->opaque;
    opaque->ffp                               = ffp;
    pipeline->func_destroy                    = func_destroy;
    pipeline->func_open_video_decoder         = func_open_video_decoder;
    pipeline->func_open_audio_output          = func_open_audio_output;
    pipeline->func_has_another_video_decoder  = func_has_another_video_decoder;
    pipeline->func_open_another_video_decoder = func_open_another_video_decoder;
    
    return pipeline;
}
