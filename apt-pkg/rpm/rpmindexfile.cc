// -*- mode: cpp; mode: fold -*-
// Description								/*{{{*/
// $Id: rpmindexfile.cc,v 1.4 2002/11/27 16:22:40 niemeyer Exp $
/* ######################################################################

   RPM Specific sources.list types and the three sorts of RPM
   index files.
   
   ##################################################################### */
									/*}}}*/
// Include Files							/*{{{*/
#ifdef __GNUG__
#pragma implementation "apt-pkg/rpmindexfile.h"
#endif

#include <config.h>

#ifdef HAVE_RPM

#include <cassert>

#include <apt-pkg/rpmindexfile.h>
#include <apt-pkg/rpmsrcrecords.h>
#include <apt-pkg/rpmlistparser.h>
#include <apt-pkg/rpmrecords.h>
#include <apt-pkg/rpmsystem.h>
#include <apt-pkg/rpmhandler.h>
#include <apt-pkg/rpmpackagedata.h>
#include <apt-pkg/sourcelist.h>
#include <apt-pkg/configuration.h>
#include <apt-pkg/progress.h>
#include <apt-pkg/error.h>
#include <apt-pkg/strutl.h>
#include <apt-pkg/acquire-item.h>
#include <apt-pkg/repomd.h>

#include <apti18n.h>

#include <sys/stat.h>
									/*}}}*/
vector<pkgRepository *> RepList;

// rpmListIndex::Release* - Return the URI to the release file		/*{{{*/
// ---------------------------------------------------------------------
/* */
inline string rpmListIndex::ReleaseFile(string Type) const
{
   return URItoFileName(ReleaseURI(Type));
}

string rpmListIndex::ReleaseURI(string Type) const
{
   RPMPackageData *rpmdata = RPMPackageData::Singleton();
   string Res;
   if (Dist[Dist.size() - 1] == '/')
   {
      if (Dist != "/")
	 Res = URI + Dist;
      else
	 Res = URI;
   }
   else
      Res = URI + Dist + "/base/";

   Res += Type;
   
   if (rpmdata->HasIndexTranslation() == true)
   {
      map<string,string> Dict;
      Dict["uri"] = URI;
      Dict["dist"] = Dist;
      Dict["sect"] = "";
      Dict["type"] = Type;
      rpmdata->TranslateIndex(Res, Dict);
   }

   return Res;
}
									/*}}}*/
// rpmListIndex::ReleaseInfo - One liner describing the index URI	/*{{{*/
// ---------------------------------------------------------------------
/* */
string rpmListIndex::ReleaseInfo(string Type) const 
{
   string Info = ::URI::SiteOnly(URI) + ' ';
   if (Dist[Dist.size() - 1] == '/')
   {
      if (Dist != "/")
	 Info += Dist;
   }
   else
      Info += Dist;   
   Info += " ";
   Info += Type;
   return Info;
}
									/*}}}*/
// rpmListIndex::GetReleases - Fetch the index files			/*{{{*/
// ---------------------------------------------------------------------
/* */
bool rpmListIndex::GetReleases(pkgAcquire *Owner) const
{
   if (!Repository->Acquire)
      return true;
   Repository->Acquire = false;
   new pkgAcqIndexRel(Owner,Repository,ReleaseURI("release"),
		      ReleaseInfo("release"), "release", true);
   return true;
}
									/*}}}*/
// rpmListIndex::Info - One liner describing the index URI		/*{{{*/
// ---------------------------------------------------------------------
/* */
string rpmListIndex::Info(string Type) const 
{
   string Info = ::URI::SiteOnly(URI) + ' ';
   if (Dist[Dist.size() - 1] == '/')
   {
      if (Dist != "/")
	 Info += Dist;
   }
   else
      Info += Dist + '/' + Section;   
   Info += " ";
   Info += Type;
   return Info;
}
									/*}}}*/
// rpmListIndex::Index* - Return the URI to the index files		/*{{{*/
// ---------------------------------------------------------------------
/* */
inline string rpmListIndex::IndexFile(string Type) const
{
   return _config->FindDir("Dir::State::lists")+URItoFileName(IndexURI(Type));
}


string rpmListIndex::IndexURI(string Type) const
{
   RPMPackageData *rpmdata = RPMPackageData::Singleton();
   string Res;
   if (Dist[Dist.size() - 1] == '/')
   {
      if (Dist != "/")
	 Res = URI + Dist;
      else 
	 Res = URI;
   }
   else
      Res = URI + Dist + "/base/";
   
   Res += Type + '.' + Section;

   if (rpmdata->HasIndexTranslation() == true)
   {
      map<string,string> Dict;
      Dict["uri"] = URI;
      Dict["dist"] = Dist; 
      Dict["sect"] = Section;
      Dict["type"] = Type;
      rpmdata->TranslateIndex(Res, Dict);
   }

   return Res;
}
									/*}}}*/
