#ifndef PKGLIB_RAPTHEADER_H
#define PKGLIB_RAPTHEADER_H

#include <vector>
#include <string>

#include "rapttypes.h"

#ifndef HAVE_RPM_RPMTAG_H
#include <rpm/rpmlib.h>
#else
#include <rpm/header.h>
#include <rpm/rpmtag.h>
#endif

using std::vector;
using std::string;

class raptHeader
{
   private:
   Header Hdr;

   public:
   bool hasTag(raptTag tag);
   bool getTag(raptTag tag, raptInt &data);
   bool getTag(raptTag tag, string &data, bool raw = false);
   bool getTag(raptTag tag, vector<raptInt> &data);
   bool getTag(raptTag tag, vector<string> &data, bool raw = false);
   string format(const string fmt);

   raptHeader(Header hdr);
   ~raptHeader();
};

#endif /* _PKGLIB_RAPTHEADER_H */

// vim:sts=3:sw=3
