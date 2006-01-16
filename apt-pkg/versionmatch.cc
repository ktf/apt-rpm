// -*- mode: cpp; mode: fold -*-
// Description								/*{{{*/
// $Id: versionmatch.cc,v 1.9 2003/05/19 17:58:26 doogie Exp $
/* ######################################################################

   Version Matching 
   
   This module takes a matching string and a type and locates the version
   record that satisfies the constraint described by the matching string.
   
   ##################################################################### */
									/*}}}*/
// Include Files							/*{{{*/
#ifdef __GNUG__
#pragma implementation "apt-pkg/versionmatch.h"
#endif
#include <apt-pkg/versionmatch.h>
// CNC:2003-11-05
#include <apt-pkg/version.h>

#include <apt-pkg/strutl.h>
#include <apt-pkg/error.h>

#include <stdio.h>
#include <ctype.h>
									/*}}}*/

// VersionMatch::pkgVersionMatch - Constructor				/*{{{*/
// ---------------------------------------------------------------------
/* Break up the data string according to the selected type */
// CNC:2003-11-05
pkgVersionMatch::pkgVersionMatch(string Data,MatchType Type,int Op) : VerOp(Op), Type(Type)
{
   MatchAll = false;
   VerPrefixMatch = false;
   RelVerPrefixMatch = false;
   
   if (Type == None || Data.length() < 1)
      return;
   
   // Cut up the version representation
   if (Type == Version)
   {
      if (Data.end()[-1] == '*')
      {
	 VerPrefixMatch = true;
	 VerStr = string(Data,0,Data.length()-1);
      }
      else
	 VerStr = Data;
      return;
   }   
   
   if (Type == Release)
   {
      // All empty = match all
      if (Data == "*")
      {
	 MatchAll = true;
	 return;
      }
      
      // Are we a simple specification?
      string::const_iterator I = Data.begin();
      for (; I != Data.end() && *I != '='; I++);
      if (I == Data.end())
      {
	 // Temporary
	 if (isdigit(Data[0]))
	    RelVerStr = Data;
	 else
	    RelArchive = Data;
	 
	 if (RelVerStr.length() > 0 && RelVerStr.end()[-1] == '*')
	 {
	    RelVerPrefixMatch = true;
	    RelVerStr = string(RelVerStr.begin(),RelVerStr.end()-1);
	 }	 
	 return;
      }
            
      char Spec[300];
      char *Fragments[20];
      snprintf(Spec,sizeof(Spec),"%s",Data.c_str());
      if (TokSplitString(',',Spec,Fragments,
			 sizeof(Fragments)/sizeof(Fragments[0])) == false)
      {
	 Type = None;
	 return;
      }
      
      for (unsigned J = 0; Fragments[J] != 0; J++)
      {
	 if (strlen(Fragments[J]) < 3)
	    continue;
	    
	 if (stringcasecmp(Fragments[J],Fragments[J]+2,"v=") == 0)
	    RelVerStr = Fragments[J]+2;
	 else if (stringcasecmp(Fragments[J],Fragments[J]+2,"o=") == 0)
	    RelOrigin = Fragments[J]+2;
	 else if (stringcasecmp(Fragments[J],Fragments[J]+2,"a=") == 0)
	    RelArchive = Fragments[J]+2;
	 else if (stringcasecmp(Fragments[J],Fragments[J]+2,"l=") == 0)
	    RelLabel = Fragments[J]+2;
	 else if (stringcasecmp(Fragments[J],Fragments[J]+2,"c=") == 0)
	    RelComponent = Fragments[J]+2;
      }
      
      if (RelVerStr.end()[-1] == '*')
      {
	 RelVerPrefixMatch = true;
	 RelVerStr = string(RelVerStr.begin(),RelVerStr.end()-1);
      }	 
      return;
   }
   
   if (Type == Origin)
   {
      OrSite = Data;
      return;
   }   
}
									/*}}}*/
// VersionMatch::MatchVer - Match a version string with prefixing	/*{{{*/
// ---------------------------------------------------------------------
/* */
bool pkgVersionMatch::MatchVer(const char *A,string B,bool Prefix)
{   
   // CNC:2003-11-05 - Patch by ALT-Linux, which ignores the release
   //                  if it was not provided, and the epoch.
   string s(A), sc(A);
   const char *Ab = s.c_str(), *Ac = sc.c_str();

   for (string::iterator i = s.begin(), k = sc.begin(); i != s.end(); ++i,++k)
   {
      if (*i == ':')
      {
         Ab = &(*i) + 1;
	 Ac = &(*k) + 1;
      }
      else if (*i == '-')
      {
         *i = 0;
	 break;
      }
   }

   const char *Ae = Ab + strlen(Ab);
   const char *Af = Ac + strlen(Ac);
   
   // Strings are not a compatible size.
   if (((unsigned)(Ae - Ab) == B.length() || Prefix == true) &&
       (unsigned)(Ae - Ab) >= B.length() &&
       stringcasecmp(B,Ab,Ab + B.length()) == 0)
       return true;
   else if (((unsigned)(Af - Ac) == B.length() || Prefix == true) &&
       (unsigned)(Af - Ac) >= B.length() &&
       stringcasecmp(B,Ac,Ac + B.length()) == 0)
       return true;

   return false;
}
									/*}}}*/