// rpmListIndex::Exists - Check if the index is available		/*{{{*/
// ---------------------------------------------------------------------
/* */
bool rpmListIndex::Exists() const
{
   return FileExists(IndexPath());
}
									/*}}}*/
// rpmListIndex::Size - Return the size of the index			/*{{{*/
// ---------------------------------------------------------------------
/* */
unsigned long rpmListIndex::Size() const
{
   struct stat S;
   if (stat(IndexPath().c_str(),&S) != 0)
      return 0;
   return S.st_size;
}
									/*}}}*/
// rpmListIndex::Describe - Give a descriptive path to the index	/*{{{*/
// ---------------------------------------------------------------------
/* */
string rpmListIndex::Describe(bool Short) const
{
   char S[300];
   if (Short == true)
      snprintf(S,sizeof(S),"%s",Info(MainType()).c_str());
   else
      snprintf(S,sizeof(S),"%s (%s)",Info(MainType()).c_str(),
         IndexFile(MainType()).c_str());
   return S;
}
									/*}}}*/

// SrcListIndex::SourceInfo - Short 1 liner describing a source		/*{{{*/
// ---------------------------------------------------------------------
string rpmSrcListIndex::SourceInfo(pkgSrcRecords::Parser const &Record,
				   pkgSrcRecords::File const &File) const
{
   string Res;
   Res = ::URI::SiteOnly(URI) + ' ';
   if (Dist[Dist.size() - 1] == '/')
   {
      if (Dist != "/")
	 Res += Dist;
   }      
   else
      Res += Dist + '/' + Section;
   
   Res += " ";
   Res += Record.Package();
   Res += " ";
   Res += Record.Version();
   if (File.Type.empty() == false)
      Res += " (" + File.Type + ")";
   return Res;
}
									/*}}}*/
// SrcListIndex::ArchiveURI - URI for the archive       	        /*{{{*/
// ---------------------------------------------------------------------
string rpmSrcListIndex::ArchiveURI(string File) const
{
   RPMPackageData *rpmdata = RPMPackageData::Singleton();
   string Res;
   
   if (Dist[Dist.size() - 1] == '/')
   {
      if (Dist != "/")
	 Res = URI + Dist;
      else
	 Res = URI;
   }
   else
      Res = URI + Dist;
   
   if (File.find("/") != string::npos)
      Res += '/' + File;
   else
      Res += "/SRPMS."+Section + '/' + File;

   if (rpmdata->HasSourceTranslation() == true)
   {
      map<string,string> Dict;
      Dict["uri"] = URI;
      Dict["dist"] = Dist; 
      Dict["sect"] = Section;
      string::size_type pos = File.rfind("/");
      if (pos != string::npos)
	 Dict["file"] = string(File, pos+1);
      else
	 Dict["file"] = File;
      
      rpmdata->TranslateSource(Res, Dict);
   }
	 
   return Res;
}
									/*}}}*/
// SrcListIndex::CreateSrcParser - Get a parser for the source files	/*{{{*/
// ---------------------------------------------------------------------
/* */
pkgSrcRecords::Parser *rpmSrcListIndex::CreateSrcParser() const
{
   return new rpmSrcRecordParser(IndexPath(), this);
}
									/*}}}*/
// SrcListIndex::GetIndexes - Fetch the index files			/*{{{*/
// ---------------------------------------------------------------------
/* */
bool rpmSrcListIndex::GetIndexes(pkgAcquire *Owner) const
{
   // Ignore indexes for repositories that could not be authenticated
   if (Repository->IsAuthenticated() == true && 
       Repository->HasRelease() == false)
      return true;
   new pkgAcqIndex(Owner,Repository,IndexURI("srclist"),Info("srclist"),
		   "srclist");
   return true;
}
									/*}}}*/

// PkgListIndex::ArchiveInfo - Short version of the archive url	        /*{{{*/
// ---------------------------------------------------------------------
/* This is a shorter version that is designed to be < 60 chars or so */
string rpmPkgListIndex::ArchiveInfo(pkgCache::VerIterator Ver) const
{
   string Res = ::URI::SiteOnly(URI) + ' ';
   if (Dist[Dist.size() - 1] == '/')
   {
      if (Dist != "/")
	 Res += Dist;
   }
   else
      Res += Dist + '/' + Section;
   
   Res += " ";
   Res += Ver.ParentPkg().Name();
   Res += " ";
   Res += Ver.VerStr();
   return Res;
}
									/*}}}*/
