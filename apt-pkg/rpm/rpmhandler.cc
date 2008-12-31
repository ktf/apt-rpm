
/*
 ######################################################################

 RPM database and hdlist related handling

 ######################################################################
 */

#include <config.h>

#ifdef HAVE_RPM

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <utime.h>
#include <unistd.h>
#include <signal.h>
#include <assert.h>
#include <libgen.h>
#include <cstring>
#include <sstream>

#include <apt-pkg/error.h>
#include <apt-pkg/configuration.h>
#include <apt-pkg/md5.h>
#include <apt-pkg/crc-16.h>

/* for rpm5.org >= 4.4.9 */
#ifdef HAVE_RPM_RPMEVR_H
#define _RPMEVR_INTERNAL
#endif

#include "rpmhandler.h"
#include "rpmpackagedata.h"
#include "raptheader.h"

#ifdef APT_WITH_REPOMD
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlreader.h>
#include <sstream>
#include "repomd.h"
#ifdef WITH_SQLITE3
#include "sqlite.h"
#endif
#include "xmlutil.h"
#endif

#include <apti18n.h>

#if RPM_VERSION >= 0x040100
#include <rpm/rpmts.h>
#include <rpm/rpmdb.h>
#include <rpm/rpmds.h>
#include <rpm/rpmfi.h>

// Newer rpm.org has rpmsqIsCaught() which suits out purposes just fine,
// for other versions define our own version of it. We'd want to include 
// rpmsq.h here but it's not valid C++ in many existing rpm versions so 
// just declare rpmsqCaught extern.. sigh.
#ifdef RPM_HAVE_RPMSQISCAUGHT
#include <rpm/rpmsq.h>
#else
extern sigset_t rpmsqCaught;

static int rpmsqIsCaught(int signum) 
{
   return sigismember(&rpmsqCaught, signum);
}
#endif
#endif

// An attempt to deal with false zero epochs from repomd. With older rpm's we
// can only blindly trust the repo admin created the repository with options
// suitable for those versions. For rpm >= 4.2.1 this is linked with
// promoteepoch behavior - if promoteepoch is used then epoch hiding must
// not happen.
bool HideZeroEpoch;

extern map<string,int> rpmIndexSizes;

string RPMHandler::EVR()
{
   string e = Epoch();
   string v = Version();
   string r = Release();
   string evr = "";
   if (e.empty() == true) {
      evr = v + '-' + r;
   } else if (HideZeroEpoch && e == "0") {
      evr = v + '-' + r;
   } else {
      evr = e + ':' + v + '-' + r;
   }
   return evr;
} 

unsigned int RPMHandler::DepOp(raptDepFlags rpmflags)
{
   unsigned int Op = 0;
   raptDepFlags flags = (raptDepFlags)(rpmflags & RPMSENSE_SENSEMASK);
   if (flags == RPMSENSE_ANY) {
      Op = pkgCache::Dep::NoOp;
   } else if (flags & RPMSENSE_LESS) {
      if (flags & RPMSENSE_EQUAL)
	  Op = pkgCache::Dep::LessEq;
      else
	  Op = pkgCache::Dep::Less;
   } else if (flags & RPMSENSE_GREATER) {
      if (flags & RPMSENSE_EQUAL)
	  Op = pkgCache::Dep::GreaterEq;
      else
	  Op = pkgCache::Dep::Greater;
   } else if (flags & RPMSENSE_EQUAL) {
      Op = pkgCache::Dep::Equals;
   } else {
      /* can't happen, right? */
      _error->Error(_("Impossible flags %d in %s"), rpmflags, Name().c_str());
   }
      
   return Op;
}

bool RPMHandler::HasFile(const char *File)
{
   if (*File == '\0')
      return false;
   
   vector<string> Files;
   FileList(Files);
   vector<string>::iterator I = find(Files.begin(), Files.end(), File);
   return (I != Files.end());
}

bool RPMHandler::InternalDep(const char *name, const char *ver, raptDepFlags flag) 
{
   if (strncmp(name, "rpmlib(", strlen("rpmlib(")) == 0) {
#if RPM_VERSION >= 0x040100
#if RPM_HAVE_DSRPMLIB
     rpmds rpmlibProv = NULL;
     rpmds ds = rpmdsSingle(RPMTAG_PROVIDENAME,
			    name, ver?ver:NULL, flag);
     rpmdsRpmlib(&rpmlibProv, NULL);
     int res = rpmdsSearch(rpmlibProv, ds) >= 0;
     rpmdsFree(ds);
     rpmdsFree(rpmlibProv);
#else
      rpmds ds = rpmdsSingle(RPMTAG_PROVIDENAME,
			     name, ver?ver:NULL, flag);
      int res = rpmCheckRpmlibProvides(ds);
      rpmdsFree(ds);
#endif
#else
      int res = rpmCheckRpmlibProvides(name, ver?ver:NULL,
				       flag);
#endif
      if (res) 
	 return true;
   }

#if RPM_HAVE_DSGETCONF
   // uhhuh, any of these changing would require full cache rebuild...
   if (strncmp(name, "getconf(", strlen("getconf(")) == 0)
   {
     rpmds getconfProv = NULL;
     rpmds ds = rpmdsSingle(RPMTAG_PROVIDENAME,
			    name, ver?ver:NULL, flag);
     rpmdsGetconf(&getconfProv, NULL);
     int res = rpmdsSearch(getconfProv, ds);
     rpmdsFree(ds);
     rpmdsFree(getconfProv);
     if (res) 
	 return true;
   }

   if (strncmp(name, "cpuinfo(", strlen("cpuinfo(")) == 0)
   {
     rpmds cpuinfoProv = NULL;
     rpmds ds = rpmdsSingle(RPMTAG_PROVIDENAME,
			    name, ver?ver:NULL, flag);
     rpmdsCpuinfo(&cpuinfoProv, NULL);
     int res = rpmdsSearch(cpuinfoProv, ds);
     rpmdsFree(ds);
     rpmdsFree(cpuinfoProv);
     if (res) 
	 return true;
   }

   if (strncmp(name, "sysinfo(", strlen("sysinfo(")) == 0)
   {
     rpmds sysinfoProv = NULL;
     rpmds ds = rpmdsSingle(RPMTAG_PROVIDENAME,
			    name, ver?ver:NULL, flag);
     rpmdsCpuinfo(&sysinfoProv, NULL);
     int res = rpmdsSearch(sysinfoProv, ds);
     rpmdsFree(ds);
     rpmdsFree(sysinfoProv);
     if (res)
	 return true;
   }

   if (strncmp(name, "uname(", strlen("uname(")) == 0)
   {
     rpmds unameProv = NULL;
     rpmds ds = rpmdsSingle(RPMTAG_PROVIDENAME,
			    name, ver?ver:NULL, flag);
     rpmdsUname(&unameProv, NULL);
     int res = rpmdsSearch(unameProv, ds);
     rpmdsFree(ds);
     rpmdsFree(unameProv);
     if (res)
	 return true;
   }

   if (strlen(name) > 5 && name[strlen(name)-1] == ')' &&
       ((strchr("Rr_", name[0]) != NULL &&
	 strchr("Ww_", name[1]) != NULL &&
	 strchr("Xx_", name[2]) != NULL &&
	 name[3] == '(') ||
	 strncmp(name, "exists(", strlen("exists(")) == 0 ||
	 strncmp(name, "executable(", strlen("executable(")) == 0 ||
	 strncmp(name, "readable(", strlen("readable(")) == 0 ||
	 strncmp(name, "writable(", strlen("writable("))== 0 ))
   {
      int res = rpmioAccess(name, NULL, X_OK);
      if (res == 0)
	 return true;
   }

   /* TODO
    * - /etc/rpm/sysinfo provides
    * - macro probe provides 
    * - actually implement soname() and access() dependencies
    */
   if (strncmp(name, "soname(", strlen("soname(")) == 0)
   {
      cout << "FIXME, ignoring soname() dependency: " << name << endl;
      return true;
   }
#endif
   return false; 
}

