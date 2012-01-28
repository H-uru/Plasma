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
//////////////////////////////////////////////////////////////////////////////
//
//  plBSDiffBuffer - A utility class for writing and applying a difference
//                  buffer--i.e. a buffer containing a series of modifications
//                  that will modify an old data buffer to match a new one. 
//                  It's a useful utility class when doing binary file
//                  patching, for example, as you can write out the changes
//                  to this class, get back a data buffer suitable for writing,
//                  then use this class again later to reconstruct the new buffer.
//
//////////////////////////////////////////////////////////////////////////////

#include "HeadSpin.h"
#include "plBSDiffBuffer.h"

#include "hsStream.h"

#define plBSDiffBuffer_MIN(x,y) (((x)<(y)) ? (x) : (y))

//////////////////////////////////////////////////////////////////////////////
//// Constructor/Destructors /////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//// Creation Constructor ////////////////////////////////////////////////////
//  Use this constructor when creating a new diff buffer. Pass in the length
//  of the final new buffer. You don't need to pass in the length of the old
//  buffer, but if you do, it'll help this class do some internal space 
//  optimization.

plBSDiffBuffer::plBSDiffBuffer( uint32_t newLength, uint32_t oldLength )
: fNewLength( newLength )
, fPatchLength( 0 )
{
    fPatchBuffer = nil;
    fWriting = true;
}

//// Application Constructor /////////////////////////////////////////////////
//  Use this constructor when taking a diff buffer and applying it to an old
//  buffer. Pass in a pointer to the diff buffer and its length. The buffer
//  will be copied, so you don't need to keep it around after you construct
//  this object.

plBSDiffBuffer::plBSDiffBuffer( void *buffer, uint32_t length )
: fNewLength( 0 )
, fPatchLength( 0 )
, fPatchBuffer( nil )
, fWriting( false )
{
    if (!buffer || length < 32)
        hsAssert(false, "Corrupt Patch Buffer!");

    if((fPatchBuffer=new unsigned char[length+1])!=nil)
    {
        fPatchLength = length;
        memcpy(fPatchBuffer, buffer, fPatchLength);
    }
}

plBSDiffBuffer::~plBSDiffBuffer()
{
    if ( fPatchBuffer )
        delete[] fPatchBuffer;
}


