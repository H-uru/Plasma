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

#include "plVTDecoder.h"

#include <mkvparser/mkvparser.h>
#include <mkvparser/mkvreader.h>

#include "plGImage/plMipmap.h"
#include "plMoviePlayer.h"
#include "plPlanarImage.h"

class plVTMovieFrame : public plMovieFrame
{
public:
    plVTMovieFrame(CVPixelBufferRef pixelBuffer) : fPixelBuffer(pixelBuffer)
    {
        CVPixelBufferLockBaseAddress(fPixelBuffer, 0);
        CVPixelBufferRetain(fPixelBuffer);
        fPlanes[0] = static_cast<unsigned char*>(CVPixelBufferGetBaseAddressOfPlane(pixelBuffer, 0));
        fPlanes[1] = static_cast<unsigned char*>(CVPixelBufferGetBaseAddressOfPlane(pixelBuffer, 1));
        fPlanes[2] = static_cast<unsigned char*>(CVPixelBufferGetBaseAddressOfPlane(pixelBuffer, 2));
        fPlanes[3] = nullptr;

        fStrides[0] = static_cast<int32_t>(CVPixelBufferGetBytesPerRowOfPlane(pixelBuffer, 0));
        fStrides[1] = static_cast<int32_t>(CVPixelBufferGetBytesPerRowOfPlane(pixelBuffer, 1));
        fStrides[2] = static_cast<int32_t>(CVPixelBufferGetBytesPerRowOfPlane(pixelBuffer, 2));
        fStrides[3] = 0;
    }

    uint32_t GetWidth() override
    {
        return uint32_t(CVPixelBufferGetWidth(fPixelBuffer));
    }

    uint32_t GetHeight() override
    {
        return uint32_t(CVPixelBufferGetHeight(fPixelBuffer));
    }

    int32_t* GetStrides() override
    {
        return fStrides;
    }

    unsigned char** GetPlanes() override
    {
        return fPlanes;
    }

    Format GetFormat() override
    {
        return plMovieFrame::Format::I420;
    }

    ~plVTMovieFrame()
    {
        CVPixelBufferUnlockBaseAddress(fPixelBuffer, 0);
        CVPixelBufferRelease(fPixelBuffer);
    }

private:
    CVPixelBufferRef fPixelBuffer;
    unsigned char*   fPlanes[4];
    int32_t          fStrides[4];
};

static OSStatus CreateFormatDescriptionFromVP9Track(
    const mkvparser::VideoTrack* video_track,
    CMFormatDescriptionRef*      format_desc_out)
{
    // We need to reconstruct the ISO/IEC 14496 atom
    // that would prepend the track in an MP4 container

    // The full box container as specified by
    // ISO/IEC 14496-12
    struct FullBox
    {
        uint8_t version;
        uint8_t flags[3];
    };

    // The VP9 specific data that extends the full box:
    // https://www.webmproject.org/vp9/mp4/
    struct vppC : FullBox
    {
        uint8_t  profile;
        uint8_t  level;
        uint8_t  bitDepthChromaAndRange;
        uint8_t  colourPrimaries;
        uint8_t  transferCharacteristics;
        uint8_t  matrixCoefficients;
        uint16_t codecInitializationDataSize;
        uint8_t  codecInitializationData;
    };

    CFMutableDataRef boxData = CFDataCreateMutable(kCFAllocatorDefault, sizeof(vppC));
    // This also zeroes
    CFDataIncreaseLength(boxData, sizeof(vppC));
    vppC& vpccBox = *reinterpret_cast<vppC*>(CFDataGetMutableBytePtr(boxData));

    // Version and flags
    vpccBox.version = 1; // version

    // Level (default to 10 = level 1.0)
    vpccBox.level = 10;

    // Bit depth (8), chroma subsampling (4:2:0 = 1), full range (0)
    vpccBox.bitDepthChromaAndRange = (8 << 4) | (1 << 1) | 0;

    // Color info - try to get from track
    const mkvparser::Colour* colour = video_track->GetColour();
    if (colour) {
        vpccBox.colourPrimaries = colour->primaries; // colour primaries
        vpccBox.transferCharacteristics = colour->transfer_characteristics;
        vpccBox.matrixCoefficients = colour->matrix_coefficients;
    } else {
        vpccBox.colourPrimaries = 2;         // Unspecified
        vpccBox.transferCharacteristics = 2; // Unspecified
        vpccBox.matrixCoefficients = 2;      // Unspecified
    }

    // Create extensions dictionary
    CFMutableDictionaryRef extensions = CFDictionaryCreateMutable(
        kCFAllocatorDefault,
        1,
        &kCFTypeDictionaryKeyCallBacks,
        &kCFTypeDictionaryValueCallBacks);

    CFMutableDictionaryRef atoms = CFDictionaryCreateMutable(
        kCFAllocatorDefault,
        1,
        &kCFTypeDictionaryKeyCallBacks,
        &kCFTypeDictionaryValueCallBacks);

    CFDictionarySetValue(atoms, CFSTR("vpcC"), boxData);

    CFDictionarySetValue(
        extensions,
        kCMFormatDescriptionExtension_SampleDescriptionExtensionAtoms,
        atoms);

    CFRelease(atoms);
    CFRelease(boxData);

    // Create format description
    OSStatus status = CMVideoFormatDescriptionCreate(
        kCFAllocatorDefault,
        kCMVideoCodecType_VP9,
        int32_t(video_track->GetWidth()),
        int32_t(video_track->GetHeight()),
        extensions,
        format_desc_out);

    CFRelease(extensions);
    return status;
}

