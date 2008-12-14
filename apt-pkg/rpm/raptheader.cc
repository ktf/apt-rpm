#include <config.h>
#include <iostream>

#include <rpm/rpmlegacy.h>
#include <rpm/rpmtag.h>
#include "raptheader.h"

using namespace std;

/*
 * Beginnings of C++ wrapper for RPM header and data.
 * Having a rpm version independent ways to access header data makes
 * things a whole lot saner in other places while permitting newer
 * rpm interfaces to be used where supported.
 *
 * Lots to do still:
 * - actually implement "raw" string mode + handle i18n strings correctly
 * - we'll need putTag() method too for genpkglist
 * - for older rpm versions we'll need to use one of rpmHeaderGetEntry(),
 *   headerGetEntry() or headerGetRawEntry() depending on tag and content
 * - we'll want to eventually replace rpmhandler's HeaderP with raptHeader(),
 *   add constructors for reading in from files, db iterators & all etc...
 */

raptHeader::raptHeader(Header h)
{
   Hdr = headerLink(h);
}

raptHeader::~raptHeader()
{
   headerUnlink(Hdr);
}

bool raptHeader::hasTag(raptTag tag)
{
   return headerIsEntry(Hdr, tag);
}

#ifdef HAVE_RPM_RPMTD_H

// use MINMEM to avoid extra copy from header if possible
#define HGDFL (headerGetFlags)(HEADERGET_EXT | HEADERGET_MINMEM)
#define HGRAW (headerGetFlags)(HGDFL | HEADERGET_RAW)

bool raptHeader::getTag(raptTag tag, raptInt &data)
{
   struct rpmtd_s td;
   bool ret = false;
   if (headerGet(Hdr, tag, &td, HGDFL)) {
      if (rpmtdType(&td) == RPM_INT32_TYPE && rpmtdCount(&td) == 1) {
	 data = *rpmtdGetUint32(&td);
	 ret = true;
      }
      rpmtdFreeData(&td);
   }
   return ret;
}

bool raptHeader::getTag(raptTag tag, string &data, bool raw)
{
   struct rpmtd_s td;
   bool ret = false;
   headerGetFlags flags = raw ? HGRAW : HGDFL;

   if (headerGet(Hdr, tag, &td, flags)) {
      if (rpmtdType(&td) == RPM_STRING_TYPE) {
	 data = rpmtdGetString(&td);
	 ret = true;
      }
      rpmtdFreeData(&td);
   }
   return ret;
}

bool raptHeader::getTag(raptTag tag, vector<string> &data, bool raw)
{
   struct rpmtd_s td;
   bool ret = false;
   headerGetFlags flags = raw ? HGRAW : HGDFL;

   if (headerGet(Hdr, tag, &td, flags)) {
      if (rpmtdType(&td) == RPM_STRING_ARRAY_TYPE) {
	 const char *str;
	 while ((str = rpmtdNextString(&td))) {
	    data.push_back(str);
	 }
	 ret = true;
      }
      rpmtdFreeData(&td);
   }
   return ret;
}

bool raptHeader::getTag(raptTag tag, vector<raptInt> &data)
{
   struct rpmtd_s td;
   bool ret = false;
   if (headerGet(Hdr, tag, &td, HGDFL)) {
      if (rpmtdType(&td) == RPM_INT32_TYPE) {
	 raptInt *num;
	 while ((num = rpmtdNextUint32(&td))) {
	    data.push_back(*num);
	 }
	 ret = true;
      }
      rpmtdFreeData(&td);
   }
   return ret;
}
#else
bool raptHeader::getTag(raptTag tag, string &data, bool raw)
{
   bool ret = false;
   void *val = NULL;
   raptTagCount count = 0;
   raptTagType type = RPM_NULL_TYPE;
   if (headerGetEntry(Hdr, tag, &type, (void **) &val, &count)) {
      if (type == RPM_STRING_TYPE && count == 1 && val != NULL) {
	 data = (const char *)val;
	 ret = true;
      }
      headerFreeData(val, type);
   }
   return ret;
}

bool raptHeader::getTag(raptTag tag, raptInt &data)
{
   bool ret = false;
   void *val = NULL;
   raptTagCount count = 0;
   raptTagType type = RPM_NULL_TYPE;
   if (headerGetEntry(Hdr, tag, &type, (void **) &val, &count)) {
      if (type == RPM_INT32_TYPE && count == 1 && val != NULL) {
	 raptInt *hdata = (raptInt *)val;
	 data = *hdata;
	 ret = true;
      }
      headerFreeData(val, type);
   }
   return ret;
}

bool raptHeader::getTag(raptTag tag, vector<string> &data, bool raw)
{
   bool ret = false;
   void *val = NULL;
   raptTagCount count = 0;
   raptTagType type = RPM_NULL_TYPE;
   if (headerGetEntry(Hdr, tag, &type, (void **) &val, &count)) {
      if (type == RPM_STRING_ARRAY_TYPE && count > 0 && val != NULL) {
	 const char **hdata = (const char **)val;
	 for (int i = 0; i < count; i++) {
	    data.push_back(hdata[i]);
	 }
	 ret = true;
      }
      headerFreeData(val, type);
   }
   return ret;
}

bool raptHeader::getTag(raptTag tag, vector<raptInt> &data)
{
   bool ret = false;
   void *val = NULL;
   raptTagCount count = 0;
   raptTagType type = RPM_NULL_TYPE;
   if (headerGetEntry(Hdr, tag, &type, (void **) &val, &count)) {
      if (type == RPM_STRING_ARRAY_TYPE && count > 0 && val != NULL) {
	 raptInt *hdata = (raptInt *)val;
	 for (int i = 0; i < count; i++) {
	    data.push_back(hdata[i]);
	 }
	 ret = true;
      }
      headerFreeData(val, type);
   }
   return ret;
}
#endif

// vim:sts=3:sw=3