//////////////////////////////////////////////////////////////////////////////
//// Creation/Write Functions ////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//// Diff /////////////////////////////////////////////////////////////////////
//  Diff() creates the diff buffer from the new and old. 
uint32_t plBSDiffBuffer::Diff( uint32_t oldLength, void *oldBuffer, uint32_t newLength, void *newBuffer )
{
    hsAssert( fWriting, "Trying to Add() to a difference buffer that's reading" );

    int32_t *I,*V;

    int32_t scan,pos,len;
    int32_t lastscan,lastpos,lastoffset;
    int32_t oldscore,scsc;

    int32_t s,Sf,lenf,Sb,lenb;
    int32_t overlap,Ss,lens;
    int32_t i;

    uint32_t cblen,dblen,eblen;
    unsigned char *oldbuf, *newbuf;
    unsigned char *cb,*db,*eb;

    unsigned char header[32];

    if (oldBuffer == nil ||
            oldLength < 0 ||
            newBuffer == nil ||
            newLength < 0)
    {
        return (-1);
    }

    oldbuf = (unsigned char *)oldBuffer;
    newbuf = (unsigned char *)newBuffer;

    if(((I=new int32_t[oldLength+1])==nil) ||
        ((V=new int32_t[oldLength+1])==nil))
    {
        delete[] I;
        delete[] V;
        return fPatchLength;
    }

    IQSuffixSort(I,V,oldbuf,oldLength);

    delete[] V;

    /*
     *  These could probably be smaller, especially cb.
     */
    if(((cb=new unsigned char[newLength+1])==nil) ||
       ((db=new unsigned char[newLength+1])==nil) ||
       ((eb=new unsigned char[newLength+1])==nil))
    {
        delete[] I;
        delete[] cb;
        delete[] db;
        delete[] eb;
        return fPatchLength;
    }
    cblen=0;
    dblen=0;
    eblen=0;

    /* Header is
        0   8    "BSDIFF40"
        8   8   length of bzip2ed ctrl block
        16  8   length of bzip2ed diff block
        24  8   length of new file */
    /* File is
        0   32  Header
        32  ??  Bzip2ed ctrl block
        ??  ??  Bzip2ed diff block
        ??  ??  Bzip2ed extra block */
    memcpy(header,"BSDIFF40",8);
    memset(header+8,0,24);

    scan=0;len=0;
    lastscan=0;lastpos=0;lastoffset=0;
    while(scan<newLength) {
        oldscore=0;

        for(scsc=scan+=len;scan<newLength;scan++) {
            len=ISearch(I,oldbuf,oldLength,newbuf+scan,newLength-scan,
                    0,oldLength,&pos);

            for(;scsc<scan+len;scsc++)
            if((scsc+lastoffset<oldLength) &&
                (oldbuf[scsc+lastoffset] == newbuf[scsc]))
                oldscore++;

            if(((len==oldscore) && (len!=0)) || 
                (len>oldscore+8)) break;

            if((scan+lastoffset<oldLength) &&
                (oldbuf[scan+lastoffset] == newbuf[scan]))
                oldscore--;
        };

        if((len!=oldscore) || (scan==newLength)) {
            s=0;Sf=0;lenf=0;
            for(i=0;(lastscan+i<scan)&&(lastpos+i<oldLength);) {
                if(oldbuf[lastpos+i]==newbuf[lastscan+i]) s++;
                i++;
                if(s*2-i>Sf*2-lenf) { Sf=s; lenf=i; };
            };

            lenb=0;
            if(scan<newLength) {
                s=0;Sb=0;
                for(i=1;(scan>=lastscan+i)&&(pos>=i);i++) {
                    if(oldbuf[pos-i]==newbuf[scan-i]) s++;
                    if(s*2-i>Sb*2-lenb) { Sb=s; lenb=i; };
                };
            };

            if(lastscan+lenf>scan-lenb) {
                overlap=(lastscan+lenf)-(scan-lenb);
                s=0;Ss=0;lens=0;
                for(i=0;i<overlap;i++) {
                    if(newbuf[lastscan+lenf-overlap+i]==
                       oldbuf[lastpos+lenf-overlap+i]) s++;
                    if(newbuf[scan-lenb+i]==
                       oldbuf[pos-lenb+i]) s--;
                    if(s>Ss) { Ss=s; lens=i+1; };
                };

                lenf+=lens-overlap;
                lenb-=lens;
            };

            for(i=0;i<lenf;i++)
                db[dblen+i]=newbuf[lastscan+i]-oldbuf[lastpos+i];
            for(i=0;i<(scan-lenb)-(lastscan+lenf);i++)
                eb[eblen+i]=newbuf[lastscan+lenf+i];

            dblen+=lenf;
            eblen+=(scan-lenb)-(lastscan+lenf);

            IWriteUnsignedint8_t(lenf,cb+cblen);
            IWriteUnsignedint8_t((scan-lenb)-(lastscan+lenf),cb+cblen+8);
            IWriteUnsignedint8_t((pos-lenb)-(lastpos+lenf),cb+cblen+16);
            cblen+=24;

            lastscan=scan-lenb;
            lastpos=pos-lenb;
            lastoffset=pos-scan;
        };
    };

    IWriteUnsignedint8_t(cblen,header+8);
    IWriteUnsignedint8_t(dblen,header+16);
    IWriteUnsignedint8_t(newLength,header+24);

    fPatchLength = 32 + cblen + dblen + eblen;
    fPatchBuffer = nil;
    if((fPatchBuffer=new unsigned char[fPatchLength])!=nil)
    {
        memcpy(fPatchBuffer,header,32);
        memcpy(fPatchBuffer+32,cb,cblen);
        memcpy(fPatchBuffer+32+cblen,db,dblen);
        memcpy(fPatchBuffer+32+cblen+dblen,eb,eblen);
    }
    else
    {
        fPatchLength = 0;
    }

    delete[] cb;
    delete[] db;
    delete[] eb;
    delete[] I;

    return fPatchLength;
}

//// GetBuffer ///////////////////////////////////////////////////////////////
//  GetBuffer() will copy the diff stream into a new buffer and return it.
//  You are responsible for freeing the buffer. Call this once you're done 
//  adding ops and want the raw data to write out somewhere. Note: this
//  function will rewind the diff stream, so once you call it, you can't do
//  anything else on the object.

