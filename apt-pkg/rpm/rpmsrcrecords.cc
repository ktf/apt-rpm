// -*- mode: c++; mode: fold -*-
// Description								/*{{{*/
// $Id: rpmsrcrecords.cc,v 1.9 2003/01/29 15:19:02 niemeyer Exp $
/* ######################################################################
   
   SRPM Records - Parser implementation for RPM style source indexes
      
   ##################################################################### 
 */
									/*}}}*/
// Include Files							/*{{{*/
#ifdef __GNUG__
#pragma implementation "apt-pkg/rpmsrcrecords.h"
#endif 

#include <config.h>

#ifdef HAVE_RPM

#include <assert.h>

#include "rpmsrcrecords.h"
#include "rpmhandler.h"

#include <apt-pkg/error.h>
#include <apt-pkg/strutl.h>
#include <apt-pkg/pkgcache.h>

#include <apti18n.h>

using namespace std;

// SrcRecordParser::rpmSrcRecordParser - Constructor			/*{{{*/
// ---------------------------------------------------------------------
/* */
rpmSrcRecordParser::rpmSrcRecordParser(string File,pkgIndexFile const *Index)
    : Parser(Index), Buffer(0), BufSize(0), BufUsed(0)
{
   struct stat Buf;
   if (stat(File.c_str(),&Buf) == 0 && S_ISDIR(Buf.st_mode))
      Handler = new RPMDirHandler(File);
   else if (flExtension(File) == "rpm")
      Handler = new RPMSingleFileHandler(File);
#ifdef APT_WITH_REPOMD
#ifdef WITH_SQLITE3
   else if (flExtension(File) == "sqlite")
      Handler = new RPMSqliteHandler(File);
#endif
   else if (flExtension(File) == "xml")
      Handler = new RPMRepomdHandler(File);
#endif
   else
      Handler = new RPMFileHandler(File);
}
									/*}}}*/
// SrcRecordParser::~rpmSrcRecordParser - Destructor			/*{{{*/
// ---------------------------------------------------------------------
/* */
rpmSrcRecordParser::~rpmSrcRecordParser()
{
   delete Handler;
   free(Buffer);
}
									/*}}}*/
// SrcRecordParser::Binaries - Return the binaries field		/*{{{*/
// ---------------------------------------------------------------------
/* This member parses the binaries field into a pair of class arrays and
   returns a list of strings representing all of the components of the
   binaries field. The returned array need not be freed and will be
   reused by the next Binaries function call. */
const char **rpmSrcRecordParser::Binaries()
{
   return NULL;

// WTF is this ?!? If we're looking for sources why would be interested
// in binaries? Maybe there's an inner Zen to this all but
// apt-cache showsrc seems to work without just fine so disabled for now...
#if 0
   int i = 0;
   char **bins;
   int type, count;
   assert(HeaderP != NULL);
   int rc = headerGetEntry(HeaderP, CRPMTAG_BINARY,
			   &type, (void**)&bins, &count);
   if (rc != 1)
       return NULL;
   for (i = 0; (unsigned)i < sizeof(StaticBinList)/sizeof(char*) && i < count;
        i++)
      StaticBinList[i] = bins[i];
   StaticBinList[i] = 0;
   return StaticBinList;
#endif
}
									/*}}}*/
// SrcRecordParser::Files - Return a list of files for this source	/*{{{*/
// ---------------------------------------------------------------------
/* This parses the list of files and returns it, each file is required to have
   a complete source package */
bool rpmSrcRecordParser::Files(vector<pkgSrcRecords::File> &List)
{
   List.clear();
   
   pkgSrcRecords::File F;

   // XXX FIXME: Ignoring the md5sum for source packages for now 
   //F.MD5Hash = Handler->MD5Sum();
   F.MD5Hash = "";
   F.Size = Handler->FileSize();
   F.Path = flCombine(Handler->Directory(), Handler->FileName());
   F.Type = "srpm";

   List.push_back(F);
   
   return true;
}
									/*}}}*/

bool rpmSrcRecordParser::Restart()
{
   Handler->Rewind();
   return true;
}

bool rpmSrcRecordParser::Step() 
{
   bool ret = Handler->Skip();
   // Repomd can have both binaries and sources, step over any binaries
   while (ret && ! Handler->IsSourceRpm()) {
      ret = Handler->Skip();
   }
   return ret;
}

bool rpmSrcRecordParser::Jump(off_t Off)
{
   return Handler->Jump(Off);
}

string rpmSrcRecordParser::Package() const
{
   return Handler->Name();
}

string rpmSrcRecordParser::Version() const
{
   return Handler->EVR();
}
    

// RecordParser::Maintainer - Return the maintainer email		/*{{{*/
// ---------------------------------------------------------------------
/* */
string rpmSrcRecordParser::Maintainer() const
{
   return Handler->Packager();
}

string rpmSrcRecordParser::Section() const
{
   return Handler->Group();
}

off_t rpmSrcRecordParser::Offset() 
{
    return Handler->Offset();
}

void rpmSrcRecordParser::BufCat(const char *text)
{
   if (text != NULL)
      BufCat(text, text+strlen(text));
}

