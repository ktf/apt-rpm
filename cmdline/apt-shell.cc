// -*- mode: c++; mode: fold -*-
// Description								/*{{{*/
// $Id: apt-get.cc,v 1.126 2003/02/12 16:14:08 doogie Exp $
/* ######################################################################
   
   apt-get - Cover for dpkg
   
   This is an allout cover for dpkg implementing a safer front end. It is
   based largely on libapt-pkg.

   The syntax is different, 
      apt-get [opt] command [things]
   Where command is:
      update - Resyncronize the package files from their sources
      upgrade - Smart-Download the newest versions of all packages
      dselect-upgrade - Follows dselect's changes to the Status: field
                       and installes new and removes old packages
      dist-upgrade - Powerfull upgrader designed to handle the issues with
                    a new distribution.
      install - Download and install a given package (by name, not by .deb)
      check - Update the package cache and check for broken packages
      clean - Erase the .debs downloaded to /var/cache/apt/archives and
              the partial dir too

   ##################################################################### */
									/*}}}*/
// Include Files							/*{{{*/
#include <apt-pkg/error.h>
#include <apt-pkg/cmndline.h>
#include <apt-pkg/init.h>
#include <apt-pkg/depcache.h>
#include <apt-pkg/sourcelist.h>
#include <apt-pkg/algorithms.h>
#include <apt-pkg/acquire-item.h>
#include <apt-pkg/strutl.h>
#include <apt-pkg/clean.h>
#include <apt-pkg/srcrecords.h>
#include <apt-pkg/version.h>
#include <apt-pkg/cachefile.h>
#include <apt-pkg/sptr.h>
#include <apt-pkg/versionmatch.h>

#include <config.h>
#include <apti18n.h>

// CNC:2003-03-17
#include <apt-pkg/luaiface.h>
    
#include "acqprogress.h"
#include "cmdline.h"

// CNC:2003-02-14 - apti18n.h includes libintl.h which includes locale.h,
// 		    as reported by Radu Greab.
//#include <locale.h>
#include <langinfo.h>
#include <fstream>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <regex.h>
#include <sys/wait.h>

#include <readline/readline.h>
#include <readline/history.h>
#include <fnmatch.h>
									/*}}}*/

using namespace std;

// class CacheFile - Cover class for some dependency cache functions	/*{{{*/
// ---------------------------------------------------------------------
/* */
class CacheFile : public cmdCacheFile
{
   bool IsRoot;
   
   public:
   bool CheckDeps(bool AllowBroken = false);
   bool BuildCaches()
   {
      OpTextProgress Prog(*_config);
      if (pkgCacheFile::BuildCaches(Prog,IsRoot) == false)
	 return false;
      return true;
   }
   bool Open() 
   {
      OpTextProgress Prog(*_config);
      if (pkgCacheFile::Open(Prog,IsRoot) == false)
	 return false;
      Sort();
      return true;
   };
   bool CanCommit()
   {
      return IsRoot;
   }
   CacheFile() : cmdCacheFile()
   {
      IsRoot = (getuid() == 0);
   };
};
									/*}}}*/

static CacheFile *GCache = NULL;

class AutoRestore
{
   pkgDepCache::State State;
   bool Guarded;
   public:
   inline pkgDepCache::State *operator ->() {return &State;};
   inline pkgDepCache::State *operator &() {return &State;};
   inline void UnGuard() { Guarded = false; };
   AutoRestore(pkgDepCache &Cache)
      : State(&Cache), Guarded(true) {};
   ~AutoRestore() { if (Guarded) State.Restore(); };
};

class AutoReOpenCache
{
   CacheFile **Cache;
   bool Guarded;
   public:
   inline void UnGuard() { Guarded = false; };
   AutoReOpenCache(CacheFile *&Cache)
      : Cache(&Cache), Guarded(true) {};
   ~AutoReOpenCache()
   {
      if (Guarded) {
	 delete *Cache;
	 *Cache = new CacheFile;
	 (*Cache)->Open();
	 if ((*Cache)->CheckDeps(true) == false) {
	    c1out << _("There are broken packages. ")
		  << _("Run `check' to see them.") << endl;
	    c1out << _("You can try to fix them automatically with `install --fix-broken'0.") << endl;
	 }
      }
   };
};

void CommandHelp(const char *Name);
bool CheckHelp(CommandLine &CmdL, unsigned int MaxArgs=512)
{
   if (CmdL.FileSize()-1 > MaxArgs) {
      _error->Error(_("Excessive arguments"));
      return true;
   }
   if (_config->FindB("help") == true) {
      CommandHelp(CmdL.FileList[0]);
      return true;
   }
   return false;
}

// ShowChanges - Show what would change between the saved state and the /*{{{*/
//	         cache file state.
// ---------------------------------------------------------------------
/* */
bool ShowChanges(CacheFile &Cache,pkgDepCache::State *State=NULL)
{
   ShowUpgraded(c1out,Cache,State);
   ShowDel(c1out,Cache,State);
   ShowNew(c1out,Cache,State);
   if (State != NULL)
      ShowKept(c1out,Cache,State);
   ShowHold(c1out,Cache,State);
   ShowDowngraded(c1out,Cache,State);
   ShowEssential(c1out,Cache,State);
   Stats(c1out,Cache,State);

   if (State != NULL) {
      double DebBytes = Cache->DebSize()-State->DebSize();
      // Number of bytes
      if (DebBytes >= 0)
	 ioprintf(c1out,_("Will need more %sB of archives.\n"),
		  SizeToStr(DebBytes).c_str());
      else
	 ioprintf(c1out,_("Will need less %sB of archives.\n"),
		  SizeToStr(-1*DebBytes).c_str());

      double UsrSize = Cache->UsrSize()-State->UsrSize();
      // Size delta
      if (UsrSize >= 0)
	 ioprintf(c1out,_("After unpacking will need more %sB of disk space.\n"),
		  SizeToStr(UsrSize).c_str());
      else
	 ioprintf(c1out,_("After unpacking will need less %sB of disk space.\n"),
		  SizeToStr(-1*UsrSize).c_str());
   } else {
      double DebBytes = Cache->DebSize();
      // Number of bytes
      ioprintf(c1out,_("Will need %sB of archives.\n"),
	       SizeToStr(DebBytes).c_str());

      // Size delta
      if (Cache->UsrSize() >= 0)
	 ioprintf(c1out,_("After unpacking %sB of additional disk space will be used.\n"),
		  SizeToStr(Cache->UsrSize()).c_str());
      else
	 ioprintf(c1out,_("After unpacking %sB disk space will be freed.\n"),
		  SizeToStr(-1*Cache->UsrSize()).c_str());
   }
   return true;
}
									/*}}}*/

bool ConfirmChanges(CacheFile &Cache, AutoRestore &StateGuard)
{
   if (StateGuard->Changed()) {
      c2out << _("Unrequested changes are needed to execute this operation.")
	    << endl;
      ShowChanges(Cache,&StateGuard);
      c2out << _("Do you want to continue? [Y/n] ") << flush;
      if (YnPrompt() == false) {
	 c2out << _("Abort.") << endl;
	 return false;
      }
   }
   StateGuard.UnGuard();
   return true;
}

// CacheFile::CheckDeps - Open the cache file				/*{{{*/
// ---------------------------------------------------------------------
/* This routine generates the caches and then opens the dependency cache
   and verifies that the system is OK. */
bool CacheFile::CheckDeps(bool AllowBroken)
{
   if (_error->PendingError() == true)
      return false;

   // Check that the system is OK
   //if (DCache->DelCount() != 0 || DCache->InstCount() != 0)
   //   return _error->Error("Internal Error, non-zero counts");
   
   // Apply corrections for half-installed packages
   if (pkgApplyStatus(*DCache) == false)
      return false;
   
   // Nothing is broken
   if (DCache->BrokenCount() == 0 || AllowBroken == true)
      return true;

   // Attempt to fix broken things
   if (_config->FindB("APT::Get::Fix-Broken",false) == true)
   {
      c1out << _("Correcting dependencies...") << flush;
      if (pkgFixBroken(*DCache) == false || DCache->BrokenCount() != 0)
      {
	 c1out << _(" failed.") << endl;
	 ShowBroken(c1out,*this,true);

	 return _error->Error(_("Unable to correct dependencies"));
      }
      if (pkgMinimizeUpgrade(*DCache) == false)
	 return _error->Error(_("Unable to minimize the upgrade set"));
      
      c1out << _(" Done") << endl;
   }
   else
   {
      c1out << _("You might want to run `install --fix-broken' to correct these.") << endl;
      ShowBroken(c1out,*this,true);

      return _error->Error(_("Unmet dependencies. Try using --fix-broken."));
   }
      
   return true;
}
									/*}}}*/

bool DoClean(CommandLine &CmdL);
bool DoAutoClean(CommandLine &CmdL);

// InstallPackages - Actually download and install the packages		/*{{{*/
// ---------------------------------------------------------------------
/* This displays the informative messages describing what is going to 
   happen and then calls the download routines */
