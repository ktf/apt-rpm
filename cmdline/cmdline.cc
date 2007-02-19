// -*- mode: c++; mode: fold -*-
#include <apt-pkg/configuration.h>
#include <apt-pkg/error.h>
#include <apt-pkg/acquire.h>
#include <apt-pkg/sptr.h>

#include <config.h>
#include <apti18n.h>

#include "cmdline.h"

#include <regex.h>
#include <sys/stat.h>
#include <fnmatch.h>

using namespace std;

// YnPrompt - Yes No Prompt.						/*{{{*/
// ---------------------------------------------------------------------
/* Returns true on a Yes.*/
bool YnPrompt()
{
   if (_config->FindB("APT::Get::Assume-Yes",false) == true)
   {
      c1out << _("Y") << endl;
      return true;
   }

   char response[1024] = "";
   cin.getline(response, sizeof(response));

   if (!cin)
      return false;

   if (strlen(response) == 0)
      return true;

   regex_t Pattern;
   int Res;

   Res = regcomp(&Pattern, nl_langinfo(YESEXPR),
                 REG_EXTENDED|REG_ICASE|REG_NOSUB);

   if (Res != 0) {
      char Error[300];        
      regerror(Res,&Pattern,Error,sizeof(Error));
      return _error->Error(_("Regex compilation error - %s"),Error);
   }
   
   Res = regexec(&Pattern, response, 0, NULL, 0);
   if (Res == 0)
      return true;
   return false;
}
									/*}}}*/
// AnalPrompt - Annoying Yes No Prompt.					/*{{{*/
// ---------------------------------------------------------------------
/* Returns true on a Yes.*/
bool AnalPrompt(const char *Text)
{
   char Buf[1024];
   cin.getline(Buf,sizeof(Buf));
   if (strcmp(Buf,Text) == 0)
      return true;
   return false;
}
									/*}}}*/
// ShowList - Show a list						/*{{{*/
// ---------------------------------------------------------------------
/* This prints out a string of space separated words with a title and 
   a two space indent line wraped to the current screen width. */
bool ShowList(ostream &out,string Title,string List,string VersionsList)
{
   if (List.empty() == true)
      return true;
   // trim trailing space
   int NonSpace = List.find_last_not_of(' ');
   if (NonSpace != -1)
   {
      List = List.erase(NonSpace + 1);
      if (List.empty() == true)
	 return true;
   }

   // Acount for the leading space
   int ScreenWidth = ::ScreenWidth - 3;
      
   out << Title << endl;
   string::size_type Start = 0;
   string::size_type VersionsStart = 0;
   while (Start < List.size())
   {
      if(_config->FindB("APT::Get::Show-Versions",false) == true &&
         VersionsList.size() > 0) {
         string::size_type End;
         string::size_type VersionsEnd;
         
         End = List.find(' ',Start);
         VersionsEnd = VersionsList.find('\n', VersionsStart);

         out << "   " << string(List,Start,End - Start) << " (" << 
            string(VersionsList,VersionsStart,VersionsEnd - VersionsStart) << 
            ")" << endl;

	 if (End == string::npos || End < Start)
	    End = Start + ScreenWidth;

         Start = End + 1;
         VersionsStart = VersionsEnd + 1;
      } else {
         string::size_type End;

         if (Start + ScreenWidth >= List.size())
            End = List.size();
         else
            End = List.rfind(' ',Start+ScreenWidth);

         if (End == string::npos || End < Start)
            End = Start + ScreenWidth;
         out << "  " << string(List,Start,End - Start) << endl;
         Start = End + 1;
      }
   }   

   return false;
}

const char *op2str(int op)
{
   switch (op & 0x0f)
   {
      case pkgCache::Dep::LessEq: return "<=";
      case pkgCache::Dep::GreaterEq: return ">=";
      case pkgCache::Dep::Less: return "<";
      case pkgCache::Dep::Greater: return ">";
      case pkgCache::Dep::Equals: return "=";
      case pkgCache::Dep::NotEquals: return "!";
      default: return "";
   }
}

// SigWinch - Window size change signal handler                         /*{{{*/
// ---------------------------------------------------------------------
/* */
void SigWinch(int)
{
   // Riped from GNU ls
#ifdef TIOCGWINSZ
   struct winsize ws;

   if (ioctl(1, TIOCGWINSZ, &ws) != -1 && ws.ws_col >= 5)
      ScreenWidth = ws.ws_col - 1;
#endif
}

// CacheFile::NameComp - QSort compare by name                          /*{{{*/
// ---------------------------------------------------------------------
/* */
pkgCache *cmdCacheFile::SortCache = 0;
int cmdCacheFile::NameComp(const void *a,const void *b)
{
   if (*(pkgCache::Package **)a == 0 || *(pkgCache::Package **)b == 0)
      return *(pkgCache::Package **)a - *(pkgCache::Package **)b;

   const pkgCache::Package &A = **(pkgCache::Package **)a;
   const pkgCache::Package &B = **(pkgCache::Package **)b;

   return strcmp(SortCache->StrP + A.Name,SortCache->StrP + B.Name);
}
                                                                        /*}}}*/
// CacheFile::Sort - Sort by name                                       /*{{{*/
// ---------------------------------------------------------------------
/* */
void cmdCacheFile::Sort()
{
   delete [] List;
   List = new pkgCache::Package *[Cache->Head().PackageCount];
   memset(List,0,sizeof(*List)*Cache->Head().PackageCount);
   pkgCache::PkgIterator I = Cache->PkgBegin();
   for (;I.end() != true; I++)
      List[I->ID] = I;

   SortCache = *this;
   qsort(List,Cache->Head().PackageCount,sizeof(*List),NameComp);
}

// ShowBroken - Debugging aide						/*{{{*/
// ---------------------------------------------------------------------
/* This prints out the names of all the packages that are broken along
   with the name of each each broken dependency and a quite version 
   description.
   
   The output looks like:
 The following packages have unmet dependencies:
     exim: Depends: libc6 (>= 2.1.94) but 2.1.3-10 is to be installed
           Depends: libldap2 (>= 2.0.2-2) but it is not going to be installed
           Depends: libsasl7 but it is not going to be installed   
 */
void ShowBroken(ostream &out,cmdCacheFile &Cache,bool Now,pkgDepCache::State *State)
{
   out << _("The following packages have unmet dependencies:") << endl;
   for (unsigned J = 0; J < Cache->Head().PackageCount; J++)
   {
      pkgCache::PkgIterator I(Cache,Cache.List[J]);
      
      if (Now == true)
      {
	 if (Cache[I].NowBroken() == false)
	    continue;
      }
      else
      {
	 if (Cache[I].InstBroken() == false)
	    continue;
      }
      
      // Print out each package and the failed dependencies
      out <<"  " <<  I.Name() << ":";
      size_t Indent = strlen(I.Name()) + 3;
      bool First = true;
      pkgCache::VerIterator Ver;
      
      if (Now == true)
	 Ver = I.CurrentVer();
      else
	 Ver = Cache[I].InstVerIter(Cache);
      
      if (Ver.end() == true)
      {
	 out << endl;
	 continue;
      }
      
      for (pkgCache::DepIterator D = Ver.DependsList(); D.end() == false;)
      {
	 // Compute a single dependency element (glob or)
	 pkgCache::DepIterator Start;
	 pkgCache::DepIterator End;
	 D.GlobOr(Start,End);

         // CNC:2003-02-22 - IsImportantDep() currently calls IsCritical(), so
         //		     these two are currently doing the same thing. Check
         //		     comments in IsImportantDep() definition.
#if 0
	 if (Cache->IsImportantDep(End) == false)
	    continue;
#else
	 if (End.IsCritical() == false)
	    continue;
#endif
	 
	 if (Now == true)
	 {
	    if ((Cache[End] & pkgDepCache::DepGNow) == pkgDepCache::DepGNow)
	       continue;
	 }
	 else
	 {
	    if ((Cache[End] & pkgDepCache::DepGInstall) == pkgDepCache::DepGInstall)
	       continue;
	 }
	 
	 bool FirstOr = true;
	 while (1)
	 {
	    if (First == false)
	       for (unsigned J = 0; J != Indent; J++)
		  out << ' ';
	    First = false;

	    if (FirstOr == false)
	    {
	       for (size_t J = 0; J != strlen(End.DepType()) + 3; J++)
		  out << ' ';
	    }
	    else
	       out << ' ' << End.DepType() << ": ";
	    FirstOr = false;
	    
	    out << Start.TargetPkg().Name();
	 
	    // Show a quick summary of the version requirements
	    if (Start.TargetVer() != 0)
	       out << " (" << Start.CompType() << " " << Start.TargetVer() << ")";
	    
	    /* Show a summary of the target package if possible. In the case
	       of virtual packages we show nothing */	 
	    pkgCache::PkgIterator Targ = Start.TargetPkg();
	    if (Targ->ProvidesList == 0)
	    {
	       out << ' ';
	       pkgCache::VerIterator Ver = Cache[Targ].InstVerIter(Cache);
	       if (Now == true)
		  Ver = Targ.CurrentVer();
	       	    
	       if (Ver.end() == false)
	       {
		  if (Now == true)
		     ioprintf(out,_("but %s is installed"),Ver.VerStr());
		  else
		     ioprintf(out,_("but %s is to be installed"),Ver.VerStr());
	       }	       
	       else
	       {
		  if (Cache[Targ].CandidateVerIter(Cache).end() == true)
		  {
		     if (Targ->ProvidesList == 0)
			out << _("but it is not installable");
		     else
			out << _("but it is a virtual package");
		  }		  
		  else
		     out << (Now?_("but it is not installed"):_("but it is not going to be installed"));
	       }	       
	    }
	    
	    if (Start != End)
	       out << _(" or");
	    out << endl;
	    
	    if (Start == End)
	       break;
	    Start++;
	 }	 
      }	    
   }   
}
									/*}}}*/
