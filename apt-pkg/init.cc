// -*- mode: c++; mode: fold -*-
// Description								/*{{{*/
// $Id: init.cc,v 1.20 2003/02/09 20:31:05 doogie Exp $
/* ######################################################################

   Init - Initialize the package library
   
   ##################################################################### */
									/*}}}*/
// Include files							/*{{{*/
#include <apt-pkg/init.h>
#include <apt-pkg/fileutl.h>
#include <apt-pkg/error.h>

#include <cstdlib>
#include <cstring>
#include <apti18n.h>
#include <config.h>
#include <sys/stat.h>

// CNC:2003-03-17
#include <apt-pkg/luaiface.h>

									/*}}}*/

#define Stringfy_(x) # x
#define Stringfy(x)  Stringfy_(x)
const char *pkgVersion = VERSION;
const char *pkgLibVersion = Stringfy(APT_PKG_MAJOR) "."
                            Stringfy(APT_PKG_MINOR) "." 
                            Stringfy(APT_PKG_RELEASE);
const char *pkgCPU = COMMON_CPU;
const char *pkgOS = COMMON_OS;
    
// pkgInitConfig - Initialize the configuration class			/*{{{*/
// ---------------------------------------------------------------------
/* Directories are specified in such a way that the FindDir function will
   understand them. That is, if they don't start with a / then their parent
   is prepended, this allows a fair degree of flexability. */
bool pkgInitConfig(Configuration &Cnf)
{
   // General APT things
   if (strcmp(COMMON_OS,"linux") == 0 ||
       strcmp(COMMON_OS,"unknown") == 0)
      Cnf.Set("APT::Architecture",COMMON_CPU);
   else
      Cnf.Set("APT::Architecture",COMMON_OS "-" COMMON_CPU);
   // CNC:2002-09-10
   //Cnf.Set("APT::Build-Essential::", "build-essential");
   Cnf.Set("Dir","/");
   
   // State   
   Cnf.Set("Dir::State","var/lib/apt/");
   
   /* Just in case something goes horribly wrong, we can fall back to the
      old /var/state paths.. */
   struct stat St;   
   if (stat("/var/lib/apt/.",&St) != 0 &&
       stat("/var/state/apt/.",&St) == 0)
      Cnf.Set("Dir::State","var/state/apt/");
       
   Cnf.Set("Dir::State::lists","lists/");
   Cnf.Set("Dir::State::cdroms","cdroms.list");
   
   // Cache
   Cnf.Set("Dir::Cache","var/cache/apt/");
   Cnf.Set("Dir::Cache::archives","archives/");
   Cnf.Set("Dir::Cache::srcpkgcache","srcpkgcache.bin");
   Cnf.Set("Dir::Cache::pkgcache","pkgcache.bin");
   
   // Configuration
   Cnf.Set("Dir::Etc","etc/apt/");
   Cnf.Set("Dir::Etc::sourcelist","sources.list");
   // CNC:2003-03-03
   Cnf.Set("Dir::Etc::sourceparts","sources.list.d");
   Cnf.Set("Dir::Etc::vendorlist","vendors.list");
   Cnf.Set("Dir::Etc::vendorparts","vendors.list.d");
   Cnf.Set("Dir::Etc::main","apt.conf");
   Cnf.Set("Dir::Etc::parts","apt.conf.d");
   Cnf.Set("Dir::Etc::preferences","preferences");
   Cnf.Set("Dir::Bin::methods",LIBDIR "/apt/methods");
	      
   bool Res = true;
   
   // Read an alternate config file
   const char *Cfg = getenv("APT_CONFIG");
   if (Cfg != 0 && FileExists(Cfg) == true)
      Res &= ReadConfigFile(Cnf,Cfg);
   
   // Read the configuration parts dir
   string Parts = Cnf.FindDir("Dir::Etc::parts");
   if (FileExists(Parts) == true)
      Res &= ReadConfigDir(Cnf,Parts);
      
   // Read the main config file
   string FName = Cnf.FindFile("Dir::Etc::main");
   if (FileExists(FName) == true)
      Res &= ReadConfigFile(Cnf,FName);
   
   if (Res == false)
      return false;
   
   if (Cnf.FindB("Debug::pkgInitConfig",false) == true)
      Cnf.Dump();
   
   bindtextdomain(APT_DOMAIN,LOCALEDIR);
   
   return true;
}
									/*}}}*/

// CNC:2003-02-16 - We must do that to force a statically linked libapt-pkg
// 		    library to include the package systems into the binary.
#include <apt-pkg/rpmsystem.h>
//#include <apt-pkg/debsystem.h>
void ForceLinkage()
{
	rpmSystem *rpmsys = &rpmSys;
	rpmsys->ArchiveSupported("");
	//debSystem *debsys = &debSys;
	//depsys->ArchiveSupported("");
}

// pkgInitSystem - Initialize the _system calss				/*{{{*/
// ---------------------------------------------------------------------
/* */
bool pkgInitSystem(Configuration &Cnf,pkgSystem *&Sys)
{
   ForceLinkage(); // CNC:2003-02-16 - See above.

   Sys = 0;
   string Label = Cnf.Find("Apt::System","");
   if (Label.empty() == false)
   {
      Sys = pkgSystem::GetSystem(Label.c_str());
      if (Sys == 0)
	 return _error->Error(_("Packaging system '%s' is not supported"),Label.c_str());
   }
   else
   {
      signed MaxScore = 0;
      for (unsigned I = 0; I != pkgSystem::GlobalListLen; I++)
      {
	 signed Score = pkgSystem::GlobalList[I]->Score(Cnf);
	 if (Score > MaxScore)
	 {
	    MaxScore = Score;
	    Sys = pkgSystem::GlobalList[I];
	 }
      }
      
      if (Sys == 0)
	 return _error->Error(_("Unable to determine a suitable system type"));
   }

   // CNC:2003-03-15
   bool Ret = Sys->Initialize(Cnf);
   // LORG:2006-02-06
   Cnf.CndSet("APT::DistroVersion", Sys->DistroVer());

#ifdef APT_WITH_LUA
   _lua->RunScripts("Scripts::Init");
#endif
   return Ret;
}
									/*}}}*/