plVTDecoder* plVTDecoder::CreateDecoder(const mkvparser::VideoTrack* track)
{
    if (__builtin_available(macOS 11.0, *)) {
        VTRegisterSupplementalVideoDecoderIfAvailable(kCMVideoCodecType_VP9);
    } else {
        return nullptr;
    }

    bool hardwareDecode = VTIsHardwareDecodeSupported(kCMVideoCodecType_VP9);
    if (!hardwareDecode)
        return nullptr;

    return new plVTDecoder(track);
}

plVTDecoder::plVTDecoder(const mkvparser::VideoTrack* track)
{
    OSStatus               err;
    CFMutableDictionaryRef decoderOptions = CFDictionaryCreateMutable(
        kCFAllocatorDefault,
        1,
        &kCFTypeDictionaryKeyCallBacks,
        &kCFTypeDictionaryValueCallBacks);

    CFDictionarySetValue(
        decoderOptions,
        kVTVideoDecoderSpecification_EnableHardwareAcceleratedVideoDecoder,
        kCFBooleanTrue);

    CreateFormatDescriptionFromVP9Track(track, &fFormat);

    SInt32      PixelFormatTypeValue = kCVPixelFormatType_420YpCbCr8PlanarFullRange;
    CFNumberRef PixelFormatTypeNumber = CFNumberCreate(
        kCFAllocatorDefault,
        kCFNumberSInt32Type,
        &PixelFormatTypeValue);

    const void* outputKeys[] = {kCVPixelBufferPixelFormatTypeKey};
    const void* outputValues[] = {PixelFormatTypeNumber};

    auto outputDescription = CFDictionaryCreate(
        kCFAllocatorDefault,
        outputKeys,
        outputValues,
        1,
        &kCFTypeDictionaryKeyCallBacks,
        &kCFTypeDictionaryValueCallBacks);

    err = VTDecompressionSessionCreate(
        kCFAllocatorDefault,
        fFormat,
        decoderOptions,
        outputDescription,
        nullptr,
        &fDecompressionSession);
    hsAssert(err == noErr, "Decoder creation failed");
}

std::unique_ptr<plMovieFrame> plVTDecoder::DecodeNextFrame(uint8_t* frameData, const size_t size)
{
    CMSampleBufferRef sampleBuffer;
    CMBlockBufferRef  buffer;

    OSStatus err;
    err = CMBlockBufferCreateWithMemoryBlock(
        nullptr,
        frameData,
        size,
        nullptr,
        nullptr,
        0,
        size,
        0,
        &buffer);
    hsAssert(err == noErr, "Could not create block buffer for frame data");
    err = CMSampleBufferCreate(
        nullptr,
        buffer,
        true,
        nullptr,
        nullptr,
        fFormat,
        1,
        0,
        nullptr,
        1,
        &size,
        &sampleBuffer);
    hsAssert(err == noErr, "Could not create sample buffer");
    __block std::unique_ptr<plMovieFrame> frame;
    VTDecompressionOutputHandler handler = ^(
        OSStatus status,
        VTDecodeInfoFlags infoFlags,
        CVImageBufferRef _Nullable imageBuffer,
        CMTime presentationTimeStamp,
        CMTime presentationDuration) {
        if (imageBuffer)
            frame = std::make_unique<plVTMovieFrame>(imageBuffer);
    };
    err = VTDecompressionSessionDecodeFrameWithOutputHandler(
        fDecompressionSession,
        sampleBuffer,
        0,
        nullptr,
        handler);
    hsAssert(err == noErr, "Decoding failed");

    return std::move(frame);
}

plVTDecoder::~plVTDecoder()
{
    CFRelease(fDecompressionSession);
    CFRelease(fFormat);
}