bool InstallPackages(CacheFile &Cache,bool ShwKept,bool Ask = true,
		     bool Saftey = true)
{
   if (_config->FindB("APT::Get::Purge",false) == true)
   {
      pkgCache::PkgIterator I = Cache->PkgBegin();
      for (; I.end() == false; I++)
      {
	 if (I.Purge() == false && Cache[I].Mode == pkgDepCache::ModeDelete)
	    Cache->MarkDelete(I,true);
      }
   }
   
   bool Fail = false;
   bool Essential = false;
   
   // Show all the various warning indicators
   // CNC:2002-03-06 - Change Show-Upgraded default to true, and move upwards.
   if (_config->FindB("APT::Get::Show-Upgraded",true) == true)
      ShowUpgraded(c1out,Cache);
   ShowDel(c1out,Cache);
   ShowNew(c1out,Cache);
   if (ShwKept == true)
      ShowKept(c1out,Cache);
   Fail |= !ShowHold(c1out,Cache);
   Fail |= !ShowDowngraded(c1out,Cache);
   Essential = !ShowEssential(c1out,Cache);
   Fail |= Essential;
   Stats(c1out,Cache);
   
   // Sanity check
   if (Cache->BrokenCount() != 0)
   {
      ShowBroken(c1out,Cache,false);
      return _error->Error("Internal Error, InstallPackages was called with broken packages!");
   }

   if (Cache->DelCount() == 0 && Cache->InstCount() == 0 &&
       Cache->BadCount() == 0)
      return true;

   // No remove flag
   if (Cache->DelCount() != 0 && _config->FindB("APT::Get::Remove",true) == false)
      return _error->Error(_("Packages need to be removed but Remove is disabled."));
       
   // Run the simulator ..
   if (_config->FindB("APT::Get::Simulate") == true)
   {
      pkgSimulate PM(Cache);
      pkgPackageManager::OrderResult Res = PM.DoInstall();
      if (Res == pkgPackageManager::Failed)
	 return false;
      if (Res != pkgPackageManager::Completed)
	 return _error->Error("Internal Error, Ordering didn't finish");
      return true;
   }
   
   // Create the text record parser
   pkgRecords Recs(Cache);
   if (_error->PendingError() == true)
      return false;
   
   // Lock the archive directory
   FileFd Lock;
   if (_config->FindB("Debug::NoLocking",false) == false &&
       _config->FindB("APT::Get::Print-URIs") == false)
   {
      Lock.Fd(GetLock(_config->FindDir("Dir::Cache::Archives") + "lock"));
      if (_error->PendingError() == true)
	 return _error->Error(_("Unable to lock the download directory"));
   }
   
   // Create the download object
   AcqTextStatus Stat(ScreenWidth,_config->FindI("quiet",0));   
   pkgAcquire Fetcher(&Stat);

   // Read the source list
   pkgSourceList List;
   if (List.ReadMainList() == false)
      return _error->Error(_("The list of sources could not be read."));
   
   // Create the package manager and prepare to download
   SPtr<pkgPackageManager> PM= _system->CreatePM(Cache);
   if (PM->GetArchives(&Fetcher,&List,&Recs) == false || 
       _error->PendingError() == true)
      return false;

   // Display statistics
   double FetchBytes = Fetcher.FetchNeeded();
   double FetchPBytes = Fetcher.PartialPresent();
   double DebBytes = Fetcher.TotalNeeded();
   if (DebBytes != Cache->DebSize())
   {
      c0out << DebBytes << ',' << Cache->DebSize() << endl;
      c0out << "How odd.. The sizes didn't match, email apt@packages.debian.org" << endl;
   }
   
   // Number of bytes
   if (DebBytes != FetchBytes)
      ioprintf(c1out,_("Need to get %sB/%sB of archives.\n"),
	       SizeToStr(FetchBytes).c_str(),SizeToStr(DebBytes).c_str());
   else
      ioprintf(c1out,_("Need to get %sB of archives.\n"),
	       SizeToStr(DebBytes).c_str());

   // Size delta
   if (Cache->UsrSize() >= 0)
      ioprintf(c1out,_("After unpacking %sB of additional disk space will be used.\n"),
	       SizeToStr(Cache->UsrSize()).c_str());
   else
      ioprintf(c1out,_("After unpacking %sB disk space will be freed.\n"),
	       SizeToStr(-1*Cache->UsrSize()).c_str());

   if (_error->PendingError() == true)
      return false;

   /* Check for enough free space, but only if we are actually going to
      download */
   if (_config->FindB("APT::Get::Print-URIs") == false &&
       _config->FindB("APT::Get::Download",true) == true)
   {
      struct statvfs Buf;
      string OutputDir = _config->FindDir("Dir::Cache::Archives");
      if (statvfs(OutputDir.c_str(),&Buf) != 0)
	 return _error->Errno("statvfs","Couldn't determine free space in %s",
			      OutputDir.c_str());
      // CNC:2002-07-11
      if (unsigned(Buf.f_bavail) < (FetchBytes - FetchPBytes)/Buf.f_bsize)
	 return _error->Error(_("You don't have enough free space in %s."),
			      OutputDir.c_str());
   }
   
   // Fail safe check
   if (_config->FindI("quiet",0) >= 2 ||
       _config->FindB("APT::Get::Assume-Yes",false) == true)
   {
      if (Fail == true && _config->FindB("APT::Get::Force-Yes",false) == false)
	 return _error->Error(_("There are problems and -y was used without --force-yes"));
   }         

   if (Essential == true && Saftey == true)
   {
      if (_config->FindB("APT::Get::Trivial-Only",false) == true)
	 return _error->Error(_("Trivial Only specified but this is not a trivial operation."));
      
      const char *Prompt = _("Yes, do as I say!");
      ioprintf(c2out,
	       _("You are about to do something potentially harmful\n"
		 "To continue type in the phrase '%s'\n"
		 " ?] "),Prompt);
      c2out << flush;
      if (AnalPrompt(Prompt) == false)
      {
	 c2out << _("Abort.") << endl;
	 exit(1);
      }     
   }
   else
   {      
      // Prompt to continue
      if (Ask == true || Fail == true)
      {            
	 if (_config->FindB("APT::Get::Trivial-Only",false) == true)
	    return _error->Error(_("Trivial Only specified but this is not a trivial operation."));
	 
	 if (_config->FindI("quiet",0) < 2 &&
	     _config->FindB("APT::Get::Assume-Yes",false) == false)
	 {
	    c2out << _("Do you want to continue? [Y/n] ") << flush;
	 
	    if (YnPrompt() == false)
	    {
	       c2out << _("Abort.") << endl;
	       exit(1);
	    }     
	 }	 
      }      
   }
   
   // Just print out the uris an exit if the --print-uris flag was used
   if (_config->FindB("APT::Get::Print-URIs") == true)
   {
      pkgAcquire::UriIterator I = Fetcher.UriBegin();
      for (; I != Fetcher.UriEnd(); I++)
	 cout << '\'' << I->URI << "' " << flNotDir(I->Owner->DestFile) << ' ' << 
	       I->Owner->FileSize << ' ' << I->Owner->MD5Sum() << endl;
      return true;
   }

   /* Unlock the dpkg lock if we are not going to be doing an install
      after. */
   if (_config->FindB("APT::Get::Download-Only",false) == true)
      _system->UnLock();

   // CNC:2003-02-24
   bool Ret = true;

   AutoReOpenCache CacheGuard(GCache);
   
   // Run it
   while (1)
   {
      bool Transient = false;
      if (_config->FindB("APT::Get::Download",true) == false)
      {
	 for (pkgAcquire::ItemIterator I = Fetcher.ItemsBegin(); I < Fetcher.ItemsEnd();)
	 {
	    if ((*I)->Local == true)
	    {
	       I++;
	       continue;
	    }

	    // Close the item and check if it was found in cache
	    (*I)->Finished();
	    if ((*I)->Complete == false)
	       Transient = true;
	    
	    // Clear it out of the fetch list
	    delete *I;
	    I = Fetcher.ItemsBegin();
	 }	 
      }
      
      if (Fetcher.Run() == pkgAcquire::Failed)
	 return false;

      // CNC:2003-02-24
      _error->PopState();
      
      // Print out errors
      bool Failed = false;
      for (pkgAcquire::ItemIterator I = Fetcher.ItemsBegin(); I != Fetcher.ItemsEnd(); I++)
      {
	 if ((*I)->Status == pkgAcquire::Item::StatDone &&
	     (*I)->Complete == true)
	    continue;
	 
	 if ((*I)->Status == pkgAcquire::Item::StatIdle)
	 {
	    Transient = true;
	    // Failed = true;
	    continue;
	 }

	 fprintf(stderr,_("Failed to fetch %s  %s\n"),(*I)->DescURI().c_str(),
		 (*I)->ErrorText.c_str());
	 Failed = true;
      }

      /* If we are in no download mode and missing files and there were
         'failures' then the user must specify -m. Furthermore, there 
         is no such thing as a transient error in no-download mode! */
      if (Transient == true &&
	  _config->FindB("APT::Get::Download",true) == false)
      {
	 Transient = false;
	 Failed = true;
      }
      
      if (_config->FindB("APT::Get::Download-Only",false) == true)
      {
	 if (Failed == true && _config->FindB("APT::Get::Fix-Missing",false) == false)
	    return _error->Error(_("Some files failed to download"));
	 c1out << _("Download complete and in download only mode") << endl;
	 return true;
      }
      
      if (Failed == true && _config->FindB("APT::Get::Fix-Missing",false) == false)
      {
	 return _error->Error(_("Unable to fetch some archives, maybe run apt-get update or try with --fix-missing?"));
      }
      
      if (Transient == true && Failed == true)
	 return _error->Error(_("--fix-missing and media swapping is not currently supported"));
      
      // Try to deal with missing package files
      if (Failed == true && PM->FixMissing() == false)
      {
	 cerr << _("Unable to correct missing packages.") << endl;
	 return _error->Error(_("Aborting Install."));
      }

      // CNC:2002-10-18
      if (Transient == false || _config->FindB("Acquire::CDROM::Copy-All", false) == false) {
	 if (Transient == true) {
	    // We must do that in a system independent way. */
	    _config->Set("RPM::Install-Options::", "--nodeps");
	 }
	 _system->UnLock();
	 pkgPackageManager::OrderResult Res = PM->DoInstall();
	 if (Res == pkgPackageManager::Failed || _error->PendingError() == true)
	 {
	    if (Transient == false)
	       return false;
	    Ret = false;
	 }

	 // CNC:2002-07-06
	 if (Res == pkgPackageManager::Completed)
	 {
	    CommandLine *CmdL = NULL; // Watch out! If used will blow up!
	    if (_config->FindB("APT::Post-Install::Clean",false) == true) 
	       Ret &= DoClean(*CmdL);
	    else if (_config->FindB("APT::Post-Install::AutoClean",false) == true) 
	       Ret &= DoAutoClean(*CmdL);
	    return Ret;
	 }
	 
	 _system->Lock();
      }

      // CNC:2003-02-24
      _error->PushState();
      
      // Reload the fetcher object and loop again for media swapping
      Fetcher.Shutdown();
      if (PM->GetArchives(&Fetcher,&List,&Recs) == false)
	 return false;
   }   
}
									/*}}}*/
#define MODE_INSTALL 0
#define MODE_REMOVE  1
#define MODE_KEEP    2
// TryToInstall - Try to install a single package			/*{{{*/
// ---------------------------------------------------------------------
/* This used to be inlined in DoInstall, but with the advent of regex package
   name matching it was split out.. */