// ShowNew - Show packages to newly install				/*{{{*/
// ---------------------------------------------------------------------
/* */
void ShowNew(ostream &out,cmdCacheFile &Cache,pkgDepCache::State *State)
{
   /* Print out a list of packages that are going to be installed extra
      to what the user asked */
   string List;
   string VersionsList;
   for (unsigned J = 0; J < Cache->Head().PackageCount; J++)
   {
      pkgCache::PkgIterator I(Cache,Cache.List[J]);
      if (Cache[I].NewInstall() == true &&
	  (State == NULL || (*State)[I].NewInstall() == false)) {
	 List += string(I.Name()) + " ";
         VersionsList += string(Cache[I].CandVersion) + "\n";
      }
   }
   
   ShowList(out,_("The following NEW packages will be installed:"),List,VersionsList);
}
									/*}}}*/
// ShowDel - Show packages to delete					/*{{{*/
// ---------------------------------------------------------------------
/* */
void ShowDel(ostream &out,cmdCacheFile &Cache,pkgDepCache::State *State)
{
   /* Print out a list of packages that are going to be removed extra
      to what the user asked */
   string List, RepList; // CNC:2002-07-25
   string VersionsList;
   for (unsigned J = 0; J < Cache->Head().PackageCount; J++)
   {
      pkgCache::PkgIterator I(Cache,Cache.List[J]);
      if (Cache[I].Delete() == true &&
	  (State == NULL || (*State)[I].Delete() == false))
      {
	 // CNC:2002-07-25
	 bool Obsoleted = false;
	 string by;
	 for (pkgCache::DepIterator D = I.RevDependsList(); D.end() == false; D++)
	 {
	    if (D->Type == pkgCache::Dep::Obsoletes &&
	        Cache[D.ParentPkg()].Install() &&
	        (pkgCache::Version*)D.ParentVer() == Cache[D.ParentPkg()].InstallVer &&
	        Cache->VS().CheckDep(I.CurrentVer().VerStr(), D) == true)
	    {
	       if (Obsoleted)
		  by += ", " + string(D.ParentPkg().Name());
	       else
	       {
		  Obsoleted = true;
		  by = D.ParentPkg().Name();
	       }
	    }
	 }
	 if (Obsoleted)
	    RepList += string(I.Name()) + " (by " + by + ")  ";
	 else
	 {
	    if ((Cache[I].iFlags & pkgDepCache::Purge) == pkgDepCache::Purge)
	       List += string(I.Name()) + "* ";
	    else
	       List += string(I.Name()) + " ";
	 }
     
     // CNC:2004-03-09
     VersionsList += string(I.CurrentVer().VerStr())+ "\n";
      }
   }
   
   // CNC:2002-07-25
   ShowList(out,_("The following packages will be REPLACED:"),RepList,VersionsList);
   ShowList(out,_("The following packages will be REMOVED:"),List,VersionsList);
}
									/*}}}*/
// ShowKept - Show kept packages					/*{{{*/
// ---------------------------------------------------------------------
/* */
void ShowKept(ostream &out,cmdCacheFile &Cache,pkgDepCache::State *State)
{
   string List;
   string VersionsList;
   for (unsigned J = 0; J < Cache->Head().PackageCount; J++)
   {	 
      pkgCache::PkgIterator I(Cache,Cache.List[J]);
      
      if (State == NULL) {
	 // Not interesting
	 if (Cache[I].Upgrade() == true || Cache[I].Upgradable() == false ||
	     I->CurrentVer == 0 || Cache[I].Delete() == true)
	    continue;
      } else {
	 // Not interesting
	 if (!((Cache[I].Install() == false && (*State)[I].Install() == true) ||
	       (Cache[I].Delete() == false && (*State)[I].Delete() == true)))
	    continue;
      }
      
      List += string(I.Name()) + " ";
      VersionsList += string(Cache[I].CurVersion) + " => " + Cache[I].CandVersion + "\n";
   }
   ShowList(out,_("The following packages have been kept back"),List,VersionsList);
}
									/*}}}*/
// ShowUpgraded - Show upgraded packages				/*{{{*/
// ---------------------------------------------------------------------
/* */
void ShowUpgraded(ostream &out,cmdCacheFile &Cache,pkgDepCache::State *State)
{
   string List;
   string VersionsList;
   for (unsigned J = 0; J < Cache->Head().PackageCount; J++)
   {
      pkgCache::PkgIterator I(Cache,Cache.List[J]);
      
      if (State == NULL) {
	 // Not interesting
	 if (Cache[I].Upgrade() == false || Cache[I].NewInstall() == true)
	    continue;
      } else {
	 // Not interesting
	 if (Cache[I].NewInstall() == true ||
	     !(Cache[I].Upgrade() == true && (*State)[I].Upgrade() == false))
	    continue;
      }
      
      List += string(I.Name()) + " ";
      VersionsList += string(Cache[I].CurVersion) + " => " + Cache[I].CandVersion + "\n";
   }
   ShowList(out,_("The following packages will be upgraded"),List,VersionsList);
}
									/*}}}*/
// ShowDowngraded - Show downgraded packages				/*{{{*/
// ---------------------------------------------------------------------
/* */
bool ShowDowngraded(ostream &out,cmdCacheFile &Cache,pkgDepCache::State *State)
{
   string List;
   string VersionsList;
   for (unsigned J = 0; J < Cache->Head().PackageCount; J++)
   {
      pkgCache::PkgIterator I(Cache,Cache.List[J]);
      
      if (State == NULL) {
	 // Not interesting
	 if (Cache[I].Downgrade() == false || Cache[I].NewInstall() == true)
	    continue;
      } else {
	 // Not interesting
	 if (Cache[I].NewInstall() == true ||
	     !(Cache[I].Downgrade() == true && (*State)[I].Downgrade() == false))
	    continue;
      }
      
      List += string(I.Name()) + " ";
      VersionsList += string(Cache[I].CurVersion) + " => " + Cache[I].CandVersion + "\n";
   }
   return ShowList(out,_("The following packages will be DOWNGRADED"),List,VersionsList);
}
									/*}}}*/
// ShowHold - Show held but changed packages				/*{{{*/
// ---------------------------------------------------------------------
/* */
bool ShowHold(ostream &out,cmdCacheFile &Cache,pkgDepCache::State *State)
{
   string List;
   string VersionsList;
   for (unsigned J = 0; J < Cache->Head().PackageCount; J++)
   {
      pkgCache::PkgIterator I(Cache,Cache.List[J]);
      if (Cache[I].InstallVer != (pkgCache::Version *)I.CurrentVer() &&
	  I->SelectedState == pkgCache::State::Hold &&
	  (State == NULL ||
	   Cache[I].InstallVer != (*State)[I].InstallVer)) {
	 List += string(I.Name()) + " ";
	 VersionsList += string(Cache[I].CurVersion) + " => " + Cache[I].CandVersion + "\n";
      }
   }

   return ShowList(out,_("The following held packages will be changed:"),List,VersionsList);
}
									/*}}}*/