// PkgListIndex::ArchiveURI - URI for the archive       	        /*{{{*/
// ---------------------------------------------------------------------
string rpmPkgListIndex::ArchiveURI(string File) const
{
   RPMPackageData *rpmdata = RPMPackageData::Singleton();
   string Res;
   if (Dist[Dist.size() - 1] == '/')
   {
      if (Dist != "/")
	 Res = URI + Dist;
      else
	 Res = URI;
   }
   else
      Res = URI + Dist;

   if (File.find("/") != string::npos)
      Res += '/' + File;
   else
      Res += "/RPMS." + Section + '/' + File;

   if (rpmdata->HasBinaryTranslation() == true)
   {
      map<string,string> Dict;
      Dict["uri"] = URI;
      Dict["dist"] = Dist; 
      Dict["sect"] = Section;
      string::size_type pos = File.rfind("/");
      if (pos != string::npos)
	 Dict["file"] = string(File, pos+1);
      else
	 Dict["file"] = File;
      rpmdata->TranslateBinary(Res, Dict);
   }
	 
   return Res;
}
									/*}}}*/
// PkgListIndex::GetIndexes - Fetch the index files			/*{{{*/
// ---------------------------------------------------------------------
/* */
bool rpmPkgListIndex::GetIndexes(pkgAcquire *Owner) const
{
   // Ignore indexes for repositories that could not be authenticated
   if (Repository->IsAuthenticated() == true && 
       Repository->HasRelease() == false)
      return true;
   new pkgAcqIndex(Owner,Repository,IndexURI("pkglist"),Info("pkglist"),
		   "pkglist");
   new pkgAcqIndexRel(Owner,Repository,IndexURI("release"),Info("release"),
		      "release");
   return true;
}
									/*}}}*/
// PkgListIndex::Merge - Load the index file into a cache		/*{{{*/
// ---------------------------------------------------------------------
/* */
bool rpmPkgListIndex::Merge(pkgCacheGenerator &Gen,OpProgress &Prog) const
{
   string PackageFile = IndexPath();
   RPMHandler *Handler = CreateHandler();

   Prog.SubProgress(0,Info(MainType()));
   ::URI Tmp(URI);
   if (Gen.SelectFile(PackageFile,Tmp.Host,*this) == false)
   {
      delete Handler;
      return _error->Error(_("Problem with SelectFile %s"),PackageFile.c_str());
   }

   // Store the IMS information
   pkgCache::PkgFileIterator File = Gen.GetCurFile();
   struct stat St;
   if (stat(PackageFile.c_str(),&St) != 0) 
   {
      delete Handler;
      return _error->Errno("stat",_("Failed to stat %s"), PackageFile.c_str());
   }
   File->Size = St.st_size;
   File->mtime = St.st_mtime;
   
   rpmListParser Parser(Handler);
   if (_error->PendingError() == true) 
   {
      delete Handler;
      return _error->Error(_("Problem opening %s"),PackageFile.c_str());
   }
   
   if (Gen.MergeList(Parser) == false)
   {
      delete Handler;
      return _error->Error(_("Problem with MergeList %s"),PackageFile.c_str());
   }
   
   delete Handler;

   // Check the release file
   string RelFile = ReleasePath();
   if (FileExists(RelFile) == true)
   {
      FileFd Rel(RelFile,FileFd::ReadOnly);
      if (_error->PendingError() == true)
	 return false;
      Parser.LoadReleaseInfo(File,Rel);
      Rel.Seek(0);
   }

   return true;
}
									/*}}}*/
// PkgListIndex::MergeFileProvides - Process file dependencies if any	/*{{{*/
// ---------------------------------------------------------------------
/* */
bool rpmPkgListIndex::MergeFileProvides(pkgCacheGenerator &Gen,
					OpProgress &Prog) const
{
   string PackageFile = IndexPath();
   RPMHandler *Handler = CreateHandler();
   rpmListParser Parser(Handler);
   if (_error->PendingError() == true) {
      delete Handler;
      return _error->Error(_("Problem opening %s"),PackageFile.c_str());
   }
   // We call SubProgress with Size(), since we won't call SelectFile() here.
   Prog.SubProgress(Size(),Info("pkglist"));
   if (Gen.MergeFileProvides(Parser) == false)
      return _error->Error(_("Problem with MergeFileProvides %s"),
			   PackageFile.c_str());
   delete Handler;
   return true;
}
									/*}}}*/
// PkgListIndex::FindInCache - Find this index				/*{{{*/
// ---------------------------------------------------------------------
/* */
pkgCache::PkgFileIterator rpmPkgListIndex::FindInCache(pkgCache &Cache) const
{
   string FileName = IndexPath();
   pkgCache::PkgFileIterator File = Cache.FileBegin();
   for (; File.end() == false; File++)
   {
      if (FileName != File.FileName())
	 continue;
      
      struct stat St;
      if (stat(File.FileName(),&St) != 0)
	 return pkgCache::PkgFileIterator(Cache);

      if ((unsigned)St.st_size != File->Size || St.st_mtime != File->mtime)
	 return pkgCache::PkgFileIterator(Cache);
      return File;
   }
   
   return File;
}
									/*}}}*/

