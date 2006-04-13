// -*- mode: cpp; mode: fold -*-
// Description								/*{{{*/
// $Id: rpmsystem.cc,v 1.9 2002/11/25 18:25:28 niemeyer Exp $
/* ######################################################################

   System - Abstraction for running on different systems.

   RPM version of the system stuff
   
   ##################################################################### 
 */
									/*}}}*/
// Include Files							/*{{{*/
#ifdef __GNUG__
#pragma implementation "apt-pkg/rpmsystem.h"
#endif

#include <config.h>

#ifdef HAVE_RPM

#include <apt-pkg/rpmsystem.h>
#include <apt-pkg/rpmversion.h>
#include <apt-pkg/rpmindexfile.h>
#include <apt-pkg/rpmpm.h>
#include <apt-pkg/rpmhandler.h>
#include <apt-pkg/configuration.h>
#include <apt-pkg/error.h>
#include <apt-pkg/fileutl.h>
#include <apt-pkg/rpmpackagedata.h>

#include <apti18n.h>
    
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <fcntl.h>
#include <rpm/rpmlib.h>
#include <assert.h>
#include <time.h>
									/*}}}*/
// for distrover
#if RPM_VERSION >= 0x040101
#include <rpmdb.h>
#endif

#if RPM_VERSION >= 0x040201
extern int _rpmds_nopromote;
#endif

rpmSystem rpmSys;

// System::rpmSystem - Constructor					/*{{{*/
// ---------------------------------------------------------------------
/* */
rpmSystem::rpmSystem()
{
   LockCount = 0;
   RpmDB = NULL;
   StatusFile = NULL;
   Label = "rpm interface";
   VS = &rpmVS;
}
									/*}}}*/
rpmSystem::~rpmSystem()
{
   delete StatusFile;
   delete RpmDB;
}

RPMDBHandler *rpmSystem::GetDBHandler()
{
   if (RpmDB == NULL)
      RpmDB = new RPMDBHandler();
   return RpmDB;
}

bool rpmSystem::LockRead()
{
   GetDBHandler();
   if (_error->PendingError() == true)
      return false;
   return true;
}

//
// System::Lock - Get the lock						/*{{{*/
// ---------------------------------------------------------------------
/* this will open the rpm database through rpmlib, which will lock the db */
bool rpmSystem::Lock()
{
   if (RpmDB != NULL && RpmDB->HasWriteLock() == false)
   {
      delete RpmDB;
      RpmDB = NULL;
   }
   if (RpmDB == NULL)
      RpmDB = new RPMDBHandler(true);
   if (_error->PendingError() == true)
      return false;
   LockCount++;
   return true;
}
									/*}}}*/
// System::UnLock - Drop a lock						/*{{{*/
// ---------------------------------------------------------------------
/* Close the rpmdb, effectively dropping it's lock */
bool rpmSystem::UnLock(bool NoErrors)
{
   if (LockCount == 0 && NoErrors == true)
      return false;
   if (LockCount < 1)
      return _error->Error("Not locked");
   if (--LockCount == 0)
   {
      delete RpmDB;
      RpmDB = NULL;
   }
   return true;
}
									/*}}}*/
// System::CreatePM - Create the underlying package manager		/*{{{*/
// ---------------------------------------------------------------------
/* */
pkgPackageManager *rpmSystem::CreatePM(pkgDepCache *Cache) const
{
   if (_config->Find("RPM::PM", "internal") == "internal")
      return new pkgRPMLibPM(Cache);
   else
      return new pkgRPMExtPM(Cache);
}
									/*}}}*/