bool TryToInstall(pkgCache::PkgIterator Pkg,pkgDepCache &Cache,
		  pkgProblemResolver &Fix,int Mode,bool BrokenFix,
		  unsigned int &ExpectedInst,bool AllowFail = true)
{
   // CNC:2004-03-03 - Improved virtual package handling.
   if (Cache[Pkg].CandidateVer == 0 && Pkg->ProvidesList != 0)
   {
      vector<pkgCache::Package *> GoodSolutions;
      for (pkgCache::PrvIterator Prv = Pkg.ProvidesList();
	   Prv.end() == false; Prv++)
      {
	 pkgCache::PkgIterator PrvPkg = Prv.OwnerPkg();
	 // Check if it's a different version of a package already
	 // considered as a good solution.
	 bool AlreadySeen = false;
	 for (vector<pkgCache::Package *>::size_type i = 0; i != GoodSolutions.size(); i++)
	 {
	    pkgCache::PkgIterator GoodPkg(Cache, GoodSolutions[i]);
	    if (PrvPkg == GoodPkg)
	    {
	       AlreadySeen = true;
	       break;
	    }
	 }
	 if (AlreadySeen)
	    continue;
	 // Is the current version the provides owner?
	 if (PrvPkg.CurrentVer() == Prv.OwnerVer())
	 {
	    // Already installed packages are good solutions, since
	    // the user might try to install something he already has
	    // without being aware.
	    GoodSolutions.push_back(PrvPkg);
	    continue;
	 }
	 pkgCache::VerIterator PrvPkgCandVer =
				 Cache[PrvPkg].CandidateVerIter(Cache);
	 if (PrvPkgCandVer.end() == true)
	 {
	    // Packages without a candidate version are not good solutions.
	    continue;
	 }
	 // Is the provides pointing to the candidate version?
	 if (PrvPkgCandVer == Prv.OwnerVer())
	 {
	    // Yes, it is. This is a good solution.
	    GoodSolutions.push_back(PrvPkg);
	    continue;
	 }
      }
      vector<string> GoodSolutionNames;
      for (vector<string>::size_type i = 0; i != GoodSolutions.size(); i++)
      {
	 pkgCache::PkgIterator GoodPkg(Cache, GoodSolutions[i]);
	 GoodSolutionNames.push_back(GoodPkg.Name());
      }
#ifdef APT_WITH_LUA
      if (GoodSolutions.size() > 1)
      {
	 vector<string> VS;
	 _lua->SetDepCache(&Cache);
	 _lua->SetDontFix();
	 _lua->SetGlobal("virtualname", Pkg.Name());
	 _lua->SetGlobal("packages", GoodSolutions);
	 _lua->SetGlobal("packagenames", GoodSolutionNames);
	 _lua->SetGlobal("selected");
	 _lua->RunScripts("Scripts::AptGet::Install::SelectPackage");
	 pkgCache::Package *selected = _lua->GetGlobalPkg("selected");
	 if (selected)
	 {
	    GoodSolutions.clear();
	    GoodSolutions.push_back(selected);
	 }
	 else
	 {
	    vector<string> Tmp = _lua->GetGlobalStrList("packagenames");
	    if (Tmp.size() == GoodSolutions.size())
	       GoodSolutionNames = Tmp;
	 }
	 _lua->ResetGlobals();
	 _lua->ResetCaches();
      }
#endif
      if (GoodSolutions.size() == 1)
      {
	 pkgCache::PkgIterator GoodPkg(Cache, GoodSolutions[0]);
	 ioprintf(c1out,_("Selecting %s for '%s'\n"),
		  GoodPkg.Name(), Pkg.Name());
	 Pkg = GoodPkg;
      }
      else if (GoodSolutions.size() == 0)
      {
	 _error->Error(_("Package %s is a virtual package with no "
			 "good providers.\n"), Pkg.Name());
	 return false;
      }
      else
      {
	 ioprintf(c1out,_("Package %s is a virtual package provided by:\n"),
		  Pkg.Name());
	 for (vector<pkgCache::Package *>::size_type i = 0; i != GoodSolutions.size(); i++)
	 {
	    pkgCache::PkgIterator GoodPkg(Cache, GoodSolutions[i]);
	    if (GoodPkg.CurrentVer().end() == false)
	       c1out << "  " << GoodSolutionNames[i]
		     << " "  << Cache[GoodPkg].CandVersion
		     << _(" [Installed]") << endl;
	    else
	       c1out << "  " << GoodSolutionNames[i]
		     << " "  << Cache[GoodPkg].CandVersion << endl;
	 }
	 c1out << _("You should explicitly select one to install.") << endl;
	 _error->Error(_("Package %s is a virtual package with multiple "
			 "good providers.\n"), Pkg.Name());
	 return false;
      }
   }
   
   // Handle the no-upgrade case
   if (_config->FindB("APT::Get::upgrade",true) == false &&
       Pkg->CurrentVer != 0)
   {
      if (AllowFail == true)
	 ioprintf(c1out,_("Skipping %s, it is already installed and upgrade is not set.\n"),
		  Pkg.Name());
      return true;
   }
   
   // Check if there is something at all to install
   pkgDepCache::StateCache &State = Cache[Pkg];
   if (Mode == MODE_REMOVE && Pkg->CurrentVer == 0)
   {
      Fix.Clear(Pkg);
      Fix.Protect(Pkg);
      Fix.Remove(Pkg);
      
      /* We want to continue searching for regex hits, so we return false here
         otherwise this is not really an error. */
      if (AllowFail == false)
	 return false;
      
      ioprintf(c1out,_("Package %s is not installed, so not removed\n"),Pkg.Name());
      return true;
   }
   
   if (State.CandidateVer == 0 && Mode == MODE_INSTALL)
   {
      if (AllowFail == false)
	 return false;
      
// CNC:2004-03-03 - Improved virtual package handling.
#if 0
      if (Pkg->ProvidesList != 0)
      {
	 ioprintf(c1out,_("Package %s is a virtual package provided by:\n"),
		  Pkg.Name());
	 
	 pkgCache::PrvIterator I = Pkg.ProvidesList();
	 for (; I.end() == false; I++)
	 {
	    pkgCache::PkgIterator Pkg = I.OwnerPkg();
	    
	    if (Cache[Pkg].CandidateVerIter(Cache) == I.OwnerVer())
	    {
	       if (Cache[Pkg].Install() == true && Cache[Pkg].NewInstall() == false)
		  c1out << "  " << Pkg.Name() << " " << I.OwnerVer().VerStr() <<
		  _(" [Installed]") << endl;
	       else
		  c1out << "  " << Pkg.Name() << " " << I.OwnerVer().VerStr() << endl;
	    }      
	 }
	 c1out << _("You should explicitly select one to install.") << endl;
      }
      else
#endif
      {
	 ioprintf(c1out,
	 _("Package %s has no available version, but exists in the database.\n"
	   "This typically means that the package was mentioned in a dependency and\n"
	   "never uploaded, has been obsoleted or is not available with the contents\n"
	   "of sources.list\n"),Pkg.Name());
	 
	 string List;
	 string VersionsList;
	 SPtrArray<bool> Seen = new bool[Cache.Head().PackageCount];
	 memset(Seen,0,Cache.Head().PackageCount*sizeof(*Seen));
	 pkgCache::DepIterator Dep = Pkg.RevDependsList();
	 for (; Dep.end() == false; Dep++)
	 {
	    // CNC:2002-07-30
	    if (Dep->Type != pkgCache::Dep::Replaces &&
	        Dep->Type != pkgCache::Dep::Obsoletes)
	       continue;
	    if (Seen[Dep.ParentPkg()->ID] == true)
	       continue;
	    Seen[Dep.ParentPkg()->ID] = true;
	    List += string(Dep.ParentPkg().Name()) + " ";
            //VersionsList += string(Dep.ParentPkg().CurVersion) + "\n"; ???
	 }	    
	 ShowList(c1out,_("However the following packages replace it:"),List,VersionsList);
      }
      
      _error->Error(_("Package %s has no installation candidate"),Pkg.Name());
      return false;
   }

   Fix.Clear(Pkg);
   Fix.Protect(Pkg);   

   if (Mode == MODE_KEEP) {
      Cache.MarkKeep(Pkg);
      return true;
   }
   
   if (Mode == MODE_REMOVE)
   {
      Fix.Remove(Pkg);
      Cache.MarkDelete(Pkg,_config->FindB("APT::Get::Purge",false));
      return true;
   }
   
   // Install it
   Cache.MarkInstall(Pkg,false);
   if (State.Install() == false)
   {
      if (_config->FindB("APT::Get::ReInstall",false) == true)
      {
	 if (Pkg->CurrentVer == 0 || Pkg.CurrentVer().Downloadable() == false)
	    ioprintf(c1out,_("Reinstallation of %s is not possible, it cannot be downloaded.\n"),
		     Pkg.Name());
	 else
	    Cache.SetReInstall(Pkg,true);
      }      
      else
      {
	 if (AllowFail == true)
	    ioprintf(c1out,_("%s is already the newest version.\n"),
		     Pkg.Name());
      }      
   }   
   else
      ExpectedInst++;
   
   // Install it with autoinstalling enabled.
   if (State.InstBroken() == true && BrokenFix == false)
      Cache.MarkInstall(Pkg,true);
   return true;
}
									/*}}}*/
// TryToChangeVer - Try to change a candidate version			/*{{{*/
// ---------------------------------------------------------------------
// CNC:2003-11-11
bool TryToChangeVer(pkgCache::PkgIterator &Pkg,pkgDepCache &Cache,
 		    int VerOp,const char *VerTag,bool IsRel)
{
   // CNC:2003-11-05
   pkgVersionMatch Match(VerTag,(IsRel == true?pkgVersionMatch::Release : 
 				 pkgVersionMatch::Version),VerOp);
   
   pkgCache::VerIterator Ver = Match.Find(Pkg);
			 
   if (Ver.end() == true)
   {
      // CNC:2003-11-05
      if (IsRel == true)
	 return _error->Error(_("Release %s'%s' for '%s' was not found"),
			      op2str(VerOp),VerTag,Pkg.Name());
      return _error->Error(_("Version %s'%s' for '%s' was not found"),
			   op2str(VerOp),VerTag,Pkg.Name());
   }
   
   if (strcmp(VerTag,Ver.VerStr()) != 0)
   {
      // CNC:2003-11-11
      if (IsRel == true)
	 ioprintf(c1out,_("Selected version %s (%s) for %s\n"),
		  Ver.VerStr(),Ver.RelStr().c_str(),Pkg.Name());
      else
	 ioprintf(c1out,_("Selected version %s for %s\n"),
		  Ver.VerStr(),Pkg.Name());
   }
   
   Cache.SetCandidateVersion(Ver);
   // CNC:2003-11-11
   Pkg = Ver.ParentPkg();
   return true;
}
									/*}}}*/

// DoUpdate - Update the package lists					/*{{{*/
// ---------------------------------------------------------------------
/* */

// CNC:2004-04-19
class UpdateLogCleaner : public pkgArchiveCleaner
{
   protected:
   virtual void Erase(const char *File,string Pkg,string Ver,struct stat &St) 
   {
      c1out << "Del " << Pkg << " " << Ver << " [" << SizeToStr(St.st_size) << "B]" << endl;
      unlink(File);      
   };
};

bool DoUpdate(CommandLine &CmdL)
{
   if (CheckHelp(CmdL) == true)
      return true;

   bool Partial = false;
   pkgSourceList List;

   if (CmdL.FileSize() != 1)
   {
      List.ReadVendors();
      for (const char **I = CmdL.FileList + 1; *I != 0; I++)
      {
	 string Repo = _config->FindDir("Dir::Etc::sourceparts") + *I;
	 if (FileExists(Repo) == false)
	    Repo += ".list";
	 if (FileExists(Repo) == true)
	 {
	    if (List.ReadAppend(Repo) == true)
	       Partial = true;
	    else
	       return _error->Error(_("Sources list %s could not be read"),Repo.c_str());
	 }
	 else
	    return _error->Error(_("Sources list %s doesn't exist"),Repo.c_str());
      }
   }
   else if (List.ReadMainList() == false)
      return false;

   // Lock the list directory
   FileFd Lock;
   if (_config->FindB("Debug::NoLocking",false) == false)
   {
      Lock.Fd(GetLock(_config->FindDir("Dir::State::Lists") + "lock"));
      if (_error->PendingError() == true)
	 return _error->Error(_("Unable to lock the list directory"));
   }

// CNC:2003-03-19
#ifdef APT_WITH_LUA
   _lua->SetDepCache(*GCache);
   _lua->RunScripts("Scripts::AptGet::Update::Pre");
   _lua->ResetCaches();
#endif
   
   // Create the download object
   AcqTextStatus Stat(ScreenWidth,_config->FindI("quiet",0));
   pkgAcquire Fetcher(&Stat);

   // CNC:2002-07-03
   bool Failed = false;
   // Populate it with release file URIs
   if (List.GetReleases(&Fetcher) == false)
      return false;
   if (_config->FindB("APT::Get::Print-URIs") == false)
   {
      Fetcher.Run();
      for (pkgAcquire::ItemIterator I = Fetcher.ItemsBegin(); I != Fetcher.ItemsEnd(); I++)
      {
	 if ((*I)->Status == pkgAcquire::Item::StatDone)
	    continue;
	 (*I)->Finished();
	 Failed = true;
      }
      if (Failed == true)
	 _error->Warning(_("Release files for some repositories could not be retrieved or authenticated. Such repositories are being ignored."));
   }
   
   // Populate it with the source selection
   if (List.GetIndexes(&Fetcher) == false)
	 return false;
   
   // Just print out the uris an exit if the --print-uris flag was used
   if (_config->FindB("APT::Get::Print-URIs") == true)
   {
      pkgAcquire::UriIterator I = Fetcher.UriBegin();
      for (; I != Fetcher.UriEnd(); I++)
	 cout << '\'' << I->URI << "' " << flNotDir(I->Owner->DestFile) << ' ' << 
	       I->Owner->FileSize << ' ' << I->Owner->MD5Sum() << endl;
      return true;
   }
   
   // Run it
   if (Fetcher.Run() == pkgAcquire::Failed)
      return false;

   for (pkgAcquire::ItemIterator I = Fetcher.ItemsBegin(); I != Fetcher.ItemsEnd(); I++)
   {
      if ((*I)->Status == pkgAcquire::Item::StatDone)
	 continue;

      (*I)->Finished();
      
      fprintf(stderr,_("Failed to fetch %s  %s\n"),(*I)->DescURI().c_str(),
	      (*I)->ErrorText.c_str());
      Failed = true;
   }
   
   // Clean out any old list files
   if (_config->FindB("APT::Get::List-Cleanup",true) == true)
   {
      if (Fetcher.Clean(_config->FindDir("Dir::State::lists")) == false ||
	  Fetcher.Clean(_config->FindDir("Dir::State::lists") + "partial/") == false)
	 return false;
   }

   {
      AutoReOpenCache CacheGuard(GCache);
   }

// CNC:2003-03-19
#ifdef APT_WITH_LUA
   _lua->SetDepCache(*GCache);
   _lua->RunScripts("Scripts::AptGet::Update::Post");
   _lua->ResetCaches();
#endif

   // CNC:2004-04-19
   if (Failed == false && _config->FindB("APT::Get::Archive-Cleanup",true) == true)
   {
      UpdateLogCleaner Cleaner;
      Cleaner.Go(_config->FindDir("Dir::Cache::archives"), *GCache);
      Cleaner.Go(_config->FindDir("Dir::Cache::archives") + "partial/",
	         *GCache);
   }
   
   if (Failed == true)
      return _error->Error(_("Some index files failed to download, they have been ignored, or old ones used instead."));
   
   return true;
}
									/*}}}*/
