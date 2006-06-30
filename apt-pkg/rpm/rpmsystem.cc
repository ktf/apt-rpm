// -*- mode: c++; mode: fold -*-
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
   LockCount++;
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
   HideZeroEpoch = (NoPromote == 1);
#else
   HideZeroEpoch = false;
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

string rpmSystem::DistroVer()
{
   string DistroVerPkg = _config->Find("APT::DistroVerPkg", "");
   if (DistroVerPkg.empty() || LockRead() == false)
      return "";

   string DistroVersion = "";
   if (RpmDB->JumpByName(DistroVerPkg) == true) {
      DistroVersion = RpmDB->Version();
   } else {
      _error->Error(_("Unable to determine version for package %s"),
		      DistroVerPkg.c_str());
   }
   UnLock(true);

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
