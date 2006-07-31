// -*- mode: c++; mode: fold -*-
// Description								/*{{{*/
/* ######################################################################

   RPM Package Manager - Provide an interface to rpm
   
   ##################################################################### 
 */
									/*}}}*/
// Includes								/*{{{*/
#ifdef __GNUG__
#pragma implementation "apt-pkg/rpmpm.h"
#endif

#include <config.h>

#ifdef HAVE_RPM

#include <apt-pkg/rpmpm.h>
#include <apt-pkg/error.h>
#include <apt-pkg/configuration.h>
#include <apt-pkg/luaiface.h>
#include <apt-pkg/depcache.h>

#include <apti18n.h>

#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <stdio.h>
#include <iostream>

#if RPM_VERSION >= 0x040100
#include <rpm/rpmdb.h>
#else
#define rpmpsPrint(a,b) rpmProblemSetPrint(a,b)
#define rpmpsFree(a) rpmProblemSetFree(a)
#define rpmReadPackageFile(a,b,c,d) rpmReadPackageHeader(b,d,0,NULL,NULL)
#if RPM_VERSION < 0x040000
#define rpmtransFlags int
#define rpmprobFilterFlags int
#endif
#endif
#include "rpmshowprogress.h"

// RPMPM::pkgRPMPM - Constructor					/*{{{*/
// ---------------------------------------------------------------------
/* */
pkgRPMPM::pkgRPMPM(pkgDepCache *Cache) : pkgPackageManager(Cache)
{
}
									/*}}}*/
// RPMPM::pkgRPMPM - Destructor					/*{{{*/
// ---------------------------------------------------------------------
/* */
pkgRPMPM::~pkgRPMPM()
{
}
									/*}}}*/
// RPMPM::Install - Install a package					/*{{{*/
// ---------------------------------------------------------------------
/* Add an install operation to the sequence list */
bool pkgRPMPM::Install(PkgIterator Pkg,string File)
{
   if (File.empty() == true || Pkg.end() == true)
      return _error->Error(_("Internal Error, No file name for %s"),Pkg.Name());

   List.push_back(Item(Item::Install,Pkg,File));
   return true;
}
									/*}}}*/
// RPMPM::Configure - Configure a package				/*{{{*/
// ---------------------------------------------------------------------
/* Add a configure operation to the sequence list */
bool pkgRPMPM::Configure(PkgIterator Pkg)
{
   if (Pkg.end() == true) {
      return false;
   }
   
   List.push_back(Item(Item::Configure,Pkg));
   return true;
}
									/*}}}*/
// RPMPM::Remove - Remove a package					/*{{{*/
// ---------------------------------------------------------------------
/* Add a remove operation to the sequence list */
bool pkgRPMPM::Remove(PkgIterator Pkg,bool Purge)
{
   if (Pkg.end() == true)
      return false;
   
   if (Purge == true)
      List.push_back(Item(Item::Purge,Pkg));
   else
      List.push_back(Item(Item::Remove,Pkg));
   return true;
}
									/*}}}*/
// RPMPM::RunScripts - Run a set of scripts				/*{{{*/
// ---------------------------------------------------------------------
/* This looks for a list of script sto run from the configuration file,
   each one is run with system from a forked child. */
bool pkgRPMPM::RunScripts(const char *Cnf)
{
   Configuration::Item const *Opts = _config->Tree(Cnf);
   if (Opts == 0 || Opts->Child == 0)
      return true;
   Opts = Opts->Child;

   bool error = false;
   for (; Opts != 0; Opts = Opts->Next)
   {
      if (Opts->Value.empty() == true)
         continue;
		
      // Purified Fork for running the script
      pid_t Process = ExecFork();      
      if (Process == 0)
      {
	 if (chdir("/tmp") != 0)
	    _exit(100);

	 const char *Args[4];
	 Args[0] = "/bin/sh";
	 Args[1] = "-c";
	 Args[2] = Opts->Value.c_str();
	 Args[3] = 0;
	 execv(Args[0],(char **)Args);
	 _exit(100);
      }
      
      // Clean up the sub process
      if (ExecWait(Process,Opts->Value.c_str()) == false) {
	 _error->Error(_("Problem executing scripts %s '%s'"),Cnf,
		       Opts->Value.c_str());
	 error = true;
      }
   }
 
   // Restore sig int/quit
   signal(SIGQUIT,SIG_DFL);
   signal(SIGINT,SIG_DFL);   

   if (error)
      return _error->Error(_("Sub-process returned an error code"));
   
   return true;
}

                                                                        /*}}}*/
