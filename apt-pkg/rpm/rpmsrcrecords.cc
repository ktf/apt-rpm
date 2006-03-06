// -*- mode: cpp; mode: fold -*-
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

#include <apt-pkg/rpmsrcrecords.h>
#include <apt-pkg/error.h>
#include <apt-pkg/strutl.h>
#include <apt-pkg/rpmhandler.h>
#include <apt-pkg/pkgcache.h>

#include <apti18n.h>

#if RPM_VERSION >= 0x040100
#include <rpm/rpmds.h>
#endif

// SrcRecordParser::rpmSrcRecordParser - Constructor			/*{{{*/
// ---------------------------------------------------------------------
/* */
rpmSrcRecordParser::rpmSrcRecordParser(string File,pkgIndexFile const *Index)
    : Parser(Index), HeaderP(0), Buffer(0), BufSize(0), BufUsed(0)
{
   struct stat Buf;
   if (stat(File.c_str(),&Buf) == 0 && S_ISDIR(Buf.st_mode))
      Handler = new RPMDirHandler(File);
   else if (flExtension(File) == "rpm")
      Handler = new RPMSingleFileHandler(File);
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
}
									/*}}}*/
// SrcRecordParser::Files - Return a list of files for this source	/*{{{*/
// ---------------------------------------------------------------------
/* This parses the list of files and returns it, each file is required to have
   a complete source package */
bool rpmSrcRecordParser::Files(vector<pkgSrcRecords::File> &List)
{
   assert(HeaderP != NULL);
    
   List.clear();
   
   pkgSrcRecords::File F;

   F.MD5Hash = Handler->MD5Sum();
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
   if (Handler->Skip() == false)
       return false;
   HeaderP = Handler->GetHeader();
   return true;
}

bool rpmSrcRecordParser::Jump(unsigned long Off)
{
   if (!Handler->Jump(Off))
       return false;
   HeaderP = Handler->GetHeader();
   return true;
}

string rpmSrcRecordParser::Package() const
{
   char *str;
   int_32 count, type;
   int rc = headerGetEntry(HeaderP, RPMTAG_NAME,
			   &type, (void**)&str, &count);
   return string(rc?str:"");
}

string rpmSrcRecordParser::Version() const
{
   char *version, *release;
   int_32 *epoch;
   int type, count;
   int rc;
   
   rc = headerGetEntry(HeaderP, RPMTAG_VERSION,
		       &type, (void **)&version, &count);
   if (rc != 1)
   {
      _error->Error(_("error parsing source list %s"), "(RPMTAG_VERSION)");
      return "";
   }
   rc = headerGetEntry(HeaderP, RPMTAG_RELEASE,
		       &type, (void **)&release, &count);
   if (rc != 1)
   {
      _error->Error(_("error parsing source list %s"), "(RPMTAG_RELEASE)");
      return "";
   }

   rc = headerGetEntry(HeaderP, RPMTAG_EPOCH,
			   &type, (void **)&epoch, &count);
   string ret;
   if (rc == 1 && count > 0) 
   {
      char buf[32];
      sprintf(buf, "%i", *epoch);
      ret = string(buf)+":"+string(version)+"-"+string(release);
   }
   else 
      ret = string(version)+"-"+string(release);
   
   return ret;
}
    

// RecordParser::Maintainer - Return the maintainer email		/*{{{*/
// ---------------------------------------------------------------------
/* */
string rpmSrcRecordParser::Maintainer() const
{
   char *str;
   int_32 count, type;
   int rc = headerGetEntry(HeaderP, RPMTAG_PACKAGER,
			   &type, (void**)&str, &count);
   return string(rc?str:"");
}

string rpmSrcRecordParser::Section() const
{
   char *str;
   int_32 count, type;
   int rc = headerGetEntry(HeaderP, RPMTAG_GROUP,
			   &type, (void**)&str, &count);
   return string(rc?str:"");
}

unsigned long rpmSrcRecordParser::Offset() 
{
    return Handler->Offset();
}

void rpmSrcRecordParser::BufCat(char *text)
{
   if (text != NULL)
      BufCat(text, text+strlen(text));
}

void rpmSrcRecordParser::BufCat(char *begin, char *end)
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

void rpmSrcRecordParser::BufCatTag(char *tag, char *value)
{
   BufCat(tag);
   BufCat(value);
}