// DoUpgrade - Upgrade all packages					/*{{{*/
// ---------------------------------------------------------------------
/* Upgrade all packages without installing new packages or erasing old
   packages */
bool DoUpgrade(CommandLine &CmdL)
{
   if (CheckHelp(CmdL, 0) == true)
      return true;
   
   CacheFile &Cache = *GCache;
   if (GCache->CanCommit() == false) {
      _error->Error(_("You have no permissions for that"));
      return false;
   }
   
   AutoRestore StateGuard(Cache);
   
   if (GCache->CheckDeps() == false)
      return false;

   // Do the upgrade
   if (pkgAllUpgrade(Cache) == false)
   {
      ShowBroken(c1out,Cache,false);
      return _error->Error(_("Internal Error, AllUpgrade broke stuff"));
   }

// CNC:2003-03-19
#ifdef APT_WITH_LUA
   _lua->SetDepCache(Cache);
   _lua->RunScripts("Scripts::AptGet::Upgrade");
   _lua->ResetCaches();
#endif

   ConfirmChanges(Cache, StateGuard);
   
   return true;
}
									/*}}}*/
// DoInstall - Install packages from the command line			/*{{{*/
// ---------------------------------------------------------------------
/* Install named packages */
bool DoInstall(CommandLine &CmdL)
{
   if (CheckHelp(CmdL) == true)
      return true;

   CacheFile &Cache = *GCache;
   if (GCache->CanCommit() == false) {
      _error->Error(_("You have no permissions for that"));
      return false;
   }
   if (Cache.CheckDeps(CmdL.FileSize() != 1) == false)
      return false;

   // Enter the special broken fixing mode if the user specified arguments
   bool BrokenFix = false;
   if (Cache->BrokenCount() != 0)
      BrokenFix = true;
   
   unsigned int ExpectedInst = 0;
   unsigned int Packages = 0;
   pkgProblemResolver Fix(Cache);
   
   int DefMode = MODE_INSTALL;
   if (strcasecmp(CmdL.FileList[0],"remove") == 0)
      DefMode = MODE_REMOVE;
   else if (strcasecmp(CmdL.FileList[0],"keep") == 0)
      DefMode = MODE_KEEP;

   AutoRestore StateGuard(Cache);

   for (const char **I = CmdL.FileList + 1; *I != 0; I++)
   {
      // Duplicate the string
      size_t Length = strlen(*I);
      char S[300];
      if (Length >= sizeof(S))
	 continue;
      strcpy(S,*I);
      
      // CNC:2003-03-15
      char OrigS[300];
      strcpy(OrigS,S);
      
      // See if we are removing and special indicators..
      int Mode = DefMode;
      char *VerTag = 0;
      bool VerIsRel = false;
      // CNC:2003-11-05
      int VerOp = 0;
      while (Cache->FindPkg(S).end() == true)
      {
	 // Handle an optional end tag indicating what to do
	 if (Length >= 1 && S[Length - 1] == '-')
	 {
	    Mode = MODE_REMOVE;
	    S[--Length] = 0;
	    continue;
	 }
	 
	 if (Length >= 1 && S[Length - 1] == '+')
	 {
	    Mode = MODE_INSTALL;
	    S[--Length] = 0;
	    continue;
	 }
	 
	 // CNC:2003-11-05
	 char *sep = strpbrk(S,"=><");
	 if (sep)
	 {
	    char *p;
	    int eq = 0, gt = 0, lt = 0;

	    VerIsRel = false;
	    for (p = sep; *p && strchr("=><",*p); ++p)
	       switch (*p)
	       {
		  case '=': eq = 1; break;
		  case '>': gt = 1; break;
		  case '<': lt = 1; break;
	       }
	    if (eq)
	    {
	       if (lt && gt)
		  return _error->Error(_("Couldn't parse name '%s'"),S);
	       else if (lt)
		  VerOp = pkgCache::Dep::LessEq;
	       else if (gt)
		  VerOp = pkgCache::Dep::GreaterEq;
	       else
		  VerOp = pkgCache::Dep::Equals;
	    }
	    else
	    {
	       if (lt && gt)
		  VerOp = pkgCache::Dep::NotEquals;
	       else if (lt)
		  VerOp = pkgCache::Dep::Less;
	       else if (gt)
		  VerOp = pkgCache::Dep::Greater;
	       else
		  return _error->Error(_("Couldn't parse name '%s'"),S);
	    }
	    *sep = '\0';
	    VerTag = p;
	 }
	 
	 // CNC:2003-11-21 - Try to handle unknown file items.
	 if (S[0] == '/')
	 {
	    pkgRecords Recs(Cache);
	    if (_error->PendingError() == true)
	       return false;
	    pkgCache::PkgIterator Pkg = (*Cache).PkgBegin();
	    for (; Pkg.end() == false; Pkg++)
	    {
	       // Should we try on all versions?
	       pkgCache::VerIterator Ver = (*Cache)[Pkg].CandidateVerIter(*Cache);
	       if (Ver.end() == false)
	       {
		  pkgRecords::Parser &Parse = Recs.Lookup(Ver.FileList());
		  if (Parse.HasFile(S)) {
		     strcpy(S, Pkg.Name());
		     // Confirm the translation.
		     ExpectedInst += 1000;
		     break;
		  }
	       }
	    }
	 }

	 char *Slash = strchr(S,'/');
	 if (Slash != 0)
	 {
	    VerIsRel = true;
	    *Slash = 0;
	    VerTag = Slash + 1;
	 }
	 
	 break;
      }
      
      // Locate the package
      pkgCache::PkgIterator Pkg = Cache->FindPkg(S);
      Packages++;
      if (Pkg.end() == true)
      {
	 // Check if the name is a regex
	 const char *I;
	 for (I = S; *I != 0; I++)
	    if (*I == '?' || *I == '*' || *I == '|' ||
	        *I == '[' || *I == '^' || *I == '$')
	       break;

	 // CNC:2003-05-15
	 if (*I == 0) {
#ifdef APT_WITH_LUA
	    vector<string> VS;
	    _lua->SetDepCache(Cache);
	    _lua->SetDontFix();
	    _lua->SetGlobal("argument", OrigS);
	    _lua->SetGlobal("translated", VS);
	    _lua->RunScripts("Scripts::AptGet::Install::TranslateArg");
	    const char *name = _lua->GetGlobalStr("translated");
	    if (name != NULL) {
	       VS.push_back(name);
	    } else {
	       VS = _lua->GetGlobalStrList("translated");
	    }
	    _lua->ResetGlobals();
	    _lua->ResetCaches();

	    // Translations must always be confirmed
	    ExpectedInst += 1000;

	    // Run over the matches
	    bool Hit = false;
	    for (vector<string>::const_iterator I = VS.begin();
	         I != VS.end(); I++) {

	       Pkg = Cache->FindPkg(*I);
	       if (Pkg.end() == true)
		  continue;

	       ioprintf(c1out,_("Selecting %s for '%s'\n"),
			Pkg.Name(),OrigS);
	    
	       Hit |= TryToInstall(Pkg,Cache,Fix,Mode,BrokenFix,
				   ExpectedInst,true);
	    }
	 
	    if (Hit == true)
	       continue;
#endif
	    return _error->Error(_("Couldn't find package %s"),S);
	 }

	 // Regexs must always be confirmed
	 ExpectedInst += 1000;
	 
	 // Compile the regex pattern
	 regex_t Pattern;
	 int Res;
	 if ((Res = regcomp(&Pattern,S,REG_EXTENDED | REG_ICASE |
		     REG_NOSUB)) != 0)
	 {
	    char Error[300];	    
	    regerror(Res,&Pattern,Error,sizeof(Error));
	    return _error->Error(_("Regex compilation error - %s"),Error);
	 }
	 
	 // Run over the matches
	 bool Hit = false;
	 for (Pkg = Cache->PkgBegin(); Pkg.end() == false; Pkg++)
	 {
	    if (regexec(&Pattern,Pkg.Name(),0,0,0) != 0)
	       continue;
	    
	    // CNC:2003-11-23
	    ioprintf(c1out,_("Selecting %s for '%s'\n"),
		     Pkg.Name(),S);
	    StateGuard->Ignore(Pkg);
	    
	    if (VerTag != 0)
	       // CNC:2003-11-05
	       if (TryToChangeVer(Pkg,Cache,VerOp,VerTag,VerIsRel) == false)
		  return false;
	    
	    Hit |= TryToInstall(Pkg,Cache,Fix,Mode,BrokenFix,
				ExpectedInst,false);
	 }
	 regfree(&Pattern);
	 
	 if (Hit == false)
	    return _error->Error(_("Couldn't find package %s"),S);
      }
      else
      {
	 if (VerTag != 0)
	    // CNC:2003-11-05
	    if (TryToChangeVer(Pkg,Cache,VerOp,VerTag,VerIsRel) == false)
	       return false;
	 if (TryToInstall(Pkg,Cache,Fix,Mode,BrokenFix,ExpectedInst) == false)
	    return false;
	 StateGuard->Ignore(Pkg);
      }
   }

// CNC:2003-03-19
#ifdef APT_WITH_LUA
   _lua->SetDepCache(Cache);
   _lua->SetDontFix();
   _lua->RunScripts("Scripts::AptGet::Install::PreResolve");
   _lua->ResetCaches();
#endif

   // CNC:2002-08-01
   if (_config->FindB("APT::Remove-Depends",false) == true)
      Fix.RemoveDepends();

   /* If we are in the Broken fixing mode we do not attempt to fix the
      problems. This is if the user invoked install without -f and gave
      packages */
   if (BrokenFix == true && Cache->BrokenCount() != 0)
   {
      ConfirmChanges(Cache, StateGuard);
      c1out << _("There are still broken packages. ")
	    << _("Run `check' to see them.") << endl;
      c1out << _("You can try to fix them automatically with `install --fix-broken'.") << endl;
      return true;
   }

   // Call the scored problem resolver
   if (DefMode != MODE_KEEP) {
      Fix.InstallProtect();
      if (Fix.Resolve(true) == false)
	 _error->Discard();
   } else {
      if (Fix.Resolve(false) == false)
	 _error->Discard();
   }

// CNC:2003-03-19
#ifdef APT_WITH_LUA
   if (Cache->BrokenCount() == 0) {
      _lua->SetDepCache(Cache);
      _lua->SetProblemResolver(&Fix);
      _lua->RunScripts("Scripts::AptGet::Install::PostResolve");
      _lua->ResetCaches();
   }
#endif


   // Now we check the state of the packages,
   if (Cache->BrokenCount() != 0)
   {
      c1out << 
       _("Some packages could not be installed. This may mean that you have\n" 
	 "requested an impossible situation or that some of the repositories\n"
	 "in use are in an inconsistent state at the moment.") << endl;
      if (Packages == 1)
      {
	 c1out << endl;
	 c1out << 
	  _("Since you only requested a single operation it is extremely likely that\n"
	    "the package is simply not installable and a bug report against\n" 
	    "that package should be filed.") << endl;
      }

      c1out << _("The following information may help to resolve the situation:") << endl;
      c1out << endl;
      ShowBroken(c1out,Cache,false);

      return _error->Error(_("Broken packages"));
   }

   ConfirmChanges(Cache, StateGuard);

   return true;
}
									/*}}}*/
