
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

#include <apt-pkg/error.h>
#include <apt-pkg/configuration.h>
#include <apt-pkg/md5.h>

#include <apt-pkg/rpmhandler.h>
#include <apt-pkg/rpmpackagedata.h>

#include <apti18n.h>

#if RPM_VERSION >= 0x040100
#include <rpm/rpmts.h>
#include <rpm/rpmdb.h>
#define rpmxxInitIterator(a,b,c,d) rpmtsInitIterator(a,(rpmTag)b,c,d)
#else
#define rpmxxInitIterator(a,b,c,d) rpmdbInitIterator(a,b,c,d)
#endif

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
   char *str;
   int_32 count, type;
   assert(HeaderP != NULL);
   int rc = headerGetEntry(HeaderP, CRPMTAG_FILENAME,
			   &type, (void**)&str, &count);
   assert(rc != 0);
   return str;
}

string RPMFileHandler::Directory()
{
   char *str;
   int_32 count, type;
   assert(HeaderP != NULL);
   int rc = headerGetEntry(HeaderP, CRPMTAG_DIRECTORY,
			   &type, (void**)&str, &count);
   return (rc?str:"");
}

unsigned long RPMFileHandler::FileSize()
{
   int_32 count, type;
   int_32 *num;
   int rc = headerGetEntry(HeaderP, CRPMTAG_FILESIZE,
			   &type, (void**)&num, &count);
   assert(rc != 0);
   return (unsigned long)num[0];
}

string RPMFileHandler::MD5Sum()
{
   char *str;
   int_32 count, type;
   assert(HeaderP != NULL);
   int rc = headerGetEntry(HeaderP, CRPMTAG_MD5,
			   &type, (void**)&str, &count);
   assert(rc != 0);
   return str;
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
   if (rpmtsOpenDB(Handler, WriteLock?O_RDWR:O_RDONLY) != 0)
   {
      _error->Error(_("could not open RPM database"));
      return;
   }
#else
   const char *RootDir = NULL;
   if (!Dir.empty())
      RootDir = Dir.c_str();
   if (rpmdbOpen(RootDir, &Handler, WriteLock?O_RDWR:O_RDONLY, 0644) != 0)
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

// vim:sts=3:sw=3