bool RPMHandler::PutDep(const char *name, const char *ver, raptDepFlags flags, 
			unsigned int Type, vector<Dependency*> &Deps)
{
   if (InternalDep(name, ver, flags) == true) {
      return true;
   }

   if (Type == pkgCache::Dep::Depends) {
      if (flags & RPMSENSE_PREREQ)
	 Type = pkgCache::Dep::PreDepends;
#if RPM_VERSION >= 0x040403
      else if (flags & RPMSENSE_MISSINGOK)
	 Type = pkgCache::Dep::Suggests;
#endif
      else
	 Type = pkgCache::Dep::Depends;
   }

   Dependency *Dep = new Dependency;
   Dep->Name = name;
   Dep->Version = ver;

   if (HideZeroEpoch && Dep->Version.substr(0, 2) == "0:") {
      Dep->Version = Dep->Version.substr(2);
   }

   Dep->Op = DepOp(flags);
   Dep->Type = Type;
   Deps.push_back(Dep);
   return true;
}

string RPMHdrHandler::Epoch()
{
   raptInt val;
   ostringstream epoch("");
   raptHeader h(HeaderP);

   if (h.getTag(RPMTAG_EPOCH, val)) {
      epoch << val;
   }
   return epoch.str();
}

off_t RPMHdrHandler::GetITag(raptTag Tag)
{
   raptInt val = 0;
   raptHeader h(HeaderP);

   h.getTag(Tag, val); 
   return val;
}

string RPMHdrHandler::GetSTag(raptTag Tag)
{
   string str = "";
   raptHeader h(HeaderP);

   h.getTag(Tag, str);
   return str;
}


bool RPMHdrHandler::PRCO(unsigned int Type, vector<Dependency*> &Deps)
#if RPM_VERSION >= 0x040100
{
   rpmTag deptype = RPMTAG_REQUIRENAME;
   switch (Type) {
      case pkgCache::Dep::Depends:
	 deptype = RPMTAG_REQUIRENAME;
	 break;
      case pkgCache::Dep::Obsoletes:
	 deptype = RPMTAG_OBSOLETENAME;
	 break;
      case pkgCache::Dep::Conflicts:
	 deptype = RPMTAG_CONFLICTNAME;
	 break;
      case pkgCache::Dep::Provides:
	 deptype = RPMTAG_PROVIDENAME;
	 break;
#if RPM_VERSION >= 0x040403
      case pkgCache::Dep::Suggests:
	 deptype = RPMTAG_SUGGESTSNAME;
	 break;
#if 0 // Enhances dep type is not even known to apt, sigh..
      case pkgCache::Dep::Enhances:
	 deptype = RPMTAG_ENHANCES;
	 break;
#endif
#endif
      default:
	 /* can't happen... right? */
	 return false;
	 break;
   }
   rpmds ds = NULL;
   ds = rpmdsNew(HeaderP, deptype, 0);
   if (ds != NULL) {
      while (rpmdsNext(ds) >= 0) {
	 PutDep(rpmdsN(ds), rpmdsEVR(ds), rpmdsFlags(ds), Type, Deps);
      }
   }
   rpmdsFree(ds);
   return true;
}
#else
{
   vector<string> names, versions;
   vector<raptInt> flags;
   raptTag deptag, depver, depflags;
   raptHeader h(HeaderP);

   switch (Type) {
      case pkgCache::Dep::Depends:
	 deptag = RPMTAG_REQUIRENAME;
	 depver = RPMTAG_REQUIREVERSION;
	 depflags = RPMTAG_REQUIREFLAGS;
	 break;
      case pkgCache::Dep::Obsoletes:
	 deptag = RPMTAG_OBSOLETENAME;
	 depver = RPMTAG_OBSOLETEVERSION;
	 depflags = RPMTAG_OBSOLETEFLAGS;
	 break;
      case pkgCache::Dep::Conflicts:
	 deptag = RPMTAG_CONFLICTNAME;
	 depver = RPMTAG_CONFLICTVERSION;
	 depflags = RPMTAG_CONFLICTFLAGS;
	 break;
      case pkgCache::Dep::Provides:
	 deptag = RPMTAG_PROVIDENAME;
	 depver = RPMTAG_PROVIDEVERSION;
	 depflags = RPMTAG_PROVIDEFLAGS;
	 break;
      default:
	 /* can't happen... right? */
	 return false;
	 break;
   }
   if (h.getTag(deptag, names)) {
      h.getTag(depver, versions);
      h.getTag(depflags, flags);

      vector<string>::const_iterator ni = names.begin();
      vector<string>::const_iterator vi = versions.begin();
      vector<raptInt>::const_iterator fi = flags.begin();
      while (ni != names.end() && vi != versions.end() && fi != flags.end()) {
	 PutDep(ni->c_str(), vi->c_str(), (raptDepFlags)*fi, Type, Deps);
	 ni++; vi++; fi++;
      }
   }
   return true;
}
#endif

bool RPMHdrHandler::FileList(vector<string> &FileList)
{
   raptHeader h(HeaderP);
   h.getTag(RAPT_FILENAMES, FileList);
   // it's ok for a package not have files 
   return true; 
}

