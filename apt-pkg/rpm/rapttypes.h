#ifndef PKGLIB_RAPTTYPES_H
#define PKGLIB_RAPTTYPES_H

/*
 * Layer of insulation against differing types used in rpm versions.
 * C happily converts enum to int etc automatically, C++ doesn't...
 */

#ifdef HAVE_RPM_RPMTYPES_H
#include <rpm/rpmtypes.h>
#include <rpm/rpmds.h>
typedef rpm_data_t raptTagData;
typedef rpm_count_t raptTagCount;
typedef rpmTag raptTag;
typedef rpmTagType raptTagType;
typedef rpmsenseFlags raptDepFlags;
typedef rpm_loff_t raptOffset;
typedef rpm_loff_t raptCallbackSize;
typedef uint32_t raptInt;
typedef uint32_t raptDbOffset;
#define RAPT_FILENAMES RPMTAG_FILENAMES
#else
#include <rpm/header.h>
typedef void * raptTagData;
typedef int_32 raptTagCount;
typedef int_32 raptTag;
typedef int_32 raptTagType;
typedef int_32 raptDepFlags;
typedef int_32 raptOffset;
typedef long unsigned int raptCallbackSize;
typedef int_32 raptInt;
typedef uint_32 raptDbOffset;
#define RAPT_FILENAMES RPMTAG_OLDFILENAMES
#endif

#if RPM_VERSION >= 0x040100
#define raptInitIterator(a,b,c,d) rpmtsInitIterator(a,(rpmTag)b,c,d)
#else
#define raptInitIterator(a,b,c,d) rpmdbInitIterator(a,b,c,d)
#endif
#endif /* PKGLIB_RAPTTYPES_H */