// ShowEssential - Show an essential package warning			/*{{{*/
// ---------------------------------------------------------------------
/* This prints out a warning message that is not to be ignored. It shows
   all essential packages and their dependents that are to be removed. 
   It is insanely risky to remove the dependents of an essential package! */
bool ShowEssential(ostream &out,cmdCacheFile &Cache,pkgDepCache::State *State)
{
   string List;
   string VersionsList;
   bool *Added = new bool[Cache->Head().PackageCount];
   for (unsigned int I = 0; I != Cache->Head().PackageCount; I++)
      Added[I] = false;
   
   for (unsigned J = 0; J < Cache->Head().PackageCount; J++)
   {
      pkgCache::PkgIterator I(Cache,Cache.List[J]);
      if ((I->Flags & pkgCache::Flag::Essential) != pkgCache::Flag::Essential &&
	  (I->Flags & pkgCache::Flag::Important) != pkgCache::Flag::Important)
	 continue;
      
      // The essential package is being removed
      if (Cache[I].Delete() == true &&
	  (State == NULL || (*State)[I].Delete() == false))
      {
	 if (Added[I->ID] == false)
	 {
	    // CNC:2003-03-21 - Do not consider a problem if that package is being obsoleted
	    //                  by something else.
	    bool Obsoleted = false;
	    for (pkgCache::DepIterator D = I.RevDependsList(); D.end() == false; D++)
	    {
	       if (D->Type == pkgCache::Dep::Obsoletes &&
		   Cache[D.ParentPkg()].Install() &&
		   ((pkgCache::Version*)D.ParentVer() == Cache[D.ParentPkg()].InstallVer ||
		    (pkgCache::Version*)D.ParentVer() == ((pkgCache::Version*)D.ParentPkg().CurrentVer())) &&
		   Cache->VS().CheckDep(I.CurrentVer().VerStr(), D) == true)
	       {
		  Obsoleted = true;
		  break;
	       }
	    }
	    if (Obsoleted == false) {
	       Added[I->ID] = true;
	       List += string(I.Name()) + " ";
	    }
        //VersionsList += string(Cache[I].CurVersion) + "\n"; ???
	 }
      }
      
      if (I->CurrentVer == 0)
	 continue;

      // Print out any essential package depenendents that are to be removed
      for (pkgCache::DepIterator D = I.CurrentVer().DependsList(); D.end() == false; D++)
      {
	 // Skip everything but depends
	 if (D->Type != pkgCache::Dep::PreDepends &&
	     D->Type != pkgCache::Dep::Depends)
	    continue;
	 
	 pkgCache::PkgIterator P = D.SmartTargetPkg();
	 if (Cache[P].Delete() == true &&
	     (State == NULL || (*State)[P].Delete() == false))
	 {
	    if (Added[P->ID] == true)
	       continue;

	    // CNC:2003-03-21 - Do not consider a problem if that package is being obsoleted
	    //                  by something else.
	    bool Obsoleted = false;
	    for (pkgCache::DepIterator D = P.RevDependsList(); D.end() == false; D++)
	    {
	       if (D->Type == pkgCache::Dep::Obsoletes &&
		   Cache[D.ParentPkg()].Install() &&
		   ((pkgCache::Version*)D.ParentVer() == Cache[D.ParentPkg()].InstallVer ||
		    (pkgCache::Version*)D.ParentVer() == ((pkgCache::Version*)D.ParentPkg().CurrentVer())) &&
		   Cache->VS().CheckDep(P.CurrentVer().VerStr(), D) == true)
	       {
		  Obsoleted = true;
		  break;
	       }
	    }
	    if (Obsoleted == true)
	       continue;

	    Added[P->ID] = true;
	    
	    char S[300];
	    snprintf(S,sizeof(S),_("%s (due to %s) "),P.Name(),I.Name());
	    List += S;
        //VersionsList += "\n"; ???
	 }	 
      }      
   }
   
   delete [] Added;
   return ShowList(out,_("WARNING: The following essential packages will be removed\n"
			 "This should NOT be done unless you know exactly what you are doing!"),List,VersionsList);
}
									/*}}}*/
// Stats - Show some statistics						/*{{{*/
// ---------------------------------------------------------------------
/* */
void Stats(ostream &out,pkgDepCache &Dep,pkgDepCache::State *State)
{
   unsigned long Upgrade = 0;
   unsigned long Downgrade = 0;
   unsigned long Install = 0;
   unsigned long ReInstall = 0;
   // CNC:2002-07-29
   unsigned long Replace = 0;
   unsigned long Remove = 0;
   unsigned long Keep = 0;
   for (pkgCache::PkgIterator I = Dep.PkgBegin(); I.end() == false; I++)
   {
      if (Dep[I].NewInstall() == true &&
	  (State == NULL || (*State)[I].NewInstall() == false))
	 Install++;
      else
      {
	 if (Dep[I].Upgrade() == true &&
	     (State == NULL || (*State)[I].Upgrade() == false))
	    Upgrade++;
	 else
	    if (Dep[I].Downgrade() == true &&
		(State == NULL || (*State)[I].Downgrade() == false))
	       Downgrade++;
	    else
	       if (State != NULL &&
		   (((*State)[I].NewInstall() == true && Dep[I].NewInstall() == false) ||
		    ((*State)[I].Upgrade() == true && Dep[I].Upgrade() == false) ||
		    ((*State)[I].Downgrade() == true && Dep[I].Downgrade() == false)))
		  Keep++;
      }
      // CNC:2002-07-29
      if (Dep[I].Delete() == true &&
	  (State == NULL || (*State)[I].Delete() == false))
      {
	 bool Obsoleted = false;
	 string by;
	 for (pkgCache::DepIterator D = I.RevDependsList();
	      D.end() == false; D++)
	 {
	    if (D->Type == pkgCache::Dep::Obsoletes &&
	        Dep[D.ParentPkg()].Install() &&
	        (pkgCache::Version*)D.ParentVer() == Dep[D.ParentPkg()].InstallVer &&
	        Dep.VS().CheckDep(I.CurrentVer().VerStr(), D) == true)
	    {
	       Obsoleted = true;
	       break;
	    }
	 }
	 if (Obsoleted)
	    Replace++;
	 else
	    Remove++;
      }
      else if ((Dep[I].iFlags & pkgDepCache::ReInstall) == pkgDepCache::ReInstall &&
	       (State == NULL || !((*State)[I].iFlags & pkgDepCache::ReInstall)))
	 ReInstall++;
   }   

   ioprintf(out,_("%lu upgraded, %lu newly installed, "),
	    Upgrade,Install);
   
   if (ReInstall != 0)
      ioprintf(out,_("%lu reinstalled, "),ReInstall);
   if (Downgrade != 0)
      ioprintf(out,_("%lu downgraded, "),Downgrade);
   // CNC:2002-07-29
   if (Replace != 0)
      ioprintf(out,_("%lu replaced, "),Replace);

   // CNC:2002-07-29
   if (State == NULL)
      ioprintf(out,_("%lu removed and %lu not upgraded.\n"),
	       Remove,Dep.KeepCount());
   else
      ioprintf(out,_("%lu removed and %lu kept.\n"),Remove,Keep);

   
   if (Dep.BadCount() != 0)
      ioprintf(out,_("%lu not fully installed or removed.\n"),
	       Dep.BadCount());
}
									/*}}}*/
bool cmdDoClean(CommandLine &CmdL)
{
   if (_config->FindB("APT::Get::Simulate") == true)
   {
      cout << "Del " << _config->FindDir("Dir::Cache::archives") << "* " <<
         _config->FindDir("Dir::Cache::archives") << "partial/*" << endl;
      return true;
   }

   // Lock the archive directory
   FileFd Lock;
   if (_config->FindB("Debug::NoLocking",false) == false)
   {
      Lock.Fd(GetLock(_config->FindDir("Dir::Cache::Archives") + "lock"));
      if (_error->PendingError() == true)
         return _error->Error(_("Unable to lock the download directory"));
   }

   pkgAcquire Fetcher;
   Fetcher.Clean(_config->FindDir("Dir::Cache::archives"));
   Fetcher.Clean(_config->FindDir("Dir::Cache::archives") + "partial/");
   return true;
}