// RPMPM::RunScriptsWithPkgs - Run scripts with package names on stdin /*{{{*/
// ---------------------------------------------------------------------
/* This looks for a list of scripts to run from the configuration file
   each one is run and is fed on standard input a list of all .deb files
   that are due to be installed. */
bool pkgRPMPM::RunScriptsWithPkgs(const char *Cnf)
{
   Configuration::Item const *Opts = _config->Tree(Cnf);
   if (Opts == 0 || Opts->Child == 0)
      return true;
   Opts = Opts->Child;
   
   for (; Opts != 0; Opts = Opts->Next)
   {
      if (Opts->Value.empty() == true)
         continue;
		
      // Create the pipes
      int Pipes[2];
      if (pipe(Pipes) != 0)
	 return _error->Errno("pipe",_("Failed to create IPC pipe to subprocess"));
      SetCloseExec(Pipes[0],true);
      SetCloseExec(Pipes[1],true);
      
      // Purified Fork for running the script
      pid_t Process = ExecFork();      
      if (Process == 0)
      {
	 // Setup the FDs
	 dup2(Pipes[0],STDIN_FILENO);
	 SetCloseExec(STDOUT_FILENO,false);
	 SetCloseExec(STDIN_FILENO,false);      
	 SetCloseExec(STDERR_FILENO,false);

	 const char *Args[4];
	 Args[0] = "/bin/sh";
	 Args[1] = "-c";
	 Args[2] = Opts->Value.c_str();
	 Args[3] = 0;
	 execv(Args[0],(char **)Args);
	 _exit(100);
      }
      close(Pipes[0]);
      FileFd Fd(Pipes[1]);

      // Feed it the filenames.
      for (vector<Item>::iterator I = List.begin(); I != List.end(); I++)
      {
	 // Only deal with packages to be installed from .rpm
	 if (I->Op != Item::Install)
	    continue;

	 // No errors here..
	 if (I->File[0] != '/')
	    continue;
	 
	 /* Feed the filename of each package that is pending install
	    into the pipe. */
	 if (Fd.Write(I->File.c_str(),I->File.length()) == false || 
	     Fd.Write("\n",1) == false)
	 {
	    kill(Process,SIGINT);	    
	    Fd.Close();   
	    ExecWait(Process,Opts->Value.c_str(),true);
	    return _error->Error(_("Failure running script %s"),Opts->Value.c_str());
	 }
      }
      Fd.Close();
      
      // Clean up the sub process
      if (ExecWait(Process,Opts->Value.c_str()) == false)
	 return _error->Error(_("Failure running script %s"),Opts->Value.c_str());
   }

   return true;
}

									/*}}}*/


