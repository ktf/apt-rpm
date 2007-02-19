
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
#include <assert.h>
#include <libgen.h>

#include <apt-pkg/error.h>
#include <apt-pkg/configuration.h>
#include <apt-pkg/md5.h>
#include <apt-pkg/crc-16.h>

#include <apt-pkg/rpmhandler.h>
#include <apt-pkg/rpmpackagedata.h>

#ifdef APT_WITH_REPOMD
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlreader.h>
#include <sstream>
#include <apt-pkg/sqlite.h>
#endif

#include <apti18n.h>

#if RPM_VERSION >= 0x040100
#include <rpm/rpmts.h>
#include <rpm/rpmdb.h>
#include <rpm/rpmds.h>
#include <rpm/rpmfi.h>
#define rpmxxInitIterator(a,b,c,d) rpmtsInitIterator(a,(rpmTag)b,c,d)
#else
#define rpmxxInitIterator(a,b,c,d) rpmdbInitIterator(a,b,c,d)
#endif

// An attempt to deal with false zero epochs from repomd. With older rpm's we
// can only blindly trust the repo admin created the repository with options
// suitable for those versions. For rpm >= 4.2.1 this is linked with
// promoteepoch behavior - if promoteepoch is used then epoch hiding must
// not happen.
bool HideZeroEpoch;

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

unsigned int RPMHandler::DepOp(int_32 rpmflags)
{
   unsigned int Op = 0;
   int_32 flags = (rpmflags & RPMSENSE_SENSEMASK);
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
   for (vector<string>::iterator I = Files.begin(); I != Files.end(); I++) {
      if (string(File) == (*I)) {
	 return true;
      }
   }
   return false;
}

bool RPMHandler::InternalDep(const char *name, const char *ver, int_32 flag) 
{
   if (strncmp(name, "rpmlib(", strlen("rpmlib(")) == 0) {
#if RPM_VERSION >= 0x040404
     rpmds rpmlibProv = NULL;
     rpmds ds = rpmdsSingle(RPMTAG_PROVIDENAME,
			    name, ver?ver:NULL, flag);
     rpmdsRpmlib(&rpmlibProv, NULL);
     int res = rpmdsSearch(rpmlibProv, ds) >= 0;
     rpmdsFree(ds);
     rpmdsFree(rpmlibProv);
#elif RPM_VERSION >= 0x040100
      rpmds ds = rpmdsSingle(RPMTAG_PROVIDENAME,
			     name, ver?ver:NULL, flag);
      int res = rpmCheckRpmlibProvides(ds);
      rpmdsFree(ds);
#else
      int res = rpmCheckRpmlibProvides(name, ver?ver:NULL,
				       flag);
#endif
      if (res) 
	 return true;
   }

#if RPM_VERSION >= 0x040404
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

bool RPMHandler::PutDep(const char *name, const char *ver, int_32 flags, 
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
   char str[512] = "";
   int_32 count, type, *epoch;
   void *val;
   assert(HeaderP != NULL);
   int rc = headerGetEntry(HeaderP, RPMTAG_EPOCH, &type, &val, &count);
   epoch = (int_32*)val;
   if (rc == 1 && count > 0) {
      snprintf(str, sizeof(str), "%i", epoch[0]);
   }
   return string(str);
}

unsigned long RPMHdrHandler::GetITag(rpmTag Tag)
{
   int_32 count, type, *num;
   void *val;
   assert(HeaderP != NULL);
   int rc = headerGetEntry(HeaderP, Tag,
			   &type, (void**)&val, &count);
   num = (int_32*)val;
   return rc?num[0]:0;
}

string RPMHdrHandler::GetSTag(rpmTag Tag)
{
   char *str;
   void *val;
   int_32 count, type;
   assert(HeaderP != NULL);
   int rc = headerGetEntry(HeaderP, Tag,
			   &type, (void**)&val, &count);
   str = (char *)val;
   return string(rc?str:"");
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
	 deptype = RPMTAG_SUGGESTNAME;
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
	 bool r = PutDep(rpmdsN(ds), rpmdsEVR(ds), rpmdsFlags(ds), Type, Deps);
      }
   }
   rpmdsFree(ds);
   return true;
}
#else
bool RPMHdrHandler::PRCO(unsigned int Type, vector<Dependency*> &Deps)
{
   char **namel = NULL;
   char **verl = NULL;
   int *flagl = NULL;
   int res, type, count;
   int_32 deptag, depver, depflags;
   void *nameval = NULL;
   void *verval = NULL;
   void *flagval = NULL;

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
   res = headerGetEntry(HeaderP, deptag, &type, (void **)&nameval, &count);
   if (res != 1)
      return true;
   res = headerGetEntry(HeaderP, depver, &type, (void **)&verval, &count);
   res = headerGetEntry(HeaderP, depflags, &type, (void **)&flagval, &count);

   namel = (char**)nameval;
   verl = (char**)verval;
   flagl = (int*)flagval;

   for (int i = 0; i < count; i++) {

      bool res = PutDep(namel[i], verl[i], flagl[i], Type, Deps);
   }
   free(namel);
   free(verl);
   return true;
      
}
#endif