bool RPMHdrHandler::ChangeLog(vector<ChangeLogEntry *> &ChangeLogs)
{
   vector<string> names, texts;
   vector<raptInt> times;
   raptHeader h(HeaderP);

   if (h.getTag(RPMTAG_CHANGELOGTIME, times)) {
      h.getTag(RPMTAG_CHANGELOGNAME, names);
      h.getTag(RPMTAG_CHANGELOGTEXT, texts);
   
      vector<raptInt>::const_iterator timei = times.begin();
      vector<string>::const_iterator namei = names.begin();
      vector<string>::const_iterator texti = texts.begin();
      while (timei != times.end() && namei != names.end() && 
				     texti != texts.end()) {
	 ChangeLogEntry *Entry = new ChangeLogEntry;
	 Entry->Time = *(timei);
	 Entry->Author = *(namei);
	 Entry->Text = *(texti);
	 timei++; namei++; texti++;
	 ChangeLogs.push_back(Entry);
      }
   }
      
   return true;
}

RPMFileHandler::RPMFileHandler(string File)
{
   ID = File;
   FD = Fopen(File.c_str(), "r");
   if (FD == NULL)
   {
      /*
      _error->Error(_("could not open RPM package list file %s: %s"),
		    File.c_str(), rpmErrorString());
      */
      return;
   }
   iSize = fdSize(FD);
   rpmIndexSizes[ID] = iSize;
}

RPMFileHandler::RPMFileHandler(FileFd *File)
{
   FD = fdDup(File->Fd());
   if (FD == NULL)
   {
      /*
      _error->Error(_("could not create RPM file descriptor: %s"),
		    rpmErrorString());
      */
      return;
   }
   iSize = fdSize(FD);
   rpmIndexSizes[ID] = iSize;
}

RPMFileHandler::~RPMFileHandler()
{
   if (HeaderP != NULL)
      headerFree(HeaderP);
   if (FD != NULL)
      Fclose(FD);
}

bool RPMFileHandler::Skip()
{
   if (FD == NULL)
      return false;
   iOffset = lseek(Fileno(FD),0,SEEK_CUR);
   if (HeaderP != NULL)
       headerFree(HeaderP);
   HeaderP = headerRead(FD, HEADER_MAGIC_YES);
   return (HeaderP != NULL);
}

bool RPMFileHandler::Jump(off_t Offset)
{
   if (FD == NULL)
      return false;
   if (lseek(Fileno(FD),Offset,SEEK_SET) != Offset)
      return false;
   return Skip();
}

void RPMFileHandler::Rewind()
{
   if (FD == NULL)
      return;
   iOffset = lseek(Fileno(FD),0,SEEK_SET);
   if (iOffset != 0)
      _error->Error(_("could not rewind RPMFileHandler"));
}

string RPMFileHandler::FileName()
{
   return GetSTag(CRPMTAG_FILENAME);
}

string RPMFileHandler::Directory()
{
   return GetSTag(CRPMTAG_DIRECTORY);
}

off_t RPMFileHandler::FileSize()
{
   return GetITag(CRPMTAG_FILESIZE);
}

string RPMFileHandler::MD5Sum()
{
   return GetSTag(CRPMTAG_MD5);
}

bool RPMSingleFileHandler::Skip()
{
   if (FD == NULL)
      return false;
   if (HeaderP != NULL) {
      headerFree(HeaderP);
      HeaderP = NULL;
      return false;
   }
#if RPM_VERSION >= 0x040100
   rpmts TS = rpmtsCreate();
   rpmtsSetVSFlags(TS, (rpmVSFlags_e)-1);
   int rc = rpmReadPackageFile(TS, FD, sFilePath.c_str(), &HeaderP);
   if (rc != RPMRC_OK && rc != RPMRC_NOTTRUSTED && rc != RPMRC_NOKEY) {
      _error->Error(_("Failed reading file %s"), sFilePath.c_str());
      HeaderP = NULL;
   }
   rpmtsFree(TS);
#else
   int rc = rpmReadPackageHeader(FD, &HeaderP, 0, NULL, NULL);
   if (rc) {
      _error->Error(_("Failed reading file %s"), sFilePath.c_str());
      HeaderP = NULL;
   }
#endif
   return (HeaderP != NULL);
}

bool RPMSingleFileHandler::Jump(off_t Offset)
{
   assert(Offset == 0);
   Rewind();
   return RPMFileHandler::Jump(Offset);
}

void RPMSingleFileHandler::Rewind()
{
   if (FD == NULL)
      return;
   if (HeaderP != NULL) {
      HeaderP = NULL;
      headerFree(HeaderP);
   }
   lseek(Fileno(FD),0,SEEK_SET);
}

off_t RPMSingleFileHandler::FileSize()
{
   struct stat S;
   if (stat(sFilePath.c_str(),&S) != 0)
      return 0;
   return S.st_size;
}

string RPMSingleFileHandler::MD5Sum()
{
   MD5Summation MD5;
   FileFd File(sFilePath, FileFd::ReadOnly);
   MD5.AddFD(File.Fd(), File.Size());
   File.Close();
   return MD5.Result().Value();
}

bool RPMSingleFileHandler::ChangeLog(vector<ChangeLogEntry* > &ChangeLogs)
{
   return RPMHdrHandler::ChangeLog(ChangeLogs);
}

RPMDirHandler::RPMDirHandler(string DirName)
   : sDirName(DirName)
{
   ID = DirName;
#if RPM_VERSION >= 0x040100
   TS = NULL;
#endif
   Dir = opendir(sDirName.c_str());
   if (Dir == NULL)
      return;
   iSize = 0;
   while (nextFileName() != NULL)
      iSize += 1;
   rewinddir(Dir);
#if RPM_VERSION >= 0x040100
   TS = rpmtsCreate();
   rpmtsSetVSFlags(TS, (rpmVSFlags_e)-1);
#endif
   rpmIndexSizes[ID] = iSize;
}

const char *RPMDirHandler::nextFileName()
{
   for (struct dirent *Ent = readdir(Dir); Ent != 0; Ent = readdir(Dir))
   {
      const char *name = Ent->d_name;

      if (name[0] == '.')
	 continue;

      if (flExtension(name) != "rpm")
	 continue;

      // Make sure it is a file and not something else
      sFilePath = flCombine(sDirName,name);
      struct stat St;
      if (stat(sFilePath.c_str(),&St) != 0 || S_ISREG(St.st_mode) == 0)
	 continue;

      sFileName = name;
      
      return name;
   } 
   return NULL;
}

RPMDirHandler::~RPMDirHandler()
{
   if (HeaderP != NULL)
      headerFree(HeaderP);
#if RPM_VERSION >= 0x040100
   if (TS != NULL)
      rpmtsFree(TS);
#endif
   if (Dir != NULL)
      closedir(Dir);
}