// PkgDirIndex::Index* - Return the URI to the index files		/*{{{*/
// ---------------------------------------------------------------------
/* */
string rpmPkgDirIndex::IndexPath() const
{
   return ::URI(ArchiveURI("")).Path;
}
									/*}}}*/
// PkgDirIndex::Release* - Return the URI to the index files		/*{{{*/
// ---------------------------------------------------------------------
/* */
string rpmPkgDirIndex::ReleasePath() const
{
   return ::URI(IndexURI("release")).Path;
}
									/*}}}*/
// PkgDirIndex::Size - Return the size of the index			/*{{{*/
// ---------------------------------------------------------------------
/* This is really only used for progress reporting. */
unsigned long rpmPkgDirIndex::Size() const
{
   // XXX: Must optimize this somehow.
   RPMHandler *Handler = CreateHandler();
   unsigned long Res = Handler->Size();
   delete Handler;
   return Res;
}
									/*}}}*/

// SrcDirIndex::Index* - Return the URI to the index files		/*{{{*/
// ---------------------------------------------------------------------
/* */
string rpmSrcDirIndex::IndexPath() const
{
   return ::URI(ArchiveURI("")).Path;
}
									/*}}}*/
// SrcDirIndex::Size - Return the size of the index			/*{{{*/
// ---------------------------------------------------------------------
/* This is really only used for progress reporting. */
unsigned long rpmSrcDirIndex::Size() const
{
   // XXX: Must optimize this somehow.
   RPMHandler *Handler = CreateHandler();
   unsigned long Res = Handler->Size();
   delete Handler;
   return Res;
}

// SinglePkgIndex::ArchiveURI - URI for the archive       	        /*{{{*/
// ---------------------------------------------------------------------
string rpmSinglePkgIndex::ArchiveURI(string File) const
{
   char *cwd = getcwd(NULL,0);
   if (File[0] == '.' && File[1] == '/')
      File = string(File, 2);
   string URI = "file://"+flCombine(cwd, File);
   free(cwd);
   return URI;
}
									/*}}}*/
// SinglePkgIndex::ArchiveURI - URI for the archive       	        /*{{{*/
// ---------------------------------------------------------------------
string rpmSingleSrcIndex::ArchiveURI(string File) const
{
   char *cwd = getcwd(NULL,0);
   if (File[0] == '.' && File[1] == '/')
      File = string(File, 2);
   string URI = "file://"+flCombine(cwd, File);
   free(cwd);
   return URI;
}

string rpmRepomdIndex::ArchiveURI(string File) const
{
   string Res;

   Res += URI + '/' + Dist + '/' + File;
   return Res;
}

string rpmRepomdIndex::ArchiveInfo(pkgCache::VerIterator Ver) const
{
   string Res = ::URI::SiteOnly(URI) + ' ';
   if (Dist[Dist.size() - 1] == '/')
   {
      if (Dist != "/")
	 Res += Dist;
   }
   else
      Res += Dist + '/';
   
   Res += " ";
   Res += Ver.ParentPkg().Name();
   Res += " ";
   Res += Ver.VerStr();
   return Res;
}
pkgCache::PkgFileIterator rpmRepomdIndex::FindInCache(pkgCache &Cache) const
{
   string FileName = IndexPath();
   pkgCache::PkgFileIterator File = Cache.FileBegin();
   for (; File.end() == false; File++)
   {
      if (FileName != File.FileName())
	 continue;
      
      struct stat St;
      if (stat(File.FileName(),&St) != 0)
	 return pkgCache::PkgFileIterator(Cache);

      if ((unsigned)St.st_size != File->Size || St.st_mtime != File->mtime)
	 return pkgCache::PkgFileIterator(Cache);
      return File;
   }
   
   return File;
}
string rpmRepomdIndex::ReleaseURI(string Type) const
{
   string Res = URI + Dist;
   assert( Res.size() > 0 );
   if ( Res[Res.size() - 1] != '/' )
     Res += '/';
   Res += "repodata/";
   Res += "repomd.xml";
   return Res;
}

string rpmRepomdIndex::ReleaseInfo(string Type) const
{
   string Info = ::URI::SiteOnly(URI) + ' ';
   assert( Dist.size() > 0 );
   if (Dist[Dist.size() - 1] == '/')
   {
      if (Dist != "/")
	 Info += Dist;
   }
   else
      Info += Dist;   
   Info += " ";
   Info += Type;
   return Info;
};

string rpmRepomdIndex::Info(string Type) const 
{
   string Info = ::URI::SiteOnly(URI) + ' ';
   assert( Dist.size() > 0 );
   if (Dist[Dist.size() - 1] == '/')
   {
      if (Dist != "/")
	 Info += Dist;
   }
   else
      Info += Dist + '/' ;   
   Info += " ";
   Info += Type;
   return Info;
}