void plBSDiffBuffer::GetBuffer( uint32_t &length, void *&bufferPtr )
{
    hsAssert( fWriting, "Trying to GetBuffer() on a difference buffer that's reading" );

    if (fPatchBuffer && fPatchLength)
    {
        length = fPatchLength;
        if ((bufferPtr = (void *)new unsigned char[ length ]))
            memcpy(bufferPtr, fPatchBuffer, length);
    }
    else
    {
        length = 0;
        bufferPtr = nil;
    }
}

//////////////////////////////////////////////////////////////////////////////
//// Application Functions ///////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

#define hsAssertAndBreak( cond, msg ) { if( cond ) { hsAssert( false, msg ); break; } }

// Patch() will take this diff buffer and apply it to the given old buffer,
// allocating and producing a new buffer. You are responsible for freeing the new buffer.
uint32_t plBSDiffBuffer::Patch( uint32_t oldLength, void *oldBuffer, uint32_t &newLength, void *&newBuffer )
{
    hsAssert( !fWriting, "Trying to Patch() a difference buffer that's writing" );

    unsigned char *ctrlpipe, *ctrlend;
    unsigned char *diffpipe, *diffend;
    unsigned char *extrapipe;
    uint32_t ctrllen,datalen;
    int version=0;
    unsigned char *newpos, *newend;
    unsigned char *oldpos, *oldend;
    unsigned char *patchend;
    uint32_t ctrl[3];
    uint32_t i;

    if (oldBuffer == nil ||
        oldLength < 0 ||
        newBuffer != nil ||
        newLength != 0 ||
        fPatchBuffer == nil ||
        fPatchLength < 32)
    {
        return (-1);
    }

    patchend = fPatchBuffer + fPatchLength;
    /*
      Ok, this is going to be messy.  There are two different patch
    formats which we need to support.

      The old format (pre-4.0) is:
        0   8   "QSUFDIFF" or "BSDIFF30"
        8   8   X
        16  8   Y
        24  8   sizeof(newfile)
        32  X   bzip2(control block)
        32+X    Y   bzip2(data block)
    with control block a set of pairs (x,y) meaning "seek forward
    in oldfile by y bytes, and add the next x bytes to x bytes from
    the data block".

      The new format (4.0) is:
        0   8   "BSDIFF40"
        8   8   X
        16  8   Y
        24  8   sizeof(newfile)
        32  X   bzip2(control block)
        32+X    Y   bzip2(diff block)
        32+X+Y  ??? bzip2(extra block)
    with control block a set of triples (x,y,z) meaning "add x bytes
    from oldfile to x bytes from the diff block; copy y bytes from the
    extra block; seek forwards in oldfile by z bytes".
    */

    if(memcmp(fPatchBuffer,"QSUFDIFF",8)==0) version=1;
    if(memcmp(fPatchBuffer,"BSDIFF30",8)==0) version=1;
    if(memcmp(fPatchBuffer,"BSDIFF40",8)==0) version=2;

    if(!version) return (-1);

    ctrllen=IReadUnsignedint8_t(fPatchBuffer+8);
    datalen=IReadUnsignedint8_t(fPatchBuffer+16);
    newLength=IReadUnsignedint8_t(fPatchBuffer+24);
    if((ctrllen<0) || (datalen<0) || (newLength<0) ||
        ((version==1) && (32+ctrllen+datalen!=fPatchLength)))
        return (-1);

    ctrlpipe=fPatchBuffer+32;
    ctrlend =ctrlpipe+ctrllen; 
    diffpipe=ctrlend;
    diffend = diffpipe + datalen;
    if(version==2) {
        extrapipe=diffpipe+datalen;
    };

    if((newBuffer=(void *)new unsigned char[newLength+1])==nil) return(-1);
    newpos = (unsigned char *)newBuffer;
    newend = (unsigned char *)newBuffer + newLength;
    oldpos = (unsigned char *)oldBuffer;
    oldend = (unsigned char *)oldBuffer + oldLength;
    while(newpos<newend) {
        for(i=0;i<=version;i++) {
            if(ctrlend-ctrlpipe < 8) return (-1);
            ctrl[i]=IReadUnsignedint8_t(ctrlpipe);
            ctrlpipe += 8;
        };

        if(version==1) oldpos+=ctrl[1];

        ISafeMemcpy(newpos, diffpipe, ctrl[0], newend, diffend);
        diffpipe += ctrl[0];

        for(i=0;i<ctrl[0];i++)
            if(oldpos<oldend)
            {
                *newpos += *oldpos;
                newpos++;
                oldpos++;
            }

        if(version==2) {
            ISafeMemcpy(newpos,extrapipe,ctrl[1],newend,patchend);
            extrapipe += ctrl[1];
            newpos+=ctrl[1];
            oldpos+=ctrl[2];
        };
    };

    if((ctrlpipe != ctrlend) ||
       (diffpipe != diffend) ||
       ((version==2) && (extrapipe != patchend)))
    {
        return (-1);
    }

    if(newpos != newend)
    {
        delete[] (unsigned char*)newBuffer;
        newLength = 0;
    }

    return newLength;
}

