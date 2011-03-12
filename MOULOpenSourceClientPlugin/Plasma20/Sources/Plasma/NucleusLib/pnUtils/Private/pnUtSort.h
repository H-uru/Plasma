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

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/
/*****************************************************************************
*
*   $/Plasma20/Sources/Plasma/NucleusLib/pnUtils/Private/pnUtSort.h
*   
***/

#ifdef PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNUTILS_PRIVATE_PNUTSORT_H
#error "Header $/Plasma20/Sources/Plasma/NucleusLib/pnUtils/Private/pnUtSort.h included more than once"
#endif
#define PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNUTILS_PRIVATE_PNUTSORT_H


/****************************************************************************
*
*   QSORT
*
*   This version of QuickSort is similar to the one in the C runtime library,
*   but is implemented as a macro to allow more flexible usage.
*
*   With the C runtime library version, when data external to the sort array 
*   is needed to make sorting decisions, that data must be stored in file- or
*   global-scope variables. This creates thread safety problems which can
*   only be resolved through the use of synchronization objects. The version
*   of QuickSort provided here does not require function calls to make 
*   sorting decisions, so all data can be kept in stack variables.
*
*   The expression used for making comparisons allows the same return values
*   as the comparison function used by the C runtime library, and can in fact
*   be a function call to a comparison function that was originally designed
*   for use by the C runtime library.
*       >  0  if elem1 greater than elem2
*       =  0  if elem1 equivalent to elem2
*       <  0  if elem1 less than elem2
*
*   However, this implementation of QuickSort never requires a distinction
*   between the case where elem1 is less than elem2 and the case where elem1
*   is equivalent to elem2, so it is possible to use the following more
*   efficient return values:
*       >  0  if elem1 is greater than elem2
*       <= 0  if elem1 is less than or equivalent to elem2
*
***/

//===========================================================================
#define  QSORT(T, ptr, count, expr) {                                       \
                                                                            \
    /* Largest possible stack count required is 1 + log2(size) */           \
    T *      loStack[32];                                                   \
    T *      hiStack[32];                                                   \
    unsigned stackPos = 0;                                                  \
                                                                            \
    if ((count) >= 2) {                                                     \
        T * lo = (ptr);                                                     \
        T * hi = lo + (count);                                              \
        for (;;) {                                                          \
                                                                            \
            /* Pick a partitioning element */                               \
            T * mid = lo + (hi - lo) / 2;                                   \
                                                                            \
            /* Swap it to the beginning of the array */                     \
            SWAP(*mid, *lo);                                                \
                                                                            \
            /* Partition the array into three pieces, one consisting of */  \
            /* elements <= the partitioning element, one of elements    */  \
            /* equal to it, and one of elements >= to it.               */  \
            T * loPart = lo;                                                \
            T * hiPart = hi;                                                \
            for (;;) {                                                      \
                /* val(i) <= val(lo) for lo <= i <= loPart */               \
                /* val(i) >= val(lo) for hiPart <= i <= hi */               \
                                                                            \
                for (;;) {                                                  \
                    if (++loPart == hi)                                     \
                        break;                                              \
                    T const & elem1 = *loPart;                              \
                    T const & elem2 = *lo;                                  \
                    int result = (expr);                                    \
                    if (result > 0)                                         \
                        break;                                              \
                }                                                           \
                                                                            \
                for (;;) {                                                  \
                    if (--hiPart == lo)                                     \
                        break;                                              \
                    T const & elem1 = *lo;                                  \
                    T const & elem2 = *hiPart;                              \
                    int result = (expr);                                    \
                    if (result > 0)                                         \
                        break;                                              \
                }                                                           \
                                                                            \
                if (hiPart < loPart)                                        \
                    break;                                                  \
                                                                            \
                /* val(loPart) > val(lo) */                                 \
                /* val(hiPart) < val(lo) */                                 \
                                                                            \
                SWAP(*loPart, *hiPart);                                     \
                                                                            \
                /* val(loPart) < val(lo) */                                 \
                /* val(hiPart) > val(lo) */                                 \
            }                                                               \
                                                                            \
            /* val(i) <= val(lo) for lo <= i <= hiPart   */                 \
            /* val(i) == val(lo) for hiPart < i < loPart */                 \
            /* val(i) >= val(lo) for loPart <= i <= hi   */                 \
                                                                            \
            /* Put the partitioning element in place */                     \
            SWAP(*lo, *hiPart);                                             \
                                                                            \
            /* val(i) <= val(hiPart) for lo <= i < hiPart     */            \
            /* val(i) == val(lo)     for hiPart <= i < loPart */            \
            /* val(i) >= val(hiPart) for loPart <= i < hi     */            \
                                                                            \
            /* Sort the subarrays [lo, hiPart-1] and [loPart, hi].    */    \
            /* We sort the smaller one first to minimize stack usage. */    \
            if (hiPart - lo >= hi - loPart) {                               \
                if (lo + 1 < hiPart) {                                      \
                    /* Store the bigger subarray */                         \
                    loStack[stackPos] = lo;                                 \
                    hiStack[stackPos] = hiPart;                             \
                    ++stackPos;                                             \
                }                                                           \
                if (loPart + 1 < hi) {                                      \
                    /* Sort the smaller subarray */                         \
                    lo = loPart;                                            \
                    continue;                                               \
                }                                                           \
            }                                                               \
            else {                                                          \
                if (loPart + 1 < hi) {                                      \
                    /* Store the bigger subarray */                         \
                    loStack[stackPos] = loPart;                             \
                    hiStack[stackPos] = hi;                                 \
                    ++stackPos;                                             \
                }                                                           \
                if (lo + 1 < hiPart) {                                      \
                    /* Sort the smaller subarray */                         \
                    hi = hiPart;                                            \
                    continue;                                               \
                }                                                           \
            }                                                               \
                                                                            \
            /* Pop the next subarray off the stack */                       \
            if (stackPos--) {                                               \
                lo = loStack[stackPos];                                     \
                hi = hiStack[stackPos];                                     \
                continue;                                                   \
            }                                                               \
                                                                            \
            break;                                                          \
        }                                                                   \
    }                                                                       \
}


/****************************************************************************
*
*   BSEARCH
*
*   This macro binary searches a sorted array to find an existing entry or
*   the position where a TRACKED_NEW entry should be placed. It returns the index of
*   the first entry for which the expression is false (zero or negative), or
*   count if the expression is true (positive) for all entries.
*
*   Typically the expression will return:
*       >  0  if (sortKey >  elem)
*       <= 0  if (sortKey <= elem)
*
*   The final parameter to the macro is the address of a variable which is
*   filled with the resulting index.
*
***/

//===========================================================================
#define  BSEARCH(T, ptr, count, expr, addrOfIndex) {                        \
                                                                            \
    const T * low  = (ptr);                                                 \
    const T * high = (ptr) + (count);  /* first entry for which */          \
                                       /* expr is false         */          \
                                                                            \
    if (low != high)                                                        \
        for (;;) {                                                          \
            const T & elem = *(low + (high - low) / 2);                     \
            int result = (expr);                                            \
            if (result > 0) {                                               \
                if (&elem == low)                                           \
                    break;                                                  \
                low = &elem;                                                \
            }                                                               \
            else {                                                          \
                high = &elem;                                               \
                if (&elem == low)                                           \
                    break;                                                  \
            }                                                               \
        }                                                                   \
                                                                            \
    *(addrOfIndex) = high - (ptr);                                          \
                                                                            \
}