void rpmSrcRecordParser::BufCatDep(char *pkg, char *version, int flags)
{
   char buf[16];
   char *ptr = (char*)buf;

   BufCat(pkg);
   if (*version) 
   {
      int c = 0;
      *ptr++ = ' ';
      *ptr++ = '(';
      if (flags & RPMSENSE_LESS)
      {
	 *ptr++ = '<';
	 c = '<';
      }
      if (flags & RPMSENSE_GREATER) 
      {
	 *ptr++ = '>';
	 c = '>';
      }
      if (flags & RPMSENSE_EQUAL) 
      {
	 *ptr++ = '=';
      }/* else {
	 if (c)
	   fputc(c, f);
      }*/
      *ptr++ = ' ';
      *ptr = '\0';

      BufCat(buf);
      BufCat(version);
      BufCat(")");
   }
}

void rpmSrcRecordParser::BufCatDescr(char *descr)
{
   char *begin = descr;

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
   // FIXME: This method is leaking memory from headerGetEntry().
   int type, type2, type3, count;
   char *str;
   char **strv;
   char **strv2;
   int num;
   int_32 *numv;
   char buf[32];

   BufUsed = 0;
   
   headerGetEntry(HeaderP, RPMTAG_NAME, &type, (void **)&str, &count);
   BufCatTag("Package: ", str);

   headerGetEntry(HeaderP, RPMTAG_GROUP, &type, (void **)&str, &count);
   BufCatTag("\nSection: ", str);

   headerGetEntry(HeaderP, RPMTAG_SIZE, &type, (void **)&numv, &count);
   snprintf(buf, sizeof(buf), "%d", numv[0]);
   BufCatTag("\nInstalled Size: ", buf);

   str = NULL;
   headerGetEntry(HeaderP, RPMTAG_PACKAGER, &type, (void **)&str, &count);
   if (!str)
       headerGetEntry(HeaderP, RPMTAG_VENDOR, &type, (void **)&str, &count);
   BufCatTag("\nMaintainer: ", str);
   
   BufCat("\nVersion: ");
   headerGetEntry(HeaderP, RPMTAG_VERSION, &type, (void **)&str, &count);
   if (headerGetEntry(HeaderP, RPMTAG_EPOCH, &type, (void **)&numv, &count)==1)
       snprintf(buf, sizeof(buf), "%i:%s-", numv[0], str);
   else
       snprintf(buf, sizeof(buf), "%s-", str);
   BufCat(buf);
   headerGetEntry(HeaderP, RPMTAG_RELEASE, &type, (void **)&str, &count);
   BufCat(str);

   headerGetEntry(HeaderP, RPMTAG_REQUIRENAME, &type, (void **)&strv, &count);
   assert(type == RPM_STRING_ARRAY_TYPE || count == 0);

   headerGetEntry(HeaderP, RPMTAG_REQUIREVERSION, &type2, (void **)&strv2, &count);
   headerGetEntry(HeaderP, RPMTAG_REQUIREFLAGS, &type3, (void **)&numv, &count);
   
   if (count > 0)
   {
      int i, j;

      for (j = i = 0; i < count; i++) 
      {
	 if ((numv[i] & RPMSENSE_PREREQ))
	 {
	    if (j == 0) 
		BufCat("\nPre-Depends: ");
	    else
		BufCat(", ");
	    BufCatDep(strv[i], strv2[i], numv[i]);
	    j++;
	 }
      }

      for (j = 0, i = 0; i < count; i++) 
      {
	 if (!(numv[i] & RPMSENSE_PREREQ)) 
	 {
	    if (j == 0)
		BufCat("\nDepends: ");
	    else
		BufCat(", ");
	    BufCatDep(strv[i], strv2[i], numv[i]);
	    j++;
	 }
      }
   }
   
   headerGetEntry(HeaderP, RPMTAG_CONFLICTNAME, &type, (void **)&strv, &count);
   assert(type == RPM_STRING_ARRAY_TYPE || count == 0);

   headerGetEntry(HeaderP, RPMTAG_CONFLICTVERSION, &type2, (void **)&strv2, &count);
   headerGetEntry(HeaderP, RPMTAG_CONFLICTFLAGS, &type3, (void **)&numv, &count);
   
   if (count > 0) 
   {
      BufCat("\nConflicts: ");
      for (int i = 0; i < count; i++) 
      {
	 if (i > 0)
	     BufCat(", ");
	 BufCatDep(strv[i], strv2[i], numv[i]);
      }
   }

   headerGetEntry(HeaderP, CRPMTAG_FILESIZE, &type, (void **)&num, &count);
   snprintf(buf, sizeof(buf), "%d", num);
   BufCatTag("\nSize: ", buf);

   headerGetEntry(HeaderP, CRPMTAG_MD5, &type, (void **)&str, &count);
   BufCatTag("\nMD5Sum: ", str);

   headerGetEntry(HeaderP, CRPMTAG_FILENAME, &type, (void **)&str, &count);
   BufCatTag("\nFilename: ", str);

   headerGetEntry(HeaderP, RPMTAG_SUMMARY, &type, (void **)&str, &count);
   BufCatTag("\nDescription: ", str);
   BufCat("\n");
   headerGetEntry(HeaderP, RPMTAG_DESCRIPTION, &type, (void **)&str, &count);
   BufCatDescr(str);
   BufCat("\n");
   
   return string(Buffer, BufUsed);
}