// XXX rpmfi originates from somewhere around 2001 but what's the version?
#if RPM_VERSION >= 0x040100
bool RPMHdrHandler::FileList(vector<string> &FileList)
{
   rpmfi fi = NULL;
   fi = rpmfiNew(NULL, HeaderP, RPMTAG_BASENAMES, 0);
   if (fi != NULL) {
      while (rpmfiNext(fi) >= 0) {
        FileList.push_back(rpmfiFN(fi));
      }
   }
   fi = rpmfiFree(fi);
   return true;
}
#else
bool RPMHdrHandler::FileList(vector<string> &FileList)
{
   const char **names = NULL;
   void *val = NULL;
   int_32 count = 0;
   bool ret = true;
   rpmHeaderGetEntry(HeaderP, RPMTAG_OLDFILENAMES,
                     NULL, (void **) &val, &count);
   names = (const char **)val;
   while (count--) {
      FileList.push_back(names[count]);
   }
   free(names);
   return ret;

}
#endif

bool RPMHdrHandler::ChangeLog(vector<ChangeLogEntry *> &ChangeLogs)
{
   int *timel = NULL;
   char **authorl = NULL;
   char **entryl = NULL;
   void *timeval, *authorval, *entryval;
   int res, type, count;

   res = headerGetEntry(HeaderP, RPMTAG_CHANGELOGTIME, &type, (void **)&timeval, &count);
   res = headerGetEntry(HeaderP, RPMTAG_CHANGELOGNAME, &type, (void **)&authorval, &count);
   res = headerGetEntry(HeaderP, RPMTAG_CHANGELOGTEXT, &type, (void **)&entryval, &count);

   timel = (int*)timeval;
   authorl = (char**)authorval;
   entryl = (char**)entryval;

   for (int i = 0; i < count; i++) {
      ChangeLogEntry *Entry = new ChangeLogEntry;
      Entry->Time = timel[i];
      Entry->Author = authorl[i];
      Entry->Text = entryl[i];
      ChangeLogs.push_back(Entry);
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

unsigned long RPMFileHandler::FileSize()
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

unsigned long RPMSingleFileHandler::FileSize()
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

unsigned long RPMDirHandler::FileSize()
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
   string Dir = _config->Find("RPM::RootDir");
   
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
   RpmIter = rpmxxInitIterator(Handler, RPMDBI_PACKAGES, NULL, 0);
   if (RpmIter == NULL) {
      _error->Error(_("could not create RPM database iterator"));
      return;
   }
   // iSize = rpmdbGetIteratorCount(RpmIter);
   // This doesn't seem to work right now. Code in rpm (4.0.4, at least)
   // returns a 0 from rpmdbGetIteratorCount() if rpmxxInitIterator() is
   // called with RPMDBI_PACKAGES or with keyp == NULL. The algorithm
   // below will be used until there's support for it.
   iSize = 0;
   rpmdbMatchIterator countIt;
   countIt = rpmxxInitIterator(Handler, RPMDBI_PACKAGES, NULL, 0);
   while (rpmdbNextIterator(countIt) != NULL)
      iSize++;
   rpmdbFreeIterator(countIt);
#else
   iSize = St.st_size;

#endif


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

   if (Handler != NULL) {
#if RPM_VERSION >= 0x040100
      rpmtsFree(Handler);
#else
      rpmdbClose(Handler);
#endif
   }

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
   uint_32 rpmOffset = iOffset;
   if (RpmIter == NULL)
      return false;
   rpmdbFreeIterator(RpmIter);
   if (iOffset == 0)
      RpmIter = rpmxxInitIterator(Handler, RPMDBI_PACKAGES, NULL, 0);
   else {
      RpmIter = rpmxxInitIterator(Handler, RPMDBI_PACKAGES,
				  &rpmOffset, sizeof(rpmOffset));
      iOffset = rpmOffset;
   }
   HeaderP = rpmdbNextIterator(RpmIter);
#else
   HeaderP = rpmdbGetRecord(Handler, iOffset);
#endif
   return true;
}

bool RPMDBHandler::JumpByName(string PkgName)
{
   if (RpmIter == NULL) return false;
   rpmdbFreeIterator(RpmIter);
#if RPM_VERSION >= 0x040100
   RpmIter = rpmtsInitIterator(Handler, (rpmTag)RPMDBI_LABEL, PkgName.c_str(), 0);
#else
   RpmIter = rpmdbInitIterator(Handler, RPMDBI_LABEL, PkgName.c_str(), 0);
#endif

   HeaderP = rpmdbNextIterator(RpmIter);
   return (HeaderP != NULL);
}

void RPMDBHandler::Rewind()
{
#if RPM_VERSION >= 0x040000
   if (RpmIter == NULL)
      return;
   rpmdbFreeIterator(RpmIter);   
   RpmIter = rpmxxInitIterator(Handler, RPMDBI_PACKAGES, NULL, 0);
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
RPMRepomdHandler::RPMRepomdHandler(string File)
{
   PrimaryFile = File;

   ID = File;
   Root = NULL;
   Primary = NULL;
   xmlChar *packages = NULL;
   off_t pkgcount = 0;
   

   Primary = xmlReadFile(File.c_str(), NULL, XML_PARSE_NONET|XML_PARSE_NOBLANKS);
   if ((Root = xmlDocGetRootElement(Primary)) == NULL) {
      _error->Error(_("Failed to open package index %s"), PrimaryFile.c_str());
      goto error;
   }
   if (xmlStrncmp(Root->name, (xmlChar*)"metadata", strlen("metadata")) != 0) {
      _error->Error(_("Corrupted package index %s"), PrimaryFile.c_str());
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
      _error->Warning(_("Inconsistent metadata, package count doesn't match in %s"), File.c_str());
      iSize = pkgcount;
   }

   return;

error:
   if (Primary) {
      xmlFreeDoc(Primary);
   }
}

bool RPMRepomdHandler::Skip()
{
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

xmlNode *RPMRepomdHandler::FindNode(const string Name)
{
   for (xmlNode *n = NodeP->children; n; n = n->next) {
      if (xmlStrcmp(n->name, (xmlChar*)Name.c_str()) == 0) {
         return n;
      }
   }
   return NULL;
}

xmlNode *RPMRepomdHandler::FindNode(xmlNode *Node, const string Name)
{
   for (xmlNode *n = Node->children; n; n = n->next) {
      if (xmlStrcmp(n->name, (xmlChar*)Name.c_str()) == 0) {
         return n;
      }
   }
   return NULL;
}

string RPMRepomdHandler::FindTag(xmlNode *Node, string Tag)
{
   xmlNode *n = FindNode(Node, Tag);
   string str = "";
   if (n) {
      xmlChar *content = xmlNodeGetContent(n);
      if (content) {
	 str = (char*)content;
	 xmlFree(content);
      }
   }
   return str;
}

string RPMRepomdHandler::GetProp(xmlNode *Node, char *Prop)
{
   string str = "";
   if (Node) {
      xmlChar *prop = xmlGetProp(Node, (xmlChar*)Prop);
      if (prop) {
	 str = (char*)prop;
	 xmlFree(prop);
      }
   }
   return str;
}

string RPMRepomdHandler::Group()
{
   xmlNode *n = FindNode("format");
   return FindTag(n, "group");
}

string RPMRepomdHandler::Vendor()
{
   xmlNode *n = FindNode("format");
   return FindTag(n, "vendor");
}

string RPMRepomdHandler::Release()
{
   xmlNode *n = FindNode("version");
   return GetProp(n, "rel");
}

string RPMRepomdHandler::Version()
{
   xmlNode *n = FindNode("version");
   return GetProp(n, "ver");
}

string RPMRepomdHandler::Epoch()
{
   string epoch;
   xmlNode *n = FindNode("version");
   epoch = GetProp(n, "epoch");
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
   if ((n = FindNode("location"))) {
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
   if ((n = FindNode("location"))) {
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
   if ((n = FindNode("checksum"))) {
      xmlChar *content = xmlNodeGetContent(n);
      str = (char*)content;
      xmlFree(content);
   }
   return str;
}
unsigned long RPMRepomdHandler::FileSize()
{
   xmlNode *n;
   unsigned long size = 0;
   if ((n = FindNode("size"))) {
      xmlChar *prop = xmlGetProp(n, (xmlChar*)"package");
      size = atol((char*)prop);
      xmlFree(prop);
   } 
   return size;
}

unsigned long RPMRepomdHandler::InstalledSize()
{
   xmlNode *n;
   unsigned long size = 0;
   if ((n = FindNode("size"))) {
      xmlChar *prop = xmlGetProp(n, (xmlChar*)"installed");
      size = atol((char*)prop);
      xmlFree(prop);
   } 
   return size;
}

string RPMRepomdHandler::SourceRpm()
{
   xmlNode *n = FindNode("format");
   return FindTag(n, "sourcerpm");
}

bool RPMRepomdHandler::PRCO(unsigned int Type, vector<Dependency*> &Deps)
{
   xmlNode *format = FindNode("format");
   xmlNode *dco = NULL;

   switch (Type) {
      case pkgCache::Dep::Depends:
         dco = FindNode(format, "requires");
         break;
      case pkgCache::Dep::Conflicts:
         dco = FindNode(format, "conflicts");
         break;
      case pkgCache::Dep::Obsoletes:
         dco = FindNode(format, "obsoletes");
         break;
      case pkgCache::Dep::Provides:
         dco = FindNode(format, "provides");
         break;
   }

   if (! dco) {
      return true;
   }
   for (xmlNode *n = dco->children; n; n = n->next) {
      int_32 RpmOp = 0;
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
	    // erm, unknown dependency type?
	    return false;
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
      bool res = PutDep((char*)depname, depver.c_str(), RpmOp, Type, Deps);
   }
   return true;
}

bool RPMRepomdHandler::FileList(vector<string> &FileList)
{
   xmlNode *format = FindNode("format");
   for (xmlNode *n = format->children; n; n = n->next) {
      if (xmlStrcmp(n->name, (xmlChar*)"file") != 0)  continue;
      xmlChar *Filename = xmlNodeGetContent(n);
      FileList.push_back(string((char*)Filename));
      xmlFree(Filename);
   }
   return true;
}

RPMRepomdHandler::~RPMRepomdHandler()
{
   xmlFreeDoc(Primary);
}

RPMRepomdFLHandler::RPMRepomdFLHandler(string File) : RPMHandler()
{
   FilelistFile = File.substr(0, File.size() - 11) + "filelists.xml";

   ID = File;
   Filelist = NULL;
   NodeP = NULL;
   iOffset = -1;

   if (FileExists(FilelistFile)) {
      Filelist = xmlReaderForFile(FilelistFile.c_str(), NULL,
                                  XML_PARSE_NONET|XML_PARSE_NOBLANKS);
      if (Filelist == NULL) {
        xmlFreeTextReader(Filelist);
        _error->Error(_("Failed to open filelist index %s"), FilelistFile.c_str());
        goto error;
      }

      // seek into first package in filelists.xml
      int ret = xmlTextReaderRead(Filelist);
      if (ret == 1) {
        xmlChar *pkgs = xmlTextReaderGetAttribute(Filelist, (xmlChar*)"packages");
        iSize = atoi((char*)pkgs);
        xmlFree(pkgs);
      }
      while (ret == 1) {
        if (xmlStrcmp(xmlTextReaderConstName(Filelist),
                     (xmlChar*)"package") == 0) {
           break;
        }
        ret = xmlTextReaderRead(Filelist);
      }
   }
   return;

error:
   if (Filelist) {
       xmlFreeTextReader(Filelist);
   }
}

bool RPMRepomdFLHandler::Jump(off_t Offset)
{
   //cerr << "RepomdFLHandler::Jump() called but not implemented!" << endl;
   return false;
}

void RPMRepomdFLHandler::Rewind()
{
   //cerr << "RepomdFLHandler::Rewind() called but not implemented!" << endl;
}

bool RPMRepomdFLHandler::Skip()
{
   if (iOffset +1 >= iSize) {
      return false;
   }
   if (iOffset >= 0) {
      xmlTextReaderNext(Filelist);
   }
   NodeP = xmlTextReaderExpand(Filelist);
   iOffset++;

   return true;
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

string RPMRepomdFLHandler::FindTag(char *Tag)
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

RPMRepomdFLHandler::~RPMRepomdFLHandler()
{
   xmlFreeTextReader(Filelist);
}

RPMSqliteHandler::RPMSqliteHandler(string File) : 
   Primary(NULL), Filelists(NULL), Other(NULL)
{
   int rc = 0;
   char **res = NULL;
   int nrow, ncol = 0;
   string sql;
   

   ID = File;
   DBPath = File; 
   // ugh, pass this in to the constructor or something..
   string DBBase = File.substr(0, File.size() - 14);
   FilesDBPath = DBBase + "filelists.sqlite";
   OtherDBPath = DBBase + "other.sqlite";

   Primary = new SqliteDB(DBPath);
   // XXX open these only if needed? 
   Filelists = new SqliteDB(FilesDBPath);
   if (FileExists(OtherDBPath)) {
      Other = new SqliteDB(OtherDBPath);
   }

   Packages = Primary->Query();

   // XXX without these indexes cache generation will take minutes.. ick
   Packages->Exec("create index requireIdx on requires (pkgKey)");
   Packages->Exec("create index provideIdx on provides (pkgKey)");
   Packages->Exec("create index obsoleteIdx on obsoletes (pkgKey)");
   Packages->Exec("create index conflictIdx on conflicts (pkgKey)");

   Packages->Exec("select * from packages");
   iSize = Packages->Size();
}

RPMSqliteHandler::~RPMSqliteHandler()
{
   if (Primary) delete Primary;
   if (Filelists) delete Filelists;
   if (Other) delete Other;
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

unsigned long RPMSqliteHandler::FileSize()
{
   return Packages->GetColI("size_package");
}

unsigned long RPMSqliteHandler::InstalledSize()
{
   return Packages->GetColI("size_installed");
}

string RPMSqliteHandler::MD5Sum()
{
   return SHA1Sum();
}

string RPMSqliteHandler::SHA1Sum()
{
   return Packages->GetCol("checksum_value");
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
      return false;
   }

   while (prco->Step()) {
      int_32 RpmOp = 0;
      string deptype, depver = "";

      if (prco->GetCol("flags").empty()) {
	 RpmOp == RPMSENSE_ANY;
      } else {
	 deptype = prco->GetCol("flags");
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
	    // erm, unknown dependency type?
	    cout << "unknown dep!? " << deptype << endl;
	    delete prco;
	    return false;
	 }
	 if (! prco->GetCol("epoch").empty()) {
	    depver += prco->GetCol("epoch") + ":";
	 }
	 if (! prco->GetCol("version").empty()) {
	    depver += prco->GetCol("version");
	 }
	 if (! prco->GetCol("release").empty()) {
	    depver += "-" + prco->GetCol("release");
	 }
      }
      string depname = prco->GetCol("name");
      bool res = PutDep(depname.c_str(), depver.c_str(), RpmOp, Type, Deps);
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
   SqliteQuery *Changes = Other->Query();
   if (!Changes->Exec(sql.str())) {
      return false;
   }

   while (Changes->Step()) {
      ChangeLogEntry *Entry = new ChangeLogEntry;
      Entry->Time = Changes->GetColI("date");
      Entry->Author = Changes->GetCol("author");
      Entry->Text = Changes->GetCol("changelog");
      ChangeLogs.push_back(Entry);
   }
   return true;
}


#endif /* APT_WITH_REPOMD */


// vim:sts=3:sw=3