// FindSrc - Find a source record					/*{{{*/
// ---------------------------------------------------------------------
/* */
pkgSrcRecords::Parser *FindSrc(const char *Name,pkgRecords &Recs,
			       pkgSrcRecords &SrcRecs,string &Src,
			       pkgDepCache &Cache)
{
   // We want to pull the version off the package specification..
   string VerTag;
   string TmpSrc = Name;
   string::size_type Slash = TmpSrc.rfind('=');
   if (Slash != string::npos)
   {
      VerTag = string(TmpSrc.begin() + Slash + 1,TmpSrc.end());
      TmpSrc = string(TmpSrc.begin(),TmpSrc.begin() + Slash);
   }
   
   /* Lookup the version of the package we would install if we were to
      install a version and determine the source package name, then look
      in the archive for a source package of the same name. In theory
      we could stash the version string as well and match that too but
      today there aren't multi source versions in the archive. */
   if (_config->FindB("APT::Get::Only-Source") == false && 
       VerTag.empty() == true)
   {
      pkgCache::PkgIterator Pkg = Cache.FindPkg(TmpSrc);
      if (Pkg.end() == false)
      {
	 pkgCache::VerIterator Ver = Cache.GetCandidateVer(Pkg);      
	 if (Ver.end() == false)
	 {
	    pkgRecords::Parser &Parse = Recs.Lookup(Ver.FileList());
	    Src = Parse.SourcePkg();
	 }
      }   
   }
   
   // No source package name..
   if (Src.empty() == true)
      Src = TmpSrc;
   
   // The best hit
   pkgSrcRecords::Parser *Last = 0;
   unsigned long Offset = 0;
   string Version;
   bool IsMatch = false;
   
   // If we are matching by version then we need exact matches to be happy
   if (VerTag.empty() == false)
      IsMatch = true;
   
   /* Iterate over all of the hits, which includes the resulting
      binary packages in the search */
   pkgSrcRecords::Parser *Parse;
   SrcRecs.Restart();
   while ((Parse = SrcRecs.Find(Src.c_str(),false)) != 0)
   {
      string Ver = Parse->Version();
      
      // Skip name mismatches
      if (IsMatch == true && Parse->Package() != Src)
	 continue;
      
      if (VerTag.empty() == false)
      {
	 /* Don't want to fall through because we are doing exact version 
	    matching. */
	 if (Cache.VS().CmpVersion(VerTag,Ver) != 0)
	    continue;
	 
	 Last = Parse;
	 Offset = Parse->Offset();
	 break;
      }
				  
      // Newer version or an exact match
      if (Last == 0 || Cache.VS().CmpVersion(Version,Ver) < 0 || 
	  (Parse->Package() == Src && IsMatch == false))
      {
	 IsMatch = Parse->Package() == Src;
	 Last = Parse;
	 Offset = Parse->Offset();
	 Version = Ver;
      }      
   }
   
   if (Last == 0)
      return 0;
   
   if (Last->Jump(Offset) == false)
      return 0;
   
   return Last;
}
									/*}}}*/
// DoAutoClean - Smartly remove downloaded archives			/*{{{*/
// ---------------------------------------------------------------------
/* This is similar to clean but it only purges things that cannot be 
   downloaded, that is old versions of cached packages. */
void LogCleaner::Erase(const char *File,string Pkg,string Ver,struct stat &St) 
{
      c1out << "Del " << Pkg << " " << Ver << " [" << SizeToStr(St.st_size) << "B]" << endl;
      
      if (_config->FindB("APT::Get::Simulate") == false)
	 unlink(File);      
}

// apt-cache stuff..

// LocalitySort - Sort a version list by package file locality		/*{{{*/
// ---------------------------------------------------------------------
/* */
int LocalityCompare(const void *a, const void *b)
{
   pkgCache::VerFile *A = *(pkgCache::VerFile **)a;
   pkgCache::VerFile *B = *(pkgCache::VerFile **)b;
   
   if (A == 0 && B == 0)
      return 0;
   if (A == 0)
      return 1;
   if (B == 0)
      return -1;
   
   if (A->File == B->File)
      return A->Offset - B->Offset;
   return A->File - B->File;
}

void LocalitySort(pkgCache::VerFile **begin,
		  unsigned long Count,size_t Size)
{   
   qsort(begin,Count,Size,LocalityCompare);
}
									/*}}}*/
// UnMet - Show unmet dependencies					/*{{{*/
// ---------------------------------------------------------------------
/* */
bool cmdUnMet(CommandLine &CmdL, pkgCache &Cache)
{
   bool Important = _config->FindB("APT::Cache::Important",false);
   
   for (pkgCache::PkgIterator P = Cache.PkgBegin(); P.end() == false; P++)
   {
      for (pkgCache::VerIterator V = P.VersionList(); V.end() == false; V++)
      {
	 bool Header = false;
	 for (pkgCache::DepIterator D = V.DependsList(); D.end() == false;)
	 {
	    // Collect or groups
	    pkgCache::DepIterator Start;
	    pkgCache::DepIterator End;
	    D.GlobOr(Start,End);
	    
	    // Skip conflicts and replaces
	    if (End->Type != pkgCache::Dep::PreDepends &&
		End->Type != pkgCache::Dep::Depends && 
		End->Type != pkgCache::Dep::Suggests &&
		End->Type != pkgCache::Dep::Recommends)
	       continue;

	    // Important deps only
	    if (Important == true)
	       if (End->Type != pkgCache::Dep::PreDepends &&
		   End->Type != pkgCache::Dep::Depends)
		  continue;
	    
	    // Verify the or group
	    bool OK = false;
	    pkgCache::DepIterator RealStart = Start;
	    do
	    {
	       // See if this dep is Ok
	       pkgCache::Version **VList = Start.AllTargets();
	       if (*VList != 0)
	       {
		  OK = true;
		  delete [] VList;
		  break;
	       }
	       delete [] VList;
	       
	       if (Start == End)
		  break;
	       Start++;
	    }
	    while (1);

	    // The group is OK
	    if (OK == true)
	       continue;
	    
	    // Oops, it failed..
	    if (Header == false)
	       ioprintf(cout,_("Package %s version %s has an unmet dep:\n"),
			P.Name(),V.VerStr());
	    Header = true;
	    
	    // Print out the dep type
	    cout << " " << End.DepType() << ": ";

	    // Show the group
	    Start = RealStart;
	    do
	    {
	       cout << Start.TargetPkg().Name();
	       if (Start.TargetVer() != 0)
		  cout << " (" << Start.CompType() << " " << Start.TargetVer() <<
		  ")";
	       if (Start == End)
		  break;
	       cout << " | ";
	       Start++;
	    }
	    while (1);
	    
	    cout << endl;
	 }	 
      }
   }   
   return true;
}
									/*}}}*/
// DumpPackage - Show a dump of a package record			/*{{{*/
// ---------------------------------------------------------------------
/* */
bool cmdDumpPackage(CommandLine &CmdL, pkgCache &Cache)
{   
   for (const char **I = CmdL.FileList + 1; *I != 0; I++)
   {
      pkgCache::PkgIterator Pkg = Cache.FindPkg(*I);
      if (Pkg.end() == true)
      {
	 _error->Warning(_("Unable to locate package %s"),*I);
	 continue;
      }

      cout << "Package: " << Pkg.Name() << endl;
      cout << "Versions: " << endl;
      for (pkgCache::VerIterator Cur = Pkg.VersionList(); Cur.end() != true; Cur++)
      {
	 cout << Cur.VerStr() << "." << Cur.Arch();
	 for (pkgCache::VerFileIterator Vf = Cur.FileList(); Vf.end() == false; Vf++)
	    cout << "(" << Vf.File().FileName() << ")";
	 cout << endl;
      }
      
      cout << endl;
      
      cout << "Reverse Depends: " << endl;
      for (pkgCache::DepIterator D = Pkg.RevDependsList(); D.end() != true; D++)
      {
	 cout << "  " << D.ParentPkg().Name() << ',' << D.TargetPkg().Name();
	 if (D->Version != 0)
	    cout << ' ' << DeNull(D.TargetVer()) << endl;
	 else
	    cout << endl;
      }
      
      cout << "Dependencies: " << endl;
      for (pkgCache::VerIterator Cur = Pkg.VersionList(); Cur.end() != true; Cur++)
      {
	 cout << Cur.VerStr() << " - ";
	 for (pkgCache::DepIterator Dep = Cur.DependsList(); Dep.end() != true; Dep++)
	    cout << Dep.TargetPkg().Name() << " (" << (int)Dep->CompareOp << " " << DeNull(Dep.TargetVer()) << ") ";
	 cout << endl;
      }      

      cout << "Provides: " << endl;
      for (pkgCache::VerIterator Cur = Pkg.VersionList(); Cur.end() != true; Cur++)
      {
	 cout << Cur.VerStr() << " - ";
	 for (pkgCache::PrvIterator Prv = Cur.ProvidesList(); Prv.end() != true; Prv++)
	    cout << Prv.ParentPkg().Name() << " ";
	 cout << endl;
      }
      cout << "Reverse Provides: " << endl;
      for (pkgCache::PrvIterator Prv = Pkg.ProvidesList(); Prv.end() != true; Prv++)
	 cout << Prv.OwnerPkg().Name() << " " << Prv.OwnerVer().VerStr() << endl;            
   }

   return true;
}
									/*}}}*/
