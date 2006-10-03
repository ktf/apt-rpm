// CNC:2002-07-03

#ifndef PKGLIB_REPOMD_H
#define PKBLIB_REPOMD_H

#ifdef __GNUG__
#pragma interface "apt-pkg/repomd.h"
#endif

#include <apt-pkg/aptconf.h>
#include <apt-pkg/repository.h>

#ifdef APT_WITH_REPOMD

#include <libxml/parser.h>
#include <libxml/tree.h>

using namespace std;

class repomdRepository : public pkgRepository
{
   protected:

   xmlDocPtr RepoMD;
   xmlNode *Root;

   public:   

   virtual bool IsAuthenticated() const { return false; };
   virtual bool ParseRelease(string File);
   
   repomdRepository(string URI,string Dist, const pkgSourceList::Vendor *Vendor,
		 string RootURI)
      : pkgRepository(URI, Dist, Vendor, RootURI) 
   {
      ComprMethod = "gz";
      // repomd always has a "release" file
      GotRelease = true;
   };

   virtual ~repomdRepository() {};
};

#endif /* APT_WITH_REPOMD */

#endif

// vim:sts=3:sw=3