// RPMPM::Go - Run the sequence						/*{{{*/
// ---------------------------------------------------------------------
/* This globs the operations and calls rpm */
bool pkgRPMPM::Go()
{
   if (List.empty() == true)
      return true;

   if (RunScripts("RPM::Pre-Invoke") == false)
      return false;

   if (RunScriptsWithPkgs("RPM::Pre-Install-Pkgs") == false)
      return false;
   
   vector<const char*> install_or_upgrade;
   vector<const char*> install;
   vector<const char*> upgrade;
   vector<const char*> uninstall;
   vector<pkgCache::Package*> pkgs_install;
   vector<pkgCache::Package*> pkgs_uninstall;

   vector<char*> unalloc;
   
   for (vector<Item>::iterator I = List.begin(); I != List.end(); I++)
   {
      string Name = I->Pkg.Name();
      string RealName = Name;
      string::size_type loc;

      switch (I->Op)
      {
      case Item::Purge:
      case Item::Remove:
	 // Unmunge our package names so rpm can find them...
	 if ((loc = Name.rfind(".32bit", Name.length())) != string::npos) {
	    RealName = Name.substr(0,loc);
	 } else if ((loc = Name.rfind("#", Name.length())) != string::npos) {
	    RealName = Name.substr(0,loc) + "-" + I->Pkg.CurrentVer().VerStr();
	 }
#if RPM_VERSION >= 0x040202
	 // This is needed for removal to work on multilib packages, but old
	 // rpm versions don't support name.arch in RPMDBI_LABEL, oh well...
	 RealName = RealName + "." + I->Pkg.CurrentVer().Arch();
#endif
	 uninstall.push_back(strdup(RealName.c_str()));
	 unalloc.push_back(strdup(RealName.c_str()));
	 pkgs_uninstall.push_back(I->Pkg);
	 break;

       case Item::Configure:
	 break;

       case Item::Install:
	 if ((loc = Name.rfind("#", Name.length())) != string::npos) {
	    RealName = Name.substr(0,loc);
	 } 
	 if (Name != RealName) {
	    PkgIterator Pkg = Cache.FindPkg(RealName);
	    PrvIterator Prv = Pkg.ProvidesList();
	    bool Installed = false;
	    for (; Prv.end() == false; Prv++) {
	       if (Prv.OwnerPkg().CurrentVer().end() == false) {
		  Installed = true;
		  break;
	       }
	    }
	    // This looks backwards but it's supposed to "fix problems where a 
	    // package with a different name is being installed with 
	    // Allow-Duplicated and requires additional dependencies, but there's 
	    // no other package with the same name in the system."
	    if (Installed)
	       install.push_back(I->File.c_str());
	    else
	       upgrade.push_back(I->File.c_str());
	 } else {
	    // perform pure installs on non-installed normal packages, not upgrades
	    if (I->Pkg->CurrentVer != NULL) {
	       upgrade.push_back(I->File.c_str());
	    } else {
	       install.push_back(I->File.c_str());
	    }
	 }
	 install_or_upgrade.push_back(I->File.c_str());
	 pkgs_install.push_back(I->Pkg);
	 break;
	  
       default:
	 return _error->Error(_("Unknown pkgRPMPM operation."));
      }
   }

   bool Ret = true;

#ifdef WITH_LUA
   if (_lua->HasScripts("Scripts::PM::Pre") == true) {
      _lua->SetGlobal("files_install", install_or_upgrade);
      _lua->SetGlobal("names_remove", uninstall);
      _lua->SetGlobal("pkgs_install", pkgs_install);
      _lua->SetGlobal("pkgs_remove", pkgs_uninstall);
      _lua->SetDepCache(&Cache);
      _lua->RunScripts("Scripts::PM::Pre");
      _lua->ResetCaches();
      _lua->ResetGlobals();
      if (_error->PendingError() == true) {
	 _error->DumpErrors();
	 Ret = false;
	 goto exit;
      }
   }
#endif

   if (Process(install, upgrade, uninstall) == false)
      Ret = false;

#ifdef WITH_LUA
   if (_lua->HasScripts("Scripts::PM::Post") == true) {
      _lua->SetGlobal("files_install", install_or_upgrade);
      _lua->SetGlobal("names_remove", uninstall);
      _lua->SetGlobal("pkgs_install", pkgs_install);
      _lua->SetGlobal("pkgs_remove", pkgs_uninstall);
      _lua->SetGlobal("transaction_success", Ret);
      _lua->SetDepCache(&Cache);
      _lua->RunScripts("Scripts::PM::Post");
      _lua->ResetCaches();
      _lua->ResetGlobals();
      if (_error->PendingError() == true) {
	 _error->DumpErrors();
	 Ret = false;
	 goto exit;
      }
   }
#endif

   
   if (Ret == true)
      Ret = RunScripts("RPM::Post-Invoke");

exit:
   for (vector<char *>::iterator I = unalloc.begin(); I != unalloc.end(); I++)
      free(*I);

   return Ret;
}
									/*}}}*/
// pkgRPMPM::Reset - Dump the contents of the command list		/*{{{*/
// ---------------------------------------------------------------------
/* */
void pkgRPMPM::Reset() 
{
   List.erase(List.begin(),List.end());
}
									/*}}}*/
// RPMExtPM::pkgRPMExtPM - Constructor					/*{{{*/
// ---------------------------------------------------------------------
/* */
pkgRPMExtPM::pkgRPMExtPM(pkgDepCache *Cache) : pkgRPMPM(Cache)
{
}
									/*}}}*/
// RPMExtPM::pkgRPMExtPM - Destructor					/*{{{*/
// ---------------------------------------------------------------------
/* */
pkgRPMExtPM::~pkgRPMExtPM()
{
}
									/*}}}*/

