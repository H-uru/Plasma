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

#import "PLSPatcher.h"
#import "NSString+StringTheory.h"

#include <archive.h>
#include <archive_entry.h>
#include <unordered_set>
#include <string_theory/format>

#include "HeadSpin.h"
#include "hsDarwin.h"
#include "hsTimer.h"

#include "pfPatcher/pfPatcher.h"
#include "pfPatcher/plManifests.h"
#include "plFileSystem.h"
#include "plNetGameLib/plNetGameLib.h"
#include "plStatusLog/plStatusLog.h"

class Patcher
{
public:
    PLSPatcher* parent;
    void IOnPatchComplete(ENetError result, const ST::string& msg);
    void IOnProgressTick(uint64_t curBytes, uint64_t totalBytes, const ST::string& status);
    void IOnDownloadBegin(const plFileName& file);
    void ISelfPatch(const plFileName& file);
    plFileName IFindBundleExe(const plFileName& file);
};

@interface PLSPatcher ()
@property BOOL selfPatched;
@property pfPatcher* patcher;
@property NSTimer* networkPumpTimer;
@property Patcher cppPatcher;
@property NSURL* updatedClientURL;
@property NSURL* temporaryDirectory;
@end

@implementation PLSPatcher

- (id)init
{
    self = [super init];
    self.selfPatched = false;

    _cppPatcher.parent = self;

    self.patcher = new pfPatcher();
    _patcher->OnFileDownloadBegin(
        std::bind(&Patcher::IOnDownloadBegin, _cppPatcher, std::placeholders::_1));
    _patcher->OnProgressTick(std::bind(&Patcher::IOnProgressTick, _cppPatcher,
                                       std::placeholders::_1, std::placeholders::_2,
                                       std::placeholders::_3));
    _patcher->OnCompletion(std::bind(&Patcher::IOnPatchComplete, _cppPatcher, std::placeholders::_1,
                                     std::placeholders::_2));
    _patcher->OnFileDownloadDesired(IApproveDownload);
    _patcher->OnSelfPatch(std::bind(&Patcher::ISelfPatch, _cppPatcher, std::placeholders::_1));
    _patcher->OnFindBundleExe(std::bind(&Patcher::IFindBundleExe, _cppPatcher, std::placeholders::_1));

    self.networkPumpTimer = [NSTimer timerWithTimeInterval:1.0 / 1000.0
                                                   repeats:true
                                                     block:^(NSTimer* _Nonnull timer) {
                                                         hsTimer::IncSysSeconds();
                                                         NetClientUpdate();
                                                     }];

    return self;
}

- (void)start
{
    [[NSRunLoop mainRunLoop] addTimer:self.networkPumpTimer forMode:NSDefaultRunLoopMode];
    self.patcher->RequestManifest(plManifest::ClientManifest());
    self.patcher->Start();
}

- (NSURL *)completeSelfPatch:(NSError **)error;
{
    NSString* destinationPath = [NSString stringWithSTString:plManifest::PatcherExecutable().AsString()];
    NSURL* destinationURL = [NSURL fileURLWithPath:destinationPath];
    
    NSError* errorInScope;
    
    if (!self.updatedClientURL) {
        // uh oh - this implies we weren't able to decompress the client
        if (error) {
            // Handle as a generic could not read file error.
            // Bad compression on the server will require correction on the server end.
            *error = [NSError errorWithDomain:NSCocoaErrorDomain code:NSFileReadNoSuchFileError userInfo:nil];
        }
        return nil;
    }
    
    if ([NSFileManager.defaultManager fileExistsAtPath:destinationPath]) {
        // need to swap
        BOOL swapSucceeded = renamex_np(destinationURL.path.fileSystemRepresentation, self.updatedClientURL.path.fileSystemRepresentation, RENAME_SWAP) == 0;
        if (swapSucceeded) {
            // delete the old version - this is very likely us
            // we want to terminate after. Our bundle will no longer be valid.
            if (self.temporaryDirectory) {
                [NSFileManager.defaultManager removeItemAtURL:self.temporaryDirectory error:&errorInScope];
            }
        } else {
            // abort and return an error
            errorInScope = [NSError errorWithDomain:NSPOSIXErrorDomain code:errno userInfo:nil];
        }
    } else {
        // no executable already present! Just move things into place.
        [NSFileManager.defaultManager moveItemAtURL:self.updatedClientURL toURL:destinationURL error:&errorInScope];
    }
    
    if (errorInScope) {
        // Try to clean up if there was an error
        [NSFileManager.defaultManager removeItemAtURL:self.updatedClientURL error:nil];
        if (error) {
            *error = errorInScope;
        }
        return nil;
    }
    
    return destinationURL;
}

