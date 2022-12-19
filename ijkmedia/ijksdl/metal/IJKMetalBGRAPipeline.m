//
//  IJKMetalBGRAPipeline.m
//  FFmpegTutorial-macOS
//
//  Created by qianlongxu on 2022/11/23.
//  Copyright © 2022 Matt Reach's Awesome FFmpeg Tutotial. All rights reserved.
//

#import "IJKMetalBGRAPipeline.h"

@implementation IJKMetalBGRAPipeline

+ (NSString *)fragmentFuctionName
{
    return @"bgraFragmentShader";
}

- (void)doUploadTextureWithEncoder:(id<MTLArgumentEncoder>)encoder
                            buffer:(CVPixelBufferRef)pixelBuffer
                      textureCache:(CVMetalTextureCacheRef)textureCache
{
    id<MTLTexture> textureY = nil;
    
    CVPixelBufferLockBaseAddress(pixelBuffer, kCVPixelBufferLock_ReadOnly);
    // textureY 设置
    {
        size_t width  = CVPixelBufferGetWidthOfPlane(pixelBuffer, 0);
        size_t height = CVPixelBufferGetHeightOfPlane(pixelBuffer, 0);
        MTLPixelFormat pixelFormat = MTLPixelFormatBGRA8Unorm;

        CVMetalTextureRef texture = NULL; // CoreVideo的Metal纹理
        CVReturn status = CVMetalTextureCacheCreateTextureFromImage(NULL, textureCache, pixelBuffer, NULL, pixelFormat, width, height, 0, &texture);
        if (status == kCVReturnSuccess) {
            textureY = CVMetalTextureGetTexture(texture); // 转成Metal用的纹理
            CFRelease(texture);
        }
    }
    CVPixelBufferUnlockBaseAddress(pixelBuffer, kCVPixelBufferLock_ReadOnly);
    
    if (textureY != nil) {
        [encoder setTexture:textureY
                    atIndex:IJKFragmentTextureIndexTextureY]; // 设置纹理
    }
}

@end
