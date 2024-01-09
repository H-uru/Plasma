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
#include "hsTimer.h"

#include "pfPatcher/pfPatcher.h"
#include "pfPatcher/plManifests.h"
#include "plFileSystem.h"
#include "plNetGameLib/plNetGameLib.h"

class Patcher
{
public:
    PLSPatcher* parent;
    void IOnPatchComplete(ENetError result, const ST::string& msg);
    void IOnProgressTick(uint64_t curBytes, uint64_t totalBytes, const ST::string& status);
    void IOnDownloadBegin(const plFileName& file);
    void ISelfPatch(const plFileName& file);
};

@interface PLSPatcher ()
@property BOOL selfPatched;
@property pfPatcher* patcher;
@property NSTimer* networkPumpTimer;
@property Patcher cppPatcher;
@property NSURL* updatedClientURL;
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

- (NSURL *)completeSelfPatch
{
    NSString* destinationPath = [NSString stringWithSTString:plManifest::PatcherExecutable().AsString()];
    NSURL *destinationURL = [NSURL fileURLWithPath:[NSString stringWithSTString:plManifest::PatcherExecutable().AsString()]];
    
    if ([NSFileManager.defaultManager fileExistsAtPath:destinationPath]) {
        // need to swap
        
        char originalPath[PATH_MAX] = {0};
        [self.updatedClientURL.path getFileSystemRepresentation:originalPath maxLength:sizeof(originalPath)];
        
        char newPath[PATH_MAX] = {0};
        [destinationURL.path getFileSystemRepresentation:newPath maxLength:sizeof(newPath)];
        
        renamex_np(newPath, originalPath, RENAME_SWAP);
        
        // delete the old version - this is very likely us
        // we want to terminate after. Our bundle will no longer be valid.
        [NSFileManager.defaultManager removeItemAtURL:self.updatedClientURL error:nil];
        return destinationURL;
    } else {
        // no executable already present! Just move things into place.
        [NSFileManager.defaultManager moveItemAtURL:self.updatedClientURL toURL:destinationURL error:nil];
        return destinationURL;
    }
    
    return nil;
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

static la_ssize_t copy_data(struct archive *ar, struct archive *aw)
{
    la_ssize_t r;
    const void *buff;
    size_t size;
    la_int64_t offset;
    
    for (;;) {
        r = archive_read_data_block(ar, &buff, &size, &offset);
        if (r == ARCHIVE_EOF)
            return (ARCHIVE_OK);
        if (r < ARCHIVE_OK)
            return (r);
        r = archive_write_data_block(aw, buff, size, offset);
        if (r < ARCHIVE_OK) {
            fprintf(stderr, "%s\n", archive_error_string(aw));
            return (r);
        }
    }
}

void Patcher::ISelfPatch(const plFileName& file)
{
    struct archive *a;
    struct archive *ext;
    struct archive_entry *entry;
    int flags;
    la_ssize_t r;

    /* Select which attributes we want to restore. */
    flags = ARCHIVE_EXTRACT_TIME;
    flags |= ARCHIVE_EXTRACT_PERM;
    flags |= ARCHIVE_EXTRACT_ACL;
    flags |= ARCHIVE_EXTRACT_FFLAGS;
    
    a = archive_read_new();
    archive_read_support_format_all(a);
    archive_read_support_filter_all(a);
    ext = archive_write_disk_new();
    archive_write_disk_set_options(ext, flags);
    archive_write_disk_set_standard_lookup(ext);
    if ((r = archive_read_open_filename(a, file.GetFileName().c_str(), 10240)))
        exit(1);
    
    plFileSystem::Unlink(plManifest::PatcherExecutable());
    
    NSURL *tempDirectory = [NSFileManager.defaultManager URLForDirectory:NSItemReplacementDirectory inDomain:NSUserDomainMask appropriateForURL:[NSURL fileURLWithPath:NSFileManager.defaultManager.currentDirectoryPath] create:YES error:nil];
    NSURL *outputURL = [tempDirectory URLByAppendingPathComponent:[NSString stringWithSTString:plManifest::PatcherExecutable().GetFileName()]];
    [NSFileManager.defaultManager createDirectoryAtURL:outputURL withIntermediateDirectories:false attributes:nil error:nil];
    ST::string outputPath = [outputURL.path STString];
    
    for (;;) {
        r = archive_read_next_header(a, &entry);
        if (r == ARCHIVE_EOF)
            break;
        if (r < ARCHIVE_OK)
            fprintf(stderr, "%s\n", archive_error_string(a));
        if (r < ARCHIVE_WARN)
            exit(1);
        const char* currentFile = archive_entry_pathname(entry);
        auto fullOutputPath = outputPath + "/" + currentFile;
        archive_entry_set_pathname(entry, fullOutputPath.c_str());
        r = archive_write_header(ext, entry);
        if (r < ARCHIVE_OK)
            fprintf(stderr, "%s\n", archive_error_string(ext));
        else if (archive_entry_size(entry) > 0) {
            r = copy_data(a, ext);
            if (r < ARCHIVE_OK)
                fprintf(stderr, "%s\n", archive_error_string(ext));
            if (r < ARCHIVE_WARN)
                exit(1);
        }
        r = archive_write_finish_entry(ext);
        if (r < ARCHIVE_OK)
            fprintf(stderr, "%s\n", archive_error_string(ext));
        if (r < ARCHIVE_WARN)
            exit(1);
    }
    archive_read_close(a);
    archive_read_free(a);
    archive_write_close(ext);
    archive_write_free(ext);
    
    plFileSystem::Unlink(file);
    
    PLSPatcher* patcher = parent;
    parent.updatedClientURL = outputURL;
}

void Patcher::IOnPatchComplete(ENetError result, const ST::string& msg)
{
    [parent.networkPumpTimer invalidate];
    if (IS_NET_SUCCESS(result)) {
        PLSPatcher* patcher = parent;
        dispatch_async(dispatch_get_main_queue(), ^{
            [patcher.delegate patcherCompleted:patcher
                                  didSelfPatch:(patcher.updatedClientURL != nil)];
        });
    } else {
        NSString* msgString = [NSString stringWithSTString:msg];

        dispatch_async(dispatch_get_main_queue(), ^{
            [parent.delegate
                patcherCompletedWithError:parent
                                    error:[NSError errorWithDomain:@"PLSPatchErrors"
                                                              code:result
                                                          userInfo:@{
                                                              NSLocalizedFailureErrorKey : msgString
                                                          }]];
        });
    }
}

@end