string rpmRepomdIndex::IndexURI(string Type) const
{
   string Res = URI + Dist;
   assert( Dist.size() > 0 );
   if (Dist[Dist.size() - 1] != '/') {
	 Res += "/";
   }
   // When called with the main type it's primary.xml we want, at other
   // times it could be eg filelists.xml
   if (Type == "repomd" || Type == "repomd-src")
      Res += "repodata/primary.xml";
   else
      Res += "repodata/" + Type;
   return Res;
}
									/*}}}*/
bool rpmRepomdIndex::GetReleases(pkgAcquire *Owner) const
{
   if (!Repository->Acquire)
      return true;
   Repository->Acquire = false;
   // ignore for now - we need to parse the file for checksum and 
   // optimally for location of other xml files as well
   new pkgAcqIndexRel(Owner,Repository,ReleaseURI("repomd.xml"),
                      ReleaseInfo("repomd.xml"), "repomd.xml", true);
   return true;
}

bool rpmRepomdIndex::GetIndexes(pkgAcquire *Owner) const
{
   new pkgAcqIndex(Owner,Repository,IndexURI("primary.xml"),
	           Info("primary.xml"), "primary.xml");
   new pkgAcqIndex(Owner,Repository,IndexURI("filelists.xml"),
		   Info("filelists.xml"), "filelists.xml");
#if 0
   new pkgAcqIndex(Owner,Repository,IndexURI("other.xml"),
   		   Info("other.xml"), "other.xml");
#endif
   return true;
}

string rpmRepomdIndex::Describe(bool Short) const
{
   char S[300];
   if (Short == true)
      snprintf(S,sizeof(S),"%s",Info(MainType()).c_str());
   else
      snprintf(S,sizeof(S),"%s (%s)",Info(MainType()).c_str(),
         IndexFile(MainType()).c_str());
   return S;
}

string rpmRepomdIndex::IndexFile(string Type) const
{
   return _config->FindDir("Dir::State::lists")+URItoFileName(IndexURI(Type));
};


bool rpmRepomdIndex::Exists() const
{
   return FileExists(IndexPath());
}

string rpmRepomdIndex::ReleasePath() const
{
   return _config->FindDir("Dir::State::lists")+URItoFileName(ReleaseURI("release.xml"));
}

// This gets called several times per each index during cache generation,
// only for progress percentage reporting. A rough estimate would do just
// fine but using xmlReader appears to be cheap enough. OTOH creating a new 
// handler like is done for rpmPkgDirIndex is hideously expensive.
unsigned long rpmRepomdIndex::Size() const
{
   xmlTextReaderPtr Index;
   unsigned long Res;
   Index = xmlReaderForFile(IndexPath().c_str(), NULL,
			  XML_PARSE_NONET|XML_PARSE_NOBLANKS);
   if (Index == NULL) return 0;

   if (xmlTextReaderRead(Index) == 1) {
      xmlChar *pkgs = xmlTextReaderGetAttribute(Index, (xmlChar*)"packages");
      Res = atoi((char*)pkgs);
      xmlFree(pkgs);
   } else {
      Res = 0;
   }
   xmlFreeTextReader(Index);
   return Res;
}

bool rpmRepomdIndex::Merge(pkgCacheGenerator &Gen,OpProgress &Prog) const
{
   string PackageFile = IndexPath();
   RPMHandler *Handler = CreateHandler();

   Prog.SubProgress(0,Info(MainType()));
   ::URI Tmp(URI);
   if (Gen.SelectFile(PackageFile,Tmp.Host,*this) == false)
   {
      delete Handler;
      return _error->Error(_("Problem with SelectFile %s"),PackageFile.c_str());
   }

   // Store the IMS information
   pkgCache::PkgFileIterator File = Gen.GetCurFile();
   struct stat St;
   if (stat(PackageFile.c_str(),&St) != 0) 
   {
      delete Handler;
      return _error->Errno("stat",_("Failed to stat %s"), PackageFile.c_str());
   }
   File->Size = St.st_size;
   File->mtime = St.st_mtime;
   
   rpmRepomdParser Parser(Handler);
   if (_error->PendingError() == true) 
   {
      delete Handler;
      return _error->Error(_("Problem opening %s"),PackageFile.c_str());
   }
   
   if (Gen.MergeList(Parser) == false)
   {
      delete Handler;
      return _error->Error(_("Problem with MergeList %s"),PackageFile.c_str());
   }
   
   delete Handler;

   // Check the release file
   string RelFile = ReleasePath();
   if (FileExists(RelFile) == true)
   {
      Parser.LoadReleaseInfo(File,RelFile,Dist);
   }

   return true;
}