// System::Initialize - Setup the configuration space..			/*{{{*/
// ---------------------------------------------------------------------
/* These are the rpm specific configuration variables.. */
bool rpmSystem::Initialize(Configuration &Cnf)
{
   Cnf.CndSet("Dir::Bin::rpm","/bin/rpm");
   Cnf.CndSet("Dir::Etc::rpmpriorities", "rpmpriorities");
   Cnf.CndSet("Dir::Etc::translatelist", "translate.list");
   Cnf.CndSet("Dir::Etc::translateparts", "translate.list.d");
   Cnf.CndSet("Dir::State::prefetch", "prefetch");
   Cnf.CndSet("Dir::Locale","/usr/share/locale");
   Cnf.CndSet("Acquire::DistroID","Conectiva"); // hee hee
   Cnf.CndSet("Acquire::CDROM::Mount", "/mnt/cdrom");
   Cnf.CndSet("Acquire::CDROM::Copy-All", "true");

   // Compatibility with obsoleted options
   if (Cnf.Exists("APT::PostInstall"))
   {
      _error->Warning("Rename obsoleted option APT::PostInstall to APT::Post-Install");
      Cnf.CndSet("APT::Post-Install::Clean",
		 Cnf.Find("APT::PostInstall::Clean","false"));
      Cnf.CndSet("APT::Post-Install::AutoClean",
		 Cnf.Find("APT::PostInstall::AutoClean","false"));
   }
   const Configuration::Item *Top;
   Top = _config->Tree("RPM::HoldPkgs");
   if (Top != 0)
   {
      _error->Warning("Rename obsoleted option RPM::HoldPkgs to RPM::Hold");
      for (Top = Top->Child; Top != 0; Top = Top->Next)
	 Cnf.Set("RPM::Hold::", Top->Value.c_str());
   }
   Top = _config->Tree("RPM::AllowedDupPkgs");
   if (Top != 0)
   {
      _error->Warning("Rename obsoleted option RPM::AllowedDupPkgs to RPM::Allow-Duplicated");
      for (Top = Top->Child; Top != 0; Top = Top->Next)
	 Cnf.Set("RPM::Allow-Duplicated::", Top->Value.c_str());
   }
   Top = _config->Tree("RPM::IgnorePkgs");
   if (Top != 0)
   {
      _error->Warning("Rename obsoleted option RPM::IgnorePkgs to RPM::Ignore");
      for (Top = (Top == 0?0:Top->Child); Top != 0; Top = Top->Next)
	 Cnf.Set("RPM::Ignore::", Top->Value.c_str());
   }
   if (Cnf.Exists("RPM::Force"))
   {
      _error->Warning("RPM::Force is obsoleted. Add \"--force\" to RPM::Options instead.");
      if (Cnf.FindB("RPM::Force",false))
	 Cnf.Set("RPM::Options::", "--force");
   }
   if (Cnf.Exists("RPM::NoDeps"))
   {
      _error->Warning("RPM::NoDeps is obsoleted. Add \"--nodeps\" to RPM::Options and RPM::Erase-Options instead.");
      if (Cnf.FindB("RPM::NoDeps",false))
	 Cnf.Set("RPM::Options::", "--nodeps");
   }

#if RPM_VERSION >= 0x040201
   const char *RPMOptions[] =
   {
      "RPM::Options",
      "RPM::Install-Options",
      "RPM::Erase-Options",
      NULL,
   };
   int NoPromote = 1;
   const char **Opt = RPMOptions;
   while (*Opt && NoPromote)
   {
      Top = _config->Tree(*Opt);
      if (Top != 0)
      {
	 for (Top = Top->Child; Top != 0; Top = Top->Next)
	    if (Top->Value == "--promoteepoch") {
	       NoPromote = 0;
	       break;
	    }
      }
      Opt++;
   }
   _rpmds_nopromote = NoPromote;
#endif

   return true;
}
									/*}}}*/
// System::ArchiveSupported - Is a file format supported		/*{{{*/
// ---------------------------------------------------------------------
/* The standard name for a rpm is 'rpm'.. There are no seperate versions
   of .rpm to worry about.. */
bool rpmSystem::ArchiveSupported(const char *Type)
{
   if (strcmp(Type,"rpm") == 0)
      return true;
   return false;
}
									/*}}}*/
