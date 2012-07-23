//
//  EAGLView.h
//  GLApp
//
//  Created by Andrew Pepper on 04.09.11.
//  Copyright 2011 Home. All rights reserved.
//

#import <UIKit/UIKit.h>

#import <OpenGLES/EAGL.h>
#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>

// This class wraps the CAEAGLLayer from CoreAnimation into a convenient UIView subclass.
// The view content is basically an EAGL surface you render your OpenGL scene into.
// Note that setting the view non-opaque will only work if the EAGL surface has an alpha channel.
@interface EAGLView : UIView
{
@private
    EAGLContext *context;
    
    // The pixel dimensions of the CAEAGLLayer.
    GLint framebufferWidth;
    GLint framebufferHeight;
    
    // The OpenGL ES names for the framebuffer and renderbuffer used to render to this view.
    GLuint defaultFramebuffer;
	GLuint colorRenderbuffer;
	bool hgeInited;
}

@property (nonatomic, retain) EAGLContext *context;
@property (readwrite, assign) GLint framebufferWidth;
@property (readwrite, assign) GLint framebufferHeight;

- (void)setFramebuffer;
- (BOOL)presentFramebuffer;

@end
