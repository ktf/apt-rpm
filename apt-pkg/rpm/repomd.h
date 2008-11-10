// CNC:2002-07-03

#ifndef PKGLIB_REPOMD_H
#define PKBLIB_REPOMD_H

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
   
   struct RepoFile {
      string Path;
      string RealPath;
      string Type;
      string TimeStamp;
   };

   map<string,RepoFile> RepoFiles;

   public:   

   virtual bool IsAuthenticated() const { return false; }
   virtual bool ParseRelease(string File);
   virtual string FindURI(string DataType);
   virtual string GetComprMethod(string URI);
   
   repomdRepository(string URI,string Dist, const pkgSourceList::Vendor *Vendor,
		 string RootURI)
      : pkgRepository(URI, Dist, Vendor, RootURI) 
   {
      // repomd always has a "release" file
      GotRelease = true;
   }

   virtual ~repomdRepository() {}
};

#endif /* APT_WITH_REPOMD */

#endif

// vim:sts=3:sw=3