// System::Score - Determine how Re**at'ish this sys is..	        /*{{{*/
// ---------------------------------------------------------------------
/* Check some symptoms that this is a Re**at like system */
signed rpmSystem::Score(Configuration const &Cnf)
{
   signed Score = 0;

   rpmReadConfigFiles(NULL, NULL);
   if (FileExists(RPMDBHandler::DataPath(false)))
      Score += 10;
   if (FileExists(Cnf.FindFile("Dir::Bin::rpm","/bin/rpm")) == true)
      Score += 10;

   return Score;
}

string rpmSystem::DistroVer(Configuration const &Cnf)
{
   string DistroVerPkg = _config->Find("Apt::DistroVerPkg");
   string DistroVersion;

   if (DistroVerPkg.empty())
      return DistroVersion;

#if RPM_VERSION >= 0x040100
   rpmts ts;
   ts = rpmtsCreate();
   rpmtsSetVSFlags(ts, (rpmVSFlags_e)-1);
   rpmtsSetRootDir(ts, NULL);
   if (rpmtsOpenDB(ts, O_RDONLY))
      return DistroVersion;
#else
   rpmdb DB;
   string RootDir = _config->Find("RPM::RootDir");
   const char *RootDirStr = RootDir.empty() ? NULL : RootDir.c_str();
   if (rpmdbOpen(RootDirStr, &DB, O_RDONLY, 0644))
      return DistroVersion;
#endif

   rpmdbMatchIterator iter;
#if RPM_VERSION >= 0x040100
   iter = rpmtsInitIterator(ts, (rpmTag)RPMDBI_LABEL, DistroVerPkg.c_str(), 0);
#else
   iter = rpmdbInitIterator(DB, RPMDBI_LABEL, DistroVerPkg.c_str(), 0);
#endif
   Header hdr;
   while ((hdr = rpmdbNextIterator(iter)) != NULL) {
      void *version;
      int type, count;

      if (headerGetEntry(hdr, RPMTAG_VERSION, &type, &version, &count)) {
         DistroVersion = (char *)version;
         headerFreeData(&version, (rpmTagType)type);
         break;
      }
   }
   rpmdbFreeIterator(iter);
#if RPM_VERSION >= 0x040100
   rpmtsFree(ts);
#else
   rpmdbClose(DB);
#endif

   return DistroVersion;
}

									/*}}}*/
// System::AddStatusFiles - Register the status files			/*{{{*/
// ---------------------------------------------------------------------
/* */
bool rpmSystem::AddStatusFiles(vector<pkgIndexFile *> &List)
{
   if (StatusFile == NULL)
      StatusFile = new rpmDatabaseIndex();
   List.push_back(StatusFile);
   return true;
}
									/*}}}*/
// System::AddSourceFiles - Register aditional source files		/*{{{*/
// ---------------------------------------------------------------------
/* */
bool rpmSystem::AddSourceFiles(vector<pkgIndexFile *> &List)
{
   const Configuration::Item *Top;
   Top = _config->Tree("APT::Arguments");
   if (Top != 0)
   {
      for (Top = Top->Child; Top != 0; Top = Top->Next) {
	 const string &S = Top->Value;
	 if (FileExists(S) && flExtension(S) == "rpm")
	 {
	    if (S.length() > 8 && string(S, S.length()-8) == ".src.rpm")
	       List.push_back(new rpmSingleSrcIndex(S));
	    else
	       List.push_back(new rpmSinglePkgIndex(S));
	 }
      }
   }
   return true;
}
									/*}}}*/
#ifdef OLD_FILEDEPS
static void gatherFileDependencies(map<string,int> &filedeps, Header header)
{
   int type, count;
   char **namel;
   //char **verl;
   //int *flagl;
   int res;
   
   res = headerGetEntry(header, RPMTAG_REQUIRENAME, &type,
			(void **)&namel, &count);
   /*
   res = headerGetEntry(header, RPMTAG_REQUIREVERSION, &type, 
			(void **)&verl, &count);
   res = headerGetEntry(header, RPMTAG_REQUIREFLAGS, &type,
			(void **)&flagl, &count);
   */
   
   while (count--) 
   {
      if (*namel[count] == '/')
	 filedeps[string(namel[count])] = 1;
   }
}
#endif