bool RPMDirHandler::Skip()
{
   if (Dir == NULL)
      return false;
   if (HeaderP != NULL) {
      headerFree(HeaderP);
      HeaderP = NULL;
   }
   const char *fname = nextFileName();
   bool Res = false;
   for (; fname != NULL; fname = nextFileName()) {
      iOffset++;
      if (fname == NULL)
	 break;
      FD_t FD = Fopen(sFilePath.c_str(), "r");
      if (FD == NULL)
	 continue;
#if RPM_VERSION >= 0x040100
      int rc = rpmReadPackageFile(TS, FD, fname, &HeaderP);
      Fclose(FD);
      if (rc != RPMRC_OK
	  && rc != RPMRC_NOTTRUSTED
	  && rc != RPMRC_NOKEY)
	 continue;
#else
      int isSource;
      int rc = rpmReadPackageHeader(FD, &HeaderP, &isSource, NULL, NULL);
      Fclose(FD);
      if (rc != 0)
	 continue;
#endif
      Res = true;
      break;
   }
   return Res;
}

bool RPMDirHandler::Jump(off_t Offset)
{
   if (Dir == NULL)
      return false;
   rewinddir(Dir);
   iOffset = 0;
   while (1) {
      if (iOffset+1 == Offset)
	 return Skip();
      if (nextFileName() == NULL)
	 break;
      iOffset++;
   }
   return false;
}

void RPMDirHandler::Rewind()
{
   rewinddir(Dir);
   iOffset = 0;
}

off_t RPMDirHandler::FileSize()
{
   if (Dir == NULL)
      return 0;
   struct stat St;
   if (stat(sFilePath.c_str(),&St) != 0) {
      _error->Errno("stat",_("Unable to determine the file size"));
      return 0;
   }
   return St.st_size;
}

string RPMDirHandler::MD5Sum()
{
   if (Dir == NULL)
      return "";
   MD5Summation MD5;
   FileFd File(sFilePath, FileFd::ReadOnly);
   MD5.AddFD(File.Fd(), File.Size());
   File.Close();
   return MD5.Result().Value();
}


RPMDBHandler::RPMDBHandler(bool WriteLock)
   : Handler(0), WriteLock(WriteLock)
{
#if RPM_VERSION >= 0x040000
   RpmIter = NULL;
#endif
   string Dir = _config->Find("RPM::RootDir", "/");
   
   rpmReadConfigFiles(NULL, NULL);
   ID = DataPath(false);

   RPMPackageData::Singleton()->InitMinArchScore();

   // Everytime we open a database for writing, it has its
   // mtime changed, and kills our cache validity. As we never
   // change any information in the database directly, we will
   // restore the mtime and save our cache.
   struct stat St;
   stat(DataPath(false).c_str(), &St);
   DbFileMtime = St.st_mtime;

#if RPM_VERSION >= 0x040100
   Handler = rpmtsCreate();
   rpmtsSetVSFlags(Handler, (rpmVSFlags_e)-1);
   rpmtsSetRootDir(Handler, Dir.c_str());
#else
   const char *RootDir = NULL;
   if (!Dir.empty())
      RootDir = Dir.c_str();
   if (rpmdbOpen(RootDir, &Handler, O_RDONLY, 0644) != 0)
   {
      _error->Error(_("could not open RPM database"));
      return;
   }
#endif
#if RPM_VERSION >= 0x040000
   RpmIter = raptInitIterator(Handler, RPMDBI_PACKAGES, NULL, 0);
   if (RpmIter == NULL) {
      _error->Error(_("could not create RPM database iterator"));
      return;
   }
   // iSize = rpmdbGetIteratorCount(RpmIter);
   // This doesn't seem to work right now. Code in rpm (4.0.4, at least)
   // returns a 0 from rpmdbGetIteratorCount() if raptInitIterator() is
   // called with RPMDBI_PACKAGES or with keyp == NULL. The algorithm
   // below will be used until there's support for it.
   iSize = 0;
   rpmdbMatchIterator countIt;
   countIt = raptInitIterator(Handler, RPMDBI_PACKAGES, NULL, 0);
   while (rpmdbNextIterator(countIt) != NULL)
      iSize++;
   rpmdbFreeIterator(countIt);
#else
   iSize = St.st_size;

#endif
   rpmIndexSizes[ID] = iSize;


   // Restore just after opening the database, and just after closing.
   if (WriteLock) {
      struct utimbuf Ut;
      Ut.actime = DbFileMtime;
      Ut.modtime = DbFileMtime;
      utime(DataPath(false).c_str(), &Ut);
   }
}

RPMDBHandler::~RPMDBHandler()
{
#if RPM_VERSION >= 0x040000
   if (RpmIter != NULL)
      rpmdbFreeIterator(RpmIter);
#else
   if (HeaderP != NULL)
       headerFree(HeaderP);
#endif

#if RPM_VERSION >= 0x040100
   /* 
    * If termination signal, do nothing as rpmdb has already freed
    * our ts set behind our back and rpmtsFree() will crash and burn with a 
    * doublefree within rpmlib.
    * There's a WTF involved as rpmCheckSignals() actually calls exit()
    * so we shouldn't even get here really?!
    */
   if (rpmsqIsCaught(SIGINT) || 
       rpmsqIsCaught(SIGQUIT) ||
       rpmsqIsCaught(SIGHUP) ||
       rpmsqIsCaught(SIGTERM) ||
       rpmsqIsCaught(SIGPIPE)) {
      /* do nothing */
   } else if (Handler != NULL) {
      rpmtsFree(Handler);
   }
#else
   if (Handler != NULL) {
      rpmdbClose(Handler);
   }
#endif

   // Restore just after opening the database, and just after closing.
   if (WriteLock) {
      struct utimbuf Ut;
      Ut.actime = DbFileMtime;
      Ut.modtime = DbFileMtime;
      utime(DataPath(false).c_str(), &Ut);
   }
}

string RPMDBHandler::DataPath(bool DirectoryOnly)
{
   string File = "packages.rpm";
   char *tmp = (char *) rpmExpand("%{_dbpath}", NULL);
   string DBPath(_config->Find("RPM::RootDir")+tmp);
   free(tmp);

#if RPM_VERSION >= 0x040000
   if (rpmExpandNumeric("%{_dbapi}") >= 3)
      File = "Packages";       
#endif
   if (DirectoryOnly == true)
       return DBPath;
   else
       return DBPath+"/"+File;
}

