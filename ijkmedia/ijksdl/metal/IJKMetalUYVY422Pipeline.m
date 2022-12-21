//
//  IJKMetalUYVY422Pipeline.m
//  FFmpegTutorial-macOS
//
//  Created by qianlongxu on 2022/11/24.
//  Copyright © 2022 Matt Reach's Awesome FFmpeg Tutotial. All rights reserved.
//

#import "IJKMetalUYVY422Pipeline.h"

@interface IJKMetalUYVY422Pipeline ()

@end

@implementation IJKMetalUYVY422Pipeline

+ (NSString *)fragmentFuctionName
{
    return @"uyvy422FragmentShader";
}

+ (MTLPixelFormat)_MTLPixelFormat
{
    return MTLPixelFormatBGRG422;
}

- (NSArray<id<MTLTexture>>*)doGenerateTexture:(CVPixelBufferRef)pixelBuffer
                                 textureCache:(CVMetalTextureCacheRef)textureCache
{
    id<MTLTexture> textureY = nil;
    
    CVPixelBufferLockBaseAddress(pixelBuffer, kCVPixelBufferLock_ReadOnly);
    // textureY 设置
    {
        size_t width  = CVPixelBufferGetWidthOfPlane(pixelBuffer, 0);
        size_t height = CVPixelBufferGetHeightOfPlane(pixelBuffer, 0);
        MTLPixelFormat pixelFormat = [[self class] _MTLPixelFormat];
        CVMetalTextureRef texture = NULL; // CoreVideo的Metal纹理
        CVReturn status = CVMetalTextureCacheCreateTextureFromImage(NULL, textureCache, pixelBuffer, NULL, pixelFormat, width, height, 0, &texture);
        if (status == kCVReturnSuccess) {
            textureY = CVMetalTextureGetTexture(texture); // 转成Metal用的纹理
            CFRelease(texture);
        }
    }
    CVPixelBufferUnlockBaseAddress(pixelBuffer, kCVPixelBufferLock_ReadOnly);
    
    self.convertMatrixType = IJKUYVYToRGBVideoRangeMatrix;
    
    if (textureY != nil) {
        return @[textureY];
    } else {
        return nil;
    }
}

@end