#ifdef OLD_BESTARCH
bool rpmSystem::processIndexFile(rpmIndexFile *Index,OpProgress &Progress)
{
   Header hdr;
   map<string,string> archmap;
   
   RPMHandler *Handler = Index->CreateHandler();
   if (_error->PendingError() == true)
      return false;

   Progress.SubProgress(0, Index->Describe());
   
   Handler->Rewind();

   while (Handler->Skip() == true)
   {
      int type, count, res;
      char *arch;

      hdr = Handler->GetHeader();
      if (!hdr)
	  break;

#ifdef OLD_FILEDEPS
      gatherFileDependencies(FileDeps, hdr);
#endif
      
      /*
       * Make it so that for each version, we keep track of the best
       * architecture.
       */
      res = headerGetEntry(hdr, RPMTAG_ARCH, &type,
			   (void **)&arch, &count);
      assert(type == RPM_STRING_TYPE);
      if (res) 
      {
	 char *name;
	 char *version;
	 char *release;
	 int_32 *epoch;
	 int res;
	 char buf[256];
	 
	 headerGetEntry(hdr, RPMTAG_NAME, &type,
			(void **)&name, &count);
	 headerGetEntry(hdr, RPMTAG_VERSION, &type,
			(void **)&version, &count);
	 headerGetEntry(hdr, RPMTAG_RELEASE, &type,
			(void **)&release, &count);
	 res = headerGetEntry(hdr, RPMTAG_EPOCH, &type,
			      (void **)&epoch, &count);
	 
	 if (res == 1)
	    snprintf(buf, sizeof(buf), "%i:%s-%s", *epoch, version, release);
	 else
	    snprintf(buf, sizeof(buf), "%s-%s", version, release);
	 string n = string(name)+"#"+string(buf);
	 
	 if (archmap.find(n) != archmap.end()) 
	 {
	    if (strcmp(archmap[n].c_str(), arch) != 0) 
	    {
	       int a = rpmMachineScore(RPM_MACHTABLE_INSTARCH, archmap[n].c_str());
	       int b = rpmMachineScore(RPM_MACHTABLE_INSTARCH, arch);
	       
	       if (b < a) 
	       {
		  MultiArchPkgs[n] = string(arch);
		  // this is a multiarch pkg
		  archmap[n] = MultiArchPkgs[n];
	       }
	       else 
	       {
		  MultiArchPkgs[n] = archmap[n];
	       }
	    }
	 }
	 else 
	 {
	    archmap[n] = string(arch);
	 }
      }
   }
   
   if (Handler->IsDatabase() == false)
       delete Handler;

   return true;
}


bool rpmSystem::PreProcess(pkgIndexFile **Start,pkgIndexFile **End,
			   OpProgress &Progress)
{
   string ListDir = _config->FindDir("Dir::State::lists");
   unsigned total, complete;
   unsigned long TotalSize = 0, CurrentSize = 0;

   // calculate size of files
   
   for (; Start != End; Start++)
   {
      if ((*Start)->HasPackages() == false)
	  continue;
      
      if ((*Start)->Exists() == false)
	  continue;
      
      unsigned long Size = (*Start)->Size();
      Progress.OverallProgress(CurrentSize,TotalSize,Size,_("Reading Package Lists"));
      CurrentSize += Size;

      if (processIndexFile((rpmIndexFile*)*Start,Progress) == false)
	 return false;
   }
  
   return true;
}

string rpmSystem::BestArchForPackage(string Pkg)
{
   if (MultiArchPkgs.find(Pkg) != MultiArchPkgs.end())
      return MultiArchPkgs[Pkg];
   else
       return string();
}
#endif

#ifdef OLD_FILEDEPS
bool rpmSystem::IsFileDep(string File)
{
   return (FileDeps.find(File) != FileDeps.end());
}
#endif

// System::FindIndex - Get an index file for status files		/*{{{*/
// ---------------------------------------------------------------------
/* */
bool rpmSystem::FindIndex(pkgCache::PkgFileIterator File,
			  pkgIndexFile *&Found) const
{
   if (StatusFile == 0)
      return false;
   if (StatusFile->FindInCache(*File.Cache()) == File)
   {
      Found = StatusFile;
      return true;
   }
   
   return false;
}
									/*}}}*/