// DisplayRecord - Displays the complete record for the package		/*{{{*/
// ---------------------------------------------------------------------
/* This displays the package record from the proper package index file. 
   It is not used by DumpAvail for performance reasons. */
bool cmdDisplayRecord(pkgCache::VerIterator V, pkgCache &Cache)
{
   // Find an appropriate file
   pkgCache::VerFileIterator Vf = V.FileList();
   for (; Vf.end() == false; Vf++)
      if ((Vf.File()->Flags & pkgCache::Flag::NotSource) == 0)
	 break;
   if (Vf.end() == true)
      Vf = V.FileList();
      
// CNC:2002-07-24
#if HAVE_RPM
   pkgRecords Recs(Cache);
   pkgRecords::Parser &P = Recs.Lookup(Vf);
   const char *Start;
   const char *End;
   P.GetRec(Start,End);
   cout << string(Start,End-Start) << endl;
#else
   // Check and load the package list file
   pkgCache::PkgFileIterator I = Vf.File();
   if (I.IsOk() == false)
      return _error->Error(_("Package file %s is out of sync."),I.FileName());
   
   FileFd PkgF(I.FileName(),FileFd::ReadOnly);
   if (_error->PendingError() == true)
      return false;
   
   // Read the record and then write it out again.
   unsigned char *Buffer = new unsigned char[Cache->HeaderP->MaxVerFileSize+1];
   Buffer[V.FileList()->Size] = '\n';
   if (PkgF.Seek(V.FileList()->Offset) == false ||
       PkgF.Read(Buffer,V.FileList()->Size) == false ||
       fwrite(Buffer,1,V.FileList()->Size+1,stdout) < V.FileList()->Size+1)
   {
      delete [] Buffer;
      return false;
   }
   
   delete [] Buffer;
#endif

   return true;
}

// Depends - Print out a dependency tree				/*{{{*/
// ---------------------------------------------------------------------
/* */
bool cmdDepends(CommandLine &CmdL, pkgCache &Cache)
{
   SPtrArray<unsigned> Colours = new unsigned[Cache.Head().PackageCount];
   memset(Colours,0,sizeof(*Colours)*Cache.Head().PackageCount);
   
   for (const char **I = CmdL.FileList + 1; *I != 0; I++)
   {
      pkgCache::PkgIterator Pkg = Cache.FindPkg(*I);
      if (Pkg.end() == true)
      {
	 _error->Warning(_("Unable to locate package %s"),*I);
	 continue;
      }
      Colours[Pkg->ID] = 1;
   }
   
   bool Recurse = _config->FindB("APT::Cache::RecurseDepends",false);
   bool Installed = _config->FindB("APT::Cache::Installed",false);
   bool DidSomething;
   do
   {
      DidSomething = false;
      for (pkgCache::PkgIterator Pkg = Cache.PkgBegin(); Pkg.end() == false; Pkg++)
      {
	 if (Colours[Pkg->ID] != 1)
	    continue;
	 Colours[Pkg->ID] = 2;
	 DidSomething = true;
	 
	 pkgCache::VerIterator Ver = Pkg.VersionList();
	 if (Ver.end() == true)
	 {
	    cout << '<' << Pkg.Name() << '>' << endl;
	    continue;
	 }
	 
	 // CNC:2003-03-03
	 cout << Pkg.Name() << "-" << Ver.VerStr() << endl;
	 
	 for (pkgCache::DepIterator D = Ver.DependsList(); D.end() == false; D++)
	 {

	    pkgCache::PkgIterator Trg = D.TargetPkg();

	    if((Installed && Trg->CurrentVer != 0) || !Installed)
	      {

		if ((D->CompareOp & pkgCache::Dep::Or) == pkgCache::Dep::Or)
		  cout << " |";
		else
		  cout << "  ";
	    
		// Show the package
	        if (Trg->VersionList == 0)
	           cout << D.DepType() << ": <" << Trg.Name() << ">" << endl;
	        // CNC:2003-03-03
	        else if (D.TargetVer() == 0)
	           cout << D.DepType() << ": " << Trg.Name() << endl;
	        else
	           cout << D.DepType() << ": " << Trg.Name()
		        << " " << D.CompType() << " " << D.TargetVer() << endl;
	    
		if (Recurse == true)
		  Colours[D.TargetPkg()->ID]++;

	      }
	    
	    // Display all solutions
	    SPtrArray<pkgCache::Version *> List = D.AllTargets();
	    pkgPrioSortList(Cache,List);
	    for (pkgCache::Version **I = List; *I != 0; I++)
	    {
	       pkgCache::VerIterator V(Cache,*I);
	       if (V != Cache.VerP + V.ParentPkg()->VersionList ||
		   V->ParentPkg == D->Package)
		  continue;
	       // CNC:2003-03-03
	       cout << "    " << V.ParentPkg().Name()
		    << "-" << V.VerStr() << endl;
	       
	       if (Recurse == true)
		  Colours[D.ParentPkg()->ID]++;
	    }
	 }
      }      
   }   
   while (DidSomething == true);
   
   return true;
}

// RDepends - Print out a reverse dependency tree - mbc			/*{{{*/
// ---------------------------------------------------------------------
/* */
bool cmdRDepends(CommandLine &CmdL, pkgCache &Cache)
{
   SPtrArray<unsigned> Colours = new unsigned[Cache.Head().PackageCount];
   memset(Colours,0,sizeof(*Colours)*Cache.Head().PackageCount);
   
   for (const char **I = CmdL.FileList + 1; *I != 0; I++)
   {
      pkgCache::PkgIterator Pkg = Cache.FindPkg(*I);
      if (Pkg.end() == true)
      {
	 _error->Warning(_("Unable to locate package %s"),*I);
	 continue;
      }
      Colours[Pkg->ID] = 1;
   }
   
   bool Recurse = _config->FindB("APT::Cache::RecurseDepends",false);
   bool Installed = _config->FindB("APT::Cache::Installed",false);
   bool DidSomething;
   do
   {
      DidSomething = false;
      for (pkgCache::PkgIterator Pkg = Cache.PkgBegin(); Pkg.end() == false; Pkg++)
      {
	 if (Colours[Pkg->ID] != 1)
	    continue;
	 Colours[Pkg->ID] = 2;
	 DidSomething = true;
	 
	 pkgCache::VerIterator Ver = Pkg.VersionList();
	 if (Ver.end() == true)
	 {
	    cout << '<' << Pkg.Name() << '>' << endl;
	    continue;
	 }
	 
	 cout << Pkg.Name() << endl;
	 
	 cout << "Reverse Depends:" << endl;
	 for (pkgCache::DepIterator D = Pkg.RevDependsList(); D.end() == false; D++)
	 {	    
	    // Show the package
	    pkgCache::PkgIterator Trg = D.ParentPkg();

	    if((Installed && Trg->CurrentVer != 0) || !Installed)
	      {

		if ((D->CompareOp & pkgCache::Dep::Or) == pkgCache::Dep::Or)
		  cout << " |";
		else
		  cout << "  ";

		if (Trg->VersionList == 0)
		  cout << D.DepType() << ": <" << Trg.Name() << ">" << endl;
		else
		  cout << Trg.Name() << endl;

		if (Recurse == true)
		  Colours[D.ParentPkg()->ID]++;

	      }
	    
	    // Display all solutions
	    SPtrArray<pkgCache::Version *> List = D.AllTargets();
	    pkgPrioSortList(Cache,List);
	    for (pkgCache::Version **I = List; *I != 0; I++)
	    {
	       pkgCache::VerIterator V(Cache,*I);
	       if (V != Cache.VerP + V.ParentPkg()->VersionList ||
		   V->ParentPkg == D->Package)
		  continue;
	       cout << "    " << V.ParentPkg().Name() << endl;
	       
	       if (Recurse == true)
		  Colours[D.ParentPkg()->ID]++;
	    }
	 }
      }      
   }   
   while (DidSomething == true);
   
   return true;
}

									/*}}}*/
