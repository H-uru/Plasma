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

#ifndef plAccessGeometry_inc
#define plAccessGeometry_inc

#include "hsRefCnt.h"

#include <vector>

class plAccessSpan;
class plDrawInterface;
class plDrawable;
class plDrawableSpans;
class plGeometrySpan;
class plIcicle;
class plParticleSpan;
class plPipeline;
class plSpan;
class plVertexSpan;

class plAccessGeometry : public hsRefCnt
{
protected:
    void                    Nilify() { fPipe = nullptr; }

    plPipeline*                     fPipe;

    static plAccessGeometry*        fInstance;
public:
    // You're welcome to make your own,
    // but this is normally just called by the global plAccessGeometry's Init() function.
    // You should normally just use the instance supplied by Instance();
    plAccessGeometry(plPipeline* pipe=nullptr);

    static plAccessGeometry*        Instance() { return fInstance; }

    // App will initialize, which will create the global instance.
    // DeInit will nil the global instance.
    static void Init(plPipeline* pipe);
    static void DeInit();

    // External DLL's will share the same plAccessGeometry. After the main App has
    // DeInited the AccessGeometry,
    // all calls to Instance()->Function() will return nil in one form or another (e.g.
    // empty triangle lists). The external DLL needs to either not try to use these
    // accessor functions after PythonInterface::WeAreInShutdown() (it won't do any good
    // anyway as any work done will be thrown away), or else be prepared for receiving
    // empty data where there was data before.
    static void SetTheIntance(plAccessGeometry* i);

    // You have 2 options in opening the data.
    // RO - Read Only. 
    //      If you specify useSnapShot=true, then for channels which have had a snapshot
    //      taken, you will get pointers to this constant original snapshot form. For
    //      channels which have no snapshot data, or if useSnapShot=false, you will get
    //      pointers to the current (possibly modified since load) data. See SnapShot functions
    //      below.
    // RW - Read/Write access to the source data. After closing, this modified source data
    //      will be used to update the buffer data used for rendering.
    // In the normal case of reading the original (disk image) data, performing some operation
    //      on it and updating the renderable data, you need to open the same data twice, once
    //      RO(useSnapShot=true) (to get the constant source data) and once RW (for destination data). Note that
    //      the memory returned by RO will may be the same as returned by RW if there has been no snapshot
    //      taken.
    // The RW permutation by itself is useful when performing a one-time operation on the data (e.g.
    //      loadtime), so the modified source data is, to everyone else, what was read from disk.
    //      The only way to retrieve the original source data is to read it from disk again,
    //      unless you've made a snapshot. Normally you would do a RW modify the original data,
    //      then take the snapshot if you are going to be performing more modifications.
    // In ALL MODIFICATION CASES, if the modified data is paged out, and the original paged back in, you will
    //      need to perform your operation again - your modifications aren't saved anywhere.
    void    OpenRO(plDrawable* drawable, uint32_t spanIdx, plAccessSpan& acc, bool useSnapShot=true) const;
    void    OpenRW(plDrawable* drawable, uint32_t spanIdx, plAccessSpan& acc, bool idxToo=false) const;

    // What do we need to close up here?
    void    Close(plAccessSpan& acc) const;

    // Second set. You have a SceneObject's DrawInterface. This can reference into
    // multiple drawables, and multiple spans within each drawable. You would rather
    // not deal with it. So you can open by passing in a DrawInterface, and get back
    // a list of geometry corresponding to that SceneObject/DrawInterface.
    // NOTE: the list is in no way suggested to be homogenous. In fact, it's guaranteed
    // not to be, because the reason the single object resolved into multiple geometry spans
    // (possibly across multiple drawables) is that the conceptual single object is composed 
    // of multiple types of data that can't be batched into a single drawprimitive call.
    // At the least, the different AccessSpans will have different materials. But it's just
    // as likely that they will have different underlying formats (number of UVs, etc.).
    // Again, if you are using the iterators supplied, you probably don't care, but sometimes
    // you will (like if you are messing with the UVs).
    void    OpenRO(const plDrawInterface* di, std::vector<plAccessSpan>& accs, bool useSnapShot=true) const;
    void    OpenRW(const plDrawInterface* di, std::vector<plAccessSpan>& accs, bool idxToo=false) const;

    void    Close(std::vector<plAccessSpan>& accs) const;

    // SnapShot functions.
    // If you need to generate channel values based on the original values (e.g. normal perterbation)
    // you need to reserve a copy of the original data. Only the channels specified will be copied.
    // Only one snapshot is ever taken, and it is the union of all channels requested. For example,
    //      taking a snapshot of positions AFTER taking a snapshot of positions/normals is a no-op,
    //      but taking a snapshot of positions/normals AFTER a snapshot of just positions will result in
    //      a copy of positions from the old snapshot, then a copy of normals from the buffergroup
    //      into the new snapshot, then freeing of the old snapshot.
    // Still, you should only snapshot the minimum set of channels you will need to be reading in their
    //      original form later.
    // The snapshot data is refcounted. You need to match your TakeSnapShots with FreeSnapShots.
    // SnapShot data is stored interleaved for efficiency, but don't count on it. Use an iterator.
    // RestoreSnapShot will copy the stored channels back into the buffer group, resetting those channels to
    //      the state when the snapshot was taken. Note that channels not SnapShotted might have been modified
    //      via OpenRW.
    // 
    void    TakeSnapShot(plDrawable* drawable, uint32_t spanIdx, uint32_t channels) const;
    void    RestoreSnapShot(plDrawable* drawable, uint32_t spanIdx, uint32_t channels) const;
    void    ReleaseSnapShot(plDrawable* drawable, uint32_t spanIdx) const;

    void    TakeSnapShot(const plDrawInterface* di, uint32_t channels) const;
    void    RestoreSnapShot(const plDrawInterface* di, uint32_t channels) const;
    void    ReleaseSnapShot(const plDrawInterface* di) const;

    // We often have geometry spans just sitting around devoid of any DI's, drawables or sceneobjects.
    // They aren't too bad to access directly (not like diving through the drawable into buffergroups),
    // but this let's them be accessed in a manner consistent with other geometry manipulations.
    void    AccessSpanFromGeometrySpan(plAccessSpan& dst, const plGeometrySpan* src) const { IAccessSpanFromSourceSpan(dst, src); }

protected:
    void    IAccessSpanFromSourceSpan(plAccessSpan& dst, const plGeometrySpan* src) const;
    void    IAccessSpanFromSpan(plAccessSpan& dst, plDrawableSpans* drawable, const plSpan* span, bool useSnap, bool readOnly) const;
    void    IAccessSpanFromVertexSpan(plAccessSpan& dst, plDrawableSpans* drawable, const plVertexSpan* span, bool readOnly) const;
    void    IAccessConnectivity(plAccessSpan& dst, plDrawableSpans* drawable, const plSpan* src) const;
    void    IAccessSpanFromIcicle(plAccessSpan& dst, plDrawableSpans* drawable, const plIcicle* span, bool readOnly) const;
    void    IAccessSpanFromParticle(plAccessSpan& dst, plDrawableSpans* drawable, const plParticleSpan* span, bool readOnly) const;
    void    IAccessSpanFromSnap(plAccessSpan& dst, plDrawableSpans* drawable, const plSpan* src) const;

    void    IOpen(plDrawable* d, uint32_t spanIdx, plAccessSpan& acc, bool useSnap, bool readOnly, bool idxToo=true) const;
};

#endif // plAccessGeometry_inc
