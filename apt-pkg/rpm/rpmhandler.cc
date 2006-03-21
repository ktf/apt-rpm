
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
#include <libxml/parser.h>
#include <libxml/tree.h>

#include <apti18n.h>

#if RPM_VERSION >= 0x040100
#include <rpm/rpmts.h>
#include <rpm/rpmdb.h>
#include <rpm/rpmds.h>
#define rpmxxInitIterator(a,b,c,d) rpmtsInitIterator(a,(rpmTag)b,c,d)
#else
#define rpmxxInitIterator(a,b,c,d) rpmdbInitIterator(a,b,c,d)
#endif

string RPMHandler::Epoch()
{
   char str[512];
   int_32 count, type, *epoch;
   assert(HeaderP != NULL);
   int rc = headerGetEntry(HeaderP, RPMTAG_EPOCH,
			   &type, (void**)&epoch, &count);
   if (rc == 1 && count > 0) {
      snprintf(str, sizeof(str), "%i", epoch[0]);
      return str;
   } else { 
      return string(rc?str:"");
   }
}

unsigned long RPMHandler::GetITag(rpmTag Tag)
{
   char *str;
   int_32 count, type, *num;
   assert(HeaderP != NULL);
   int rc = headerGetEntry(HeaderP, Tag,
			   &type, (void**)&num, &count);
   return rc?num[0]:0;
}

string RPMHandler::GetSTag(rpmTag Tag)
{
   char *str;
   int_32 count, type;
   assert(HeaderP != NULL);
   int rc = headerGetEntry(HeaderP, Tag,
			   &type, (void**)&str, &count);
   return string(rc?str:"");
}

bool RPMHandler::HasFile(const char *File)
{
   if (*File == '\0')
      return false;
   char **names = NULL;
   int_32 count = 0;
   rpmHeaderGetEntry(HeaderP, RPMTAG_OLDFILENAMES,
                     NULL, (void **) &names, &count);
   while (count--)
   {
      char *name = names[count];
      if (strcmp(name, File) == 0)
         return true;
   }
   free(names);
   return false;
}

bool RPMHandler::InternalDep(const char *name, const char *ver, int_32 flag) 
{
   if (strncmp(name, "rpmlib(", sizeof("rpmlib(")-1) == 0) {
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
   if (strncmp(name, "getconf(", sizeof("getconf(")-1) == 0)
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

   if (strncmp(name, "cpuinfo(", sizeof("cpuinfo(")-1) == 0)
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

   if (strncmp(name, "sysinfo(", sizeof("sysinfo(")-1) == 0)
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

   if (strncmp(name, "uname(", sizeof("uname(")-1) == 0)
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
	 strncmp(name, "exists(", sizeof("exists(")-1) == 0 ||
	 strncmp(name, "executable(", sizeof("executable(")-1) == 0 ||
	 strncmp(name, "readable(", sizeof("readable(")-1) == 0 ||
	 strncmp(name, "writable(", sizeof("writable(")-1)== 0 ))
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
   if (strncmp(name, "soname(", sizeof("soname(")-1) == 0)
   {
      cout << "FIXME, ignoring soname() dependency: " << name << endl;
      return true;
   }
#endif
   return false; 
}