bool pkgRPMExtPM::ExecRPM(Item::RPMOps op, vector<const char*> &files)
{
   const char *Args[10000];      
   const char *operation = NULL;
   unsigned int n = 0;
   bool Interactive = _config->FindB("RPM::Interactive",true);
   
   Args[n++] = _config->Find("Dir::Bin::rpm","rpm").c_str();

   bool nodeps = false;

   switch (op)
   {
      case Item::RPMInstall:
	 if (Interactive)
	    operation = "-ivh";
	 else
	    operation = "-iv";
	 nodeps = true;
	 break;

      case Item::RPMUpgrade:
	 if (Interactive)
	    operation = "-Uvh";
	 else
	    operation = "-Uv";
	 break;

      case Item::RPMErase:
	 operation = "-e";
	 nodeps = true;
	 break;
   }
   Args[n++] = operation;

   if (Interactive == false && op != Item::RPMErase)
      Args[n++] = "--percent";
    
   string rootdir = _config->Find("RPM::RootDir", "");
   if (!rootdir.empty()) 
   {
       Args[n++] = "-r";
       Args[n++] = rootdir.c_str();
   }

   Configuration::Item const *Opts;
   if (op == Item::RPMErase)
   {
      Opts = _config->Tree("RPM::Erase-Options");
      if (Opts != 0)
      {
	 Opts = Opts->Child;
	 for (; Opts != 0; Opts = Opts->Next)
	 {
	    if (Opts->Value == "--nodeps")
	       nodeps = false;
	    else if (Opts->Value.empty() == true)
	       continue;
	    Args[n++] = Opts->Value.c_str();
	 }
      }
   }
   else
   {
      bool oldpackage = _config->FindB("RPM::OldPackage",
				       (op == Item::RPMUpgrade));
      bool replacepkgs = _config->FindB("APT::Get::ReInstall",false);
      bool replacefiles = _config->FindB("APT::Get::ReInstall",false);
      Opts = _config->Tree("RPM::Install-Options");
      if (Opts != 0)
      {
	 Opts = Opts->Child;
	 for (; Opts != 0; Opts = Opts->Next)
	 {
	    if (Opts->Value == "--oldpackage")
	       oldpackage = false;
	    else if (Opts->Value == "--replacepkgs")
	       replacepkgs = false;
	    else if (Opts->Value == "--replacefiles")
	       replacefiles = false;
	    else if (Opts->Value == "--nodeps")
	       nodeps = false;
	    else if (Opts->Value.empty() == true)
	       continue;
	    Args[n++] = Opts->Value.c_str();
	 }	 
      }
      if (oldpackage == true)
	 Args[n++] = "--oldpackage";
      if (replacepkgs == true)
	 Args[n++] = "--replacepkgs";
      if (replacefiles == true)
	 Args[n++] = "--replacefiles";
   }

   if (nodeps == true)
      Args[n++] = "--nodeps";

   Opts = _config->Tree("RPM::Options");
   if (Opts != 0)
   {
      Opts = Opts->Child;
      for (; Opts != 0; Opts = Opts->Next)
      {
	 if (Opts->Value.empty() == true)
	    continue;
	 Args[n++] = Opts->Value.c_str();
      }	 
   }

   if (_config->FindB("RPM::Order", true) == false)
      Args[n++] = "--noorder";
    
   bool FilesInArgs = true;
   char *ArgsFileName = NULL;
#if RPM_VERSION >= 0x040000
   if (op != Item::RPMErase && files.size() > 50) {
      string FileName = _config->FindDir("Dir::Cache", "/tmp/") +
			"filelist.XXXXXX";
      ArgsFileName = strdup(FileName.c_str());
      if (ArgsFileName) {
	 int fd = mkstemp(ArgsFileName);
	 if (fd != -1) {
	    FileFd File(fd);
	    for (vector<const char*>::iterator I = files.begin();
		 I != files.end(); I++) {
	       File.Write(*I, strlen(*I));
	       File.Write("\n", 1);
	    }
	    File.Close();
	    FilesInArgs = false;
	    Args[n++] = ArgsFileName;
	 }
      }
   }
#endif

   if (FilesInArgs == true) {
      for (vector<const char*>::iterator I = files.begin();
	   I != files.end(); I++)
	 Args[n++] = *I;
   }
   
   Args[n++] = 0;

   if (_config->FindB("Debug::pkgRPMPM",false) == true)
   {
      for (unsigned int k = 0; k < n; k++)
	  clog << Args[k] << ' ';
      clog << endl;
      if (ArgsFileName) {
	 unlink(ArgsFileName);
	 free(ArgsFileName);
      }
      return true;
   }

   cout << _("Executing RPM (")<<operation<<")..." << endl;

   cout << flush;
   clog << flush;
   cerr << flush;

   /* Mask off sig int/quit. We do this because dpkg also does when 
    it forks scripts. What happens is that when you hit ctrl-c it sends
    it to all processes in the group. Since dpkg ignores the signal 
    it doesn't die but we do! So we must also ignore it */
   //akk ??
   signal(SIGQUIT,SIG_IGN);
   signal(SIGINT,SIG_IGN);

   // Fork rpm
   pid_t Child = ExecFork();
            
   // This is the child
   if (Child == 0)
   {
      if (chdir(_config->FindDir("RPM::Run-Directory","/").c_str()) != 0)
	  _exit(100);
	 
      if (_config->FindB("RPM::FlushSTDIN",true) == true)
      {
	 int Flags,dummy;
	 if ((Flags = fcntl(STDIN_FILENO,F_GETFL,dummy)) < 0)
	     _exit(100);
	 
	 // Discard everything in stdin before forking dpkg
	 if (fcntl(STDIN_FILENO,F_SETFL,Flags | O_NONBLOCK) < 0)
	     _exit(100);
	 
	 while (read(STDIN_FILENO,&dummy,1) == 1);
	 
	 if (fcntl(STDIN_FILENO,F_SETFL,Flags & (~(long)O_NONBLOCK)) < 0)
	     _exit(100);
      }

      execvp(Args[0],(char **)Args);
      cerr << _("Could not exec ") << Args[0] << endl;
      _exit(100);
   }      
   
   // Wait for rpm
   int Status = 0;
   while (waitpid(Child,&Status,0) != Child)
   {
      if (errno == EINTR)
	  continue;
      RunScripts("RPM::Post-Invoke");
      if (ArgsFileName) {
	 unlink(ArgsFileName);
	 free(ArgsFileName);
      }
      return _error->Errno("waitpid",_("Couldn't wait for subprocess"));
   }
   if (ArgsFileName) {
      unlink(ArgsFileName);
      free(ArgsFileName);
   }

   // Restore sig int/quit
   signal(SIGQUIT,SIG_DFL);
   signal(SIGINT,SIG_DFL);
       
   // Check for an error code.
   if (WIFEXITED(Status) == 0 || WEXITSTATUS(Status) != 0)
   {
      RunScripts("RPM::Post-Invoke");
      if (WIFSIGNALED(Status) != 0 && WTERMSIG(Status) == SIGSEGV)
	  return _error->Error(_("Sub-process %s recieved a segmentation fault."),Args[0]);
      
      if (WIFEXITED(Status) != 0)
	  return _error->Error(_("Sub-process %s returned an error code (%u)"),Args[0],
			       WEXITSTATUS(Status));
      
      return _error->Error(_("Sub-process %s exited unexpectedly"),Args[0]);
   }

   if (Interactive == true)
      cout << _("Done.") << endl;

   return true;
}