void Patcher::IOnDownloadBegin(const plFileName& file)
{
    NSString* fileName = [NSString stringWithSTString:file.AsString()];
    dispatch_async(dispatch_get_main_queue(), ^{
        [parent.delegate patcher:parent beganDownloadOfFile:fileName];
    });
}

void Patcher::IOnProgressTick(uint64_t curBytes, uint64_t totalBytes, const ST::string& status)
{
    NSString* statusString = [NSString stringWithSTString:status];

    dispatch_async(dispatch_get_main_queue(), ^{
        [parent.delegate patcher:parent
                 updatedProgress:statusString
                       withBytes:curBytes
                           outOf:totalBytes];
    });
}

bool IApproveDownload(const plFileName& file)
{
    ST::string ext = file.GetFileExt();
    // nothing from Windows, please
    // temporary measure until macOS has its own manifest
    static std::unordered_set<ST::string, ST::hash_i, ST::equal_i> extExcludeList{
        "exe",
        "dll",
        "pdb"
    };
    return extExcludeList.find(file.GetFileExt()) == extExcludeList.end();
}

static la_ssize_t copy_data(struct archive* ar, struct archive* aw)
{
    while (true) {
        la_ssize_t r;
        const void* buff;
        size_t size;
        la_int64_t offset;
        
        r = archive_read_data_block(ar, &buff, &size, &offset);
        if (r == ARCHIVE_EOF)
            return (ARCHIVE_OK);
        if (r < ARCHIVE_OK)
            return (r);
        r = archive_write_data_block(aw, buff, size, offset);
        if (r < ARCHIVE_OK) {
            pfPatcher::GetLog()->AddLine(plStatusLog::kRed, archive_error_string(aw));
            return (r);
        }
    }
}

