/*==LICENSE==*

CyanWorlds.com Engine - MMOG client, server and tools
Copyright (C) 2011  Cyan Worlds, Inc.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

Additional permissions under GNU GPL version 3 section 7

If you modify this Program, or any covered work, by linking or
combining it with any of RAD Game Tools Bink SDK, Autodesk 3ds Max SDK,
NVIDIA PhysX SDK, Microsoft DirectX SDK, OpenSSL library, Independent
JPEG Group JPEG library, Microsoft Windows Media SDK, or Apple QuickTime SDK
(or a modified version of those libraries),
containing parts covered by the terms of the Bink SDK EULA, 3ds Max EULA,
PhysX SDK EULA, DirectX SDK EULA, OpenSSL and SSLeay licenses, IJG
JPEG Library README, Windows Media SDK EULA, or QuickTime SDK EULA, the
licensors of this Program grant you additional
permission to convey the resulting work. Corresponding Source for a
non-source form of such a combination shall include the source code for
the parts of OpenSSL and IJG JPEG Library used as well as that of the covered
work.

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/

#import "PLSView.h"
#include "plInputCore/plInputManager.h"
#include "plMessage/plInputEventMsg.h"

@interface PLSView ()

@property plInputManager* inputManager;
@property (strong) UIHoverGestureRecognizer* hoverGestureRecognizer;

@end

@implementation PLSView

-(void)setupGestureRecognizer
{
    self.hoverGestureRecognizer = [[UIHoverGestureRecognizer alloc] initWithTarget:self action:@selector(hovering:)];
    [self addGestureRecognizer:self.hoverGestureRecognizer];
}

-(void)hovering:(UIHoverGestureRecognizer *)recognizer
{
    if(recognizer.state == UIGestureRecognizerStateEnded || recognizer.state == UIGestureRecognizerStatePossible)
    {
        return;
    }
    CGPoint viewLocation = [recognizer locationInView:self];

    CGRect windowViewBounds = self.bounds;
    CGFloat xNormal = (viewLocation.x) / windowViewBounds.size.width;
    CGFloat yNormal =
        viewLocation.y / windowViewBounds.size.height;

    plIMouseXEventMsg* pXMsg = new plIMouseXEventMsg;
    plIMouseYEventMsg* pYMsg = new plIMouseYEventMsg;

    pXMsg->fX = xNormal;
    pYMsg->fY = yNormal;

    // Plasma internally uses input coords as display coords
    pXMsg->fWx = viewLocation.x * self.contentScaleFactor;
    pYMsg->fWy = viewLocation.y * self.contentScaleFactor;

    if (self.inputManager) {
        self.inputManager->MsgReceive(pXMsg);
        self.inputManager->MsgReceive(pYMsg);
    }
}

+ (Class)layerClass
{
    return [CAMetalLayer class];
}

- (void)setFrame:(CGRect)frame
{
    [super setFrame:frame];
    self.contentScaleFactor = UIScreen.mainScreen.scale;
    CGSize drawableSize = CGSizeMake(self.bounds.size.width * self.contentScaleFactor, self.bounds.size.height * self.contentScaleFactor);
    ((CAMetalLayer *)self.layer).drawableSize = drawableSize;
    
    
    [self.delegate renderView:self
          didChangeOutputSize:drawableSize
                        scale:self.contentScaleFactor];
}

- (CAMetalLayer *)metalLayer
{
    return ((CAMetalLayer *)self.layer);
}

- (void)touchesBegan:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event
{
    [self handleMouseButtonEvent:event];
}

- (void)touchesEnded:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event
{
    
    [self handleMouseButtonEvent:event];
}

- (void)touchesMoved:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event
{
    [self updateClientMouseLocation:event];
}

- (void)handleMouseButtonEvent:(UIEvent*)event
{
    [self updateClientMouseLocation:event];

    UITouch *touch = [event touchesForView:self].anyObject;
    CGPoint viewLocation = [touch locationInView:self];

    plIMouseBEventMsg* pBMsg = new plIMouseBEventMsg;

    if (touch.phase == UITouchPhaseEnded) {
        pBMsg->fButton |= kLeftButtonUp;
    //} else if (event.type == NSEventTypeRightMouseUp) {
        //pBMsg->fButton |= kRightButtonUp;
    } else if (touch.phase == UITouchPhaseBegan) {
        pBMsg->fButton |= kLeftButtonDown;
    }// else if (event.type == NSEventTypeRightMouseDown) {
    //    pBMsg->fButton |= kRightButtonDown;
    //}

    @synchronized(self.layer) {
        self.inputManager->MsgReceive(pBMsg);
    }

    delete (pBMsg);
}

- (void)updateClientMouseLocation:(UIEvent*)event
{
    UITouch *touch = [event touchesForView:self].anyObject;
    CGPoint viewLocation = [touch locationInView:self];

    CGRect windowViewBounds = self.bounds;
    CGFloat xNormal = (viewLocation.x) / windowViewBounds.size.width;
    CGFloat yNormal =
        viewLocation.y / windowViewBounds.size.height;

    plIMouseXEventMsg* pXMsg = new plIMouseXEventMsg;
    plIMouseYEventMsg* pYMsg = new plIMouseYEventMsg;

    pXMsg->fX = xNormal;
    pYMsg->fY = yNormal;

    // Plasma internally uses input coords as display coords
    pXMsg->fWx = viewLocation.x * self.contentScaleFactor;
    pYMsg->fWy = viewLocation.y * self.contentScaleFactor;

    @synchronized(self.layer) {
        if (self.inputManager) {
            self.inputManager->MsgReceive(pXMsg);
            self.inputManager->MsgReceive(pYMsg);
        }
    }

    /*if (self.inputManager->RecenterMouse()) {
        CGPoint warpPoint = [self.window convertPointToScreen:windowLocation];
        CGPoint newWindowLocation = windowLocation;

        if (self.inputManager->RecenterMouse() && (pXMsg->fX <= 0.1 || pXMsg->fX >= 0.9)) {
            newWindowLocation.x = CGRectGetMidX(self.window.contentView.bounds);

            // macOS won't generate a new message on warp, need to tell Plasma by hand
            pXMsg->fWx = newWindowLocation.x * self.window.screen.backingScaleFactor;;
            pXMsg->fX = 0.5f;
            self.inputManager->MsgReceive(pXMsg);
        }

        if (self.inputManager->RecenterMouse() && (pYMsg->fY <= 0.1 || pYMsg->fY >= 0.9)) {
            newWindowLocation.y = CGRectGetMidY(self.window.contentView.bounds);

            // macOS won't generate a new message on warp, need to tell Plasma by hand
            pYMsg->fWy = newWindowLocation.y * self.window.screen.backingScaleFactor;;
            pYMsg->fY = 0.5f;
            self.inputManager->MsgReceive(pYMsg);
        }

        if (!CGPointEqualToPoint(newWindowLocation, windowLocation)) {
            warpPoint = [self.window convertPointToScreen:newWindowLocation];
            warpPoint.y = [[NSScreen screens][0] frame].size.height - warpPoint.y;
            CGWarpMouseCursorPosition(warpPoint);
            // macOS will pause input afer warp, turn that off
            CGEventSourceRef eventSourceRef =
                CGEventSourceCreate(kCGEventSourceStateCombinedSessionState);
            CGEventSourceSetLocalEventsSuppressionInterval(eventSourceRef, 0.0);
            CFRelease(eventSourceRef);
        }*/
    //}

    delete (pXMsg);
    delete (pYMsg);
}
/*
// Only override drawRect: if you perform custom drawing.
// An empty implementation adversely affects performance during animation.
- (void)drawRect:(CGRect)rect {
    // Drawing code
}
*/

@end