//////////////////////////////////////////////////////////////////////////////
//// Private Helper Functions ///////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

// Reads in an 8-uint8_t unsigned integer least significant uint8_t first and returns a
// uint32_t. We'll ignore the top four bytes.
uint32_t plBSDiffBuffer::IReadUnsignedint8_t(unsigned char *buf)
{
    uint32_t y;

    y=buf[7]&0x7F;
    y=y*256;y+=buf[6];
    y=y*256;y+=buf[5];
    y=y*256;y+=buf[4];
    y=y*256;y+=buf[3];
    y=y*256;y+=buf[2];
    y=y*256;y+=buf[1];
    y=y*256;y+=buf[0];

    // casting added so compiler doesn't warn about negating an unsigned value
    int32_t signedY = (int32_t)y;
    if(buf[7]&0x80) signedY=-signedY;
    y = (uint32_t)signedY;

    return y;
}

void plBSDiffBuffer::IWriteUnsignedint8_t(uint32_t x,unsigned char *buf)
{
    uint32_t y;
    int32_t signedY, signedX = (int32_t)x;

    // casting added so compiler doesn't warn about negating an unsigned value
    if(x<0) signedY=-signedX; else signedY=signedX;
    y = (uint32_t)signedY;

    buf[0]=(unsigned char)(y%256);y-=buf[0];
    y=y/256;buf[1]=(unsigned char)(y%256);y-=buf[1];
    y=y/256;buf[2]=(unsigned char)(y%256);y-=buf[2];
    y=y/256;buf[3]=(unsigned char)(y%256);y-=buf[3];
    y=y/256;buf[4]=(unsigned char)(y%256);y-=buf[4];
    y=y/256;buf[5]=(unsigned char)(y%256);y-=buf[5];
    y=y/256;buf[6]=(unsigned char)(y%256);y-=buf[6];
    y=y/256;buf[7]=(unsigned char)(y%256);

    if(x<0) buf[7]|=0x80;
}

/*
 *  This function will ensure that no boundary errors occur. It also increments *src
 *  by nBytes when done.
 */
void plBSDiffBuffer::ISafeMemcpy(unsigned char *dest, unsigned char *src, size_t nBytes,
                                 unsigned char *destend, unsigned char *srcend)
{
    if((destend - dest < nBytes) ||
       (srcend - src < nBytes))
    {
        hsAssert(false,"Corrupt patch\n");
    }
    memcpy(dest,src,nBytes);
}