// DoDistUpgrade - Automatic smart upgrader				/*{{{*/
// ---------------------------------------------------------------------
/* Intelligent upgrader that will install and remove packages at will */
bool DoDistUpgrade(CommandLine &CmdL)
{
   if (CheckHelp(CmdL,0) == true)
      return true;

   CacheFile &Cache = *GCache;
   if (GCache->CanCommit() == false) {
      _error->Error(_("You have no permissions for that"));
      return false;
   }

   AutoRestore StateGuard(Cache);

   if (GCache->CheckDeps() == false)
      return false;

   c0out << _("Calculating Upgrade... ") << flush;
   if (pkgDistUpgrade(*Cache) == false)
   {
      c0out << _("Failed") << endl;
      ShowBroken(c1out,Cache,false);
      return false;
   }

// CNC:2003-03-19
#ifdef APT_WITH_LUA
   _lua->SetDepCache(Cache);
   _lua->RunScripts("Scripts::AptGet::DistUpgrade");
   _lua->ResetCaches();
#endif
   
   c0out << _("Done") << endl;
   
   ConfirmChanges(Cache, StateGuard);

   return true;
}
									/*}}}*/
// DoClean - Remove download archives					/*{{{*/
// ---------------------------------------------------------------------
/* */
bool DoClean(CommandLine &CmdL)
{
   if (CheckHelp(CmdL,0) == true)
      return true;

   return cmdDoClean(CmdL);
}
									/*}}}*/
bool DoAutoClean(CommandLine &CmdL)
{
   if (CheckHelp(CmdL,0) == true)
      return true;

   // Lock the archive directory
   FileFd Lock;
   if (_config->FindB("Debug::NoLocking",false) == false)
   {
      Lock.Fd(GetLock(_config->FindDir("Dir::Cache::Archives") + "lock"));
      if (_error->PendingError() == true)
	 return _error->Error(_("Unable to lock the download directory"));
   }
   
   CacheFile &Cache = *GCache;
#if 0
   if (Cache.Open() == false)
      return false;
#endif
   
   LogCleaner Cleaner;
   
   return Cleaner.Go(_config->FindDir("Dir::Cache::archives"),*Cache) &&
      Cleaner.Go(_config->FindDir("Dir::Cache::archives") + "partial/",*Cache);
}
									/*}}}*/
// DoCheck - Perform the check operation				/*{{{*/
// ---------------------------------------------------------------------
/* Opening automatically checks the system, this command is mostly used
   for debugging */
bool DoCheck(CommandLine &CmdL)
{
   if (CheckHelp(CmdL,0) == true)
      return true;

   CacheFile &Cache = *GCache;
   AutoRestore StateGuard(Cache);

   if (GCache->CheckDeps() == false)
      return false;
   
   return true;
}
									/*}}}*/
// DoBuildDep - Install/removes packages to satisfy build dependencies  /*{{{*/
// ---------------------------------------------------------------------
/* This function will look at the build depends list of the given source 
   package and install the necessary packages to make it true, or fail. */
bool DoBuildDep(CommandLine &CmdL)
{
   if (CheckHelp(CmdL) == true)
      return true;

   CacheFile &Cache = *GCache;
   if (GCache->CanCommit() == false) {
      _error->Error(_("You have no permissions for that"));
      return false;
   }
   
   AutoRestore StateGuard(Cache);
   
   if (GCache->CheckDeps() == false)
      return false;

   if (CmdL.FileSize() <= 1)
      return _error->Error(_("Must specify at least one package to check builddeps for"));
   
   // Read the source list
   pkgSourceList List;
   if (List.ReadMainList() == false)
      return _error->Error(_("The list of sources could not be read."));
   
   // Create the text record parsers
   pkgRecords Recs(Cache);
   pkgSrcRecords SrcRecs(List);
   if (_error->PendingError() == true)
      return false;

   // Create the download object
   AcqTextStatus Stat(ScreenWidth,_config->FindI("quiet",0));   
   pkgAcquire Fetcher(&Stat);

   unsigned J = 0;
   for (const char **I = CmdL.FileList + 1; *I != 0; I++, J++)
   {
      string Src;
      pkgSrcRecords::Parser *Last = FindSrc(*I,Recs,SrcRecs,Src,*Cache);
      if (Last == 0)
	 return _error->Error(_("Unable to find a source package for %s"),Src.c_str());
            
      // Process the build-dependencies
      vector<pkgSrcRecords::Parser::BuildDepRec> BuildDeps;
      if (Last->BuildDepends(BuildDeps, _config->FindB("APT::Get::Arch-Only",false)) == false)
      	return _error->Error(_("Unable to get build-dependency information for %s"),Src.c_str());
   
      // Also ensure that build-essential packages are present
      Configuration::Item const *Opts = _config->Tree("APT::Build-Essential");
      if (Opts) 
	 Opts = Opts->Child;
      for (; Opts; Opts = Opts->Next)
      {
	 if (Opts->Value.empty() == true)
	    continue;

         pkgSrcRecords::Parser::BuildDepRec rec;
	 rec.Package = Opts->Value;
	 rec.Type = pkgSrcRecords::Parser::BuildDependIndep;
	 rec.Op = 0;
	 BuildDeps.push_back(rec);
      }

      if (BuildDeps.size() == 0)
      {
	 ioprintf(c1out,_("%s has no build depends.\n"),Src.c_str());
	 continue;
      }
      
      // Install the requested packages
      unsigned int ExpectedInst = 0;
      vector <pkgSrcRecords::Parser::BuildDepRec>::iterator D;
      pkgProblemResolver Fix(Cache);
      bool skipAlternatives = false; // skip remaining alternatives in an or group
      for (D = BuildDeps.begin(); D != BuildDeps.end(); D++)
      {
         bool hasAlternatives = (((*D).Op & pkgCache::Dep::Or) == pkgCache::Dep::Or);

         if (skipAlternatives == true)
         {
            if (!hasAlternatives)
               skipAlternatives = false; // end of or group
            continue;
         }

         if ((*D).Type == pkgSrcRecords::Parser::BuildConflict ||
	     (*D).Type == pkgSrcRecords::Parser::BuildConflictIndep)
         {
            pkgCache::PkgIterator Pkg = Cache->FindPkg((*D).Package);
            // Build-conflicts on unknown packages are silently ignored
            if (Pkg.end() == true)
               continue;

            pkgCache::VerIterator IV = (*Cache)[Pkg].InstVerIter(*Cache);

            /* 
             * Remove if we have an installed version that satisfies the 
             * version criteria
             */
            if (IV.end() == false && 
                Cache->VS().CheckDep(IV.VerStr(),(*D).Op,(*D).Version.c_str()) == true)
               TryToInstall(Pkg,Cache,Fix,MODE_REMOVE,false,ExpectedInst);
         }
	 else // BuildDep || BuildDepIndep
         {
            if (_config->FindB("Debug::BuildDeps",false) == true)
                 cout << "Looking for " << (*D).Package << "...\n";

	    pkgCache::PkgIterator Pkg = Cache->FindPkg((*D).Package);

	    // CNC:2003-11-21 - Try to handle unknown file deps.
	    if (Pkg.end() == true && (*D).Package[0] == '/')
	    {
	       const char *File = (*D).Package.c_str();
	       Pkg = (*Cache).PkgBegin();
	       for (; Pkg.end() == false; Pkg++)
	       {
		  // Should we try on all versions?
		  pkgCache::VerIterator Ver = (*Cache)[Pkg].CandidateVerIter(*Cache);
		  if (Ver.end() == false)
		  {
		     pkgRecords::Parser &Parse = Recs.Lookup(Ver.FileList());
		     if (Parse.HasFile(File))
			break;
		  }
	       }
	    }

	    if (Pkg.end() == true)
            {
               if (_config->FindB("Debug::BuildDeps",false) == true)
                    cout << " (not found)" << (*D).Package << endl;

               if (hasAlternatives)
                  continue;

               return _error->Error(_("%s dependency for %s cannot be satisfied "
                                      "because the package %s cannot be found"),
                                    Last->BuildDepType((*D).Type),Src.c_str(),
                                    (*D).Package.c_str());
            }

            /*
             * if there are alternatives, we've already picked one, so skip
             * the rest
             *
             * TODO: this means that if there's a build-dep on A|B and B is
             * installed, we'll still try to install A; more importantly,
             * if A is currently broken, we cannot go back and try B. To fix 
             * this would require we do a Resolve cycle for each package we 
             * add to the install list. Ugh
             */
                       
	    /* 
	     * If this is a virtual package, we need to check the list of
	     * packages that provide it and see if any of those are
	     * installed
	     */
            pkgCache::PrvIterator Prv = Pkg.ProvidesList();
            for (; Prv.end() != true; Prv++)
	    {
               if (_config->FindB("Debug::BuildDeps",false) == true)
                    cout << "  Checking provider " << Prv.OwnerPkg().Name() << endl;

	       if ((*Cache)[Prv.OwnerPkg()].InstVerIter(*Cache).end() == false)
	          break;
            }
            
            // Get installed version and version we are going to install
	    pkgCache::VerIterator IV = (*Cache)[Pkg].InstVerIter(*Cache);

            if ((*D).Version[0] != '\0') {
                 // Versioned dependency

                 pkgCache::VerIterator CV = (*Cache)[Pkg].CandidateVerIter(*Cache);

                 for (; CV.end() != true; CV++)
                 {
                      if (Cache->VS().CheckDep(CV.VerStr(),(*D).Op,(*D).Version.c_str()) == true)
                           break;
                 }
                 if (CV.end() == true)
		   if (hasAlternatives)
		   {
		      continue;
		   }
		   else
		   {
                      return _error->Error(_("%s dependency for %s cannot be satisfied "
                                             "because no available versions of package %s "
                                             "can satisfy version requirements"),
                                           Last->BuildDepType((*D).Type),Src.c_str(),
                                           (*D).Package.c_str());
		   }
            }
            else
            {
               // Only consider virtual packages if there is no versioned dependency
               if (Prv.end() == false)
               {
                  if (_config->FindB("Debug::BuildDeps",false) == true)
                     cout << "  Is provided by installed package " << Prv.OwnerPkg().Name() << endl;
                  skipAlternatives = hasAlternatives;
                  continue;
               }
            }
            if (IV.end() == false)
            {
               if (_config->FindB("Debug::BuildDeps",false) == true)
                  cout << "  Is installed\n";

               if (Cache->VS().CheckDep(IV.VerStr(),(*D).Op,(*D).Version.c_str()) == true)
               {
                  skipAlternatives = hasAlternatives;
                  continue;
               }

               if (_config->FindB("Debug::BuildDeps",false) == true)
                  cout << "    ...but the installed version doesn't meet the version requirement\n";

               if (((*D).Op & pkgCache::Dep::LessEq) == pkgCache::Dep::LessEq)
               {
                  return _error->Error(_("Failed to satisfy %s dependency for %s: Installed package %s is too new"),
                                       Last->BuildDepType((*D).Type),
                                       Src.c_str(),
                                       Pkg.Name());
               }
            }


            if (_config->FindB("Debug::BuildDeps",false) == true)
               cout << "  Trying to install " << (*D).Package << endl;

            if (TryToInstall(Pkg,Cache,Fix,false,false,ExpectedInst) == true)
            {
               // We successfully installed something; skip remaining alternatives
               skipAlternatives = hasAlternatives;
               continue;
            }
            else if (hasAlternatives)
            {
               if (_config->FindB("Debug::BuildDeps",false) == true)
                  cout << "  Unsatisfiable, trying alternatives\n";
               continue;
            }
            else
            {
               return _error->Error(_("Failed to satisfy %s dependency for %s: %s"),
                                    Last->BuildDepType((*D).Type),
                                    Src.c_str(),
                                    (*D).Package.c_str());
            }
	 }	       
      }
      
      Fix.InstallProtect();
      if (Fix.Resolve(true) == false)
	 _error->Discard();
      
      // Now we check the state of the packages,
      if (Cache->BrokenCount() != 0)
	 return _error->Error(_("Some broken packages were found while trying to process build-dependencies for %s.\n"
				"You might want to run `apt-get --fix-broken install' to correct these."),*I);
   }
  
   ConfirmChanges(Cache, StateGuard);

   return true;
}
									/*}}}*/