// SrcRecordParser::BuildDepends - Return the Build-Depends information	/*{{{*/
// ---------------------------------------------------------------------
bool rpmSrcRecordParser::BuildDepends(vector<pkgSrcRecords::Parser::BuildDepRec> &BuildDeps,
				      bool ArchOnly)
{
   // FIXME: This method is leaking memory from headerGetEntry().
   int RpmTypeTag[] = {RPMTAG_REQUIRENAME,
		       RPMTAG_REQUIREVERSION,
		       RPMTAG_REQUIREFLAGS,
		       RPMTAG_CONFLICTNAME,
		       RPMTAG_CONFLICTVERSION,
		       RPMTAG_CONFLICTFLAGS};
   int BuildType[] = {pkgSrcRecords::Parser::BuildDepend,
		      pkgSrcRecords::Parser::BuildConflict};
   BuildDepRec rec;

   BuildDeps.clear();

   for (unsigned char Type = 0; Type != 2; Type++)
   {
      char **namel = NULL;
      char **verl = NULL;
      int *flagl = NULL;
      int res, type, count;

      res = headerGetEntry(HeaderP, RpmTypeTag[0+Type*3], &type, 
			 (void **)&namel, &count);
      if (res != 1)
	 return true;
      res = headerGetEntry(HeaderP, RpmTypeTag[1+Type*3], &type, 
			 (void **)&verl, &count);
      res = headerGetEntry(HeaderP, RpmTypeTag[2+Type*3], &type,
			 (void **)&flagl, &count);
      
      for (int i = 0; i < count; i++) 
      {
#if RPM_VERSION >= 0x040404
         if (namel[i][0] == 'g' && strncmp(namel[i], "getconf", 7) == 0)
         {
            rpmds getconfProv = NULL;
            rpmds ds = rpmdsSingle(RPMTAG_PROVIDENAME,
                                   namel[i], verl?verl[i]:NULL, flagl[i]);
            rpmdsGetconf(&getconfProv, NULL);
            int res = rpmdsSearch(getconfProv, ds) >= 0;
            rpmdsFree(ds);
            rpmdsFree(getconfProv);
            if (res) continue;
         }
#endif
	 if (strncmp(namel[i], "rpmlib", 6) == 0) 
	 {
#if RPM_VERSION >= 0x040404
	    rpmds rpmlibProv = NULL;
	    rpmds ds = rpmdsSingle(RPMTAG_PROVIDENAME,
				   namel[i], verl?verl[i]:NULL, flagl[i]);
	    rpmdsRpmlib(&rpmlibProv, NULL);
	    int res = rpmdsSearch(rpmlibProv, ds) >= 0;
	    rpmdsFree(ds);
	    rpmdsFree(rpmlibProv);
#elif RPM_VERSION >= 0x040100
	    rpmds ds = rpmdsSingle(RPMTAG_PROVIDENAME,
				   namel[i], verl?verl[i]:NULL, flagl[i]);
	    int res = rpmCheckRpmlibProvides(ds);
	    rpmdsFree(ds);
#else
	    int res = rpmCheckRpmlibProvides(namel[i], verl?verl[i]:NULL,
					     flagl[i]);
#endif
	    if (res) continue;
	 }

	 if (verl) 
	 {
	    if (!*verl[i]) 
	       rec.Op = pkgCache::Dep::NoOp;
	    else 
	    {
	       if (flagl[i] & RPMSENSE_LESS) 
	       {
		  if (flagl[i] & RPMSENSE_EQUAL)
		      rec.Op = pkgCache::Dep::LessEq;
		  else
		      rec.Op = pkgCache::Dep::Less;
	       } 
	       else if (flagl[i] & RPMSENSE_GREATER) 
	       {
		  if (flagl[i] & RPMSENSE_EQUAL)
		      rec.Op = pkgCache::Dep::GreaterEq;
		  else
		      rec.Op = pkgCache::Dep::Greater;
	       } 
	       else if (flagl[i] & RPMSENSE_EQUAL) 
		  rec.Op = pkgCache::Dep::Equals;
	    }
	    
	    rec.Version = verl[i];
	 }
	 else
	 {
	    rec.Op = pkgCache::Dep::NoOp;
	    rec.Version = "";
	 }

	 rec.Type = BuildType[Type];
	 rec.Package = namel[i];
	 BuildDeps.push_back(rec);
      }
   }
   return true;
}
									/*}}}*/
#endif /* HAVE_RPM */

// vim:sts=3:sw=3