void Patcher::ISelfPatch(const plFileName& file)
{
    /*
     Note on errors:
     This function does not return errors, but a self patch
     without a populated updatedClientURL will imply something
     went wrong during decompress.
     */
    
    PLSPatcher* patcher = parent;
    patcher.selfPatched = true;
    
    int flags;
    la_ssize_t r;

    /* Select which attributes we want to restore. */
    flags = ARCHIVE_EXTRACT_TIME | ARCHIVE_EXTRACT_PERM;
    
    struct archive* a = archive_read_new();
    struct archive* ext = archive_write_disk_new();
    
    {
        int error;
        error = archive_read_support_format_tar(a);
        hsAssert(error == ARCHIVE_OK, "Unable to set tar format option");
        error = archive_read_support_filter_gzip(a);
        hsAssert(error == ARCHIVE_OK, "Unable to set gzip filter");
        error = archive_read_support_filter_bzip2(a);
        hsAssert(error == ARCHIVE_OK, "Unable to set bzip filter");
        
        error = archive_write_disk_set_options(ext, flags);
        hsAssert(error == ARCHIVE_OK, "Unable to set write options");
        error = archive_write_disk_set_standard_lookup(ext);
        hsAssert(error == ARCHIVE_OK, "Unable to set write standard lookup");
    }
    
    if ((r = archive_read_open_filename(a, file.GetFileName().c_str(), 10240)) != ARCHIVE_OK) {
        // couldn't read
        archive_read_free(a);
        archive_write_close(ext);
        archive_write_free(ext);
        return;
    }
    
    NSError* error;
    NSURL* currentDirectory = [NSURL fileURLWithPath:NSFileManager.defaultManager.currentDirectoryPath];
    patcher.temporaryDirectory = [NSFileManager.defaultManager
                                  URLForDirectory:NSItemReplacementDirectory
                                  inDomain:NSUserDomainMask
                                  appropriateForURL:currentDirectory
                                  create:YES error:&error];
    NSURL* outputURL;
    if (patcher.temporaryDirectory) {
        outputURL = [patcher.temporaryDirectory URLByAppendingPathComponent:[NSString stringWithSTString:plManifest::PatcherExecutable().GetFileName()]];
        [NSFileManager.defaultManager createDirectoryAtURL:outputURL withIntermediateDirectories:false attributes:nil error:&error];
    }
    
    if (error) {
        // Not sure why things would go wrong, we should be able to
        // get a writable temp directory. But if we could not, bail.
        // Not populating the patched client path will be caught
        // later.
        archive_read_close(a);
        archive_read_free(a);
        archive_write_close(ext);
        archive_write_free(ext);
        return;
    }
    
    ST::string outputPath = [outputURL.path STString];
    
    bool succeeded = true;
    
    struct archive_entry* entry;
    while (true) {
        r = archive_read_next_header(a, &entry);
        if (r == ARCHIVE_EOF)
            break;
        if (r < ARCHIVE_OK)
            pfPatcher::GetLog()->AddLineF(plStatusLog::kRed, "Failed to read bundle archive: {}", archive_error_string(a));
        if (r < ARCHIVE_WARN) {
            succeeded = false;
            break;
        }
        const char* currentFile = archive_entry_pathname(entry);
        auto fullOutputPath = plFileName::Join(outputPath, currentFile);
        archive_entry_set_pathname(entry, fullOutputPath.AsString().c_str());
        r = archive_write_header(ext, entry);
        if (r < ARCHIVE_OK)
            pfPatcher::GetLog()->AddLineF(plStatusLog::kRed, "Failed to extract file while patching app bundle: {}", archive_error_string(ext));
        else if (archive_entry_size(entry) > 0) {
            r = copy_data(a, ext);
            if (r < ARCHIVE_OK)
                pfPatcher::GetLog()->AddLineF(plStatusLog::kRed, "Failed to extract file while patching app bundle: {}", archive_error_string(ext));
            if (r < ARCHIVE_WARN) {
                succeeded = false;
                break;
            }
        }
        r = archive_write_finish_entry(ext);
        if (r < ARCHIVE_OK)
            pfPatcher::GetLog()->AddLineF(plStatusLog::kRed, "Failed to extract file while patching app bundle: {}", archive_error_string(ext));
        if (r < ARCHIVE_WARN) {
            succeeded = false;
            break;
        }
    }
    archive_read_close(a);
    archive_read_free(a);
    archive_write_close(ext);
    archive_write_free(ext);
    
    plFileSystem::Unlink(file);
    
    if (succeeded) {
        parent.updatedClientURL = outputURL;
    }
}

void Patcher::IOnPatchComplete(ENetError result, const ST::string& msg)
{
    [parent.networkPumpTimer invalidate];
    if (IS_NET_SUCCESS(result)) {
        PLSPatcher* patcher = parent;
        dispatch_async(dispatch_get_main_queue(), ^{
            [patcher.delegate patcherCompleted:patcher
                                  didSelfPatch:patcher.selfPatched];
        });
    } else {
        NSString* msgString = [NSString stringWithSTString:msg];

        dispatch_async(dispatch_get_main_queue(), ^{
            ST::string errorString = ST::string::from_wchar(NetErrorToString(result));
            NSString* errorNSString = [NSString stringWithSTString:errorString];
            [parent.delegate
                patcherCompletedWithError:parent
                                    error:[NSError errorWithDomain:@"PLSPatchErrors"
                                                              code:result
                                                          userInfo:@{
                                                                NSLocalizedFailureErrorKey : errorNSString,
                                                                NSLocalizedFailureReasonErrorKey: msgString
                                                          }]];
        });
    }
}

plFileName Patcher::IFindBundleExe(const plFileName& clientPath)
{
    // If this is a Mac app bundle, MD5 the executable. The executable will hold the
    // code signing hash - and thus unique the entire bundle.
    
    @autoreleasepool {
        NSURL* bundleURL = [NSURL fileURLWithPath:[NSString stringWithSTString:clientPath.AsString()]];
        NSBundle* bundle = [NSBundle bundleWithURL:bundleURL];
        NSURL* executableURL = [bundle executableURL];
        
        if (executableURL) {
            NSString* executablePath = [executableURL path];
            return plFileName([[executableURL path] STString]);
        }
        
        return clientPath;
    }
}

@end