bool rpmRepomdIndex::MergeFileProvides(pkgCacheGenerator &Gen,
					OpProgress &Prog) const
{
   string PackageFile = IndexPath();
   RPMHandler *Handler = CreateHandler();
   rpmListParser Parser(Handler);
   if (_error->PendingError() == true) {
      delete Handler;
      return _error->Error(_("Problem opening %s"),PackageFile.c_str());
   }
   // We call SubProgress with Size(), since we won't call SelectFile() here.
   Prog.SubProgress(Size(),Info("pkglist"));
   if (Gen.MergeFileProvides(Parser) == false)
      return _error->Error(_("Problem with MergeFileProvides %s"),
			   PackageFile.c_str());
   delete Handler;
   return true;
}

pkgSrcRecords::Parser *rpmRepomdIndex::CreateSrcParser() const
{
   return new rpmSrcRecordParser(IndexPath(), this);
}

string rpmRepomdSrcIndex::SourceInfo(pkgSrcRecords::Parser const &Record,
		  		     pkgSrcRecords::File const &File) const
{
   string Res;
   Res = ::URI::SiteOnly(URI) + ' ';
   if (Dist[Dist.size() - 1] == '/')
   {
      if (Dist != "/")
	 Res += Dist;
   }      
   else
      Res += Dist + '/' + Section;
   
   Res += " ";
   Res += Record.Package();
   Res += " ";
   Res += Record.Version();
   if (File.Type.empty() == false)
      Res += " (" + File.Type + ")";
   return Res;
}

// DatabaseIndex::rpmDatabaseIndex - Constructor			/*{{{*/
// ---------------------------------------------------------------------
/* */
rpmDatabaseIndex::rpmDatabaseIndex()
{
}
									/*}}}*/
// DatabaseIndex::Size - Return the size of the index			/*{{{*/
// ---------------------------------------------------------------------
/* */
unsigned long rpmDatabaseIndex::Size() const
{
   return rpmSys.GetDBHandler()->Size();
}
									/*}}}*/
// DatabaseIndex::CreateHandler - Create a RPMHandler for this file	/*{{{*/
// ---------------------------------------------------------------------
RPMHandler *rpmDatabaseIndex::CreateHandler() const
{
   return rpmSys.GetDBHandler();
}
									/*}}}*/
// DatabaseIndex::Merge - Load the index file into a cache		/*{{{*/
// ---------------------------------------------------------------------
/* */
bool rpmDatabaseIndex::Merge(pkgCacheGenerator &Gen,OpProgress &Prog) const
{
   RPMDBHandler *Handler = rpmSys.GetDBHandler();
   rpmListParser Parser(Handler);
   if (_error->PendingError() == true)
      return _error->Error(_("Problem opening RPM database"));
   
   Prog.SubProgress(0,"RPM Database");
   if (Gen.SelectFile(Handler->DataPath(false),string(),*this,pkgCache::Flag::NotSource) == false)
      return _error->Error(_("Problem with SelectFile RPM Database"));

   // Store the IMS information
   pkgCache::PkgFileIterator CFile = Gen.GetCurFile();
   struct stat St;
   if (stat(Handler->DataPath(false).c_str(),&St) != 0)
      return _error->Errno("fstat",_("Failed to stat %s"), Handler->DataPath(false).c_str());
   CFile->Size = St.st_size;
   CFile->mtime = Handler->Mtime();
   
   if (Gen.MergeList(Parser) == false)
      return _error->Error(_("Problem with MergeList %s"),
			   Handler->DataPath(false).c_str());
   return true;
}
									/*}}}*/
// DatabaseIndex::MergeFileProvides - Process file dependencies if any	/*{{{*/
// ---------------------------------------------------------------------
/* */
bool rpmDatabaseIndex::MergeFileProvides(pkgCacheGenerator &Gen,
		   		  	 OpProgress &Prog) const
{
   RPMDBHandler *Handler = rpmSys.GetDBHandler();
   rpmListParser Parser(Handler);
   if (_error->PendingError() == true)
      return _error->Error(_("Problem opening RPM database"));
   // We call SubProgress with Size(), since we won't call SelectFile() here.
   Prog.SubProgress(Size(),"RPM Database");
   if (Gen.MergeFileProvides(Parser) == false)
      return _error->Error(_("Problem with MergeFileProvides %s"),
			   Handler->DataPath(false).c_str());
   return true;
}
									/*}}}*/