bool pkgRPMExtPM::Process(vector<const char*> &install, 
		       vector<const char*> &upgrade,
		       vector<const char*> &uninstall)
{
   if (uninstall.empty() == false)
       ExecRPM(Item::RPMErase, uninstall);
   if (install.empty() == false)
       ExecRPM(Item::RPMInstall, install);
   if (upgrade.empty() == false)
       ExecRPM(Item::RPMUpgrade, upgrade);
   return true;
}

// RPMLibPM::pkgRPMLibPM - Constructor					/*{{{*/
// ---------------------------------------------------------------------
/* */
pkgRPMLibPM::pkgRPMLibPM(pkgDepCache *Cache) : pkgRPMPM(Cache)
{
}
									/*}}}*/
// RPMLibPM::pkgRPMLibPM - Destructor					/*{{{*/
// ---------------------------------------------------------------------
/* */
pkgRPMLibPM::~pkgRPMLibPM()
{
}
									/*}}}*/

bool pkgRPMLibPM::AddToTransaction(Item::RPMOps op, vector<const char*> &files)
{
   int rc;
   FD_t fd;
   rpmHeader hdr;

   for (vector<const char*>::iterator I = files.begin(); I != files.end(); I++)
   {
      int upgrade = 0;

      switch (op)
      {
	 case Item::RPMUpgrade:
	    upgrade = 1;
	 case Item::RPMInstall:
	    fd = Fopen(*I, "r.ufdio");
	    if (fd == NULL)
	       _error->Error(_("Failed opening %s"), *I);
#if RPM_VERSION >= 0x040100
            rc = rpmReadPackageFile(TS, fd, *I, &hdr);
	    if (rc != RPMRC_OK && rc != RPMRC_NOTTRUSTED && rc != RPMRC_NOKEY)
	       _error->Error(_("Failed reading file %s"), *I);
	    rc = rpmtsAddInstallElement(TS, hdr, *I, upgrade, 0);
#else
	    rc = rpmReadPackageHeader(fd, &hdr, 0, NULL, NULL);
	    if (rc)
	       _error->Error(_("Failed reading file %s"), *I);
	    rc = rpmtransAddPackage(TS, hdr, NULL, *I, upgrade, 0);
#endif
	    if (rc)
	       _error->Error(_("Failed adding %s to transaction %s"),
			     *I, "(install)");
	    headerFree(hdr);
	    Fclose(fd);
	    break;

	 case Item::RPMErase:
#if RPM_VERSION >= 0x040000
            rpmdbMatchIterator MI;
#if RPM_VERSION >= 0x040100
	    MI = rpmtsInitIterator(TS, (rpmTag)RPMDBI_LABEL, *I, 0);
#else
	    MI = rpmdbInitIterator(DB, RPMDBI_LABEL, *I, 0);
#endif
	    while ((hdr = rpmdbNextIterator(MI)) != NULL) 
	    {
	       unsigned int recOffset = rpmdbGetIteratorOffset(MI);
	       if (recOffset) {
#if RPM_VERSION >= 0x040100
		  rc = rpmtsAddEraseElement(TS, hdr, recOffset);
#else
		  rc = rpmtransRemovePackage(TS, recOffset);
#endif
		  if (rc)
		     _error->Error(_("Failed adding %s to transaction %s"),
				   *I, "(erase)");
	       }
	    }
	    MI = rpmdbFreeIterator(MI);
#else // RPM 3.X
	    dbiIndexSet matches;
	    rc = rpmdbFindByLabel(DB, *I, &matches);
	    if (rc == 0) {
	       for (int i = 0; i < dbiIndexSetCount(matches); i++) {
		  unsigned int recOffset = dbiIndexRecordOffset(matches, i);
		  if (recOffset)
		     rpmtransRemovePackage(TS, recOffset);
	       }
	    }
#endif
	    break;
      }
   }
   return true;
}

