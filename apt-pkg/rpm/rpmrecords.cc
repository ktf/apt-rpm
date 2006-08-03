// -*- mode: c++; mode: fold -*-
// Description								/*{{{*/
/* ######################################################################
   
   RPM Package Records - Parser for RPM package records
     
   ##################################################################### 
 */
									/*}}}*/
// Include Files							/*{{{*/
#ifdef __GNUG__
#pragma implementation "apt-pkg/rpmrecords.h"
#endif

#include <config.h>

#ifdef HAVE_RPM

#include <assert.h>

#include <apt-pkg/rpmrecords.h>
#include <apt-pkg/error.h>
#include <apt-pkg/rpmhandler.h>
#include <apt-pkg/rpmsystem.h>

#include <apti18n.h>

using namespace std;

// RecordParser::rpmRecordParser - Constructor				/*{{{*/
// ---------------------------------------------------------------------
/* */
rpmRecordParser::rpmRecordParser(string File, pkgCache &Cache)
   : Handler(0), Buffer(0), BufSize(0), BufUsed(0)
{
   if (File == RPMDBHandler::DataPath(false)) {
      IsDatabase = true;
      Handler = rpmSys.GetDBHandler();
   } else {
      IsDatabase = false;
      struct stat Buf;
      if (stat(File.c_str(),&Buf) == 0 && S_ISDIR(Buf.st_mode))
	 Handler = new RPMDirHandler(File);
      else if (flExtension(File) == "rpm")
	 Handler = new RPMSingleFileHandler(File);
#ifdef WITH_REPOMD
      else if (flExtension(File) == "xml")
	 Handler = new RPMRepomdHandler(File);
#endif
      else
	 Handler = new RPMFileHandler(File);
   }
}
									/*}}}*/
// RecordParser::~rpmRecordParser - Destructor				/*{{{*/
// ---------------------------------------------------------------------
/* */
rpmRecordParser::~rpmRecordParser()
{
   // Can't use Handler->IsDatabase here, since the RPMDBHandler
   // could already have been destroyed.
   if (IsDatabase == false)
      delete Handler;
   free(Buffer);
}
									/*}}}*/
// RecordParser::Jump - Jump to a specific record			/*{{{*/
// ---------------------------------------------------------------------
/* */
bool rpmRecordParser::Jump(pkgCache::VerFileIterator const &Ver)
{
   return Handler->Jump(Ver->Offset);
}
									/*}}}*/
// RecordParser::FileName - Return the archive filename on the site	/*{{{*/
// ---------------------------------------------------------------------
/* */
string rpmRecordParser::FileName()
{
   string Dir = Handler->Directory();
   if (Dir.empty() == true)
      return Handler->FileName();
   return flCombine(Dir, Handler->FileName());
}
									/*}}}*/
// RecordParser::Name - Return the package name				/*{{{*/
// ---------------------------------------------------------------------
/* */
string rpmRecordParser::Name()
{
   return Handler->Name();
}
									/*}}}*/
// RecordParser::MD5Hash - Return the archive hash			/*{{{*/
// ---------------------------------------------------------------------
/* */
string rpmRecordParser::MD5Hash()
{
   return Handler->MD5Sum();
}
									/*}}}*/
string rpmRecordParser::SHA1Hash()
{
   return Handler->SHA1Sum();
}

// RecordParser::Maintainer - Return the maintainer email		/*{{{*/
// ---------------------------------------------------------------------
/* */
string rpmRecordParser::Maintainer()
{
   return Handler->Packager();
}
									/*}}}*/
// RecordParser::ShortDesc - Return a 1 line description		/*{{{*/
// ---------------------------------------------------------------------
/* */
string rpmRecordParser::ShortDesc()
{
   return Handler->Summary();
}
									/*}}}*/
// RecordParser::LongDesc - Return a longer description			/*{{{*/
// ---------------------------------------------------------------------
/* */
string rpmRecordParser::LongDesc()
{
   return Handler->Description();
}
									/*}}}*/
// RecordParser::SourcePkg - Return the source package name if any	/*{{{*/
// ---------------------------------------------------------------------
/* */
string rpmRecordParser::SourcePkg()
{
   // This must be the *package* name, not the *file* name. We have no
   // current way to extract it safely from the file name.

// A wild guess, hopefully covering most cases:
// Check for string "-$(version)-$(release)." in string srpm
   string srpm = Handler->SourceRpm();
   string versarch = "-" + Handler->Version() + "-" + Handler->Release() + ".";
   string::size_type idx1 = srpm.find(versarch);

// not found
   if ( idx1 == string::npos )
     return "";

// check if the first dot in "srpm" is the dot at the end of versarch
   string::size_type idx2 = srpm.find('.');

   if ( idx2 < idx1 )
// no, the packager is playing dirty tricks with srpm names
     return "";

   return srpm.substr(0,idx1);
}
									/*}}}*/

void rpmRecordParser::BufCat(const char *text)
{
   if (text != NULL)
      BufCat(text, text+strlen(text));
}

void rpmRecordParser::BufCat(const char *begin, const char *end)
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

void rpmRecordParser::BufCatTag(const char *tag, const char *value)
{
   BufCat(tag);
   BufCat(value);
}

void rpmRecordParser::BufCatDep(Dependency *Dep)
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

void rpmRecordParser::BufCatDescr(const char *descr)
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


// RecordParser::GetRec - The record in raw text, in std Debian format	/*{{{*/
// ---------------------------------------------------------------------
void rpmRecordParser::GetRec(const char *&Start,const char *&Stop) 
{
   // FIXME: This method is leaking memory from headerGetEntry().
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


   vector<Dependency*> Deps, Provides, Obsoletes, Conflicts;
   vector<Dependency*>::iterator I;
   bool start = true;

   Handler->Depends(pkgCache::Dep::Depends, Deps);
   for (I = Deps.begin(); I != Deps.end(); I++) {
      if ((*I)->Type != pkgCache::Dep::PreDepends)
	 continue;
      if (start) {
	 BufCat("\nPre-Depends: ");
	 start = false;
      } else {
	 BufCat(", ");
      }
      BufCatDep(*I);
   }

   start = true;
   for (I = Deps.begin(); I != Deps.end(); I++) {
      if ((*I)->Type != pkgCache::Dep::Depends)
	 continue;
      if (start) {
	 BufCat("\nDepends: ");
	 start = false;
      } else {
	 BufCat(", ");
      }
      BufCatDep(*I);
   }
      
   Handler->Depends(pkgCache::Dep::Conflicts, Conflicts);
   start = true;
   for (I = Conflicts.begin(); I != Conflicts.end(); I++) {
      if (start) {
	 BufCat("\nConflicts: ");
	 start = false;
      } else {
	 BufCat(", ");
      }
      BufCatDep(*I);
   }

   Handler->Provides(Provides);
   start = true;
   for (I = Provides.begin(); I != Provides.end(); I++) {
      if (start) {
	 BufCat("\nProvides: ");
	 start = false;
      } else {
	 BufCat(", ");
      }
      BufCatDep(*I);
   }

   Handler->Depends(pkgCache::Dep::Obsoletes, Obsoletes);
   start = true;
   for (I = Obsoletes.begin(); I != Obsoletes.end(); I++) {
      if (start) {
	 BufCat("\nObsoletes: ");
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
   
   Start = Buffer;
   Stop = Buffer + BufUsed;
}
									/*}}}*/

bool rpmRecordParser::HasFile(const char *File)
{
   return Handler->HasFile(File);
}

#endif /* HAVE_RPM */

// vim:sts=3:sw=3