bool RPMDBHandler::Skip()
{
#if RPM_VERSION >= 0x040000
   if (RpmIter == NULL)
       return false;
   HeaderP = rpmdbNextIterator(RpmIter);
   iOffset = rpmdbGetIteratorOffset(RpmIter);
   if (HeaderP == NULL)
      return false;
#else
   if (iOffset == 0)
      iOffset = rpmdbFirstRecNum(Handler);
   else
      iOffset = rpmdbNextRecNum(Handler, iOffset);
   if (HeaderP != NULL)
   {
      headerFree(HeaderP);
      HeaderP = NULL;
   }
   if (iOffset == 0)
       return false;
   HeaderP = rpmdbGetRecord(Handler, iOffset);
#endif
   return true;
}

bool RPMDBHandler::Jump(off_t Offset)
{
   iOffset = Offset;
#if RPM_VERSION >= 0x040000
   // rpmdb indexes are hardcoded uint32_t, the size must match here
   raptDbOffset rpmOffset = iOffset;
   if (RpmIter == NULL)
      return false;
   rpmdbFreeIterator(RpmIter);
   if (iOffset == 0)
      RpmIter = raptInitIterator(Handler, RPMDBI_PACKAGES, NULL, 0);
   else {
      RpmIter = raptInitIterator(Handler, RPMDBI_PACKAGES,
				  &rpmOffset, sizeof(rpmOffset));
      iOffset = rpmOffset;
   }
   HeaderP = rpmdbNextIterator(RpmIter);
#else
   HeaderP = rpmdbGetRecord(Handler, iOffset);
#endif
   return true;
}

bool RPMDBHandler::JumpByName(string PkgName, bool Provides)
{
   raptTag tag = (raptTag)(Provides ? RPMTAG_PROVIDES : RPMDBI_LABEL);
   if (RpmIter == NULL) return false;
   rpmdbFreeIterator(RpmIter);
   RpmIter = raptInitIterator(Handler, tag, PkgName.c_str(), 0);
   HeaderP = rpmdbNextIterator(RpmIter);
   return (HeaderP != NULL);
}

void RPMDBHandler::Rewind()
{
#if RPM_VERSION >= 0x040000
   if (RpmIter == NULL)
      return;
   rpmdbFreeIterator(RpmIter);   
   RpmIter = raptInitIterator(Handler, RPMDBI_PACKAGES, NULL, 0);
#else
   if (HeaderP != NULL)
   {
      headerFree(HeaderP);
      HeaderP = NULL;
   }
#endif
   iOffset = 0;
}
#endif

#ifdef APT_WITH_REPOMD
RPMRepomdHandler::RPMRepomdHandler(repomdXML const *repomd): RPMHandler(),
      Primary(NULL), Root(NULL), HavePrimary(false)
{
   ID = repomd->ID();
   // Try to figure where in the world our files might be... 
   string base = ID.substr(0, ID.size() - strlen("repomd.xml"));
   PrimaryPath = base + flNotDir(repomd->FindURI("primary"));
   FilelistPath = base + flNotDir(repomd->FindURI("filelists"));
   OtherPath = base + flNotDir(repomd->FindURI("other"));

   xmlTextReaderPtr Index;
   Index = xmlReaderForFile(PrimaryPath.c_str(), NULL,
                          XML_PARSE_NONET|XML_PARSE_NOBLANKS);
   if (Index == NULL) {
      _error->Error(_("Failed to open package index %s"), PrimaryPath.c_str());
      return;
   }

   if (xmlTextReaderRead(Index) == 1) {
      xmlChar *pkgs = xmlTextReaderGetAttribute(Index, (xmlChar*)"packages");
      iSize = atoi((char*)pkgs);
      xmlFree(pkgs);
   } else {
      iSize = 0;
   }
   xmlFreeTextReader(Index);
   rpmIndexSizes[ID] = iSize;

}

bool RPMRepomdHandler::LoadPrimary()
{
   xmlChar *packages = NULL;
   off_t pkgcount = 0;

   Primary = xmlReadFile(PrimaryPath.c_str(), NULL, XML_PARSE_NONET|XML_PARSE_NOBLANKS);
   if ((Root = xmlDocGetRootElement(Primary)) == NULL) {
      _error->Error(_("Failed to open package index %s"), PrimaryPath.c_str());
      goto error;
   }
   if (xmlStrncmp(Root->name, (xmlChar*)"metadata", strlen("metadata")) != 0) {
      _error->Error(_("Corrupted package index %s"), PrimaryPath.c_str());
      goto error;
   }

   packages = xmlGetProp(Root, (xmlChar*)"packages");
   iSize = atoi((char*)packages);
   xmlFree(packages);
   for (xmlNode *n = Root->children; n; n = n->next) {
      if (n->type != XML_ELEMENT_NODE ||
          xmlStrcmp(n->name, (xmlChar*)"package") != 0)
         continue;
      Pkgs.push_back(n);
      pkgcount++;
   }
   PkgIter = Pkgs.begin();

   // There seem to be broken version(s) of createrepo around which report
   // to have one more package than is in the repository. Warn and work around.
   if (iSize != pkgcount) {
      _error->Warning(_("Inconsistent metadata, package count doesn't match in %s"), ID.c_str());
      iSize = pkgcount;
   }
   rpmIndexSizes[ID] = iSize;
   HavePrimary = true;

   return true;

error:
   if (Primary) {
      xmlFreeDoc(Primary);
   }
   return false;
}

bool RPMRepomdHandler::Skip()
{
   if (HavePrimary == false) {
      LoadPrimary();
   }
   if (PkgIter == Pkgs.end()) {
      return false;
   }
   NodeP = *PkgIter;
   iOffset = PkgIter - Pkgs.begin();

   PkgIter++;
   return true;
}

bool RPMRepomdHandler::Jump(off_t Offset)
{
   if (HavePrimary == false) {
      LoadPrimary();
   }
   if (Offset >= iSize) {
      return false;
   }
   iOffset = Offset;
   NodeP = Pkgs[Offset];
   // This isn't strictly necessary as Skip() and Jump() aren't mixed
   // in practise but doesn't hurt either...
   PkgIter = Pkgs.begin() + Offset + 1;
   return true;

}

void RPMRepomdHandler::Rewind()
{
   iOffset = 0;
   PkgIter = Pkgs.begin();
}

string RPMRepomdHandler::Name() 
{
   return XmlFindNodeContent(NodeP, "name");
}

string RPMRepomdHandler::Arch() 
{
   return XmlFindNodeContent(NodeP, "arch");
}

string RPMRepomdHandler::Packager() 
{
   return XmlFindNodeContent(NodeP, "packager");
}

string RPMRepomdHandler::Summary() 
{
   return XmlFindNodeContent(NodeP, "summary");
}

string RPMRepomdHandler::Description() 
{
   return XmlFindNodeContent(NodeP, "description");
}

string RPMRepomdHandler::Group()
{
   xmlNode *n = XmlFindNode(NodeP, "format");
   return XmlFindNodeContent(n, "group");
}