// * - Scripting stuff.							/*{{{*/
// ---------------------------------------------------------------------
/* */
bool DoScript(CommandLine &CmdL)
{
   if (CheckHelp(CmdL) == true)
      return true;

   CacheFile &Cache = *GCache;
   AutoRestore StateGuard(Cache);

   for (const char **I = CmdL.FileList+1; *I != 0; I++)
      _config->Set("Scripts::AptShell::Script::", *I);

// CNC:2003-03-19
#ifdef APT_WITH_LUA
   _lua->SetDepCache(Cache);
   _lua->RunScripts("Scripts::AptShell::Script");
   _lua->RunScripts("Scripts::AptGet::Script");
   _lua->RunScripts("Scripts::AptCache::Script");
   _lua->ResetCaches();
#endif

   _config->Clear("Scripts::AptShell::Script");

   ConfirmChanges(Cache, StateGuard);

   return true;
}

									/*}}}*/

// --- Unchanged stuff from apt-cache.


// Depends - Print out a dependency tree				/*{{{*/
// ---------------------------------------------------------------------
/* */
bool Depends(CommandLine &CmdL)
{
   if (CheckHelp(CmdL) == true)
      return true;

   return cmdDepends(CmdL, *GCache);
}
									/*}}}*/
// RDepends - Print out a reverse dependency tree - mbc			/*{{{*/
// ---------------------------------------------------------------------
/* */
bool RDepends(CommandLine &CmdL)
{
   if (CheckHelp(CmdL) == true)
      return true;

   return cmdRDepends(CmdL, *GCache);
}

									/*}}}*/
// CNC:2003-02-19
// WhatDepends - Print out a reverse dependency tree			/*{{{*/
// ---------------------------------------------------------------------
/* */
bool WhatDepends(CommandLine &CmdL)
{
   if (CheckHelp(CmdL) == true)
      return true;

   return cmdWhatDepends(CmdL, *GCache);
}

bool WhatProvides(CommandLine &CmdL)
{
   if (CheckHelp(CmdL) == true)
      return true;

   return cmdWhatProvides(CmdL, *GCache);
}
									/*}}}*/
// UnMet - Show unmet dependencies					/*{{{*/
// ---------------------------------------------------------------------
/* */
bool UnMet(CommandLine &CmdL)
{
   if (CheckHelp(CmdL,0) == true)
      return true;

   return cmdUnMet(CmdL, *GCache);
}
									/*}}}*/
// DumpPackage - Show a dump of a package record			/*{{{*/
// ---------------------------------------------------------------------
/* */
bool DumpPackage(CommandLine &CmdL)
{   
   if (CheckHelp(CmdL) == true)
      return true;

   return cmdDumpPackage(CmdL, *GCache);
}
									/*}}}*/
// DisplayRecord - Displays the complete record for the package		/*{{{*/
// ---------------------------------------------------------------------
/* This displays the package record from the proper package index file. 
   It is not used by DumpAvail for performance reasons. */
bool DisplayRecord(pkgCache::VerIterator V)
{
   return cmdDisplayRecord(V, *GCache);
}
									/*}}}*/
bool Search(CommandLine &CmdL)
{
   if (CheckHelp(CmdL) == true)
      return true;

   return cmdSearch(CmdL, *GCache);
}

bool SearchFile(CommandLine &CmdL)
{
   if (CheckHelp(CmdL) == true)
      return true;

   return cmdSearchFile(CmdL, *GCache);
}

bool FileList(CommandLine &CmdL)
{
   if (CheckHelp(CmdL) == true)
      return true;

   return cmdFileList(CmdL, *GCache);
}

bool ChangeLog(CommandLine &CmdL)
{
   if (CheckHelp(CmdL) == true)
      return true;

   return cmdChangeLog(CmdL, *GCache);
}

// DoList - List packages.	 					/*{{{*/
// ---------------------------------------------------------------------
/* */
bool DoList(CommandLine &CmdL)
{
   if (CheckHelp(CmdL) == true)
      return true;

   return cmdDoList(CmdL, *GCache);
}

									/*}}}*/
// ShowPackage - Dump the package record to the screen			/*{{{*/
// ---------------------------------------------------------------------
/* */
bool ShowPackage(CommandLine &CmdL)
{   
   if (CheckHelp(CmdL) == true)
      return true;

   return cmdShowPackage(CmdL, *GCache);
}
// --- End of stuff from apt-cache.


// ShowHelp - Show a help screen					/*{{{*/
// ---------------------------------------------------------------------
/* */
bool ShowHelp(CommandLine &CmdL)
{
   if (CheckHelp(CmdL) == true)
      return true;

   if (CmdL.FileSize() > 1) {
      CommandHelp(CmdL.FileList[1]);
      return true;
   }

   ioprintf(cout,_("%s %s for %s %s compiled on %s %s\n"),PACKAGE,VERSION,
	    COMMON_OS,COMMON_CPU,__DATE__,__TIME__);
	    
   if (_config->FindB("version") == true)
   {
      cout << _("Supported Modules:") << endl;
      
      for (unsigned I = 0; I != pkgVersioningSystem::GlobalListLen; I++)
      {
	 pkgVersioningSystem *VS = pkgVersioningSystem::GlobalList[I];
	 if (_system != 0 && _system->VS == VS)
	    cout << '*';
	 else
	    cout << ' ';
	 cout << "Ver: " << VS->Label << endl;
	 
	 /* Print out all the packaging systems that will work with 
	    this VS */
	 for (unsigned J = 0; J != pkgSystem::GlobalListLen; J++)
	 {
	    pkgSystem *Sys = pkgSystem::GlobalList[J];
	    if (_system == Sys)
	       cout << '*';
	    else
	       cout << ' ';
	    if (Sys->VS->TestCompatibility(*VS) == true)
	       cout << "Pkg:  " << Sys->Label << " (Priority " << Sys->Score(*_config) << ")" << endl;
	 }
      }
      
      for (unsigned I = 0; I != pkgSourceList::Type::GlobalListLen; I++)
      {
	 pkgSourceList::Type *Type = pkgSourceList::Type::GlobalList[I];
	 cout << " S.L: '" << Type->Name << "' " << Type->Label << endl;
      }      
      
      for (unsigned I = 0; I != pkgIndexFile::Type::GlobalListLen; I++)
      {
	 pkgIndexFile::Type *Type = pkgIndexFile::Type::GlobalList[I];
	 cout << " Idx: " << Type->Label << endl;
      }      
      
      return true;
   }
   
   cout << 
    _("\n"
      "Main commands:\n"
      "   status - Show the current selections\n"
      "   install - Install new packages\n"
      "   remove - Remove packages\n"
      "   keep - Keep packages\n"
      "   upgrade - Perform a global upgrade\n"
      "   dist-upgrade - Perform a global distribution upgrade\n"
      "   build-dep - Install build-dependencies for source packages\n"
//      "   dselect-upgrade - Follow dselect selections\n"
      "   update - Retrieve new lists of packages\n"
      "   commit - Apply the changes in the system\n"
      "   quit - Leave the APT shell\n"
      "\n"
      "Auxiliar commands:\n"
      "   show - Show a readable record for the package\n"
      "   showpkg - Show some general information for a single package\n"
      "   list/ls - List packages\n"
      "   search - Search the package list for a regex pattern\n"
      "   searchfile - Search the packages for a file\n"
      "   files - Show file list of the package(s)\n"
      "   changelog - Show changelog entries of the package(s)\n"
      "   script - Run scripts.\n"
      "   depends - Show raw dependency information for a package\n"
      "   whatdepends - Show packages depending on given capabilities\n"
      // "   rdepends - Show reverse dependency information for a package\n"
      "   whatprovides - Show packages that provide given capabilities\n"
      "   check - Verify that there are no broken dependencies\n"
      "   unmet - Show unmet dependencies\n"
      "   clean - Erase downloaded archive files\n"
      "   autoclean - Erase old downloaded archive files\n"
      "\n"
      "For more information type \"help <cmd>\" or \"<cmd> [-h|--help]\".\n"
      "\n"
      "                       This APT has Super Cow Powers.\n");
   return true;
}