bool pkgRPMLibPM::Process(vector<const char*> &install, 
			  vector<const char*> &upgrade,
			  vector<const char*> &uninstall)
{
   int rc = 0;
   bool Success = false;
   bool Interactive = _config->FindB("RPM::Interactive",true);
   string Dir = _config->Find("RPM::RootDir");
   rpmReadConfigFiles(NULL, NULL);

   int probFilter = 0;
   int notifyFlags = 0;
   int tsFlags = 0;

   if (uninstall.empty() == false)
      ParseRpmOpts("RPM::Erase-Options", &tsFlags, &probFilter);
   if (install.empty() == false || upgrade.empty() == false)
      ParseRpmOpts("RPM::Install-Options", &tsFlags, &probFilter);
   ParseRpmOpts("RPM::Options", &tsFlags, &probFilter);

#if RPM_VERSION >= 0x040100
   rpmps probs;
   TS = rpmtsCreate();
   rpmtsSetVSFlags(TS, (rpmVSFlags_e)-1);
   // 4.1 needs this always set even if NULL,
   // otherwise all scriptlets fail
   rpmtsSetRootDir(TS, Dir.c_str());
#else
   rpmProblemSet probs;
   const char *RootDir = NULL;
   if (!Dir.empty())
      RootDir = Dir.c_str();
   if (rpmdbOpen(RootDir, &DB, O_RDWR, 0644) != 0)
   {
      _error->Error(_("Could not open RPM database"));
      goto exit;
   }
   TS = rpmtransCreateSet(DB, Dir.c_str());
#endif

#if RPM_VERSION >= 0x040000
   if (rpmExpandNumeric("%{?_repackage_all_erasures}"))
      tsFlags |= RPMTRANS_FLAG_REPACKAGE;
#endif
		     
#if RPM_VERSION >= 0x040300
   /* Initialize security context patterns for SELinux */
   if (!(tsFlags & RPMTRANS_FLAG_NOCONTEXTS)) {
      rpmsx sx = rpmtsREContext(TS);
      if (sx == NULL) {
         const char *fn = rpmGetPath("%{?_install_file_context_path}", NULL);
         if (fn != NULL && *fn != '\0') {
            sx = rpmsxNew(fn);
            (void) rpmtsSetREContext(TS, sx);
         }
         fn = (const char *) _free(fn);
      }
      sx = rpmsxFree(sx);
   }
#endif

   if (_config->FindB("RPM::OldPackage", true) || !upgrade.empty()) {
      probFilter |= RPMPROB_FILTER_OLDPACKAGE;
   }
   if (_config->FindB("APT::Get::ReInstall", false)) {
      probFilter |= RPMPROB_FILTER_REPLACEPKG;
      probFilter |= RPMPROB_FILTER_REPLACEOLDFILES;
      probFilter |= RPMPROB_FILTER_REPLACENEWFILES;
   }

   if (_config->FindI("quiet",0) >= 1)
       notifyFlags |= INSTALL_LABEL;
   else if (Interactive == true) 
       notifyFlags |= INSTALL_LABEL | INSTALL_HASH;
   else 
       notifyFlags |= INSTALL_LABEL | INSTALL_PERCENT;

   if (uninstall.empty() == false)
       AddToTransaction(Item::RPMErase, uninstall);
   if (install.empty() == false)
       AddToTransaction(Item::RPMInstall, install);
   if (upgrade.empty() == false)
       AddToTransaction(Item::RPMUpgrade, upgrade);

   packagesTotal = install.size() + upgrade.size() * 2 + uninstall.size();
   if (tsFlags & RPMTRANS_FLAG_REPACKAGE) {
      packagesTotal += upgrade.size() + uninstall.size();
   }

#if RPM_VERSION >= 0x040100
   if (_config->FindB("RPM::NoDeps", false) == false) {
      rc = rpmtsCheck(TS);
      probs = rpmtsProblems(TS);
      if (rc || probs->numProblems > 0) {
	 rpmpsPrint(NULL, probs);
	 rpmpsFree(probs);
	 _error->Error(_("Transaction set check failed"));
	 goto exit;
      }
   }
#else
#if RPM_VERSION < 0x040000
   rpmDependencyConflict *conflicts;
#else
   rpmDependencyConflict conflicts;
#endif
   if (_config->FindB("RPM::NoDeps", false) == false) {
      int numConflicts;
      if (rpmdepCheck(TS, &conflicts, &numConflicts)) {
	 _error->Error(_("Transaction set check failed"));
	 if (conflicts) {
	    printDepProblems(stderr, conflicts, numConflicts);
	    rpmdepFreeConflicts(conflicts, numConflicts);
	 }
	 goto exit;
      }
   }
#endif

   rc = 0;
#if RPM_VERSION >= 0x040100
   if (_config->FindB("RPM::Order", true) == true)
      rc = rpmtsOrder(TS);
#else
   if (_config->FindB("RPM::Order", true) == true)
      rc = rpmdepOrder(TS);
#endif

   if (rc > 0) {
      _error->Error(_("Ordering failed for %d packages"), rc);
      goto exit;
   }

   cout << _("Committing changes...") << endl << flush;

#if RPM_VERSION >= 0x040100
   probFilter |= rpmtsFilterFlags(TS);
   rpmtsSetFlags(TS, (rpmtransFlags)(rpmtsFlags(TS) | tsFlags));
   rpmtsClean(TS);
   rc = rpmtsSetNotifyCallback(TS, rpmpmShowProgress, (void *)notifyFlags);
   rc = rpmtsRun(TS, NULL, (rpmprobFilterFlags)probFilter);
   probs = rpmtsProblems(TS);
#else
   rc = rpmRunTransactions(TS, rpmpmShowProgress, (void *)notifyFlags, NULL,
                           &probs, (rpmtransFlags)tsFlags,
			   (rpmprobFilterFlags)probFilter);
#endif

   if (rc > 0) {
      _error->Error(_("Error while running transaction"));
      if (probs->numProblems > 0)
	 rpmpsPrint(stderr, probs);
   } else {
      Success = true;
      if (rc < 0)
	 _error->Warning(_("Some errors occurred while running transaction"));
      else if (Interactive == true)
	 cout << _("Done.") << endl;
   }
   rpmpsFree(probs);

exit:

#if RPM_VERSION >= 0x040100
   rpmtsFree(TS);
#else
   rpmdbClose(DB);
#endif

   return Success;
}