string RPMRepomdHandler::Vendor()
{
   xmlNode *n = XmlFindNode(NodeP, "format");
   return XmlFindNodeContent(n, "vendor");
}

string RPMRepomdHandler::Release()
{
   xmlNode *n = XmlFindNode(NodeP, "version");
   return XmlGetProp(n, "rel");
}

string RPMRepomdHandler::Version()
{
   xmlNode *n = XmlFindNode(NodeP, "version");
   return XmlGetProp(n, "ver");
}

string RPMRepomdHandler::Epoch()
{
   string epoch;
   xmlNode *n = XmlFindNode(NodeP, "version");
   epoch = XmlGetProp(n, "epoch");
   // XXX createrepo stomps epoch zero on packages without epoch, hide
   // them. Rpm treats zero and empty equally anyway so it doesn't matter.
   if (epoch == "0")
      epoch = "";
   return epoch;
}

string RPMRepomdHandler::FileName()
{
   xmlNode *n;
   string str = "";
   if ((n = XmlFindNode(NodeP, "location"))) {
      xmlChar *prop = xmlGetProp(n, (xmlChar*)"href");
      str = basename((char*)prop);
      xmlFree(prop);
   }
   return str;
}

string RPMRepomdHandler::Directory()
{
   xmlNode *n;
   string str = "";
   if ((n = XmlFindNode(NodeP, "location"))) {
      xmlChar *prop = xmlGetProp(n, (xmlChar*)"href");
      if (prop) {
	 str = dirname((char*)prop);
	 xmlFree(prop);
      }
   }
   return str;
}

string RPMRepomdHandler::MD5Sum()
{
   // XXX FIXME the method should be an abstract Checksum type using
   // md5 / sha1 appropriately, for now relying on hacks elsewhere..
   return SHA1Sum();
}

string RPMRepomdHandler::SHA1Sum()
{
   xmlNode *n;
   string str = "";
   if ((n = XmlFindNode(NodeP, "checksum"))) {
      xmlChar *content = xmlNodeGetContent(n);
      str = (char*)content;
      xmlFree(content);
   }
   return str;
}

off_t RPMRepomdHandler::FileSize()
{
   xmlNode *n;
   off_t size = 0;
   if ((n = XmlFindNode(NodeP, "size"))) {
      xmlChar *prop = xmlGetProp(n, (xmlChar*)"package");
      size = atol((char*)prop);
      xmlFree(prop);
   } 
   return size;
}

off_t RPMRepomdHandler::InstalledSize()
{
   xmlNode *n;
   off_t size = 0;
   if ((n = XmlFindNode(NodeP, "size"))) {
      xmlChar *prop = xmlGetProp(n, (xmlChar*)"installed");
      size = atol((char*)prop);
      xmlFree(prop);
   } 
   return size;
}

string RPMRepomdHandler::SourceRpm()
{
   xmlNode *n = XmlFindNode(NodeP, "format");
   return XmlFindNodeContent(n, "sourcerpm");
}

bool RPMRepomdHandler::PRCO(unsigned int Type, vector<Dependency*> &Deps)
{
   xmlNode *format = XmlFindNode(NodeP, "format");
   xmlNode *prco = NULL;

   switch (Type) {
      case pkgCache::Dep::Depends:
         prco = XmlFindNode(format, "requires");
         break;
      case pkgCache::Dep::Conflicts:
         prco = XmlFindNode(format, "conflicts");
         break;
      case pkgCache::Dep::Obsoletes:
         prco = XmlFindNode(format, "obsoletes");
         break;
      case pkgCache::Dep::Provides:
         prco = XmlFindNode(format, "provides");
         break;
   }

   if (! prco) {
      return true;
   }
   for (xmlNode *n = prco->children; n; n = n->next) {
      unsigned int RpmOp = 0;
      string deptype, depver;
      xmlChar *depname, *flags;
      if ((depname = xmlGetProp(n, (xmlChar*)"name")) == NULL) continue;

      if ((flags = xmlGetProp(n, (xmlChar*)"flags"))) {
         deptype = string((char*)flags);
	 xmlFree(flags);

         xmlChar *epoch = xmlGetProp(n, (xmlChar*)"epoch");
         if (epoch) {
            depver += string((char*)epoch) + ":";
	    xmlFree(epoch);
	 }
         xmlChar *ver = xmlGetProp(n, (xmlChar*)"ver");
         if (ver) {
            depver += string((char*)ver);
	    xmlFree(ver);
	 }
         xmlChar *rel = xmlGetProp(n, (xmlChar*)"rel");
         if (rel) {
            depver += "-" + string((char*)rel);
	    xmlFree(rel);
	 }


         if (deptype == "EQ") {
	    RpmOp = RPMSENSE_EQUAL;
	 } else if (deptype == "GE") {
	    RpmOp = RPMSENSE_GREATER | RPMSENSE_EQUAL;
	 } else if (deptype == "GT") {
	    RpmOp = RPMSENSE_GREATER;
	 } else if (deptype == "LE") {
	    RpmOp = RPMSENSE_LESS | RPMSENSE_EQUAL;
	 } else if (deptype == "LT") {
	    RpmOp = RPMSENSE_LESS;
	 } else {
	    // wtf, unknown dependency type?
	    _error->Warning(_("Ignoring unknown dependency type %s"), 
			      deptype.c_str());
	    continue;
	 }
      } else {
	 RpmOp = RPMSENSE_ANY;
      }

      if (Type == pkgCache::Dep::Depends) {
	 xmlChar *pre = xmlGetProp(n, (xmlChar*)"pre"); 
	 if (pre) {
	    RpmOp |= RPMSENSE_PREREQ;
	    xmlFree(pre);
	 }
      }
      PutDep((char*)depname, depver.c_str(), (raptDepFlags) RpmOp, Type, Deps);
      xmlFree(depname);
   }
   return true;
}

// XXX HasFile() usage with repomd with full filelists is slower than
// having the user manually look it up, literally. So we only support the 
// more common files which are stored in primary.xml which supports fast
// random access.
bool RPMRepomdHandler::HasFile(const char *File)
{
   if (*File == '\0')
      return false;
   
   vector<string> Files;
   ShortFileList(Files);
   vector<string>::iterator I = find(Files.begin(), Files.end(), File);
   return (I != Files.end());
}

bool RPMRepomdHandler::ShortFileList(vector<string> &FileList)
{
   xmlNode *format = XmlFindNode(NodeP, "format");
   for (xmlNode *n = format->children; n; n = n->next) {
      if (xmlStrcmp(n->name, (xmlChar*)"file") != 0)  continue;
      xmlChar *Filename = xmlNodeGetContent(n);
      FileList.push_back(string((char*)Filename));
      xmlFree(Filename);
   }
   return true;
}