void rpmSrcRecordParser::BufCat(const char *begin, const char *end)
{
   unsigned len = end - begin;
    
   if (BufUsed+len+1 >= BufSize)
   {
      BufSize += 512;
      char *tmp = (char*)realloc(Buffer, BufSize);
      if (tmp == NULL)
      {
	 _error->Errno("realloc", _("Could not allocate buffer for record text"));
	 return;
      }
      Buffer = tmp;
   }

   strncpy(Buffer+BufUsed, begin, len);
   BufUsed += len;
}

void rpmSrcRecordParser::BufCatTag(const char *tag, const char *value)
{
   BufCat(tag);
   BufCat(value);
}

void rpmSrcRecordParser::BufCatDep(Dependency *Dep)
{
   string buf;

   BufCat(Dep->Name.c_str());
   if (Dep->Version.empty() == false) 
   {
      BufCat(" ");
      switch (Dep->Op) {
	 case pkgCache::Dep::Less:
	    buf += "<";
	    break;
	 case pkgCache::Dep::LessEq:
	    buf += "<=";
	    break;
	 case pkgCache::Dep::Equals: 
	    buf += "=";
	    break;
	 case pkgCache::Dep::Greater:
	    buf += ">";
	    break;
	 case pkgCache::Dep::GreaterEq:
	    buf += ">=";
	    break;
      }

      BufCat(buf.c_str());
      BufCat(" ");
      BufCat(Dep->Version.c_str());
   }
}

void rpmSrcRecordParser::BufCatDescr(const char *descr)
{
   const char *begin = descr;

   while (*descr) 
   {
      if (*descr=='\n') 
      {
	 BufCat(" ");
	 BufCat(begin, descr+1);
	 begin = descr+1;
      }
      descr++;
   }
   BufCat(" ");
   BufCat(begin, descr);
   BufCat("\n");
}

// SrcRecordParser::AsStr - The record in raw text
// -----------------------------------------------
string rpmSrcRecordParser::AsStr() 
{
   char buf[32];

   BufUsed = 0;

   BufCatTag("Package: ", Handler->Name().c_str());

   BufCatTag("\nSection: ", Handler->Group().c_str());

   snprintf(buf, sizeof(buf), "%lu", Handler->InstalledSize());
   BufCatTag("\nInstalled Size: ", buf);

   BufCatTag("\nPackager: ", Handler->Packager().c_str());
   //BufCatTag("\nVendor: ", Handler->Vendor().c_str());

   BufCat("\nVersion: ");
   BufCat(Handler->EVR().c_str());

   vector<Dependency*> Deps, Conflicts;
   vector<Dependency*>::iterator I;
   bool start = true;

   Handler->PRCO(pkgCache::Dep::Depends, Deps);
   for (I = Deps.begin(); I != Deps.end(); I++) {
      if ((*I)->Type != pkgCache::Dep::Depends)
	 continue;
      if (start) {
	 BufCat("\nBuild-Depends: ");
	 start = false;
      } else {
	 BufCat(", ");
      }
      BufCatDep(*I);
   }

   // Doesn't do anything yet, build conflicts aren't recorded yet...
   Handler->PRCO(pkgCache::Dep::Conflicts, Conflicts);
   start = true;
   for (I = Conflicts.begin(); I != Conflicts.end(); I++) {
      if (start) {
	 BufCat("\nBuild-Conflicts: ");
	 start = false;
      } else {
	 BufCat(", ");
      }
      BufCatDep(*I);
   }

   BufCatTag("\nArchitecture: ", Handler->Arch().c_str());

   snprintf(buf, sizeof(buf), "%lu", Handler->FileSize());
   BufCatTag("\nSize: ", buf);

   BufCatTag("\nMD5Sum: ", Handler->MD5Sum().c_str());

   BufCatTag("\nFilename: ", Handler->FileName().c_str());

   BufCatTag("\nSummary: ", Handler->Summary().c_str());
   BufCat("\nDescription: ");
   BufCat("\n");
   BufCatDescr(Handler->Description().c_str());
   BufCat("\n");

   return string(Buffer, BufUsed);
}


// SrcRecordParser::BuildDepends - Return the Build-Depends information	/*{{{*/
// ---------------------------------------------------------------------
bool rpmSrcRecordParser::BuildDepends(vector<pkgSrcRecords::Parser::BuildDepRec> &BuildDeps,
				      bool ArchOnly)
{
   BuildDepRec rec;
   BuildDeps.clear();

   vector<Dependency*> Deps, Conflicts;
   Handler->PRCO(pkgCache::Dep::Depends, Deps);

   for (vector<Dependency*>::iterator I = Deps.begin(); I != Deps.end(); I++) {
      rec.Package = (*I)->Name;
      rec.Version = (*I)->Version;
      rec.Op = (*I)->Op;
      rec.Type = pkgSrcRecords::Parser::BuildDepend;
      BuildDeps.push_back(rec);
   }
      
   Handler->PRCO(pkgCache::Dep::Conflicts, Conflicts);

   for (vector<Dependency*>::iterator I = Conflicts.begin(); I != Conflicts.end(); I++) {
      rec.Package = (*I)->Name;
      rec.Version = (*I)->Version;
      rec.Op = (*I)->Op;
      rec.Type = pkgSrcRecords::Parser::BuildConflict;
      BuildDeps.push_back(rec);
   }
   return true;
}
									/*}}}*/
#endif /* HAVE_RPM */

// vim:sts=3:sw=3