// CNC:2003-02-19
// WhatDepends - Print out a reverse dependency tree			/*{{{*/
// ---------------------------------------------------------------------
/* */
bool cmdWhatDepends(CommandLine &CmdL, pkgCache &Cache)
{
   SPtrArray<unsigned> Colours = new unsigned[Cache.Head().PackageCount];
   memset(Colours,0,sizeof(*Colours)*Cache.Head().PackageCount);
   
   for (const char **I = CmdL.FileList + 1; *I != 0; I++)
   {
      pkgCache::PkgIterator Pkg = Cache.FindPkg(*I);
      if (Pkg.end() == true)
      {
	 _error->Warning(_("Unable to locate package %s"),*I);
	 continue;
      }
      Colours[Pkg->ID] = 1;
   }
   
   bool Recurse = _config->FindB("APT::Cache::RecurseDepends",false);
   bool DidSomething;
   do
   {
      DidSomething = false;
      for (pkgCache::PkgIterator Pkg = Cache.PkgBegin(); Pkg.end() == false; Pkg++)
      {
	 if (Colours[Pkg->ID] != 1)
	    continue;
	 Colours[Pkg->ID] = 2;
	 DidSomething = true;
	 
	 pkgCache::VerIterator Ver = Pkg.VersionList();
	 if (Ver.end() == true)
	    cout << '<' << Pkg.Name() << '>' << endl;
	 else
	    cout << Pkg.Name() << "-" << Ver.VerStr() << endl;

	 SPtrArray<unsigned> LocalColours = 
		     new unsigned[Cache.Head().PackageCount];
	 memset(LocalColours,0,sizeof(*LocalColours)*Cache.Head().PackageCount);
	    
	 // Display all dependencies directly on the package.
	 for (pkgCache::DepIterator RD = Pkg.RevDependsList();
	      RD.end() == false; RD++)
	 {
	    pkgCache::PkgIterator Parent = RD.ParentPkg();

	    if (LocalColours[Parent->ID] == 1)
	       continue;
	    LocalColours[Parent->ID] = 1;
	       
	    if (Ver.end() == false && RD.TargetVer() &&
	        Cache.VS->CheckDep(Ver.VerStr(),RD) == false)
	       continue;

	    if (Recurse == true && Colours[Parent->ID] == 0)
	       Colours[Parent->ID] = 1;

	    pkgCache::VerIterator ParentVer = Parent.VersionList();

	    // Show the package
	    cout << "  " << Parent.Name()
		 << "-" << ParentVer.VerStr() << endl;

	    // Display all dependencies from that package that relate
	    // to the queried package.
	    for (pkgCache::DepIterator D = ParentVer.DependsList();
	         D.end() == false; D++)
	    {
	       // If this is a virtual package, there's no provides.
	       if (Ver.end() == true) {
		  // If it's not the same package, and there's no provides
		  // skip that package.
		  if (D.TargetPkg() != Pkg)
		     continue;
	       } else if (D.TargetPkg() != Pkg ||
			  Cache.VS->CheckDep(Ver.VerStr(),D) == false) {
		  // Oops. Either it's not the same package, or the
		  // version didn't match. Check virtual provides from
		  // the queried package version and verify if this
		  // dependency matches one of those.
		  bool Hit = false;
		  for (pkgCache::PrvIterator Prv = Ver.ProvidesList();
		       Prv.end() == false; Prv++) {
		     if (Prv.ParentPkg() == D.TargetPkg() &&
			 (Prv.ParentPkg()->VersionList == 0 ||
			  Cache.VS->CheckDep(Prv.ProvideVersion(),D)==false)) {
			Hit = true;
			break;
		     }
		  }
		  if (Hit == false)
		     continue;
	       }

	       // Bingo!
	       pkgCache::PkgIterator Trg = D.TargetPkg();
	       if (Trg->VersionList == 0)
		  cout << "    " << D.DepType()
				 << ": <" << Trg.Name() << ">" << endl;
	       else if (D.TargetVer() == 0)
		  cout << "    " << D.DepType()
				 << ": " << Trg.Name() << endl;
	       else
		  cout << "    " << D.DepType()
				 << ": " << Trg.Name()
				 << " " << D.CompType() << " "
				 << D.TargetVer() << endl;

	       // Display all solutions
	       SPtrArray<pkgCache::Version *> List = D.AllTargets();
	       pkgPrioSortList(Cache,List);
	       for (pkgCache::Version **I = List; *I != 0; I++)
	       {
		  pkgCache::VerIterator V(Cache,*I);
		  if (V != Cache.VerP + V.ParentPkg()->VersionList ||
		      V->ParentPkg == D->Package)
		     continue;
		  cout << "      " << V.ParentPkg().Name()
		       << "-" << V.VerStr() << endl;
		  
		  if (Recurse == true)
		     Colours[D.ParentPkg()->ID]++;
	       }
	    }
	 }

	 // Is this a virtual package the user queried directly?
	 if (Ver.end())
	    continue;

	 // Display all dependencies on virtual provides, which were not
	 // yet shown in the step above.
	 for (pkgCache::PrvIterator RDPrv = Ver.ProvidesList();
	      RDPrv.end() == false; RDPrv++) {
	    for (pkgCache::DepIterator RD = RDPrv.ParentPkg().RevDependsList();
	         RD.end() == false; RD++)
	    {
	       pkgCache::PkgIterator Parent = RD.ParentPkg();

	       if (LocalColours[Parent->ID] == 1)
		  continue;
	       LocalColours[Parent->ID] = 1;
		  
	       if (Ver.end() == false &&
		   Cache.VS->CheckDep(Ver.VerStr(),RD) == false)
		  continue;

	       if (Recurse == true && Colours[Parent->ID] == 0)
		  Colours[Parent->ID] = 1;

	       pkgCache::VerIterator ParentVer = Parent.VersionList();

	       // Show the package
	       cout << "  " << Parent.Name()
		    << "-" << ParentVer.VerStr() << endl;

	       for (pkgCache::DepIterator D = ParentVer.DependsList();
		    D.end() == false; D++)
	       {
		  // Go on if it's the same package and version or
		  // if it's the same package and has no versions
		  // (a virtual package).
		  if (D.TargetPkg() != RDPrv.ParentPkg() ||
		      (RDPrv.ProvideVersion() != 0 &&
		       Cache.VS->CheckDep(RDPrv.ProvideVersion(),D) == false))
		     continue;

		  // Bingo!
		  pkgCache::PkgIterator Trg = D.TargetPkg();
		  if (Trg->VersionList == 0)
		     cout << "    " << D.DepType()
				    << ": <" << Trg.Name() << ">" << endl;
		  else if (D.TargetVer() == 0)
		     cout << "    " << D.DepType()
				    << ": " << Trg.Name() << endl;
		  else
		     cout << "    " << D.DepType()
				    << ": " << Trg.Name()
				    << " " << D.CompType() << " "
				    << D.TargetVer() << endl;

		  // Display all solutions
		  SPtrArray<pkgCache::Version *> List = D.AllTargets();
		  pkgPrioSortList(Cache,List);
		  for (pkgCache::Version **I = List; *I != 0; I++)
		  {
		     pkgCache::VerIterator V(Cache,*I);
		     if (V != Cache.VerP + V.ParentPkg()->VersionList ||
			 V->ParentPkg == D->Package)
			continue;
		     cout << "      " << V.ParentPkg().Name()
			  << "-" << V.VerStr() << endl;
		     
		     if (Recurse == true)
			Colours[D.ParentPkg()->ID]++;
		  }
	       }
	    }
	 }
      } 
   }
   while (DidSomething == true);
   
   return true;
}
									/*}}}*/

// WhatProvides - Show all packages that provide the given (virtual) package
// ---------------------------------------------------------------------
/* */
bool cmdWhatProvides(CommandLine &CmdL, pkgCache &Cache)
{
  bool    found;

  for (const char **I = CmdL.FileList + 1; *I != 0; I++)
  {
       cout << "<" << *I << ">" << endl;
       found = false;
       for (pkgCache::PkgIterator Pkg = Cache.PkgBegin(); Pkg.end() == false; Pkg++)
       {
	    for (pkgCache::VerIterator Ver = Pkg.VersionList(); !Ver.end(); Ver++)
	    {
		 if (!strcmp(Pkg.Name(), *I))
		 {
		      // match on real package, ignore any provides in the package
		      // since they would be meaningless
		      cout << "  " << Pkg.Name() << "-" << Ver.VerStr() << endl;
		      found = true;
		 }
		 else
		 {
		      // seach in package's provides list
		      for (pkgCache::PrvIterator Prv = Ver.ProvidesList(); Prv.end() == false; Prv++)
		      {
			   if (!strcmp(Prv.Name(), *I))
			   {
				cout << "  " << Pkg.Name() << "-" << Ver.VerStr() << endl;
				cout << "    Provides: <" << Prv.Name();
				if (Prv.ProvideVersion() != 0)
					cout << " = " << Prv.ProvideVersion();
				cout << ">" << endl;
				found = true;
			   }
		      }
		 }
	    }
       }
       
       if (!found)
	    cout << "  nothing provides <" << *I << ">" << endl;
  }
  
  return true;
}
								  /*}}}*/