bool RPMRepomdHandler::FileList(vector<string> &FileList)
{
   RPMRepomdFLHandler *FL = new RPMRepomdFLHandler(FilelistPath);
   bool res = FL->Jump(iOffset);
   res &= FL->FileList(FileList);
   delete FL;
   return res; 
}

bool RPMRepomdHandler::ChangeLog(vector<ChangeLogEntry* > &ChangeLogs)
{
   RPMRepomdOtherHandler *OL = new RPMRepomdOtherHandler(OtherPath);
   bool res = OL->Jump(iOffset);
   res &= OL->ChangeLog(ChangeLogs);
   delete OL;
   return res; 
}

RPMRepomdHandler::~RPMRepomdHandler()
{
   xmlFreeDoc(Primary);
}

RPMRepomdReaderHandler::RPMRepomdReaderHandler(string File) : RPMHandler(),
   XmlFile(NULL), XmlPath(File), NodeP(NULL)
{
   ID = File;
   iOffset = -1;

   if (FileExists(XmlPath)) {
      XmlFile = xmlReaderForFile(XmlPath.c_str(), NULL,
                                  XML_PARSE_NONET|XML_PARSE_NOBLANKS);
      if (XmlFile == NULL) {
        xmlFreeTextReader(XmlFile);
        _error->Error(_("Failed to open filelist index %s"), XmlPath.c_str());
        goto error;
      }

      // seek into first package in xml
      int ret = xmlTextReaderRead(XmlFile);
      if (ret == 1) {
        xmlChar *pkgs = xmlTextReaderGetAttribute(XmlFile, (xmlChar*)"packages");
        iSize = atoi((char*)pkgs);
        xmlFree(pkgs);
      }
      while (ret == 1) {
        if (xmlStrcmp(xmlTextReaderConstName(XmlFile),
                     (xmlChar*)"package") == 0) {
           break;
        }
        ret = xmlTextReaderRead(XmlFile);
      }
   }
   rpmIndexSizes[ID] = iSize;
   return;

error:
   if (XmlFile) {
       xmlFreeTextReader(XmlFile);
   }
}

bool RPMRepomdReaderHandler::Jump(off_t Offset)
{
   bool res = false;
   while (iOffset != Offset) {
      res = Skip();
      if (res == false)
	 break;
   }
      
   return res;
}

void RPMRepomdReaderHandler::Rewind()
{
   // XXX Ignore rewinds when already at start, any other cases we can't
   // handle at the moment. Other cases shouldn't be needed due to usage
   // patterns but just in case...
   if (iOffset != -1) {
      _error->Error(_("Internal error: xmlReader cannot rewind"));
   }
}

bool RPMRepomdReaderHandler::Skip()
{
   if (iOffset +1 >= iSize) {
      return false;
   }
   if (iOffset >= 0) {
      xmlTextReaderNext(XmlFile);
   }
   NodeP = xmlTextReaderExpand(XmlFile);
   iOffset++;

   return true;
}

string RPMRepomdReaderHandler::FindTag(const char *Tag)
{
   string str = "";
   if (NodeP) {
       xmlChar *attr = xmlGetProp(NodeP, (xmlChar*)Tag);
       if (attr) {
          str = (char*)attr;
          xmlFree(attr);
       }
   }
   return str;
}

string RPMRepomdReaderHandler::FindVerTag(const char *Tag)
{
   string str = "";
   for (xmlNode *n = NodeP->children; n; n = n->next) {
      if (xmlStrcmp(n->name, (xmlChar*)"version") != 0)  continue;
      xmlChar *attr = xmlGetProp(n, (xmlChar*)Tag);
      if (attr) {
	 str = (char*)attr;
	 xmlFree(attr);
      }
   }
   return str;
}

RPMRepomdReaderHandler::~RPMRepomdReaderHandler()
{
   xmlFreeTextReader(XmlFile);
}

bool RPMRepomdFLHandler::FileList(vector<string> &FileList)
{
   for (xmlNode *n = NodeP->children; n; n = n->next) {
      if (xmlStrcmp(n->name, (xmlChar*)"file") != 0)  continue;
      xmlChar *Filename = xmlNodeGetContent(n);
      FileList.push_back(string((char*)Filename));
      xmlFree(Filename);
   }
   return true;
}

bool RPMRepomdOtherHandler::ChangeLog(vector<ChangeLogEntry* > &ChangeLogs)
{
   // Changelogs aren't necessarily available at all
   if (! XmlFile) {
      return false;
   }

   for (xmlNode *n = NodeP->children; n; n = n->next) {
      if (xmlStrcmp(n->name, (xmlChar*)"changelog") != 0)  continue;
      ChangeLogEntry *Entry = new ChangeLogEntry;
      xmlChar *Text = xmlNodeGetContent(n);
      xmlChar *Time = xmlGetProp(n, (xmlChar*)"date");
      xmlChar *Author = xmlGetProp(n, (xmlChar*)"author");
      Entry->Text = string((char*)Text);
      Entry->Time = atoi((char*)Time);
      Entry->Author = string((char*)Author);
      ChangeLogs.push_back(Entry);
      xmlFree(Text);
      xmlFree(Time);
      xmlFree(Author);
   }
   return true;
}

#ifdef WITH_SQLITE3
RPMSqliteHandler::RPMSqliteHandler(repomdXML const *repomd) : 
   Primary(NULL), Filelists(NULL), Other(NULL), Packages(NULL)
{
   ID = repomd->ID();
   // Try to figure where in the world our files might be... 
   string base = ID.substr(0, ID.size() - strlen("repomd.xml"));
   DBPath = base + flNotDir(repomd->FindURI("primary_db"));
   FilesDBPath = base + flNotDir(repomd->FindURI("filelists_db"));
   OtherDBPath = base + flNotDir(repomd->FindURI("other_db"));

   Primary = new SqliteDB(DBPath);
   Primary->Exclusive(true);
   // XXX open these only if needed? 
   Filelists = new SqliteDB(FilesDBPath);
   Filelists->Exclusive(true);
   if (FileExists(OtherDBPath)) {
      Other = new SqliteDB(OtherDBPath);
      Other->Exclusive(true);
   }

   Packages = Primary->Query();

   // see if it's a db scheme we support
   SqliteQuery *DBI = Primary->Query();
   DBI->Exec("select * from db_info");
   DBI->Step();
   DBVersion = DBI->GetColI("dbversion");
   delete DBI;
   if (DBVersion < 10) {
      _error->Error(_("Unsupported database scheme (%d)"), DBVersion);
      return;
   } 

   Packages->Exec("select * from packages");
   iSize = Packages->Size();
   rpmIndexSizes[ID] = iSize;
}