// System::ProcessCache - Do specific changes in the cache  		/*{{{*/
// ---------------------------------------------------------------------
/* */
bool rpmSystem::ProcessCache(pkgDepCache &Cache,pkgProblemResolver &Fix)
{
   RPMPackageData *rpmdata = RPMPackageData::Singleton();
   for (pkgCache::PkgIterator I = Cache.PkgBegin(); I.end() == false; I++)
   {
      // Ignore virtual packages
      if (I->VersionList == 0)
	 continue;
	 
      // Do package holding
      if (I->CurrentVer != 0)
      {
	 if (rpmdata->HoldPackage(I.Name()))
	 {
	    Cache.MarkKeep(I);
	    Fix.Protect(I);
	 }
      }
   }
   return true;
}
									/*}}}*/

// System::IgnoreDep - Check if this dependency should be ignored       /*{{{*/
// ---------------------------------------------------------------------
/* For strong hearts only */
bool rpmSystem::IgnoreDep(pkgVersioningSystem &VS,pkgCache::DepIterator &Dep)
{
   RPMPackageData *rpmdata = RPMPackageData::Singleton();
   return rpmdata->IgnoreDep(VS,Dep);
}
									/*}}}*/

// System::CacheBuilt - free caches used during cache build		/*{{{*/
// ---------------------------------------------------------------------
/* */
void rpmSystem::CacheBuilt()
{
   RPMPackageData *rpmdata = RPMPackageData::Singleton();
   rpmdata->CacheBuilt();
}
									/*}}}*/

// System::OptionsHash - Identify options which change the cache	/*{{{*/
// ---------------------------------------------------------------------
/* */
static void HashString(unsigned long &Hash, const char *Str)
{
   for (const char *I = Str; *I != 0; I++)
      Hash = 5*Hash + *I;
}
static void HashEnv(unsigned long &Hash, const char *Name)
{
   const char *Value = getenv(Name);
   if (Value)
      HashString(Hash, Value);
}
static void HashOption(unsigned long &Hash, const char *Name)
{
   const Configuration::Item *Top = _config->Tree(Name);
   if (Top != 0)
      HashString(Hash, Top->Value.c_str());
}
static void HashOptionTree(unsigned long &Hash, const char *Name)
{
   const Configuration::Item *Top = _config->Tree(Name);
   if (Top != 0)
      for (Top = Top->Child; Top != 0; Top = Top->Next)
	 HashString(Hash, Top->Value.c_str());
}
static void HashOptionFile(unsigned long &Hash, const char *Name)
{
   string FileName = _config->FindFile(Name);
   struct stat st;
   stat(FileName.c_str(), &st);
   Hash += st.st_mtime;
}

#if RPM_VERSION >= 0x040404
static void HashTime(unsigned long &Hash)
{
   Hash += time(NULL);
}
#endif

unsigned long rpmSystem::OptionsHash() const
{
   unsigned long Hash = 0;
   HashOption(Hash, "RPM::Architecture");
   HashOptionTree(Hash, "RPM::Allow-Duplicated");
   HashOptionTree(Hash, "RPM::MultiArch");
   HashOptionTree(Hash, "RPM::Ignore");
   HashOptionFile(Hash, "Dir::Etc::rpmpriorities");
   HashEnv(Hash, "LANG");
   HashEnv(Hash, "LC_ALL");
   HashEnv(Hash, "LC_MESSAGES");
#if RPM_VERSION >= 0x040404
   // This is really draconian but until apt can made somehow deal with
   // runtime dependencies the cache has to be rebuilt for each run for
   // accuracy. Allow turning it off via configuration if not needed.
   if (_config->FindB("RPM::RuntimeDeps", true) == true)
      HashTime(Hash);
#endif
   return Hash;
}
									/*}}}*/

#endif /* HAVE_RPM */

// vim:sts=3:sw=3
