/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997-2010 Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

    Sam Lantinga
    slouken@libsdl.org
*/

#import <UIKit/UIKit.h>
#import "SDL_uikitopenglview.h"

/* *INDENT-OFF* */
@interface SDLUIKitDelegate:NSObject<UIApplicationDelegate> {
    SDL_Window *_sdlWindow;
    UIWindow *_window;
}

@property (readwrite, assign) SDL_Window *sdl_window;
@property (readonly, strong) UIWindow *window;

+(SDLUIKitDelegate *)sharedAppDelegate;
-(SDL_uikitopenglview*)screen;
-(void)setWindowTitle:(char*)title;

@end
/* *INDENT-ON* */