// DatabaseIndex::FindInCache - Find this index				/*{{{*/
// ---------------------------------------------------------------------
/* */
pkgCache::PkgFileIterator rpmDatabaseIndex::FindInCache(pkgCache &Cache) const
{
   pkgCache::PkgFileIterator File = Cache.FileBegin();
   for (; File.end() == false; File++)
   {
      if (rpmSys.GetDBHandler()->DataPath(false) != File.FileName())
	 continue;
      struct stat St;
      if (stat(File.FileName(),&St) != 0)
	 return pkgCache::PkgFileIterator(Cache);
      if ((unsigned)St.st_size != File->Size || St.st_mtime != File->mtime)
	 return pkgCache::PkgFileIterator(Cache);
      return File;
   }   
   return File;
}
									/*}}}*/

// Source List types for rpm						/*{{{*/

class rpmSLTypeGen : public pkgSourceList::Type
{
   public:

   rpmSLTypeGen()
   {
      Name = "rpm";
      Label = "Standard RPM source tree";
   }   

   pkgRepository *FindRepository(string URI,string Dist,
				 const pkgSourceList::Vendor *Vendor) const
   {
      for (vector<pkgRepository *>::const_iterator iter = RepList.begin();
	   iter != RepList.end(); iter++) 
      {
	 if ((*iter)->URI == URI && (*iter)->Dist == Dist) 
	 {	 
	    if (Vendor != NULL)
	       (*iter)->FingerPrint = Vendor->FingerPrint;
	    return *iter;
	 }
      }
      return NULL;
   }

   pkgRepository *GetRepository(string URI,string Dist,
				const pkgSourceList::Vendor *Vendor) const
   {
      pkgRepository *Rep = FindRepository(URI,Dist,Vendor);
      if (Rep != NULL)
	 return Rep;

      string BaseURI;
      if (Dist[Dist.size() - 1] == '/')
      {
	 if (Dist != "/")
	    BaseURI = URI + Dist;
	 else 
	    BaseURI = URI + '/';
      }
      else
	 BaseURI = URI + Dist + '/';

      Rep = new pkgRepository(URI,Dist,Vendor,BaseURI);
      RepList.push_back(Rep);
      return Rep;
   }
};


class rpmSLTypeRpm : public rpmSLTypeGen
{
   public:

   bool CreateItem(vector<pkgIndexFile *> &List,
		   string URI, string Dist, string Section,
		   pkgSourceList::Vendor const *Vendor) const 
   {
      pkgRepository *Rep = GetRepository(URI,Dist,Vendor);
      List.push_back(new rpmPkgListIndex(URI,Dist,Section,Rep));
      return true;
   };

   rpmSLTypeRpm()
   {
      Name = "rpm";
      Label = "Standard RPM binary tree";
   }   
};

class rpmSLTypeSrpm : public rpmSLTypeGen
{
   public:

   bool CreateItem(vector<pkgIndexFile *> &List,
		   string URI, string Dist, string Section,
		   pkgSourceList::Vendor const *Vendor) const 
   {
      pkgRepository *Rep = GetRepository(URI,Dist,Vendor);
      List.push_back(new rpmSrcListIndex(URI,Dist,Section,Rep));
      return true;
   };  
   
   rpmSLTypeSrpm()
   {
      Name = "rpm-src";
      Label = "Standard RPM source tree";
   }   
};

class rpmSLTypeRpmDir : public rpmSLTypeGen
{
   public:

   bool CreateItem(vector<pkgIndexFile *> &List,
		   string URI, string Dist, string Section,
		   pkgSourceList::Vendor const *Vendor) const 
   {
      pkgRepository *Rep = GetRepository(URI,Dist,Vendor);
      List.push_back(new rpmPkgDirIndex(URI,Dist,Section,Rep));
      return true;
   };

   rpmSLTypeRpmDir()
   {
      Name = "rpm-dir";
      Label = "Local RPM directory tree";
   }   
};

class rpmSLTypeSrpmDir : public rpmSLTypeGen
{
   public:

   bool CreateItem(vector<pkgIndexFile *> &List,
		   string URI, string Dist, string Section,
		   pkgSourceList::Vendor const *Vendor) const 
   {
      pkgRepository *Rep = GetRepository(URI,Dist,Vendor);
      List.push_back(new rpmSrcDirIndex(URI,Dist,Section,Rep));
      return true;
   };

   rpmSLTypeSrpmDir()
   {
      Name = "rpm-src-dir";
      Label = "Local SRPM directory tree";
   }   
};

class rpmSLTypeRepomd : public rpmSLTypeGen
{
   public:

   pkgRepository *GetRepository(string URI,string Dist,
				const pkgSourceList::Vendor *Vendor) const
   {
      pkgRepository *Rep = FindRepository(URI,Dist,Vendor);
      if (Rep != NULL)
	 return Rep;

      string BaseURI;
      if (Dist[Dist.size() - 1] == '/')
      {
	 if (Dist != "/")
	    BaseURI = URI + Dist;
	 else 
	    BaseURI = URI + '/';
      }
      else
	 BaseURI = URI + Dist + '/';

      Rep = new repomdRepository(URI,Dist,Vendor,BaseURI);
      RepList.push_back(Rep);
      return Rep;
   }