// ShowPackage - Dump the package record to the screen			/*{{{*/
// ---------------------------------------------------------------------
/* */
bool cmdShowPackage(CommandLine &CmdL, pkgCache &Cache)
{   
   pkgDepCache::Policy Plcy;

   unsigned found = 0;
   
   for (const char **I = CmdL.FileList + 1; *I != 0; I++)
   {
      pkgCache::PkgIterator Pkg = Cache.FindPkg(*I);
      if (Pkg.end() == true)
      {
	 _error->Warning(_("Unable to locate package %s"),*I);
	 continue;
      }

      ++found;

      // CNC:2004-07-09
      // If it's a virtual package, require user to select similarly to apt-get
      if (Pkg.VersionList().end() == true and Pkg->ProvidesList != 0)
      {
         ioprintf(cout, _("Package %s is a virtual package provided by:\n"),
                  Pkg.Name());
         for (pkgCache::PrvIterator Prv = Pkg.ProvidesList();
             Prv.end() == false; Prv++)
         {
	    pkgCache::VerIterator V = Plcy.GetCandidateVer(Prv.OwnerPkg());
            if (V.end() == true)
               continue;
            if (V != Prv.OwnerVer())
               continue;
            cout << "  " << Prv.OwnerPkg().Name() << " " << V.VerStr() << endl;
         }
         cout << _("You should explicitly select one to show.") << endl;
         _error->Error(_("Package %s is a virtual package with multiple providers."), Pkg.Name());
         return false;
      }

      // Find the proper version to use.
      if (_config->FindB("APT::Cache::AllVersions", false) == true)
      {
	 pkgCache::VerIterator V;
	 for (V = Pkg.VersionList(); V.end() == false; V++)
	 {
	    if (cmdDisplayRecord(V, Cache) == false)
	       return false;
	 }
      }
      else
      {
	 pkgCache::VerIterator V = Plcy.GetCandidateVer(Pkg);
	 if (V.end() == true || V.FileList().end() == true)
	    continue;
	 if (cmdDisplayRecord(V, Cache) == false)
	    return false;
      }      
   }

   if (found > 0)
        return true;
   return _error->Error(_("No packages found"));
}
									/*}}}*/
bool cmdDoList(CommandLine &CmdL, cmdCacheFile &Cache)
{
   bool MatchAll = (CmdL.FileSize() == 1);
   bool ShowUpgradable = _config->FindB("APT::Cache::ShowUpgradable", false);
   bool ShowInstalled = _config->FindB("APT::Cache::ShowInstalled", false);

   const char **PatternList = &CmdL.FileList[1];
   int NumPatterns = CmdL.FileSize()-1;

   bool ShowVersion = _config->FindB("APT::Cache::ShowVersion", false);
   bool ShowSummary = _config->FindB("APT::Cache::ShowSummary", false);

   const char *PkgName;
   vector<int> Matches(Cache->Head().PackageCount);
   size_t NumMatches = 0;
   size_t Len = 0, NameMaxLen = 0, VerMaxLen = 0;
   bool Matched;
   for (unsigned int J = 0; J < Cache->Head().PackageCount; J++)
   {
      Matched = false;
      pkgCache::PkgIterator Pkg(Cache,Cache.List[J]);
      if (Pkg->VersionList == 0)
	 continue;
      if (ShowInstalled && Pkg->CurrentVer == 0)
	 continue;
      if (ShowUpgradable &&
	  (Pkg->CurrentVer == 0 || Cache[Pkg].Upgradable() == false))
	 continue;
      PkgName = Pkg.Name();
      if (MatchAll == true)
	 Matched = true;
      else for (int i=0; i != NumPatterns; i++) {
	 if (fnmatch(PatternList[i], PkgName, 0) == 0) {
	    Matched = true;
	    break;
	 }
      }
      if (Matched == true) {
	 Matches[NumMatches++] = J;
	 Len = strlen(PkgName);
	 if (Len > NameMaxLen)
	    NameMaxLen = Len;
	 if (ShowVersion == true && Pkg->CurrentVer != 0) {
	    Len = strlen(Pkg.CurrentVer().VerStr());
	    if (Len > VerMaxLen)
	       VerMaxLen = Len;
	 }
      }
   }

   if (NumMatches == 0)
      return true;

   if (ShowVersion == true) {
      const char *NameLabel = _("Name");
      const char *InstalledLabel = _("Installed");
      const char *CandidateLabel = _("Candidate");
      size_t NameLen = strlen(NameLabel);
      size_t InstalledLen = strlen(InstalledLabel);
      size_t CandidateLen = strlen(CandidateLabel);

      unsigned int FirstColumn = NameMaxLen+2;
      if (FirstColumn < NameLen+2)
	 FirstColumn = NameLen+2;

      unsigned int SecondColumn = VerMaxLen+2;
      if (SecondColumn < InstalledLen+2)
	 SecondColumn = InstalledLen+2;

      vector<char> BlankFirst(FirstColumn+1,' ');
      BlankFirst[FirstColumn] = 0;

      vector<char> BlankSecond(SecondColumn+1,' ');
      BlankSecond[SecondColumn] = 0;

      vector<char> Bar(ScreenWidth+1,'-');
      Bar[ScreenWidth] = 0;

      c2out << NameLabel << &BlankFirst[NameLen]
	    << InstalledLabel << &BlankSecond[InstalledLen]
	    << CandidateLabel << endl;
      c2out << &Bar[ScreenWidth-NameLen] << &BlankFirst[NameLen]
	    << &Bar[ScreenWidth-InstalledLen] << &BlankSecond[InstalledLen]
	    << &Bar[ScreenWidth-CandidateLen] << endl;

      const char *Str;
      size_t StrLen;
      for (unsigned int K = 0; K != NumMatches; K++) {
	 pkgCache::PkgIterator Pkg(Cache,Cache.List[Matches[K]]);
	 Str = Pkg.Name();
	 StrLen = strlen(Str);
	 c2out << Str << &BlankFirst[StrLen];
	 if (Pkg->CurrentVer != 0) {
	    Str = Pkg.CurrentVer().VerStr();
	    StrLen = strlen(Str);
	    if (Len < SecondColumn-1)
	       c2out << Str << &BlankSecond[StrLen];
	    else
	       c2out << Str << " ";
	 } else {
	    c2out << "-" << &BlankSecond[1];
	 }
	 Str = "-";
	 if (Cache[Pkg].CandidateVer != 0) {
	    Str = Cache[Pkg].CandidateVerIter(Cache).VerStr();
	    if (Pkg->CurrentVer != 0 &&
	        strcmp(Str, Pkg.CurrentVer().VerStr()) == 0)
	       Str = "-";
	 }
	 c2out << Str << endl;
      }
   } else if (ShowSummary == true) {
      pkgRecords Recs(Cache);
      if (_error->PendingError() == true)
	 return false;
      for (unsigned int K = 0; K != NumMatches; K++) {
	 pkgCache::PkgIterator Pkg(Cache,Cache.List[Matches[K]]);
	 pkgRecords::Parser &Parse = Recs.Lookup(Pkg.VersionList().FileList());
	 c2out << Pkg.Name() << " - " << Parse.ShortDesc() << endl;
      }
   } else {
      unsigned int PerLine = ScreenWidth/(NameMaxLen+2);
      if (PerLine == 0) PerLine = 1;
      unsigned int ColumnLen = ScreenWidth/PerLine;
      unsigned int NumLines = (NumMatches+PerLine-1)/PerLine;
      vector<char> Blank(ColumnLen+1,' ');
      Blank[ColumnLen] = 0;

      const char *Str;
      size_t StrLen;
      unsigned int K;
      for (unsigned int Line = 0; Line != NumLines; Line++) {
	 for (unsigned int Entry = 0; Entry != PerLine; Entry++) {
	    K = Line+(Entry*NumLines);
	    if (K >= NumMatches)
	       break;
	    pkgCache::PkgIterator Pkg(Cache,Cache.List[Matches[K]]);
	    Str = Pkg.Name();
	    StrLen = strlen(Str);
	    if (Len < ColumnLen-1)
	       c2out << Str << &Blank[StrLen];
	    else
	       c2out << Str << " ";
	 }
	 c2out << endl;
      }
   }
   
   return true;
}