void CommandHelp(const char *Name)
{
   unsigned long Hash = 0;
   for (const char *p=Name; *p; p++)
      Hash = 5*Hash + *p;
   switch (Hash%1000000) {
      case 73823: // install
	 c2out << _(
	    "Usage: install [options] pkg1[=ver] [pkg2 ...]\n"
	    "\n"
	    "Try to mark the given packages for installation (new packages,\n"
	    "upgrades or downgrades) changing other packages if necessary.\n"
	    "\n"
	    "Options:\n"
	    "  -h  This help text.\n"
	    "  -y  Assume Yes to all queries and do not prompt\n"
	    "  -f  Attempt to continue if the integrity check fails\n"
	    "  -m  Attempt to continue if archives are unlocatable\n"
	    "  -D  When removing packages, remove dependencies as possible\n"
	    "  -o=? Set an arbitary configuration option, eg -o dir::cache=/tmp\n"
	    "\n"
	 );
	 break;

      case 436466: // remove
	 c2out << _(
	    "Usage: remove [options] pkg1 [pkg2 ...]\n"
	    "\n"
	    "Try to mark the given packages for deletion, changing other\n"
	    "packages if necessary.\n"
	    "\n"
	    "Options:\n"
	    "  -h  This help text.\n"
	    "  -y  Assume Yes to all queries and do not prompt\n"
	    "  -f  Attempt to continue if the integrity check fails\n"
	    "  -m  Attempt to continue if archives are unlocatable\n"
	    "  -D  When removing packages, remove dependencies as possible\n"
	    "  -o=? Set an arbitary configuration option, eg -o dir::cache=/tmp\n"
	    "\n"
	 );
	 break;

      case 16517: // keep
	 c2out << _(
	    "Usage: keep [options] pkg1 [pkg2 ...]\n"
	    "\n"
	    "Try to keep the given packages, currently marked for deletion\n"
	    "or installation, in the system, changing other packages if\n"
	    "necessary.\n"
	    "\n"
	    "Options:\n"
	    "  -h  This help text.\n"
	    "  -y  Assume Yes to all queries and do not prompt\n"
	    "  -f  Attempt to continue if the integrity check fails\n"
	    "  -m  Attempt to continue if archives are unlocatable\n"
	    "  -D  When removing packages, remove dependencies as possible\n"
	    "  -o=? Set an arbitary configuration option, eg -o dir::cache=/tmp\n"
	    "\n"
	 );
	 break;

      case 259776: // upgrade
	 c2out << _(
	    "Usage: upgrade [options]\n"
	    "\n"
	    "Perform a global upgrade.\n"
	    "\n"
	    "Options:\n"
	    "  -h  This help text.\n"
	    "  -y  Assume Yes to all queries and do not prompt\n"
	    "  -f  Attempt to continue if the integrity check fails\n"
	    "  -m  Attempt to continue if archives are unlocatable\n"
	    "  -D  When removing packages, remove dependencies as possible\n"
	    "  -o=? Set an arbitary configuration option, eg -o dir::cache=/tmp\n"
	    "\n"
	 );

      case 900401: // dist-upgrade
	 c2out << _(
	    "Usage: dist-upgrade [options]\n"
	    "\n"
	    "Perform a global distribution upgrade.\n"
	    "\n"
	    "Options:\n"
	    "  -h  This help text.\n"
	    "  -y  Assume Yes to all queries and do not prompt\n"
	    "  -f  Attempt to continue if the integrity check fails\n"
	    "  -m  Attempt to continue if archives are unlocatable\n"
	    "  -D  When removing packages, remove dependencies as possible\n"
	    "  -o=? Set an arbitary configuration option, eg -o dir::cache=/tmp\n"
	    "\n"
	 );

      case 209563: // showpkg
	 c2out << _(
	    "Usage: showpkg [options] pkg1 [pkg2 ...]\n"
	    "\n"
	    "Show some general information for the given packages.\n"
	    "\n"
	    "Options:\n"
	    "  -h  This help text.\n"
	    "  -o=? Set an arbitary configuration option, eg -o dir::cache=/tmp\n"
	    "\n"
	 );
	 break;

      case 17649: // show
	 c2out << _(
	    "Usage: show [options] pkg1 [pkg2 ...]\n"
	    "\n"
	    "Show a readable record for the given packages.\n"
	    "\n"
	    "Options:\n"
	    "  -h  This help text.\n"
	    "  -a  Show information about all versions.\n"
	    "  -o=? Set an arbitary configuration option, eg -o dir::cache=/tmp\n"
	    "\n"
	 );
	 break;

      case 964115: // depends
	 c2out << _(
	    "Usage: depends [options] pkg1 [pkg2 ...]\n"
	    "\n"
	    "Show dependency relations for the given packages.\n"
	    "\n"
	    "Options:\n"
	    "  -h  This help text.\n"
	    "  -o=? Set an arbitary configuration option, eg -o dir::cache=/tmp\n"
	    "\n"
	 );
	 break;

      case 151615: // whatdepends
	 c2out << _(
	    "Usage: whatdepends [options] pkg1 [pkg2 ...]\n"
	    "\n"
	    "Show dependency relations on the given packages.\n"
	    "\n"
	    "Options:\n"
	    "  -h  This help text.\n"
	    "  -o=? Set an arbitary configuration option, eg -o dir::cache=/tmp\n"
	    "\n"
	 );
	 break;

      case 90221: // unmet
	 c2out << _(
	    "Usage: unmet [options]\n"
	    "\n"
	    "Show unsolvable relations in the cache.\n"
	    "\n"
	    "Options:\n"
	    "  -h  This help text.\n"
	    "  -o=? Set an arbitary configuration option, eg -o dir::cache=/tmp\n"
	    "\n"
	 );
	 break;

      case 438074: // search
	 c2out << _(
	    "Usage: search [options] <regex>\n"
	    "\n"
	    "Search for the given regular expression in package names and\n"
	    "descriptions.\n"
	    "\n"
	    "Options:\n"
	    "  -h  This help text.\n"
	    "  -n  Search only in package names.\n"
	    "  -f  Show full records for found packages.\n"
	    "  -o=? Set an arbitary configuration option, eg -o dir::cache=/tmp\n"
	    "\n"
	 );
	 break;

      case 16816: // list
      case 655: // ls
	 c2out << _(
	    "Usage: list/ls [options] [pattern ...]\n"
	    "\n"
	    "List packages matching the given patterns, or all packages if\n"
	    "no pattern is given. Wildcards are accepted.\n"
	    "\n"
	    "Options:\n"
	    "  -h  This help text.\n"
	    "  -i  Show only installed packages.\n"
	    "  -u  Show only installed packages that are upgradable.\n"
	    "  -v  Show installed and candidate versions.\n"
	    "  -s  Show summaries.\n"
	    "  -o=? Set an arbitary configuration option, eg -o dir::cache=/tmp\n"
	    "\n"
	 );
	 break;

      case 395741: // commit
	 c2out << _(
	    "Usage: commit [options]\n"
	    "\n"
	    "Apply the changes in the system.\n"
	    "\n"
	    "Options:\n"
	    "  -h  This help text.\n"
	    "  -o=? Set an arbitary configuration option, eg -o dir::cache=/tmp\n"
	    "\n"
	 );
	 break;

      case 438801: // script
	 c2out << _(
	    "Usage: script [options] script1 [script2]\n"
	    "\n"
	    "Run the given scripts.\n"
	    "\n"
	    "Options:\n"
	    "  -h  This help text.\n"
	    "  -o=? Set an arbitary configuration option, eg -o dir::cache=/tmp\n"
	    "\n"
	 );
	 break;
      default:
	 _error->Error(_("No help for that"));
   }
}
									/*}}}*/

bool DoQuit(CommandLine &CmdL)
{
   _config->Set("quit", "true");
   return true;
}

bool DoCommit(CommandLine &CmdL)
{
   if (CheckHelp(CmdL,0) == true)
      return true;

   if (GCache->CanCommit() == false) {
      _error->Error(_("You have no permissions for that"));
      return false;
   }
   return InstallPackages(*GCache,false);
}

bool DoStatus(CommandLine &CmdL)
{
   if (CheckHelp(CmdL,0) == true)
      return true;

   ShowChanges(*GCache);
   return true;
}

// GetInitialize - Initialize things for apt-get			/*{{{*/
// ---------------------------------------------------------------------
/* */
void GetInitialize()
{
   _config->Set("quiet",0);
   _config->Set("help",false);
   _config->Set("APT::Get::Download-Only",false);
   _config->Set("APT::Get::Simulate",false);
   _config->Set("APT::Get::Assume-Yes",false);
   _config->Set("APT::Get::Fix-Broken",false);
   _config->Set("APT::Get::Force-Yes",false);
   _config->Set("APT::Get::APT::Get::No-List-Cleanup",true);
}
									/*}}}*/
// ReadLine* - readline library stuff					/*{{{*/
// ---------------------------------------------------------------------
/* */
char *ReadLineCompCommands(const char *Text, int State)
{
   static const char *Commands[] =  {"update", "upgrade", "install", "remove",
	 "keep", "dist-upgrade", "dselect-upgrade", "build-dep", "clean",
	 "autoclean", "check", "help", "commit", "exit", "quit", "status",
	 "showpkg", "unmet", "search", "depends", "whatdepends", "rdepends",
	 "show", "script", "searchfile", "files", "changelog", 0};
   static int Last;
   static size_t Len;
   if (State == 0) {
      Last = 0;
      Len = strlen(Text);
   }
   const char *Cmd;
   while ((Cmd = Commands[Last++])) {
      if (strncmp(Cmd, Text, Len) == 0)
	 return strdup(Cmd);
   }
   return NULL;
}

static int CompPackagesMode;

char *ReadLineCompPackages(const char *Text, int State)
{
   CacheFile &Cache = *GCache;
   static pkgCache::PkgIterator Pkg;
   static size_t Len;
   if (State == 0) {
      Pkg = Cache->PkgBegin();
      Len = strlen(Text);
   } else {
      Pkg++;
   }
   const char *Name;
   for (; Pkg.end() == false; Pkg++) {
      Name = Pkg.Name();
      if (Pkg->VersionList == 0) {
	 continue;
      } else if (CompPackagesMode == MODE_REMOVE) {
	 if (Pkg->CurrentVer == 0)
	    continue;
      } else if (CompPackagesMode == MODE_KEEP) {
	 if (Cache[Pkg].Delete() == false && Cache[Pkg].Install() == false)
	    continue;
      }
      if (strncmp(Name, Text, Len) == 0)
	 return strdup(Name);
   }
   return NULL;
}

static const char *CompVersionName;

char *ReadLineCompVersion(const char *Text, int State)
{
   CacheFile &Cache = *GCache;
   static pkgCache::VerIterator Ver;
   static int Len;
   if (State == 0) {
      pkgCache::PkgIterator Pkg = Cache->FindPkg(CompVersionName);
      if (Pkg.end() == true)
	 return NULL;
      Ver = Pkg.VersionList();
      Len = strlen(Text);
   } else {
      Ver++;
   }
   const char *Version;
   for (; Ver.end() == false; Ver++) {
      Version = Ver.VerStr();
      if (strncmp(Version, Text, Len) == 0)
	 return strdup(Version);
   }
   return NULL;
}


unsigned long ReadLineHashCmd(const char *Cmd, const char *CmdEnd)
{
   unsigned long Hash = 0;
   for (; Cmd != CmdEnd; Cmd++)
      Hash = 5*Hash + *Cmd;
   return Hash;
}
   
char **ReadLineCompletion(const char *Text, int Start, int End)
{
   char **Matches = (char **)NULL;
   char *Cmd = rl_line_buffer;
   int Pos = 0;
   for (; *Cmd != 0 && isspace(*Cmd); Pos++, Cmd++);
   rl_attempted_completion_over = 1;
   if (Start == Pos)
      Matches = rl_completion_matches(Text, ReadLineCompCommands);
   else {
      char *CmdEnd = Cmd;
      while (!isspace(*CmdEnd)) CmdEnd++;
      CompPackagesMode = -1;
      switch (ReadLineHashCmd(Cmd,CmdEnd)) {
	 case 2073823: // install
	    if (*(rl_line_buffer+Start-1) == '=') {
	       const char *NameEnd = rl_line_buffer+Start-1;
	       const char *NameStart = NameEnd-1;
	       while (!isspace(*NameStart)) NameStart--;
	       if (++NameStart == NameEnd)
		  return NULL;
	       string PkgName(NameStart, NameEnd-NameStart);
	       CompVersionName = PkgName.c_str();
	       Matches = rl_completion_matches(Text, ReadLineCompVersion);
	    } else {
	       Matches = rl_completion_matches(Text, ReadLineCompPackages);
	    }
	    break;

	 case 436466: // remove
	    CompPackagesMode = MODE_REMOVE;
	 case 16517: // keep
	    if (CompPackagesMode == -1)
	       CompPackagesMode = MODE_KEEP;
	 case 2209563: // showpkg
	 case 17649: // show
	 case 16816: // list
	 case 655: // ls
	 case 1964115: // depends
	 case 1414151615: // whatdepends
	 case 10870365: // rdepends
	    Matches = rl_completion_matches(Text, ReadLineCompPackages);
	    break;

	 default:
	    break;
      }
   }
   return Matches;
}

string ReadLineHistoryFile()
{
   string Path;
   const char *Dir = getenv("HOME");
   if (Dir != NULL) {
      Path = Dir;
      Path += "/.apt_history";
   }
   return Path;
}

void ReadLineInit()
{
   rl_readline_name = "APTShell";
   rl_attempted_completion_function = ReadLineCompletion;
   string History = ReadLineHistoryFile();
   if (History.empty() == false)
      read_history_range(History.c_str(), 0, -1);
}

void ReadLineFinish()
{
   string History = ReadLineHistoryFile();
   if (History.empty() == false)
      write_history(History.c_str());
}
									/*}}}*/