void plBSDiffBuffer::ISplit(int32_t *I,int32_t *V,uint32_t start,uint32_t len,uint32_t h)
{
    uint32_t i,j,k,x,tmp,jj,kk;

    if(len<16) {
        for(k=start;k<start+len;k+=j) {
            j=1;x=V[I[k]+h];
            for(i=1;k+i<start+len;i++) {
                if(V[I[k+i]+h]<x) {
                    x=V[I[k+i]+h];
                    j=0;
                };
                if(V[I[k+i]+h]==x) {
                    tmp=I[k+j];I[k+j]=I[k+i];I[k+i]=tmp;
                    j++;
                };
            };
            for(i=0;i<j;i++) V[I[k+i]]=k+j-1;
            if(j==1) I[k]=-1;
        };
        return;
    };

    x=V[I[start+len/2]+h];
    jj=0;kk=0;
    for(i=start;i<start+len;i++) {
        if(V[I[i]+h]<x) jj++;
        if(V[I[i]+h]==x) kk++;
    };
    jj+=start;kk+=jj;

    i=start;j=0;k=0;
    while(i<jj) {
        if(V[I[i]+h]<x) {
            i++;
        } else if(V[I[i]+h]==x) {
            tmp=I[i];I[i]=I[jj+j];I[jj+j]=tmp;
            j++;
        } else {
            tmp=I[i];I[i]=I[kk+k];I[kk+k]=tmp;
            k++;
        };
    };

    while(jj+j<kk) {
        if(V[I[jj+j]+h]==x) {
            j++;
        } else {
            tmp=I[jj+j];I[jj+j]=I[kk+k];I[kk+k]=tmp;
            k++;
        };
    };

    if(jj>start) ISplit(I,V,start,jj-start,h);

    for(i=0;i<kk-jj;i++) V[I[jj+i]]=kk-1;
    if(jj==kk-1) I[jj]=-1;

    if(start+len>kk) ISplit(I,V,kk,start+len-kk,h);
}

void plBSDiffBuffer::IQSuffixSort(int32_t *I,int32_t *V,unsigned char *old,uint32_t oldsize)
{
    uint32_t buckets[256];
    uint32_t i,h,len;

    for(i=0;i<256;i++) buckets[i]=0;
    for(i=0;i<oldsize;i++) buckets[old[i]]++;
    for(i=1;i<256;i++) buckets[i]+=buckets[i-1];
    for(i=255;i>0;i--) buckets[i]=buckets[i-1];
    buckets[0]=0;

    for(i=0;i<oldsize;i++) I[++buckets[old[i]]]=i;
    I[0]=oldsize;
    for(i=0;i<oldsize;i++) V[i]=buckets[old[i]];
    V[oldsize]=0;
    for(i=1;i<256;i++) if(buckets[i]==buckets[i-1]+1) I[buckets[i]]=-1;
    I[0]=-1;

    // oldsize converted to int32_t to stop warning about negating unsigned numbers
    for(h=1;I[0]!=-(int32_t)(oldsize+1);h+=h) {
        len=0;
        for(i=0;i<oldsize+1;) {
            if(I[i]<0) {
                len-=I[i];
                i-=I[i];
            } else {
                // len converted to int32_t to stop the warning about negating an unsigned number
                if(len) I[i-len]=-(int32_t)len;
                len=V[I[i]]+1-i;
                ISplit(I,V,i,len,h);
                i+=len;
                len=0;
            };
        };
        // len converted to int32_t to stop the warning about negating an unsigned number
        if(len) I[i-len]=-(int32_t)len;
    };

    for(i=0;i<oldsize+1;i++) I[V[i]]=i;
}

uint32_t plBSDiffBuffer::IMatchLen( unsigned char *oldBuffer, uint32_t oldLength,
                                  unsigned char *newBuffer, uint32_t newLength)
{
    uint32_t i;

    for(i=0;(i<oldLength)&&(i<newLength);i++)
        if(oldBuffer[i]!=newBuffer[i]) break;

    return i;
}

uint32_t plBSDiffBuffer::ISearch( int32_t *I,
                                unsigned char *oldBuffer, uint32_t oldLength,
                                unsigned char *newBuffer, uint32_t newLength,
                                uint32_t st, uint32_t en, int32_t *pos)
{
    uint32_t x,y;

    if(en-st<2) {
        x=IMatchLen(oldBuffer+I[st],oldLength-I[st],newBuffer,newLength);
        y=IMatchLen(oldBuffer+I[en],oldLength-I[en],newBuffer,newLength);

        if(x>y) {
            *pos=I[st];
            return x;
        } else {
            *pos=I[en];
            return y;
        }
    };

    x=st+(en-st)/2;
    if(memcmp(oldBuffer+I[x],newBuffer,plBSDiffBuffer_MIN(oldLength-I[x],newLength))<0) {
        return ISearch(I,oldBuffer,oldLength,newBuffer,newLength,x,en,pos);
    } else {
        return ISearch(I,oldBuffer,oldLength,newBuffer,newLength,st,x,pos);
    };
}