bool pkgRPMLibPM::ParseRpmOpts(const char *Cnf, int *tsFlags, int *probFilter)
{
   Configuration::Item const *Opts = _config->Tree(Cnf);
   
   if (Opts != 0)
   {
      Opts = Opts->Child;
      for (; Opts != 0; Opts = Opts->Next)
      {
	 if (Opts->Value.empty() == true)
	    continue;
	 // Transaction set flags
	 if (Opts->Value == "--noscripts")
	    *tsFlags |= RPMTRANS_FLAG_NOSCRIPTS;
	 else if (Opts->Value == "--notriggers")
	    *tsFlags |= RPMTRANS_FLAG_NOTRIGGERS;
	 else if (Opts->Value == "--nodocs" ||
	          Opts->Value == "--excludedocs")
	    *tsFlags |= RPMTRANS_FLAG_NODOCS;
	 else if (Opts->Value == "--allfiles")
	    *tsFlags |= RPMTRANS_FLAG_ALLFILES;
	 else if (Opts->Value == "--justdb")
	    *tsFlags |= RPMTRANS_FLAG_JUSTDB;
	 else if (Opts->Value == "--test")
	    *tsFlags |= RPMTRANS_FLAG_TEST;
#if RPM_VERSION >= 0x040000
	 else if (Opts->Value == "--nomd5")
	    *tsFlags |= RPMTRANS_FLAG_NOMD5;
	 else if (Opts->Value == "--repackage")
	    *tsFlags |= RPMTRANS_FLAG_REPACKAGE;
#endif
#if RPM_VERSION >= 0x040200
	 else if (Opts->Value == "--noconfigs" ||
	          Opts->Value == "--excludeconfigs")
	    *tsFlags |= RPMTRANS_FLAG_NOCONFIGS;
#endif
#if RPM_VERSION >= 0x040300
	 else if (Opts->Value == "--nocontexts")
            *tsFlags |= RPMTRANS_FLAG_NOCONTEXTS;
#endif

	 // Problem filter flags
	 else if (Opts->Value == "--replacefiles")
	 {
	    *probFilter |= RPMPROB_FILTER_REPLACEOLDFILES;
	    *probFilter |= RPMPROB_FILTER_REPLACENEWFILES;
	 }
	 else if (Opts->Value == "--replacepkgs")
	    *probFilter |= RPMPROB_FILTER_REPLACEPKG;
	 else if (Opts->Value == "--ignoresize")
	 {
	    *probFilter |= RPMPROB_FILTER_DISKSPACE;
#if RPM_VERSION >= 0x040000
	    *probFilter |= RPMPROB_FILTER_DISKNODES;
#endif
	 }
	 else if (Opts->Value == "--badreloc")
	    *probFilter |= RPMPROB_FILTER_FORCERELOCATE;

	 // Misc things having apt config counterparts
	 else if (Opts->Value == "--force")
	    _config->Set("APT::Get::ReInstall", true);
	 else if (Opts->Value == "--oldpackage")
	    _config->Set("RPM::OldPackage", true);
	 else if (Opts->Value == "--nodeps")
	    _config->Set("RPM::NoDeps", true);
	 else if (Opts->Value == "--noorder")
	    _config->Set("RPM::Order", false);
	 else if (Opts->Value == "-v") {
	    rpmIncreaseVerbosity();
	 } else if (Opts->Value == "-vv") {
	    rpmIncreaseVerbosity();
	    rpmIncreaseVerbosity();
	 } else if (Opts->Value == "-vvv") {
	    rpmIncreaseVerbosity();
	    rpmIncreaseVerbosity();
	    rpmIncreaseVerbosity();
	 }
	 // TODO: --root, --relocate, --prefix, --excludepath etc...

      }
   }
   return true;
} 
#endif /* HAVE_RPM */

// vim:sts=3:sw=3