CommandLine::Args *CommandArgs(const char *Name)
{
   static CommandLine::Args ChangeCacheArgs[] = {
      {'h',"help","help",0},
      {'q',"quiet","quiet",CommandLine::IntLevel},
      {'q',"silent","quiet",CommandLine::IntLevel},
      {'y',"yes","APT::Get::Assume-Yes",0},
      {'y',"assume-yes","APT::Get::Assume-Yes",0},      
      {'s',"simulate","APT::Get::Simulate",0},
      {'s',"just-print","APT::Get::Simulate",0},
      {'s',"recon","APT::Get::Simulate",0},
      {'s',"dry-run","APT::Get::Simulate",0},
      {'s',"no-act","APT::Get::Simulate",0},
      {'f',"fix-broken","APT::Get::Fix-Broken",0},
      {'D',"remove-deps","APT::Remove-Depends",0}, // CNC:2002-08-01
      {'V',"verbose-versions","APT::Get::Show-Versions",0},
      {'t',"target-release","APT::Default-Release",CommandLine::HasArg},
      {'t',"default-release","APT::Default-Release",CommandLine::HasArg},
      {0,"reinstall","APT::Get::ReInstall",0},
      {0,"upgrade","APT::Get::upgrade",0},
      {0,"force-yes","APT::Get::Force-Yes",0},
      {0,"ignore-hold","APT::Ignore-Hold",0},      
      {0,"purge","APT::Get::Purge",0},
      {0,"remove","APT::Get::Remove",0},
      {0,"arch-only","APT::Get::Arch-Only",0},
      {'c',"config-file",0,CommandLine::ConfigFile},
      {'o',"option",0,CommandLine::ArbItem},
      {0,0,0,0}};

   static CommandLine::Args CommitArgs[] = {
      {'h',"help","help",0},
      {'q',"quiet","quiet",CommandLine::IntLevel},
      {'q',"silent","quiet",CommandLine::IntLevel},
      {'y',"yes","APT::Get::Assume-Yes",0},
      {'y',"assume-yes","APT::Get::Assume-Yes",0},      
      {'d',"download-only","APT::Get::Download-Only",0},
      {'s',"simulate","APT::Get::Simulate",0},
      {'s',"just-print","APT::Get::Simulate",0},
      {'s',"recon","APT::Get::Simulate",0},
      {'s',"dry-run","APT::Get::Simulate",0},
      {'s',"no-act","APT::Get::Simulate",0},
      {'m',"ignore-missing","APT::Get::Fix-Missing",0},
      {0,"trivial-only","APT::Get::Trivial-Only",0},
      {0,"print-uris","APT::Get::Print-URIs",0},
      {0,"force-yes","APT::Get::Force-Yes",0},
      {0,"download","APT::Get::Download",0},
      {0,"fix-missing","APT::Get::Fix-Missing",0},
      {'c',"config-file",0,CommandLine::ConfigFile},
      {'o',"option",0,CommandLine::ArbItem},
      {0,0,0,0}};

   static CommandLine::Args ShowArgs[] = {
      {'h',"help","help",0},
      {'a',"all-versions","APT::Cache::AllVersions",0},
      {'n',"names-only","APT::Cache::NamesOnly",0},
      {'f',"show-full","APT::Cache::ShowFull",0},
      {'c',"config-file",0,CommandLine::ConfigFile},
      {'o',"option",0,CommandLine::ArbItem},
      {0,0,0,0}};

   static CommandLine::Args SearchArgs[] = {
      {'h',"help","help",0},
      {'n',"names-only","APT::Cache::NamesOnly",0},
      {'f',"show-full","APT::Cache::ShowFull",0},
      {'c',"config-file",0,CommandLine::ConfigFile},
      {'o',"option",0,CommandLine::ArbItem},
      {0,0,0,0}};

   static CommandLine::Args ListArgs[] = {
      {'h',"help","help",0},
      {'u',"upgradable","APT::Cache::ShowUpgradable",0},
      {'i',"installed","APT::Cache::ShowInstalled",0},
      {'v',"version","APT::Cache::ShowVersion",0},
      {'s',"summary","APT::Cache::ShowSummary",0},
      {'n',"installed","APT::Cache::Installed",0},
      {'c',"config-file",0,CommandLine::ConfigFile},
      {'o',"option",0,CommandLine::ArbItem},
      {0,0,0,0}};

   static CommandLine::Args NoArgs[] = {
      {'h',"help","help",0},
      {'v',"version","version",0},
      {'c',"config-file",0,CommandLine::ConfigFile},
      {'o',"option",0,CommandLine::ArbItem},
      {0,0,0,0}};


   unsigned long Hash = 0;
   for (const char *p=Name; *p; p++)
      Hash = 5*Hash + *p;
   switch (Hash%1000000) {
      case 73823: // install
      case 436466: // remove
      case 16517: // keep
      case 259776: // upgrade
      case 900401: // dist-upgrade
	 return ChangeCacheArgs;

      case 395741: // commit
	 return CommitArgs;

      case 209563: // showpkg
      case 17649: // show
	 return ShowArgs;

      case 438074: // search
	 return SearchArgs;

      case 16816: // list
      case 655: // ls
	 return ListArgs;

      default:
	 return NoArgs;
   }
}
									/*}}}*/
int main(int argc,const char *argv[])
{
   CommandLine::Dispatch Cmds[] = {{"update",&DoUpdate},
                                   {"upgrade",&DoUpgrade},
                                   {"install",&DoInstall},
                                   {"remove",&DoInstall},
                                   {"keep",&DoInstall},
                                   {"dist-upgrade",&DoDistUpgrade},
				   {"build-dep",&DoBuildDep},
                                   {"clean",&DoClean},
                                   {"autoclean",&DoAutoClean},
                                   {"check",&DoCheck},
				   {"help",&ShowHelp},
				   {"commit",&DoCommit},
				   {"quit",&DoQuit},
				   {"exit",&DoQuit},
				   {"status",&DoStatus},
				   {"script",&DoScript},
				   // apt-cache
				   {"showpkg",&DumpPackage},
                                   {"unmet",&UnMet},
                                   {"search",&Search},
                                   {"search",&SearchFile},
                                   {"files",&FileList},
                                   {"changelog",&ChangeLog},
                                   {"list",&DoList},
                                   {"ls",&DoList},
                                   {"depends",&Depends},
                                   {"whatdepends",&WhatDepends},
                                   {"whatprovides",&WhatProvides},
                                   {"rdepends",&RDepends},
                                   {"show",&ShowPackage},
                                   {0,0}};

   // Set up gettext support
   setlocale(LC_ALL,"");
   textdomain(PACKAGE);

   // Parse the command line and initialize the package library
   CommandLine CmdL(CommandArgs(""),_config);
   if (pkgInitConfig(*_config) == false ||
       CmdL.Parse(argc,argv) == false ||
       pkgInitSystem(*_config,_system) == false)
   {
      if (_config->FindB("version") == true)
	 ShowHelp(CmdL);
	 
      _error->DumpErrors();
      return 100;
   }

   // Deal with stdout not being a tty
   if (ttyname(STDOUT_FILENO) == 0 && _config->FindI("quiet",0) < 1)
      _config->Set("quiet","1");

   // Setup the output streams
   c0out.rdbuf(cout.rdbuf());
   c1out.rdbuf(cout.rdbuf());
   c2out.rdbuf(cout.rdbuf());
   if (_config->FindI("quiet",0) > 0)
      c0out.rdbuf(devnull.rdbuf());
   if (_config->FindI("quiet",0) > 1)
      c1out.rdbuf(devnull.rdbuf());

   // Setup the signals
   signal(SIGPIPE,SIG_IGN);
   signal(SIGINT,SIG_IGN);
   signal(SIGWINCH,SigWinch);
   SigWinch(0);

   // Prepare the cache
   GCache = new CacheFile();
   GCache->Open();

   // CNC:2004-02-18
   if (_error->empty() == false)
   {
      bool Errors = _error->PendingError();
      _error->DumpErrors();
      return Errors == true?100:0;
   }

   if (GCache->CheckDeps(true) == false) {
      c1out << _("There are broken packages. ")
	    << _("Run `check' to see them.") << endl;
      c1out << _("You can try to fix them automatically with `install --fix-broken'.") << endl;
   }

   // Make a copy of the configuration. Each command will modify its
   // own copy of the whole configuration.
   Configuration GlobalConfig(*_config);

   ReadLineInit();
   c1out << _("Welcome to the APT shell. Type \"help\" for more information.") << endl;

// CNC:2003-03-19
#ifdef APT_WITH_LUA
   _lua->SetDepCache(*GCache);
   _lua->RunScripts("Scripts::AptShell::Init");
   _lua->ResetCaches();
   bool HasCmdScripts = (_lua->HasScripts("Scripts::AptGet::Command") ||
			 _lua->HasScripts("Scripts::AptCache::Command"));
#endif

   size_t largc;
   const char *largv[1024];
   char *line, *p, *q;
   largv[0] = "";
   while (_config->FindB("quit") == false)
   {
      if (_error->empty() == false)
      {
	 _error->DumpErrors();
	 continue;
      }
      
      line = readline(_config->Find("APT::Shell::Prompt", "apt> ").c_str());
      if (!line || !*line) {
	 free(line);
	 continue;
      }
      add_history(line);

      largc = 1; // CommandLine.Parse() ignores the first option.

      // Split the line into arguments, handling quotes.
      p = q = line;
      // *p = parsed contents, assigned from *q
      // *q = buffer checker, copying valid stuff to *p
      while (*q != 0)
      {
	 if (largc > sizeof(largv)/sizeof(*largv))
	 {
	    _error->Error(_("Exceeded maximum number of command arguments"));
	    break;
	 }
	 while (isspace(*q)) q++;
	 if (*q == 0)
	    break;
	 largv[largc++] = p = q;
	 while (*q != 0 && !isspace(*q)) {
	    if (*q == '"' || *q == '\'') {
	       char quote = *q++;
	       while (*q != 0) {
		  if (*q == quote) {
		     q++;
		     break;
		  } else if (*q == '\\') {
		     switch (*(++q)) {
			case '\0':
			   break;
			case '\\':
			   *p++ = '\\';
			   break;
			default:
			   *p++ = *q;
			   break;
		     }
		     q++;
		  } else {
		     *p++ = *q++;
		  }
	       }
	    } else {
	       *p++ = *q++;
	    }
	 }
	 if (*q != 0)
	    q++;
	 *p++ = 0;
      }
      if (largc == 1 || _error->empty() == false)
	 continue;
      largv[largc] = 0;
      
      // Make our own copy of the configuration.
      delete _config;
      _config = new Configuration(GlobalConfig);

      // Prepare the command line
      CommandLine CmdL(CommandArgs(largv[1]),_config);
      CmdL.Parse(largc,largv);

// CNC:2003-03-19
#ifdef APT_WITH_LUA
      if (HasCmdScripts == true && _error->PendingError() == false) {
	 _lua->SetDepCache(*GCache);
	 _lua->SetGlobal("command_args", CmdL.FileList);
	 _lua->SetGlobal("command_consume", 0.0);
	 _lua->RunScripts("Scripts::AptGet::Command", true);
	 _lua->RunScripts("Scripts::AptCache::Command", true);
	 double Consume = _lua->GetGlobalNum("command_consume");
	 _lua->ResetGlobals();
	 _lua->ResetCaches();
	 if (Consume == 1) {
	    free(line);
	    continue;
	 }
      }
#endif

      if (_error->PendingError() == false)
	 CmdL.DispatchArg(Cmds);
      
      free(line);
   }

   ReadLineFinish();

   delete GCache;

   // Print any errors or warnings found during parsing
   if (_error->empty() == false)
   {
      bool Errors = _error->PendingError();
      _error->DumpErrors();
      return Errors == true?100:0;
   }
   
   return 0;   
}

// vim:sts=3:sw=3