bool RPMHandler::Depends(unsigned int Type, vector<Dependency*> &Deps)
{
   char **namel = NULL;
   char **verl = NULL;
   int *flagl = NULL;
   int res, type, count;

   switch (Type)
   {
   case pkgCache::Dep::Depends:
      res = headerGetEntry(HeaderP, RPMTAG_REQUIRENAME, &type,
                           (void **)&namel, &count);
      if (res != 1)
          return true;
      res = headerGetEntry(HeaderP, RPMTAG_REQUIREVERSION, &type,
                           (void **)&verl, &count);
      res = headerGetEntry(HeaderP, RPMTAG_REQUIREFLAGS, &type,
                           (void **)&flagl, &count);
      break;

   case pkgCache::Dep::Obsoletes:
      res = headerGetEntry(HeaderP, RPMTAG_OBSOLETENAME, &type,
                           (void **)&namel, &count);
      if (res != 1)
          return true;
      res = headerGetEntry(HeaderP, RPMTAG_OBSOLETEVERSION, &type,
                           (void **)&verl, &count);
      res = headerGetEntry(HeaderP, RPMTAG_OBSOLETEFLAGS, &type,
                           (void **)&flagl, &count);
      break;
   case pkgCache::Dep::Conflicts:
      res = headerGetEntry(HeaderP, RPMTAG_CONFLICTNAME, &type,
                           (void **)&namel, &count);
      if (res != 1)
          return true;
      res = headerGetEntry(HeaderP, RPMTAG_CONFLICTVERSION, &type,
                           (void **)&verl, &count);
      res = headerGetEntry(HeaderP, RPMTAG_CONFLICTFLAGS, &type,
                           (void **)&flagl, &count);
      break;
#if RPM_VERSION >= 0x040403
   case pkgCache::Dep::Suggests:
      res = headerGetEntry(HeaderP, RPMTAG_SUGGESTSNAME, &type,
                           (void **)&namel, &count);
      if (res != 1)
          return true; 
      res = headerGetEntry(HeaderP, RPMTAG_SUGGESTSVERSION, &type,
                           (void **)&verl, &count);
      res = headerGetEntry(HeaderP, RPMTAG_SUGGESTSFLAGS, &type,
                           (void **)&flagl, &count);
      break;
#if 0 // Enhances is not even known to apt, sigh...
   case pkgCache::Dep::Enhances:
      res = headerGetEntry(HeaderP, RPMTAG_ENHANCESNAME, &type,
                           (void **)&namel, &count);
      if (res != 1)
          return true;
      res = headerGetEntry(HeaderP, RPMTAG_ENHANCESVERSION, &type,
                           (void **)&verl, &count);
      res = headerGetEntry(HeaderP, RPMTAG_ENHANCESFLAGS, &type,
                           (void **)&flagl, &count);
      break;
#endif
#endif
   }

   unsigned int Op = 0;
   bool DepMode = false;
   if (Type == pkgCache::Dep::Depends)
      DepMode = true;

   for (int i = 0; i < count; i++) {

      if (InternalDep(namel[i], verl[i] ? verl[i]:"", flagl[i]) == true) {
	 continue;
      }
      if (DepMode == true) {
         if (flagl[i] & RPMSENSE_PREREQ)
            Type = pkgCache::Dep::PreDepends;
#if RPM_VERSION >= 0x040403
         else if (flagl[i] & RPMSENSE_MISSINGOK)
            Type = pkgCache::Dep::Suggests;
#endif
         else
            Type = pkgCache::Dep::Depends;
      }

      Dependency *Dep = new Dependency;
      Dep->Name = namel[i];
      Dep->Version = verl[i] ? verl[i]:"";
      if (!verl[i]) {
         Op = pkgCache::Dep::NoOp;
      } else {
         if (flagl[i] & RPMSENSE_LESS) {
            if (flagl[i] & RPMSENSE_EQUAL)
                Op = pkgCache::Dep::LessEq;
            else
                Op = pkgCache::Dep::Less;
         } else if (flagl[i] & RPMSENSE_GREATER) {
            if (flagl[i] & RPMSENSE_EQUAL)
                Op = pkgCache::Dep::GreaterEq;
            else
                Op = pkgCache::Dep::Greater;
         } else if (flagl[i] & RPMSENSE_EQUAL) {
            Op = pkgCache::Dep::Equals;
	 }
      }
      Dep->Op = Op;
      Dep->Type = Type;
      Deps.push_back(Dep);
   }
   free(namel);
   free(verl);
   return true;
      
}

bool RPMHandler::Provides(vector<Dependency*> &Provs)
{
   int type, count;
   char **namel = NULL;
   char **verl = NULL;
   int res;

   res = headerGetEntry(HeaderP, RPMTAG_PROVIDENAME, &type,
                        (void **)&namel, &count);
   if (res != 1)
       return true;

   res = headerGetEntry(HeaderP, RPMTAG_PROVIDEVERSION, &type,
                        (void **)&verl, NULL);

   if (res != 1)
      verl = NULL;

   for (int i = 0; i < count; i++) {
      Dependency *Dep = new Dependency;
      Dep->Name = namel[i];
      if (verl) {
	 Dep->Version = *verl[i] ? verl[i]:"";
	 Dep->Op = pkgCache::Dep::Equals;
      } else {
	 Dep->Version = "";
      }
      Provs.push_back(Dep);
   }
   return true;

}

bool RPMHandler::FileProvides(vector<string> &FileProvs)
{
   const char **names = NULL;
   int_32 count = 0;
   bool ret = true;
   rpmHeaderGetEntry(HeaderP, RPMTAG_OLDFILENAMES,
                     NULL, (void **) &names, &count);
   while (count--) {
      FileProvs.push_back(names[count]);
   }
   free(names);
   return ret;

}

