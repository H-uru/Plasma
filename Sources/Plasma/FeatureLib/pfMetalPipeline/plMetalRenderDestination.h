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

#ifndef plMetalRenderSurface_h
#define plMetalRenderSurface_h

#import <Metal/Metal.hpp>
#import <QuartzCore/QuartzCore.hpp>
#import <optional>

struct plMetalRenderSurface
{
    // Required - the color texture to draw to.
    MTL::Texture*      colorTexture;
    // Optional - only provided for a CA based windowing system
    CA::MetalDrawable* drawable;

    // Initialize with new objects - retain them
    plMetalRenderSurface(MTL::Texture* colorTexture, CA::MetalDrawable* drawable)
        : colorTexture(colorTexture->retain()), drawable(drawable->retain())
    { }

    // Move constructor - transfer ownership directly
    plMetalRenderSurface(plMetalRenderSurface&& other)
        : colorTexture(other.colorTexture), drawable(other.drawable)
    {
        other.colorTexture = nullptr;
        other.drawable = nullptr;
    }

    // Copy constructor - retain to create independent copy
    plMetalRenderSurface(const plMetalRenderSurface& other)
        : colorTexture(other.colorTexture), drawable(other.drawable)
    { }

    // Copy assignment with self-assignment protection
    plMetalRenderSurface& operator=(const plMetalRenderSurface& other)
    {
        if (this != &other) {
            colorTexture->release();
            drawable->release();
            colorTexture = other.colorTexture->retain();
            drawable = other.drawable->retain();
        }
        return *this;
    }

    // Move assignment - transfer ownership without releasing
    plMetalRenderSurface& operator=(plMetalRenderSurface&& other)
    {
        if (this != &other) {
            colorTexture = other.colorTexture;
            drawable = other.drawable;
            other.colorTexture = nullptr;
            other.drawable = nullptr;
        }
        return *this;
    }

    // Destructor - release retained resources
    ~plMetalRenderSurface()
    {
        if (colorTexture) {
            colorTexture->release();
        }
        if (drawable) {
            drawable->release();
        }
    }
};

// Swift cannot specialize optionals right now
// Declare the specialization here
typedef std::optional<plMetalRenderSurface> plOptionalMetalRenderSurface;

/*
 plMetalRenderDestination represents a destination we can render to
 from the Metal renderer. On macOS and iOS this will usually
 be a drawable - but on visionOS it might be something like
 a RealityKit LowLevelTexture.

 A client provides a platform specific plMetalRenderDestination to
 the engine, and then the engine calls into the render destination
 when it needs a frame. A render destination might be vsync'd,
 and if the engine asks for a render destination before the next
 frame is ready the render destination can return null.

 One issue is that the client might have to provide a render destination
 as a Swift type - but Swift does not support subclassing C++ types.
 We can specialize a C++ template on a Swift type - but the engine
 won't know what that specialization is at compile time.

 This code does both. It declares a plMetalRenderDestinationType that
 anonymizes render destinations, and then a plMetalRenderDestination
 that can be specialized on something like a Swift type.

 In the future - if Swift supports inheriting from C++ types with dynamic
 dispatch this could be simplified.
 */

class plMetalRenderDestinationType
{
public:
    virtual plOptionalMetalRenderSurface GetNextRenderSurface(MTL::CommandBuffer* buffer) = 0;
    virtual void                         SetOutputSize(CGSize size) = 0;

    virtual ~plMetalRenderDestinationType() = default;
};

/*
 One final twist - Swift exports it's types as value types,
 but they behave like references - similar to std::shared_ptr.
 That means we need to have a version of plMetalRenderDestination
 for value types, and another for pointer types.

 The pointer types version is used when we have a real C++
 type that we need to specialize on. The value type is used
 when this template specializes on a Swift type.
 */

template <typename T>
class plMetalRenderDestination : public plMetalRenderDestinationType
{
public:
    plMetalRenderDestination(T provider) : fProvider(provider) {}
    plOptionalMetalRenderSurface GetNextRenderSurface(MTL::CommandBuffer* buffer) override { return fProvider.GetNextRenderSurface(buffer); }
    void                         SetOutputSize(CGSize size) override { fProvider.SetOutputSize(size); }

private:
    T fProvider;
};

template <typename T>
class plMetalRenderDestination<T*> : public plMetalRenderDestinationType
{
public:
    template <typename... Args>
    plMetalRenderDestination(Args&&... args)
        : fProvider(std::make_unique<T>(std::forward<Args>(args)...))
    { }
    plOptionalMetalRenderSurface GetNextRenderSurface(MTL::CommandBuffer* buffer) override { return fProvider->GetNextRenderSurface(buffer); }
    void                         SetOutputSize(CGSize size) override { fProvider->SetOutputSize(size); }

private:
    std::unique_ptr<T> fProvider;
};

struct plMetalDestinationWindow
{
    plMetalRenderDestinationType* fMetalRenderDestination;
};

#endif
