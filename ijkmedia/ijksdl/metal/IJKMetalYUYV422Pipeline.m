//
//  IJKMetalYUYV422Pipeline.m
//  FFmpegTutorial-macOS
//
//  Created by qianlongxu on 2022/11/24.
//  Copyright © 2022 Matt Reach's Awesome FFmpeg Tutotial. All rights reserved.
//

#import "IJKMetalYUYV422Pipeline.h"

@implementation IJKMetalYUYV422Pipeline

+ (MTLPixelFormat)_MTLPixelFormat
{
    return MTLPixelFormatGBGR422;
}

@end