RPMSqliteHandler::~RPMSqliteHandler()
{
   if (Primary) delete Primary;
   if (Filelists) delete Filelists;
   if (Other) delete Other;
   if (Packages) delete Packages;
}


bool RPMSqliteHandler::Skip()
{
   bool res = Packages->Step();
   if (res)
      iOffset++;
   return res;
}

bool RPMSqliteHandler::Jump(off_t Offset)
{
   bool res = Packages->Jump(Offset);
   if (!res)
      return false;
   iOffset = Packages->Offset();
   return true;
}

void RPMSqliteHandler::Rewind()
{
   Packages->Rewind();
   iOffset = 0;
}

string RPMSqliteHandler::Name()
{
   return Packages->GetCol("name");
}

string RPMSqliteHandler::Version()
{
   return Packages->GetCol("version");
}

string RPMSqliteHandler::Release()
{
   return Packages->GetCol("release");
}

string RPMSqliteHandler::Epoch()
{
   return Packages->GetCol("epoch");
}

string RPMSqliteHandler::Arch()
{
   return Packages->GetCol("arch");
}

string RPMSqliteHandler::Group()
{
   return Packages->GetCol("rpm_group");
}

string RPMSqliteHandler::Packager()
{
   return Packages->GetCol("rpm_packager");
}
string RPMSqliteHandler::Vendor()
{
   return Packages->GetCol("rpm_vendor");
}

string RPMSqliteHandler::Summary()
{
   return Packages->GetCol("summary");
}

string RPMSqliteHandler::Description()
{
   return Packages->GetCol("description");
}

string RPMSqliteHandler::SourceRpm()
{
   return Packages->GetCol("rpm_sourcerpm");
}

string RPMSqliteHandler::FileName()
{
   return flNotDir(Packages->GetCol("location_href"));
}

string RPMSqliteHandler::Directory()
{
   return flNotFile(Packages->GetCol("location_href"));
}

off_t RPMSqliteHandler::FileSize()
{
   return Packages->GetColI("size_package");
}

off_t RPMSqliteHandler::InstalledSize()
{
   return Packages->GetColI("size_installed");
}

string RPMSqliteHandler::MD5Sum()
{
   return SHA1Sum();
}

string RPMSqliteHandler::SHA1Sum()
{
   return Packages->GetCol("pkgId");
}

bool RPMSqliteHandler::PRCO(unsigned int Type, vector<Dependency*> &Deps)
{
   string what = "";
   switch (Type) {
      case pkgCache::Dep::Depends:
	 what = "requires";
         break;
      case pkgCache::Dep::Conflicts:
	 what = "conflicts";
         break;
      case pkgCache::Dep::Obsoletes:
	 what = "obsoletes";
         break;
      case pkgCache::Dep::Provides:
	 what = "provides";
         break;
   }

   ostringstream sql;
   unsigned long pkgKey = Packages->GetColI("pkgKey");
   sql  << "select * from " << what << " where pkgKey=" << pkgKey << endl;
   SqliteQuery *prco = Primary->Query();
   if (!prco->Exec(sql.str())) {
      delete prco;
      return false;
   }

   while (prco->Step()) {
      unsigned int RpmOp = 0;
      string deptype, depver = "";
      string e, v, r;

      deptype = prco->GetCol("flags");
      if (deptype.empty()) {
	 RpmOp = RPMSENSE_ANY;
      } else {
	 if (deptype == "EQ") {
	    RpmOp = RPMSENSE_EQUAL;
	 } else if (deptype == "GE") {
	    RpmOp = RPMSENSE_GREATER | RPMSENSE_EQUAL;
	 } else if (deptype == "GT") {
	    RpmOp = RPMSENSE_GREATER;
	 } else if (deptype == "LE") {
	    RpmOp = RPMSENSE_LESS | RPMSENSE_EQUAL;
	 } else if (deptype == "LT") {
	    RpmOp = RPMSENSE_LESS;
	 } else {
	    // wtf, unknown dependency type?
	    _error->Warning(_("Ignoring unknown dependency type %s"), 
			      deptype.c_str());
	    continue;
	 }
	 e = prco->GetCol("epoch");
	 v = prco->GetCol("version");
	 r = prco->GetCol("release");
	 if (! e.empty()) {
	    depver += e + ":";
	 }
	 if (! v.empty()) {
	    depver += v;
	 }
	 if (! r.empty()) {
	    depver += "-" + r;
	 }
      }
      string depname = prco->GetCol("name");
      PutDep(depname.c_str(), depver.c_str(), (raptDepFlags) RpmOp, Type, Deps);
   }
   delete prco;
   return true;
}

bool RPMSqliteHandler::FileList(vector<string> &FileList)
{
   ostringstream sql;
   unsigned long pkgKey = Packages->GetColI("pkgKey");
   sql  << "select * from filelist where pkgKey=" << pkgKey << endl;
   SqliteQuery *Files = Filelists->Query();
   if (!Files->Exec(sql.str())) {
      delete Files;
      return false;
   }

   string delimiters = "/";
   while (Files->Step()) {
      string dir = Files->GetCol("dirname");
      string filenames = Files->GetCol("filenames");

      string::size_type lastPos = filenames.find_first_not_of(delimiters, 0);
      string::size_type pos     = filenames.find_first_of(delimiters, lastPos);

      while (string::npos != pos || string::npos != lastPos)
      {
	 FileList.push_back(dir + "/" + filenames.substr(lastPos, pos - lastPos));
	 lastPos = filenames.find_first_not_of(delimiters, pos);
	 pos = filenames.find_first_of(delimiters, lastPos);
      } 
   }
   delete Files;
   return true;
}

bool RPMSqliteHandler::ChangeLog(vector<ChangeLogEntry* > &ChangeLogs)
{
   ostringstream sql;
   unsigned long pkgKey = Packages->GetColI("pkgKey");
   sql  << "select * from changelog where pkgKey=" << pkgKey << endl;
   if (! Other) {
      return false;
   }

   SqliteQuery *Changes = Other->Query();
   if (!Changes->Exec(sql.str())) {
      delete Changes;
      return false;
   }

   while (Changes->Step()) {
      ChangeLogEntry *Entry = new ChangeLogEntry;
      Entry->Time = Changes->GetColI("date");
      Entry->Author = Changes->GetCol("author");
      Entry->Text = Changes->GetCol("changelog");
      ChangeLogs.push_back(Entry);
   }
   delete Changes;
   return true;
}
#endif /* WITH_SQLITE3 */

#endif /* APT_WITH_REPOMD */


// vim:sts=3:sw=3