unsigned short RPMHandler::VersionHash()
{
   int Sections[] = {
          RPMTAG_VERSION,
          RPMTAG_RELEASE,
          RPMTAG_ARCH,
          RPMTAG_REQUIRENAME,
          RPMTAG_OBSOLETENAME,
          RPMTAG_CONFLICTNAME,
          0
   };
   unsigned long Result = INIT_FCS;

   for (const int *sec = Sections; *sec != 0; sec++)
   {
      char *Str;
      int Len;
      int type, count;
      int res;
      char **strings = NULL;

      res = headerGetEntry(HeaderP, *sec, &type, (void **)&strings, &count);
      if (res != 1)
         continue;

      switch (type)
      {
      case RPM_STRING_ARRAY_TYPE:
         //qsort(strings, count, sizeof(char*), compare);
         while (count-- > 0)
         {
            Str = strings[count];
            Len = strlen(Str);
            /* Suse patch.rpm hack. */
            if (Len == 17 && *Str == 'r' && *sec == RPMTAG_REQUIRENAME &&
                strcmp(Str, "rpmlib(PatchRPMs)") == 0)
               continue;

            Result = AddCRC16(Result,Str,Len);
         }
         free(strings);
         break;

      case RPM_STRING_TYPE:
         Str = (char*)strings;
         Len = strlen(Str);
         Result = AddCRC16(Result,Str,Len);
         break;
      }
   }

   return Result;
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

bool RPMFileHandler::Jump(unsigned Offset)
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

bool RPMSingleFileHandler::Jump(unsigned Offset)
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

bool RPMDirHandler::Jump(unsigned Offset)
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
   : WriteLock(WriteLock), Handler(0)
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
   if (rpmtsOpenDB(Handler, O_RDONLY) != 0)
   {
      _error->Error(_("could not open RPM database"));
      return;
   }
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

bool RPMDBHandler::Jump(unsigned int Offset)
{
   iOffset = Offset;
#if RPM_VERSION >= 0x040000
   if (RpmIter == NULL)
      return false;
   rpmdbFreeIterator(RpmIter);
   if (iOffset == 0)
      RpmIter = rpmxxInitIterator(Handler, RPMDBI_PACKAGES, NULL, 0);
   else
      RpmIter = rpmxxInitIterator(Handler, RPMDBI_PACKAGES,
				  &iOffset, sizeof(iOffset));
   HeaderP = rpmdbNextIterator(RpmIter);
#else
   HeaderP = rpmdbGetRecord(Handler, iOffset);
#endif
   return true;
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

RPMRepomdHandler::RPMRepomdHandler(string File)
{
   //cout << "Repomd handler constr. " << File << endl;
   string FilelistFile;
   ID = File;
   Root = NULL;
   FlRoot = NULL;

   FilelistFile = File.substr(0, File.size() - 11) + "filelists.xml";
   if (FileExists(FilelistFile)) {
      Filelist = xmlReadFile(FilelistFile.c_str(), NULL, XML_PARSE_NONET);
      if ((FlRoot = xmlDocGetRootElement(Filelist)) == NULL ) {
	 xmlFreeDoc(Primary);
	 cout << "getting root element failed" << endl;
      } 
   } else {
      cout << "no filelist " << FilelistFile << endl;
   }

   Primary = xmlReadFile(File.c_str(), NULL, XML_PARSE_NONET);
   if ((Root = xmlDocGetRootElement(Primary)) == NULL) {
      xmlFreeDoc(Primary);
      cout << "getting root element failed" << endl;
   }
   NodeP = Root->children;
   FlNodeP = FlRoot->children;
   iSize = atoi((char*)xmlGetProp(Root, (xmlChar*)"packages"));

   if (NodeP == NULL)
      cout << "NodeP is null, ugh..." << endl;
}

bool RPMRepomdHandler::Skip()
{
   //iOffset++;
   //cout << "Repomd handler skip, offset " << iOffset << endl;
   for (NodeP = NodeP->next; NodeP; NodeP = NodeP->next) {
      //cout << "skip() current  " << NodeP << endl;
      FlNodeP = FlNodeP->next;
      if (NodeP->type != XML_ELEMENT_NODE || strcmp((char*)NodeP->name, "package") != 0) {
	 continue;
      } else {
	 iOffset++;
	 //cout << "in node " << xmlNodePGetContent(node) << endl;
	 return true;
      }
   } 
   return false;
   
}

bool RPMRepomdHandler::Jump(unsigned int Offset)
{
   NodeP = Root->children;
   FlNodeP = FlRoot->children;
   iOffset = 0;
   for (NodeP = NodeP->next; NodeP; NodeP = NodeP->next) {
      FlNodeP = FlNodeP->next;
      if (iOffset+1 == Offset) {
	 return Skip();
      }
      if (NodeP->type != XML_ELEMENT_NODE || 
	  strcmp((char*)NodeP->name, "package") != 0) {
	 continue;
      } else {
	 iOffset++;
	 //cout << "at offset " << iOffset << " of " << Offset << endl;
      }
   }
   return false;
}

void RPMRepomdHandler::Rewind()
{
   //cout << "Repomd handler rewind" << endl;
   iOffset = 0;
   NodeP = Root->children;
   FlNodeP = FlRoot->children;
}

xmlNode *RPMRepomdHandler::FindNode(const string Name)
{
   for (xmlNode *n = NodeP->children; n; n = n->next) {
      if (strcmp((char*)n->name, Name.c_str()) == 0) {
         return n;
      }
   }
   return NULL;
}

xmlNode *RPMRepomdHandler::FindNode(xmlNode *Node, const string Name)
{
   for (xmlNode *n = Node->children; n; n = n->next) {
      if (strcmp((char*)n->name, Name.c_str()) == 0) {
         return n;
      }
   }
   return NULL;
}

string RPMRepomdHandler::FindTag(xmlNode *Node, string Tag)
{
   xmlNode *n = FindNode(Node, Tag);
   if (n) {
      return (char*)xmlNodeGetContent(n);
   } else {
      return "";
   }
}

string RPMRepomdHandler::GetContent(xmlNode *Node, string Tag)
{
   if (Node) {
      return (char*)xmlNodeGetContent(Node);
   } else {
      return "";
   }
}

string RPMRepomdHandler::GetProp(xmlNode *Node, char *Prop)
{
   if (Node) {
      return (char*)xmlGetProp(Node, (xmlChar*)Prop);
   } else {
      return "";
   }
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
   if ((n = FindNode("location"))) {
      return basename((char*)xmlGetProp(n, (xmlChar*)"href"));
   } else {
      return "";
   }
}

string RPMRepomdHandler::Directory()
{
   xmlNode *n;
   char *dir = "";
   if ((n = FindNode("location"))) {
      dir = dirname((char*)xmlGetProp(n, (xmlChar*)"href"));
   }
   return dir;
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
   if ((n = FindNode("checksum"))) {
      return (char*)xmlNodeGetContent(n);
   } else {
      return "";
   }
}
unsigned long RPMRepomdHandler::FileSize()
{
   xmlNode *n;
   if ((n = FindNode("size"))) {
      return atol((char*)xmlGetProp(n, (xmlChar*)"package"));
   } else {
      return 0;
   }
}

unsigned long RPMRepomdHandler::InstalledSize()
{
   xmlNode *n;
   if ((n = FindNode("size"))) {
      return atol((char*)xmlGetProp(n, (xmlChar*)"installed"));
   } else {
      return 0;
   }
}

bool RPMRepomdHandler::HasFile(const char *File)
{
   if (*File == '\0')
      return false;

   bool inprimary = false;
   bool found = false;

   xmlNode *format = FindNode("format");
   for (xmlNode *n = format->children; n; n = n->next) {
      if (strcmp((char*)n->name, "file") != 0) 
	 continue;
      if (strcmp(File, (char*)xmlNodeGetContent(n)) == 0) {
	 found = true;
	 break;
      }
   }

   // look through filelists.xml for the file if not in primary.xml
   if (! found) {
      for (xmlNode *n = FlNodeP->children; n; n = n->next) {
	 if (strcmp((char*)n->name, "file") != 0) 
	    continue;
	 if (strcmp(File, (char*)xmlNodeGetContent(n)) == 0) {
	    found = true;
	    break;
	 }
      }
   }
   return found;

}

bool RPMRepomdHandler::Depends(unsigned int Type, vector<Dependency*> &Deps)
{
   xmlNode *format = FindNode("format");
   xmlNode *dco;

   xmlNode *n;

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
   }

   if (! dco) {
      return true;
   }
   for (n = dco->children; n; n = n->next) {
      unsigned int Op;
      int_32 RpmOp;
      string deptype, depver;
      char *ver, *rel, *epoch, *depname, *flags, *pre;
      if ((depname = (char*)xmlGetProp(n, (xmlChar*)"name")) == NULL) continue;

      if ((flags = (char*)xmlGetProp(n, (xmlChar*)"flags"))) {
         ver = (char*)xmlGetProp(n, (xmlChar*)"ver");
         rel = (char*)xmlGetProp(n, (xmlChar*)"rel");
         epoch = (char*)xmlGetProp(n, (xmlChar*)"epoch");
         if (epoch)
            depver += string(epoch) + ":";
         ver = (char*)xmlGetProp(n, (xmlChar*)"ver");
         if (ver)
            depver += string(ver);
         rel = (char*)xmlGetProp(n, (xmlChar*)"rel");
         if (rel)
            depver += "-" + string(rel);

         deptype = flags;

         if (deptype == "EQ") {
            Op = pkgCache::Dep::Equals;
	    RpmOp = RPMSENSE_EQUAL;
	 } else if (deptype == "GE") {
            Op = pkgCache::Dep::GreaterEq;
	    RpmOp = RPMSENSE_GREATER | RPMSENSE_EQUAL;
	 } else if (deptype == "GT") {
            Op = pkgCache::Dep::Greater;
	    RpmOp = RPMSENSE_GREATER;
	 } else if (deptype == "LE") {
            Op = pkgCache::Dep::LessEq;
	    RpmOp = RPMSENSE_LESS | RPMSENSE_EQUAL;
	 } else if (deptype == "LT") {
            Op = pkgCache::Dep::Less;
	    RpmOp = RPMSENSE_LESS;
	 } else {
	    Op = pkgCache::Dep::NoOp;
	    RpmOp = RPMSENSE_ANY;
	 }
      }
      if (InternalDep(depname, depver.c_str(), RpmOp) == true) {
	 continue;
      }
      if (Type == pkgCache::Dep::Depends) {
	 pre = (char*)xmlGetProp(n, (xmlChar*)"pre"); 
	 if (pre) {
	    Type = pkgCache::Dep::PreDepends;
	 }
      }
      Dependency *Dep = new Dependency;
      Dep->Name = depname;
      Dep->Version = depver;
      Dep->Op = Op;
      Dep->Type = Type;
      Deps.push_back(Dep);
   }
   return true;
}

bool RPMRepomdHandler::Provides(vector<Dependency*> &Provs)
{
   xmlNode *format = FindNode("format");
   xmlNode *provides = FindNode(format, "provides");
   bool ret = true;

   if (! provides)
      return true;

   for (xmlNode *n = provides->children; n; n = n->next) {
      string depver = "";
      char *ver, *rel, *epoch, *depname, *flags;

      if (strcmp((char*)n->name, "entry") != 0)  continue;

      Dependency *Dep = new Dependency;
      if ((depname = (char*)xmlGetProp(n, (xmlChar*)"name")) == NULL) continue;

      if ((flags = (char*)xmlGetProp(n, (xmlChar*)"flags"))) {
         epoch = (char*)xmlGetProp(n, (xmlChar*)"epoch");
         if (epoch)
            depver += string(epoch) + ":";
         ver = (char*)xmlGetProp(n, (xmlChar*)"ver");
         if (ver)
            depver += string(ver);
         rel = (char*)xmlGetProp(n, (xmlChar*)"rel");
         if (rel)
            depver += "-" + string(rel);

      }
      Dep->Name = depname;
      Dep->Version = depver;
      if (depver.empty() == false)
	 Dep->Op = pkgCache::Dep::Equals;
      Provs.push_back(Dep);
   }
   return true;
}

bool RPMRepomdHandler::FileProvides(vector<string> &FileProvs)
{
   xmlNode *format = FindNode("format");
   for (xmlNode *n = format->children; n; n = n->next) {
      if (strcmp((char*)n->name, "file") != 0)  continue;
      FileProvs.push_back((char*)xmlNodeGetContent(n));
   }
#if 1 // XXX maybe this should be made configurable?
   for (xmlNode *n = FlNodeP->children; n; n = n->next) {
      if (strcmp((char*)n->name, "file") != 0) 
	    continue;
      FileProvs.push_back((char*)xmlNodeGetContent(n));
   }
#endif
   return true;
}

unsigned short RPMRepomdHandler::VersionHash()
{
   // XXX FIXME: rpmlistparser versionhash for all the things we should do here
   unsigned long Result = INIT_FCS;
   string nevra = Name() + Version() + Arch();
   Result = AddCRC16(Result, nevra.c_str(), nevra.length());
   //cout << "versionhash: " << Result << endl;
   return Result;
}



RPMRepomdHandler::~RPMRepomdHandler()
{
   xmlFreeDoc(Primary);
   xmlFreeDoc(Filelist);
}

// vim:sts=3:sw=3
