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

import CxxStdlib
import Metal
import QuartzCore

/*
 RenderLoop is a modern CADisplayLink based render loop. It supports
 OpenGL or Metal - but doesn't provide any automatic performance
 monitoring of Metal. It is available on all Apple platforms, but
 only supports macOS 14 and higher.
 */
@available(macOS 14.0, *)
public class RenderLoop
{
    private var displayLink: CADisplayLink?
#if os(macOS)
    private weak var window: NSWindow?
#endif
    
    /// Optional callback to be invoked when a new drawable is available.
    private var renderCallback: plRenderCallback?
    
    init(window: NSWindow) {
        self.window = window
    }
    
    public func SetRenderCallback(_ callback: plRenderCallback) {
        self.renderCallback = callback
    }
    
    public func StartRenderLoop()
    {
        guard let window else {
            return
        }
        #if os(macOS)
        var displayLink = window.displayLink(target: self, selector: #selector(RenderLoop.render))
        #else
        var displayLink = CADisplayLink(target: self, selector: #selector(self.render()))
        #endif
        
        displayLink.add(to: .main, forMode: .common)
        displayLink.isPaused = false
        
        self.displayLink = displayLink
    }
    
    public func StopRenderLoop()
    {
        guard let displayLink else {
            // If the display link was never created nothing to stop
            return;
        }
        displayLink.invalidate()
        self.displayLink = nil
    }
    
    static public func Create(metalLayer: CAMetalLayer) -> MetalRenderLoop
    {
        return MetalRenderLoop(metalLayer: metalLayer)
    }
    
    @objc func render()
    {
        renderCallback?.callAsFunction()
    }
}

/*
 MetalRenderLoop is a CAMetalDisplayLink based render loop.
 It provides deep integration with Metal and to provide superior
 syncronization and run loop management. This render loop only
 works with Metal. When this render loop is active, the renderer
 cannot directly interact with the framebuffers provided by the
 window server. The renderer must wait for this render loop to
 provide a drawable.
 
 Because CAMetalDisplayLink also needs to manage the render
 destination - it is both a render loop and a render destination.
 */
@available(macOS 14.0, *)
public class MetalRenderLoop: CAMetalDisplayLinkDelegate
{
    private weak var metalLayer: CAMetalLayer?
    private var displayLink: CAMetalDisplayLink?
    private var lastUpdate: plMetalRenderSurface?
    
    /// Optional callback to be invoked when a new drawable is available.
    private var renderCallback: plRenderCallback?
    
    public func SetRenderCallback(_ callback: plRenderCallback) {
        self.renderCallback = callback
    }
    
    init(metalLayer: CAMetalLayer) {
        // Prime the first framebuffer so something is available
        // before the vsync timer starts. We're only allowed to directly
        // fetch a framebuffer while vsync is not running. So don't get a
        // framebuffer in this way again!
        let drawable = metalLayer.nextDrawable()
        
        if let drawable {
            self.lastUpdate = plMetalRenderSurface(
                OpaquePointer(Unmanaged.passUnretained(drawable.texture).toOpaque()),
                OpaquePointer(Unmanaged.passUnretained(drawable).toOpaque())
            )
        }
        
        self.metalLayer = metalLayer
    }
    
    public func StartRenderLoop()
    {
        guard let metalLayer else {
            // If our Metal layer went somewhere nothing to configure
            return;
        }
        let displayLink = CAMetalDisplayLink(metalLayer: metalLayer)
        displayLink.delegate = self
        displayLink.add(to: .main, forMode: .common)
        displayLink.isPaused = false
        
        self.displayLink = displayLink
    }
    
    public func StopRenderLoop()
    {
        guard let displayLink else {
            // If the display link was never created nothing to stop
            return;
        }
        displayLink.invalidate()
        self.displayLink = nil
    }
    
    static public func Create(metalLayer: CAMetalLayer) -> MetalRenderLoop
    {
        return MetalRenderLoop(metalLayer: metalLayer)
    }
    
    public func SetOutputSize(_ size: CGSize)
    {
        self.metalLayer?.drawableSize = size
    }
    
    public func GetNextRenderSurface(_ buffer: UnsafeMutableRawPointer) -> plOptionalMetalRenderSurface
    {
        guard let metalLayer, let lastUpdate else {
            return nil
        }
        
        let buffer = Unmanaged<MTLCommandBuffer>.fromOpaque(buffer).takeUnretainedValue()
        if metalLayer.device !== buffer.device {
            metalLayer.device = buffer.device
        }
        
        defer {
            // Don't provide the same framebuffer twice
            // Clear the last update
            self.lastUpdate = nil
        }
        
        return plOptionalMetalRenderSurface(lastUpdate)
    }
    
    public func metalDisplayLink(_ link: CAMetalDisplayLink, needsUpdate update: CAMetalDisplayLink.Update) {
        self.lastUpdate = plMetalRenderSurface(
            OpaquePointer(Unmanaged.passUnretained(update.drawable.texture).toOpaque()),
            OpaquePointer(Unmanaged.passUnretained(update.drawable).toOpaque())
            )
        // Invoke the render callback if it has been set
        renderCallback?.callAsFunction()
    }
    
}
