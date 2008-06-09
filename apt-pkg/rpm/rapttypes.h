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
typedef rpm_off_t raptOffset;
typedef rpm_off_t raptCallbackSize;
#else
#include <rpm/header.h>
typedef void * raptTagData;
typedef int_32 raptTagCount;
typedef int_32 raptTag;
typedef int_32 raptTagType;
typedef int_32 raptDepFlags;
typedef int_32 raptOffset;
typedef long unsigned int raptCallbackSize;
#endif

#endif /* PKGLIB_RAPTTYPES_H */
