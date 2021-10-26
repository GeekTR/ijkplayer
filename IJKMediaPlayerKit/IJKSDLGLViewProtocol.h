/*
 * IJKSDLGLViewProtocol.h
 *
 * Copyright (c) 2017 Bilibili
 * Copyright (c) 2017 raymond <raymondzheng1412@gmail.com>
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

#ifndef IJKSDLGLViewProtocol_h
#define IJKSDLGLViewProtocol_h
#import <TargetConditionals.h>
#if TARGET_OS_OSX
typedef NSOpenGLView UIView;
typedef NSImage UIImage;
#import <AppKit/AppKit.h>
#else
#import <UIKit/UIKit.h>
#endif

typedef NS_ENUM(NSInteger, IJKContentMode) {
    IJKContentModeScaleNone,
    IJKContentModeScaleAspectFit,      // contents scaled to fit with fixed aspect. remainder is transparent
    IJKContentModeScaleAspectFill,     // contents scaled to fill with fixed aspect. some portion of content may be clipped.
    IJKContentModeScaleToFill,
};

typedef struct IJKOverlay IJKOverlay;
struct IJKOverlay {
    int w;
    int h;
    UInt32 format;
    int planes;
    UInt16 *pitches;
    UInt8 **pixels;
    int sar_num;
    int sar_den;
    CVPixelBufferRef pixel_buffer;
};

@protocol IJKSDLGLViewProtocol <NSObject>
#if !TARGET_OS_OSX
- (UIImage*) snapshot;
- (void)display_pixels: (IJKOverlay *) overlay;
#endif
- (void)setContentMode:(IJKContentMode)contentMode;
@property(nonatomic, readonly) CGFloat  fps;
@property(nonatomic)        CGFloat  scaleFactor;
@property(nonatomic)        BOOL  isThirdGLView;

@end

#endif /* IJKSDLGLViewProtocol_h */