struct ExVerFile
{
   pkgCache::VerFile *Vf;
   bool NameMatch;
};

bool cmdSearch(CommandLine &CmdL, pkgCache &Cache)
{
   bool ShowFull = _config->FindB("APT::Cache::ShowFull",false);
   bool NamesOnly = _config->FindB("APT::Cache::NamesOnly",false);
   unsigned NumPatterns = CmdL.FileSize() -1;
   
   pkgDepCache::Policy Plcy;
   
   // Make sure there is at least one argument
   if (NumPatterns < 1)
      return _error->Error(_("You must give exactly one pattern"));
   
   // Compile the regex pattern
   regex_t *Patterns = new regex_t[NumPatterns];
   memset(Patterns,0,sizeof(*Patterns)*NumPatterns);
   for (unsigned I = 0; I != NumPatterns; I++)
   {
      if (regcomp(&Patterns[I],CmdL.FileList[I+1],REG_EXTENDED | REG_ICASE | 
		  REG_NOSUB) != 0)
      {
	 for (; I != 0; I--)
	    regfree(&Patterns[I]);
	 return _error->Error("Regex compilation error");
      }      
   }
   
   // Create the text record parser
   pkgRecords Recs(Cache);
   if (_error->PendingError() == true)
   {
      for (unsigned I = 0; I != NumPatterns; I++)
	 regfree(&Patterns[I]);
      return false;
   }
   
   ExVerFile *VFList = new ExVerFile[Cache.HeaderP->PackageCount+1];
   memset(VFList,0,sizeof(*VFList)*Cache.HeaderP->PackageCount+1);

   // Map versions that we want to write out onto the VerList array.
   for (pkgCache::PkgIterator P = Cache.PkgBegin(); P.end() == false; P++)
   {
      VFList[P->ID].NameMatch = NumPatterns != 0;
      for (unsigned I = 0; I != NumPatterns; I++)
      {
	 if (regexec(&Patterns[I],P.Name(),0,0,0) == 0)
	    VFList[P->ID].NameMatch &= true;
	 else
	    VFList[P->ID].NameMatch = false;
      }
        
      // Doing names only, drop any that dont match..
      if (NamesOnly == true && VFList[P->ID].NameMatch == false)
	 continue;
	 
      // Find the proper version to use. 
      pkgCache::VerIterator V = Plcy.GetCandidateVer(P);
      if (V.end() == false)
	 VFList[P->ID].Vf = V.FileList();
   }
      
   // Include all the packages that provide matching names too
   for (pkgCache::PkgIterator P = Cache.PkgBegin(); P.end() == false; P++)
   {
      if (VFList[P->ID].NameMatch == false)
	 continue;

      for (pkgCache::PrvIterator Prv = P.ProvidesList() ; Prv.end() == false; Prv++)
      {
	 pkgCache::VerIterator V = Plcy.GetCandidateVer(Prv.OwnerPkg());
	 if (V.end() == false)
	 {
	    VFList[Prv.OwnerPkg()->ID].Vf = V.FileList();
	    VFList[Prv.OwnerPkg()->ID].NameMatch = true;
	 }
      }
   }

   LocalitySort(&VFList->Vf,Cache.HeaderP->PackageCount,sizeof(*VFList));

   // Iterate over all the version records and check them
   for (ExVerFile *J = VFList; J->Vf != 0; J++)
   {
      pkgRecords::Parser &P = Recs.Lookup(pkgCache::VerFileIterator(Cache,J->Vf));

      bool Match = true;
      if (J->NameMatch == false)
      {
	 string LongDesc = P.LongDesc(); 
	 // CNC 2004-04-10
	 string ShortDesc = P.ShortDesc();
	 Match = NumPatterns != 0;
	 for (unsigned I = 0; I != NumPatterns; I++)
	 {
	    if (regexec(&Patterns[I],LongDesc.c_str(),0,0,0) == 0 ||
		regexec(&Patterns[I],ShortDesc.c_str(),0,0,0) == 0)
	       Match &= true;
	    else
	       Match = false;
	 }
      }
      
      if (Match == true)
      {
	 if (ShowFull == true)
	 {
	    const char *Start;
	    const char *End;
	    P.GetRec(Start,End);
	    cout << string(Start,End-Start) << endl;
	 }	 
	 else
	    cout << P.Name() << " - " << P.ShortDesc() << endl;
      }
   }
   
   delete [] VFList;
   for (unsigned I = 0; I != NumPatterns; I++)
      regfree(&Patterns[I]);
   if (ferror(stdout))
       return _error->Error("Write to stdout failed");
   return true;
}
									/*}}}*/
bool cmdSearchFile(CommandLine &CmdL, pkgCache &Cache)
{
   pkgRecords Recs(Cache);
   pkgDepCache::Policy Plcy;

   for (const char **I = CmdL.FileList + 1; *I != 0; I++) {
      pkgCache::PkgIterator Pkg = Cache.PkgBegin();
      for (; Pkg.end() == false; Pkg++) {
	 if (_config->FindB("APT::Cache::AllVersions", false) == true) {
	    pkgCache::VerIterator Ver = Pkg.VersionList();
	    for (; Ver.end() == false; Ver++) {
	       pkgRecords::Parser &Parse = Recs.Lookup(Ver.FileList());
	       if (Parse.HasFile(*I)) {
		  cout << *I << " " << Pkg.Name() << "-" << Ver.VerStr() << endl;
	       }
	    }
	 } else {
	    pkgCache::VerIterator Ver = Plcy.GetCandidateVer(Pkg);
	    if (Ver.end() == false) {
	       pkgRecords::Parser &Parse = Recs.Lookup(Ver.FileList());
	       if (Parse.HasFile(*I)) {
		  cout << *I << " " << Pkg.Name() << "-" << Ver.VerStr() << endl;
	       }
	    }
	 }
      }
   }

   return true;
}

bool cmdFileList(CommandLine &CmdL, pkgCache &Cache)
{
   pkgDepCache::Policy Plcy;

   for (const char **I = CmdL.FileList + 1; *I != 0; I++)
   {
      pkgCache::PkgIterator Pkg = Cache.FindPkg(*I);
      if (Pkg.end() == true)
      {
         _error->Warning(_("Unable to locate package %s"),*I);
         continue;
      }

      pkgCache::VerIterator Ver = Plcy.GetCandidateVer(Pkg);
      pkgRecords Recs(Cache);
      if (Ver.end() == false) {
         pkgRecords::Parser &Parse = Recs.Lookup(Ver.FileList());
         vector<string> Files;
         Parse.FileList(Files);
         for (vector<string>::iterator F = Files.begin(); F != Files.end(); F++) {
            cout << (*F) << endl;
         }
      }
   }

   return true;
}

bool cmdChangeLog(CommandLine &CmdL, pkgCache &Cache)
{
   pkgDepCache::Policy Plcy;

   for (const char **I = CmdL.FileList + 1; *I != 0; I++)
   {
      pkgCache::PkgIterator Pkg = Cache.FindPkg(*I);
      if (Pkg.end() == true)
      {
         _error->Warning(_("Unable to locate package %s"),*I);
         continue;
      }

      pkgCache::VerIterator Ver = Plcy.GetCandidateVer(Pkg);
      pkgRecords Recs(Cache);
      if (Ver.end() == false) {
         pkgRecords::Parser &Parse = Recs.Lookup(Ver.FileList());
         vector<ChangeLogEntry *> ChangeLog;
         if (Parse.ChangeLog(ChangeLog) == false) {
	    _error->Warning(_("Changelog data not available for the repository."));
	    return true;
	 }
	 cout << Pkg.Name() << "-" << Ver.VerStr() << ":" << endl;
	 tm *ptm;
	 char buf[512];
         for (vector<ChangeLogEntry *>::iterator F = ChangeLog.begin(); F != ChangeLog.end(); F++) {
	    ptm = localtime(&(*F)->Time);
	    strftime(buf, sizeof(buf), "%a %b %d %Y", ptm);
            cout << "* " << buf << " " << (*F)->Author << endl;
	    cout << (*F)->Text << endl << endl;
         }
      }
   }

   return true;
}
// vim:sts=3:sw=3