// VersionMatch::Find - Locate the best match for the select type	/*{{{*/
// ---------------------------------------------------------------------
/* */
pkgCache::VerIterator pkgVersionMatch::Find(pkgCache::PkgIterator Pkg)
{
   // CNC:2003-11-05
   pkgVersioningSystem *VS = Pkg.Cache()->VS;
   pkgCache::VerIterator Ver = Pkg.VersionList();

   for (; Ver.end() == false; Ver++)
   {
      if (Type == Version)
      {
	 // CNC:2003-11-05
         if (VerPrefixMatch)
	 {
	    if (MatchVer(Ver.VerStr(),VerStr,VerPrefixMatch) == true)
	       return Ver;
	 } else {
	    if (VS->CheckDep(Ver.VerStr(),VerOp,VerStr.c_str()) == true)
	       return Ver;
	 }

	 continue;
      }
      
      for (pkgCache::VerFileIterator VF = Ver.FileList(); VF.end() == false; VF++)
	 if (FileMatch(VF.File()) == true)
	    return Ver;
   }
      
   // CNC:2003-11-11 - Virtual package handling.
   if (Type == Version)
   {
      bool HasRelease = (strchr(VerStr.c_str(), '-') != NULL);
      pkgCache::PrvIterator Prv = Pkg.ProvidesList();
      for (; Prv.end() == false; Prv++)
      {
	 const char *PrvVerStr = Prv.ProvideVersion();
	 if (PrvVerStr == NULL || PrvVerStr[0] == 0)
	    continue;
         if (VerPrefixMatch || (HasRelease && strchr(PrvVerStr, '-') == NULL))
         {
            if (MatchVer(PrvVerStr,VerStr,VerPrefixMatch) == true)
               return Prv.OwnerVer();
         } else {
            if (VS->CheckDep(PrvVerStr,VerOp,VerStr.c_str()) == true)
               return Prv.OwnerVer();
         }
      }
   }

   // This will be Ended by now.
   return Ver;
}
									/*}}}*/
// VersionMatch::FileMatch - Match against an index file		/*{{{*/
// ---------------------------------------------------------------------
/* This matcher checks against the release file and the origin location 
   to see if the constraints are met. */
bool pkgVersionMatch::FileMatch(pkgCache::PkgFileIterator File)
{
   if (Type == Release)
   {
      if (MatchAll == true)
	 return true;
      
/*      cout << RelVerStr << ',' << RelOrigin << ',' << RelArchive << ',' << RelLabel << endl;
      cout << File.Version() << ',' << File.Origin() << ',' << File.Archive() << ',' << File.Label() << endl;*/
      
      if (RelVerStr.empty() == true && RelOrigin.empty() == true &&
	  RelArchive.empty() == true && RelLabel.empty() == true &&
	  RelComponent.empty() == true)
	 return false;
      
      if (RelVerStr.empty() == false)
	 if (File->Version == 0 ||
	     MatchVer(File.Version(),RelVerStr,RelVerPrefixMatch) == false)
	    return false;
      if (RelOrigin.empty() == false)
	 if (File->Origin == 0 ||
	     stringcasecmp(RelOrigin,File.Origin()) != 0)
	    return false;
      if (RelArchive.empty() == false)
      {
	 if (File->Archive == 0 || 
	     stringcasecmp(RelArchive,File.Archive()) != 0)
	    return false;
      }      
      if (RelLabel.empty() == false)
	 if (File->Label == 0 ||
	     stringcasecmp(RelLabel,File.Label()) != 0)
	    return false;
      if (RelComponent.empty() == false)
	 if (File->Component == 0 ||
	     stringcasecmp(RelComponent,File.Component()) != 0)
	    return false;
      return true;
   }
   
   if (Type == Origin)
   {
      if (OrSite.empty() == false) {
	 if (File->Site == 0 || OrSite != File.Site())
	    return false;
      } else // so we are talking about file:// or status file
	 if (strcmp(File.Site(),"") == 0 && File->Archive != 0) // skip the status file
	    return false;
      return (OrSite == File.Site());		/* both strings match */
   }
   
   return false;
}
									/*}}}*/

// vim:sts=3:sw=3