   bool CreateItem(vector<pkgIndexFile *> &List,
		   string URI, string Dist, string Section,
		   pkgSourceList::Vendor const *Vendor) const 
   {
      pkgRepository *Rep = GetRepository(URI,Dist,Vendor);
      List.push_back(new rpmRepomdPkgIndex(URI,Dist,Section,Rep));
      return true;
   };

   bool ParseLine(vector<pkgIndexFile *> &List,
                  pkgSourceList::Vendor const *Vendor,
                  const char *Buffer,
                  unsigned long CurLine,string File) const
   {
      string URI;
      string Dist;
      if (ParseQuoteWord(Buffer,URI) == false)
	 return _error->Error(_("Malformed line %lu in source list %s (URI)"),CurLine,File.c_str());
      if (ParseQuoteWord(Buffer,Dist) == false)
	 return _error->Error(_("Malformed line %lu in source list %s (dist)"),CurLine,File.c_str());

      if (FixupURI(URI) == false)
	 return _error->Error(_("Malformed line %lu in source list %s (URI parse)"),CurLine,File.c_str());
      
      Dist = SubstVar(Dist,"$(ARCH)",_config->Find("APT::Architecture"));
      Dist = SubstVar(Dist,"$(VERSION)",_config->Find("APT::DistroVersion"));

      if (CreateItem(List,URI,Dist,"",Vendor) == false)
	 return false;

      return true;
   };


   rpmSLTypeRepomd()
   {
      Name = "repomd";
      Label = "RepoMD tree";
   }   
};
class rpmSLTypeRepomdSrc : public rpmSLTypeRepomd
{
   public:

   bool CreateItem(vector<pkgIndexFile *> &List,
		   string URI, string Dist, string Section,
		   pkgSourceList::Vendor const *Vendor) const 
   {
      pkgRepository *Rep = GetRepository(URI,Dist,Vendor);
      List.push_back(new rpmRepomdSrcIndex(URI,Dist,Section,Rep));
      return true;
   };

   rpmSLTypeRepomdSrc()
   {
      Name = "repomd-src";
      Label = "RepoMD src tree";
   }   
};

rpmSLTypeRpm _apt_rpmType;
rpmSLTypeSrpm _apt_rpmSrcType;
rpmSLTypeRpmDir _apt_rpmDirType;
rpmSLTypeSrpmDir _apt_rpmSrcDirType;
rpmSLTypeRepomd _apt_repomdType;
rpmSLTypeRepomdSrc _apt_repomdSrcType;
									/*}}}*/
// Index File types for rpm						/*{{{*/
class rpmIFTypeSrc : public pkgIndexFile::Type
{
   public:
   
   rpmIFTypeSrc() {Label = "RPM Source Index";};
};
class rpmIFTypePkg : public pkgIndexFile::Type
{
   public:
   
   virtual pkgRecords::Parser *CreatePkgParser(pkgCache::PkgFileIterator File) const
   {
      return new rpmRecordParser(File.FileName(),*File.Cache());
   };
   rpmIFTypePkg() {Label = "RPM Package Index";};
};
class rpmIFTypeDatabase : public pkgIndexFile::Type
{
   public:
   
   virtual pkgRecords::Parser *CreatePkgParser(pkgCache::PkgFileIterator File) const
   {
      return new rpmRecordParser(File.FileName(),*File.Cache());
   };
   rpmIFTypeDatabase() {Label = "RPM Database";};
};
static rpmIFTypeSrc _apt_Src;
static rpmIFTypePkg _apt_Pkg;
static rpmIFTypeDatabase _apt_DB;

const pkgIndexFile::Type *rpmSrcListIndex::GetType() const
{
   return &_apt_Src;
}
const pkgIndexFile::Type *rpmPkgListIndex::GetType() const
{
   return &_apt_Pkg;
}
const pkgIndexFile::Type *rpmSrcDirIndex::GetType() const
{
   return &_apt_Src;
}
const pkgIndexFile::Type *rpmPkgDirIndex::GetType() const
{
   return &_apt_Pkg;
}
const pkgIndexFile::Type *rpmSinglePkgIndex::GetType() const
{
   return &_apt_Pkg;
}
const pkgIndexFile::Type *rpmSingleSrcIndex::GetType() const
{
   return &_apt_Src;
}
const pkgIndexFile::Type *rpmDatabaseIndex::GetType() const
{
   return &_apt_DB;
}
const pkgIndexFile::Type *rpmRepomdPkgIndex::GetType() const
{
   return &_apt_Pkg;
}
const pkgIndexFile::Type *rpmRepomdSrcIndex::GetType() const
{
   return &_apt_Src;
}

									/*}}}*/
#endif /* HAVE_RPM */

// vim:sts=3:sw=3
