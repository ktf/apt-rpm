// -*- mode: c++; mode: fold -*-
// Description								/*{{{*/
// $Id: apt-cache.cc,v 1.67 2003/08/02 19:53:23 mdz Exp $
/* ######################################################################
   
   apt-cache - Manages the cache files
   
   apt-cache provides some functions fo manipulating the cache files.
   It uses the command line interface common to all the APT tools. 
   
   Returns 100 on failure, 0 on success.
   
   ##################################################################### */
									/*}}}*/
// Include Files							/*{{{*/
#include <apt-pkg/error.h>
#include <apt-pkg/pkgcachegen.h>
#include <apt-pkg/init.h>
#include <apt-pkg/progress.h>
#include <apt-pkg/sourcelist.h>
#include <apt-pkg/cmndline.h>
#include <apt-pkg/strutl.h>
#include <apt-pkg/pkgrecords.h>
#include <apt-pkg/srcrecords.h>
#include <apt-pkg/version.h>
#include <apt-pkg/policy.h>
#include <apt-pkg/tagfile.h>
#include <apt-pkg/algorithms.h>
#include <apt-pkg/sptr.h>

#include <config.h>
#include <apti18n.h>

// CNC:2003-02-14 - apti18n.h includes libintl.h which includes locale.h,
// 		    as reported by Radu Greab.
//#include <locale.h>
#include <iostream>
#include <unistd.h>
#include <errno.h>
#include <regex.h>
#include <stdio.h>

// CNC:2003-11-23
#include <apt-pkg/luaiface.h>
    
#include "cmdline.h"
									/*}}}*/

using namespace std;

ostream c0out(0);
ostream c1out(0);
ostream c2out(0);
ofstream devnull("/dev/null");
unsigned int ScreenWidth = 80;

pkgCache *GCache = 0;
pkgSourceList *SrcList = 0;

// CNC:2003-11-23
// Script - Scripting stuff.						/*{{{*/
// ---------------------------------------------------------------------
/* */
#ifdef APT_WITH_LUA
bool Script(CommandLine &CmdL)
{
   for (const char **I = CmdL.FileList+1; *I != 0; I++)
      _config->Set("Scripts::AptCache::Script::", *I);

   _lua->SetCache(GCache);
   _lua->RunScripts("Scripts::AptCache::Script");
   _lua->ResetGlobals();

   return true;
}
#endif
									/*}}}*/
// UnMet - Show unmet dependencies					/*{{{*/
// ---------------------------------------------------------------------
/* */
bool UnMet(CommandLine &CmdL)
{
   return cmdUnMet(CmdL, *GCache);
}
									/*}}}*/
// DumpPackage - Show a dump of a package record			/*{{{*/
// ---------------------------------------------------------------------
/* */
bool DumpPackage(CommandLine &CmdL)
{   
   return cmdDumpPackage(CmdL, *GCache);
}
									/*}}}*/
// Stats - Dump some nice statistics					/*{{{*/
// ---------------------------------------------------------------------
/* */
bool Stats(CommandLine &Cmd)
{
   pkgCache &Cache = *GCache;
   cout << _("Total Package Names : ") << Cache.Head().PackageCount << " (" <<
      SizeToStr(Cache.Head().PackageCount*Cache.Head().PackageSz) << ')' << endl;

   int Normal = 0;
   int Virtual = 0;
   int NVirt = 0;
   int DVirt = 0;
   int Missing = 0;
   pkgCache::PkgIterator I = Cache.PkgBegin();
   for (;I.end() != true; I++)
   {
      if (I->VersionList != 0 && I->ProvidesList == 0)
      {
	 Normal++;
	 continue;
      }

      if (I->VersionList != 0 && I->ProvidesList != 0)
      {
	 NVirt++;
	 continue;
      }
      
      if (I->VersionList == 0 && I->ProvidesList != 0)
      {
	 // Only 1 provides
	 if (I.ProvidesList()->NextProvides == 0)
	 {
	    DVirt++;
	 }
	 else
	    Virtual++;
	 continue;
      }
      if (I->VersionList == 0 && I->ProvidesList == 0)
      {
	 Missing++;
	 continue;
      }
   }
   cout << _("  Normal Packages: ") << Normal << endl;
   cout << _("  Pure Virtual Packages: ") << Virtual << endl;
   cout << _("  Single Virtual Packages: ") << DVirt << endl;
   cout << _("  Mixed Virtual Packages: ") << NVirt << endl;
   cout << _("  Missing: ") << Missing << endl;
   
   cout << _("Total Distinct Versions: ") << Cache.Head().VersionCount << " (" <<
      SizeToStr(Cache.Head().VersionCount*Cache.Head().VersionSz) << ')' << endl;
   cout << _("Total Dependencies: ") << Cache.Head().DependsCount << " (" << 
      SizeToStr(Cache.Head().DependsCount*Cache.Head().DependencySz) << ')' << endl;
   
   cout << _("Total Ver/File relations: ") << Cache.Head().VerFileCount << " (" <<
      SizeToStr(Cache.Head().VerFileCount*Cache.Head().VerFileSz) << ')' << endl;
   cout << _("Total Provides Mappings: ") << Cache.Head().ProvidesCount << " (" <<
      SizeToStr(Cache.Head().ProvidesCount*Cache.Head().ProvidesSz) << ')' << endl;
   
   // String list stats
   size_t Size = 0;
   unsigned long Count = 0;
   for (pkgCache::StringItem *I = Cache.StringItemP + Cache.Head().StringList;
        I!= Cache.StringItemP; I = Cache.StringItemP + I->NextItem)
   {
      Count++;
      Size += strlen(Cache.StrP + I->String) + 1;
   }
   cout << _("Total Globbed Strings: ") << Count << " (" << SizeToStr(Size) << ')' << endl;

   size_t DepVerSize = 0;
   for (pkgCache::PkgIterator P = Cache.PkgBegin(); P.end() == false; P++)
   {
      for (pkgCache::VerIterator V = P.VersionList(); V.end() == false; V++)
      {
	 for (pkgCache::DepIterator D = V.DependsList(); D.end() == false; D++)
	 {
	    if (D->Version != 0)
	       DepVerSize += strlen(D.TargetVer()) + 1;
	 }
      }
   }
   cout << _("Total Dependency Version space: ") << SizeToStr(DepVerSize) << endl;
   
   unsigned long Slack = 0;
   for (int I = 0; I != 7; I++)
      Slack += Cache.Head().Pools[I].ItemSize*Cache.Head().Pools[I].Count;
   cout << _("Total Slack space: ") << SizeToStr(Slack) << endl;
   
   unsigned long Total = 0;
   Total = Slack + Size + Cache.Head().DependsCount*Cache.Head().DependencySz + 
           Cache.Head().VersionCount*Cache.Head().VersionSz +
           Cache.Head().PackageCount*Cache.Head().PackageSz + 
           Cache.Head().VerFileCount*Cache.Head().VerFileSz +
           Cache.Head().ProvidesCount*Cache.Head().ProvidesSz;
   cout << _("Total Space Accounted for: ") << SizeToStr(Total) << endl;
   
   return true;
}
									/*}}}*/
// Dump - show everything						/*{{{*/
// ---------------------------------------------------------------------
/* This is worthless except fer debugging things */
bool Dump(CommandLine &Cmd)
{
   pkgCache &Cache = *GCache;
   cout << "Using Versioning System: " << Cache.VS->Label << endl;
   
   for (pkgCache::PkgIterator P = Cache.PkgBegin(); P.end() == false; P++)
   {
      cout << "Package: " << P.Name() << endl;
      for (pkgCache::VerIterator V = P.VersionList(); V.end() == false; V++)
      {
	 cout << " Version: " << V.VerStr() << endl;
	 cout << "     File: " << V.FileList().File().FileName() << endl;
	 for (pkgCache::DepIterator D = V.DependsList(); D.end() == false; D++)
	    cout << "  Depends: " << D.TargetPkg().Name() << ' ' << 
	                     DeNull(D.TargetVer()) << endl;
      }      
   }

   for (pkgCache::PkgFileIterator F = Cache.FileBegin(); F.end() == false; F++)
   {
      cout << "File: " << F.FileName() << endl;
      cout << " Type: " << F.IndexType() << endl;
      cout << " Size: " << F->Size << endl;
      cout << " ID: " << F->ID << endl;
      cout << " Flags: " << F->Flags << endl;
      cout << " Time: " << TimeRFC1123(F->mtime) << endl;
      cout << " Archive: " << DeNull(F.Archive()) << endl;
      cout << " Component: " << DeNull(F.Component()) << endl;
      cout << " Version: " << DeNull(F.Version()) << endl;
      cout << " Origin: " << DeNull(F.Origin()) << endl;
      cout << " Site: " << DeNull(F.Site()) << endl;
      cout << " Label: " << DeNull(F.Label()) << endl;
      cout << " Architecture: " << DeNull(F.Architecture()) << endl;
   }

   return true;
}
									/*}}}*/
// DumpAvail - Print out the available list				/*{{{*/
// ---------------------------------------------------------------------
/* This is needed to make dpkg --merge happy.. I spent a bit of time to 
   make this run really fast, perhaps I went a little overboard.. */
bool DumpAvail(CommandLine &Cmd)
{
   pkgCache &Cache = *GCache;

   pkgPolicy Plcy(&Cache);
   if (ReadPinFile(Plcy) == false)
      return false;
   
   unsigned long Count = Cache.HeaderP->PackageCount+1;
   pkgCache::VerFile **VFList = new pkgCache::VerFile *[Count];
   memset(VFList,0,sizeof(*VFList)*Count);
   
   // Map versions that we want to write out onto the VerList array.
   for (pkgCache::PkgIterator P = Cache.PkgBegin(); P.end() == false; P++)
   {    
      if (P->VersionList == 0)
	 continue;
      
      /* Find the proper version to use. If the policy says there are no
         possible selections we return the installed version, if available..
       	 This prevents dselect from making it obsolete. */
      pkgCache::VerIterator V = Plcy.GetCandidateVer(P);
      if (V.end() == true)
      {
	 if (P->CurrentVer == 0)
	    continue;
	 V = P.CurrentVer();
      }
      
      pkgCache::VerFileIterator VF = V.FileList();
      for (; VF.end() == false ; VF++)
	 if ((VF.File()->Flags & pkgCache::Flag::NotSource) == 0)
	    break;
      
      /* Okay, here we have a bit of a problem.. The policy has selected the
         currently installed package - however it only exists in the
       	 status file.. We need to write out something or dselect will mark
         the package as obsolete! Thus we emit the status file entry, but
         below we remove the status line to make it valid for the 
         available file. However! We only do this if their do exist *any*
         non-source versions of the package - that way the dselect obsolete
         handling works OK. */
      if (VF.end() == true)
      {
	 for (pkgCache::VerIterator Cur = P.VersionList(); Cur.end() != true; Cur++)
	 {
	    for (VF = Cur.FileList(); VF.end() == false; VF++)
	    {	 
	       if ((VF.File()->Flags & pkgCache::Flag::NotSource) == 0)
	       {
		  VF = V.FileList();
		  break;
	       }
	    }
	    
	    if (VF.end() == false)
	       break;
	 }
      }
      
// CNC:2002-07-24
#if HAVE_RPM
      if (VF.end() == false)
      {
	 pkgRecords Recs(Cache);
	 pkgRecords::Parser &P = Recs.Lookup(VF);
	 const char *Start;
	 const char *End;
	 P.GetRec(Start,End);
	 cout << string(Start,End-Start) << endl;
      }
   }
   return !_error->PendingError();
#else
      VFList[P->ID] = VF;
   }
#endif
   
   LocalitySort(VFList,Count,sizeof(*VFList));

   // Iterate over all the package files and write them out.
   char *Buffer = new char[Cache.HeaderP->MaxVerFileSize+10];
   for (pkgCache::VerFile **J = VFList; *J != 0;)
   {
      pkgCache::PkgFileIterator File(Cache,(*J)->File + Cache.PkgFileP);
      if (File.IsOk() == false)
      {
	 _error->Error(_("Package file %s is out of sync."),File.FileName());
	 break;
      }

      FileFd PkgF(File.FileName(),FileFd::ReadOnly);
      if (_error->PendingError() == true)
	 break;
      
      /* Write all of the records from this package file, since we
       	 already did locality sorting we can now just seek through the
       	 file in read order. We apply 1 more optimization here, since often
       	 there will be < 1 byte gaps between records (for the \n) we read that
       	 into the next buffer and offset a bit.. */
      unsigned long Pos = 0;
      for (; *J != 0; J++)
      {
	 if ((*J)->File + Cache.PkgFileP != File)
	    break;
	 
	 const pkgCache::VerFile &VF = **J;

	 // Read the record and then write it out again.
	 unsigned long Jitter = VF.Offset - Pos;
	 if (Jitter > 8)
	 {
	    if (PkgF.Seek(VF.Offset) == false)
	       break;
	    Jitter = 0;
	 }
	 
	 if (PkgF.Read(Buffer,VF.Size + Jitter) == false)
	    break;
	 Buffer[VF.Size + Jitter] = '\n';
	 
	 // See above..
	 if ((File->Flags & pkgCache::Flag::NotSource) == pkgCache::Flag::NotSource)
	 {
	    pkgTagSection Tags;
	    TFRewriteData RW[] = {{"Status",0,0},{"Config-Version",0,0},{0,0,0}};
	    const char *Zero = 0;
	    if (Tags.Scan(Buffer+Jitter,VF.Size+1) == false ||
		TFRewrite(stdout,Tags,&Zero,RW) == false)
	    {
	       _error->Error("Internal Error, Unable to parse a package record");
	       break;
	    }
	    fputc('\n',stdout);
	 }
	 else
	 {
	    if (fwrite(Buffer+Jitter,VF.Size+1,1,stdout) != 1)
	       break;
	 }
	 
	 Pos = VF.Offset + VF.Size;
      }

      fflush(stdout);
      if (_error->PendingError() == true)
         break;
   }
   
   delete [] Buffer;
   delete [] VFList;
   return !_error->PendingError();
}
									/*}}}*/
bool Depends(CommandLine &CmdL)
{
   return cmdDepends(CmdL, *GCache);
}

bool RDepends(CommandLine &CmdL)
{
   return cmdRDepends(CmdL, *GCache);
}

bool WhatDepends(CommandLine &CmdL)
{
   return cmdWhatDepends(CmdL, *GCache);
}

bool WhatProvides(CommandLine &CmdL)
{
   return cmdWhatProvides(CmdL, *GCache);
}

// xvcg - Generate a graph for xvcg					/*{{{*/
// ---------------------------------------------------------------------
// Code contributed from Junichi Uekawa <dancer@debian.org> on 20 June 2002.

bool XVcg(CommandLine &CmdL)
{
   pkgCache &Cache = *GCache;
   bool GivenOnly = _config->FindB("APT::Cache::GivenOnly",false);
   
   /* Normal packages are boxes
      Pure Provides are triangles
      Mixed are diamonds
      rhomb are missing packages*/
   const char *Shapes[] = {"ellipse","triangle","box","rhomb"};
   
   /* Initialize the list of packages to show.
      1 = To Show
      2 = To Show no recurse
      3 = Emitted no recurse
      4 = Emitted
      0 = None */
   enum States {None=0, ToShow, ToShowNR, DoneNR, Done};
   enum TheFlags {ForceNR=(1<<0)};
   unsigned char *Show = new unsigned char[Cache.Head().PackageCount];
   unsigned char *Flags = new unsigned char[Cache.Head().PackageCount];
   unsigned char *ShapeMap = new unsigned char[Cache.Head().PackageCount];
   
   // Show everything if no arguments given
   if (CmdL.FileList[1] == 0)
      for (unsigned long I = 0; I != Cache.Head().PackageCount; I++)
	 Show[I] = ToShow;
   else
      for (unsigned long I = 0; I != Cache.Head().PackageCount; I++)
	 Show[I] = None;
   memset(Flags,0,sizeof(*Flags)*Cache.Head().PackageCount);
   
   // Map the shapes
   for (pkgCache::PkgIterator Pkg = Cache.PkgBegin(); Pkg.end() == false; Pkg++)
   {   
      if (Pkg->VersionList == 0)
      {
	 // Missing
	 if (Pkg->ProvidesList == 0)
	    ShapeMap[Pkg->ID] = 0;
	 else
	    ShapeMap[Pkg->ID] = 1;
      }
      else
      {
	 // Normal
	 if (Pkg->ProvidesList == 0)
	    ShapeMap[Pkg->ID] = 2;
	 else
	    ShapeMap[Pkg->ID] = 3;
      }
   }
   
   // Load the list of packages from the command line into the show list
   for (const char **I = CmdL.FileList + 1; *I != 0; I++)
   {
      // Process per-package flags
      string P = *I;
      bool Force = false;
      if (P.length() > 3)
      {
	 if (P.end()[-1] == '^')
	 {
	    Force = true;
	    P.erase(P.end()-1);
	 }
	 
	 if (P.end()[-1] == ',')
	    P.erase(P.end()-1);
      }
      
      // Locate the package
      pkgCache::PkgIterator Pkg = Cache.FindPkg(P);
      if (Pkg.end() == true)
      {
	 _error->Warning(_("Unable to locate package %s"),*I);
	 continue;
      }
      Show[Pkg->ID] = ToShow;
      
      if (Force == true)
	 Flags[Pkg->ID] |= ForceNR;
   }
   
   // Little header
   cout << "graph: { title: \"packages\"" << endl <<
     "xmax: 700 ymax: 700 x: 30 y: 30" << endl <<
     "layout_downfactor: 8" << endl;

   bool Act = true;
   while (Act == true)
   {
      Act = false;
      for (pkgCache::PkgIterator Pkg = Cache.PkgBegin(); Pkg.end() == false; Pkg++)
      {
	 // See we need to show this package
	 if (Show[Pkg->ID] == None || Show[Pkg->ID] >= DoneNR)
	    continue;

	 //printf ("node: { title: \"%s\" label: \"%s\" }\n", Pkg.Name(), Pkg.Name());
	 
	 // Colour as done
	 if (Show[Pkg->ID] == ToShowNR || (Flags[Pkg->ID] & ForceNR) == ForceNR)
	 {
	    // Pure Provides and missing packages have no deps!
	    if (ShapeMap[Pkg->ID] == 0 || ShapeMap[Pkg->ID] == 1)
	       Show[Pkg->ID] = Done;
	    else
	       Show[Pkg->ID] = DoneNR;
	 }	 
	 else
	    Show[Pkg->ID] = Done;
	 Act = true;

	 // No deps to map out
	 if (Pkg->VersionList == 0 || Show[Pkg->ID] == DoneNR)
	    continue;
	 
	 pkgCache::VerIterator Ver = Pkg.VersionList();
	 for (pkgCache::DepIterator D = Ver.DependsList(); D.end() == false; D++)
	 {
	    // See if anything can meet this dep
	    // Walk along the actual package providing versions
	    bool Hit = false;
	    pkgCache::PkgIterator DPkg = D.TargetPkg();
	    for (pkgCache::VerIterator I = DPkg.VersionList();
		      I.end() == false && Hit == false; I++)
	    {
	       if (Cache.VS->CheckDep(I.VerStr(),D->CompareOp,D.TargetVer()) == true)
		  Hit = true;
	    }
	    
	    // Follow all provides
	    for (pkgCache::PrvIterator I = DPkg.ProvidesList(); 
		      I.end() == false && Hit == false; I++)
	    {
	       if (Cache.VS->CheckDep(I.ProvideVersion(),D->CompareOp,D.TargetVer()) == false)
		  Hit = true;
	    }
	    

	    // Only graph critical deps	    
	    if (D.IsCritical() == true)
	    {
	       printf ("edge: { sourcename: \"%s\" targetname: \"%s\" class: 2 ",Pkg.Name(), D.TargetPkg().Name() );
	       
	       // Colour the node for recursion
	       if (Show[D.TargetPkg()->ID] <= DoneNR)
	       {
		  /* If a conflicts does not meet anything in the database
		     then show the relation but do not recurse */
		  if (Hit == false && 
		      (D->Type == pkgCache::Dep::Conflicts ||
		       D->Type == pkgCache::Dep::Obsoletes))
		  {
		     if (Show[D.TargetPkg()->ID] == None && 
			 Show[D.TargetPkg()->ID] != ToShow)
			Show[D.TargetPkg()->ID] = ToShowNR;
		  }		  
		  else
		  {
		     if (GivenOnly == true && Show[D.TargetPkg()->ID] != ToShow)
			Show[D.TargetPkg()->ID] = ToShowNR;
		     else
			Show[D.TargetPkg()->ID] = ToShow;
		  }
	       }
	       
	       // Edge colour
	       switch(D->Type)
	       {
		  case pkgCache::Dep::Conflicts:
		    printf("label: \"conflicts\" color: lightgreen }\n");
		    break;
		  case pkgCache::Dep::Obsoletes:
		    printf("label: \"obsoletes\" color: lightgreen }\n");
		    break;
		  
		  case pkgCache::Dep::PreDepends:
		    printf("label: \"predepends\" color: blue }\n");
		    break;
		  
		  default:
		    printf("}\n");
		  break;
	       }	       
	    }	    
	 }
      }
   }   
   
   /* Draw the box colours after the fact since we can not tell what colour
      they should be until everything is finished drawing */
   for (pkgCache::PkgIterator Pkg = Cache.PkgBegin(); Pkg.end() == false; Pkg++)
   {
      if (Show[Pkg->ID] < DoneNR)
	 continue;

      if (Show[Pkg->ID] == DoneNR)
	 printf("node: { title: \"%s\" label: \"%s\" color: orange shape: %s }\n", Pkg.Name(), Pkg.Name(),
		Shapes[ShapeMap[Pkg->ID]]);
      else
	printf("node: { title: \"%s\" label: \"%s\" shape: %s }\n", Pkg.Name(), Pkg.Name(), 
		Shapes[ShapeMap[Pkg->ID]]);
      
   }
   
   printf("}\n");
   return true;
}
									/*}}}*/


// Dotty - Generate a graph for Dotty					/*{{{*/
// ---------------------------------------------------------------------
/* Dotty is the graphvis program for generating graphs. It is a fairly
   simple queuing algorithm that just writes dependencies and nodes. 
   http://www.research.att.com/sw/tools/graphviz/ */
bool Dotty(CommandLine &CmdL)
{
   pkgCache &Cache = *GCache;
   bool GivenOnly = _config->FindB("APT::Cache::GivenOnly",false);
   
   /* Normal packages are boxes
      Pure Provides are triangles
      Mixed are diamonds
      Hexagons are missing packages*/
   const char *Shapes[] = {"hexagon","triangle","box","diamond"};
   
   /* Initialize the list of packages to show.
      1 = To Show
      2 = To Show no recurse
      3 = Emitted no recurse
      4 = Emitted
      0 = None */
   enum States {None=0, ToShow, ToShowNR, DoneNR, Done};
   enum TheFlags {ForceNR=(1<<0)};
   unsigned char *Show = new unsigned char[Cache.Head().PackageCount];
   unsigned char *Flags = new unsigned char[Cache.Head().PackageCount];
   unsigned char *ShapeMap = new unsigned char[Cache.Head().PackageCount];
   
   // Show everything if no arguments given
   if (CmdL.FileList[1] == 0)
      for (unsigned long I = 0; I != Cache.Head().PackageCount; I++)
	 Show[I] = ToShow;
   else
      for (unsigned long I = 0; I != Cache.Head().PackageCount; I++)
	 Show[I] = None;
   memset(Flags,0,sizeof(*Flags)*Cache.Head().PackageCount);
   
   // Map the shapes
   for (pkgCache::PkgIterator Pkg = Cache.PkgBegin(); Pkg.end() == false; Pkg++)
   {   
      if (Pkg->VersionList == 0)
      {
	 // Missing
	 if (Pkg->ProvidesList == 0)
	    ShapeMap[Pkg->ID] = 0;
	 else
	    ShapeMap[Pkg->ID] = 1;
      }
      else
      {
	 // Normal
	 if (Pkg->ProvidesList == 0)
	    ShapeMap[Pkg->ID] = 2;
	 else
	    ShapeMap[Pkg->ID] = 3;
      }
   }
   
   // Load the list of packages from the command line into the show list
   for (const char **I = CmdL.FileList + 1; *I != 0; I++)
   {
      // Process per-package flags
      string P = *I;
      bool Force = false;
      if (P.length() > 3)
      {
	 if (P.end()[-1] == '^')
	 {
	    Force = true;
	    P.erase(P.end()-1);
	 }
	 
	 if (P.end()[-1] == ',')
	    P.erase(P.end()-1);
      }
      
      // Locate the package
      pkgCache::PkgIterator Pkg = Cache.FindPkg(P);
      if (Pkg.end() == true)
      {
	 _error->Warning(_("Unable to locate package %s"),*I);
	 continue;
      }
      Show[Pkg->ID] = ToShow;
      
      if (Force == true)
	 Flags[Pkg->ID] |= ForceNR;
   }
   
   // Little header
   printf("digraph packages {\n");
   printf("concentrate=true;\n");
   printf("size=\"30,40\";\n");
   
   bool Act = true;
   while (Act == true)
   {
      Act = false;
      for (pkgCache::PkgIterator Pkg = Cache.PkgBegin(); Pkg.end() == false; Pkg++)
      {
	 // See we need to show this package
	 if (Show[Pkg->ID] == None || Show[Pkg->ID] >= DoneNR)
	    continue;
	 
	 // Colour as done
	 if (Show[Pkg->ID] == ToShowNR || (Flags[Pkg->ID] & ForceNR) == ForceNR)
	 {
	    // Pure Provides and missing packages have no deps!
	    if (ShapeMap[Pkg->ID] == 0 || ShapeMap[Pkg->ID] == 1)
	       Show[Pkg->ID] = Done;
	    else
	       Show[Pkg->ID] = DoneNR;
	 }	 
	 else
	    Show[Pkg->ID] = Done;
	 Act = true;

	 // No deps to map out
	 if (Pkg->VersionList == 0 || Show[Pkg->ID] == DoneNR)
	    continue;
	 
	 pkgCache::VerIterator Ver = Pkg.VersionList();
	 for (pkgCache::DepIterator D = Ver.DependsList(); D.end() == false; D++)
	 {
	    // See if anything can meet this dep
	    // Walk along the actual package providing versions
	    bool Hit = false;
	    pkgCache::PkgIterator DPkg = D.TargetPkg();
	    for (pkgCache::VerIterator I = DPkg.VersionList();
		      I.end() == false && Hit == false; I++)
	    {
	       if (Cache.VS->CheckDep(I.VerStr(),D->CompareOp,D.TargetVer()) == true)
		  Hit = true;
	    }
	    
	    // Follow all provides
	    for (pkgCache::PrvIterator I = DPkg.ProvidesList(); 
		      I.end() == false && Hit == false; I++)
	    {
	       if (Cache.VS->CheckDep(I.ProvideVersion(),D->CompareOp,D.TargetVer()) == false)
		  Hit = true;
	    }
	    
	    // Only graph critical deps	    
	    if (D.IsCritical() == true)
	    {
	       printf("\"%s\" -> \"%s\"",Pkg.Name(),D.TargetPkg().Name());
	       
	       // Colour the node for recursion
	       if (Show[D.TargetPkg()->ID] <= DoneNR)
	       {
		  /* If a conflicts does not meet anything in the database
		     then show the relation but do not recurse */
		  if (Hit == false && 
		      (D->Type == pkgCache::Dep::Conflicts ||
		       D->Type == pkgCache::Dep::Obsoletes))
		  {
		     if (Show[D.TargetPkg()->ID] == None && 
			 Show[D.TargetPkg()->ID] != ToShow)
			Show[D.TargetPkg()->ID] = ToShowNR;
		  }		  
		  else
		  {
		     if (GivenOnly == true && Show[D.TargetPkg()->ID] != ToShow)
			Show[D.TargetPkg()->ID] = ToShowNR;
		     else
			Show[D.TargetPkg()->ID] = ToShow;
		  }
	       }
	       
	       // Edge colour
	       switch(D->Type)
	       {
		  case pkgCache::Dep::Conflicts:
		  case pkgCache::Dep::Obsoletes:
		  printf("[color=springgreen];\n");
		  break;
		  
		  case pkgCache::Dep::PreDepends:
		  printf("[color=blue];\n");
		  break;
		  
		  default:
		  printf(";\n");
		  break;
	       }	       
	    }	    
	 }
      }
   }   
   
   /* Draw the box colours after the fact since we can not tell what colour
      they should be until everything is finished drawing */
   for (pkgCache::PkgIterator Pkg = Cache.PkgBegin(); Pkg.end() == false; Pkg++)
   {
      if (Show[Pkg->ID] < DoneNR)
	 continue;
      
      // Orange box for early recursion stoppage
      if (Show[Pkg->ID] == DoneNR)
	 printf("\"%s\" [color=orange,shape=%s];\n",Pkg.Name(),
		Shapes[ShapeMap[Pkg->ID]]);
      else
	 printf("\"%s\" [shape=%s];\n",Pkg.Name(),
		Shapes[ShapeMap[Pkg->ID]]);
   }
   
   printf("}\n");
   return true;
}
									/*}}}*/
// DoAdd - Perform an adding operation					/*{{{*/
// ---------------------------------------------------------------------
/* */
bool DoAdd(CommandLine &CmdL)
{
   return _error->Error("Unimplemented");
#if 0   
   // Make sure there is at least one argument
   if (CmdL.FileSize() <= 1)
      return _error->Error("You must give at least one file name");
   
   // Open the cache
   FileFd CacheF(_config->FindFile("Dir::Cache::pkgcache"),FileFd::WriteAny);
   if (_error->PendingError() == true)
      return false;
   
   DynamicMMap Map(CacheF,MMap::Public);
   if (_error->PendingError() == true)
      return false;

   OpTextProgress Progress(*_config);
   pkgCacheGenerator Gen(Map,Progress);
   if (_error->PendingError() == true)
      return false;

   unsigned long Length = CmdL.FileSize() - 1;
   for (const char **I = CmdL.FileList + 1; *I != 0; I++)
   {
      Progress.OverallProgress(I - CmdL.FileList,Length,1,"Generating cache");
      Progress.SubProgress(Length);

      // Do the merge
      FileFd TagF(*I,FileFd::ReadOnly);
      debListParser Parser(TagF);
      if (_error->PendingError() == true)
	 return _error->Error("Problem opening %s",*I);
      
      if (Gen.SelectFile(*I,"") == false)
	 return _error->Error("Problem with SelectFile");
	 
      if (Gen.MergeList(Parser) == false)
	 return _error->Error("Problem with MergeList");
   }

   Progress.Done();
   GCache = &Gen.GetCache();
   Stats(CmdL);
   
   return true;
#endif   
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
// Search - Perform a search						/*{{{*/
// ---------------------------------------------------------------------
/* This searches the package names and pacakge descriptions for a pattern */
bool Search(CommandLine &CmdL)
{
   return cmdSearch(CmdL, *GCache);
}
									/*}}}*/
bool SearchFile(CommandLine &CmdL)
{
   return cmdSearchFile(CmdL, *GCache);
}

// ShowPackage - Dump the package record to the screen			/*{{{*/
// ---------------------------------------------------------------------
/* */
bool ShowPackage(CommandLine &CmdL)
{   
   return cmdShowPackage(CmdL, *GCache);
}

bool FileList(CommandLine &CmdL)
{
   return cmdFileList(CmdL, *GCache);
}
									/*}}}*/
bool ChangeLog(CommandLine &CmdL)
{
   return cmdChangeLog(CmdL, *GCache);
}

// ShowPkgNames - Show package names					/*{{{*/
// ---------------------------------------------------------------------
/* This does a prefix match on the first argument */
bool ShowPkgNames(CommandLine &CmdL)
{
   pkgCache &Cache = *GCache;
   pkgCache::PkgIterator I = Cache.PkgBegin();
   bool All = _config->FindB("APT::Cache::AllNames", false);
   
   if (CmdL.FileList[1] != 0)
   {
      for (;I.end() != true; I++)
      {
	 if (All == false && I->VersionList == 0)
	    continue;
	 
	 if (strncmp(I.Name(),CmdL.FileList[1],strlen(CmdL.FileList[1])) == 0)
	    cout << I.Name() << endl;
      }

      return true;
   }
   
   // Show all pkgs
   for (;I.end() != true; I++)
   {
      if (All == false && I->VersionList == 0)
	 continue;
      cout << I.Name() << endl;
   }
   
   return true;
}
									/*}}}*/
// ShowSrcPackage - Show source package records				/*{{{*/
// ---------------------------------------------------------------------
/* */
bool ShowSrcPackage(CommandLine &CmdL)
{
   pkgSourceList List;
   List.ReadMainList();
   
   // Create the text record parsers
   pkgSrcRecords SrcRecs(List);
   if (_error->PendingError() == true)
      return false;

   for (const char **I = CmdL.FileList + 1; *I != 0; I++)
   {
      SrcRecs.Restart();
      
      pkgSrcRecords::Parser *Parse;
      while ((Parse = SrcRecs.Find(*I,false)) != 0)
	 cout << Parse->AsStr() << endl;;
   }      
   return true;
}
									/*}}}*/
// Policy - Show the results of the preferences file			/*{{{*/
// ---------------------------------------------------------------------
/* */
bool Policy(CommandLine &CmdL)
{
   if (SrcList == 0)
      return _error->Error("Generate must be enabled for this function");
   
   pkgCache &Cache = *GCache;
   pkgPolicy Plcy(&Cache);
   if (ReadPinFile(Plcy) == false)
      return false;
   
   // Print out all of the package files
   if (CmdL.FileList[1] == 0)
   {
      cout << _("Package Files:") << endl;   
      for (pkgCache::PkgFileIterator F = Cache.FileBegin(); F.end() == false; F++)
      {
	 // Locate the associated index files so we can derive a description
	 pkgIndexFile *Indx;
	 if (SrcList->FindIndex(F,Indx) == false &&
	     _system->FindIndex(F,Indx) == false)
	    return _error->Error(_("Cache is out of sync, can't x-ref a package file"));
	 printf(_("%4i %s\n"),
		Plcy.GetPriority(F),Indx->Describe(true).c_str());
	 
	 // Print the reference information for the package
	 string Str = F.RelStr();
	 if (Str.empty() == false)
	    printf("     release %s\n",F.RelStr().c_str());
	 if (F.Site() != 0 && F.Site()[0] != 0)
	    printf("     origin %s\n",F.Site());
      }
      
      // Show any packages have explicit pins
      cout << _("Pinned Packages:") << endl;
      pkgCache::PkgIterator I = Cache.PkgBegin();
      for (;I.end() != true; I++)
      {
	 if (Plcy.GetPriority(I) == 0)
	    continue;

	 // Print the package name and the version we are forcing to
	 cout << "     " << I.Name() << " -> ";
	 
	 pkgCache::VerIterator V = Plcy.GetMatch(I);
	 if (V.end() == true)
	    cout << _("(not found)") << endl;
	 else
	    cout << V.VerStr() << endl;
      }     
      
      return true;
   }
   
   // Print out detailed information for each package
   for (const char **I = CmdL.FileList + 1; *I != 0; I++)
   {
      pkgCache::PkgIterator Pkg = Cache.FindPkg(*I);
      if (Pkg.end() == true)
      {
	 _error->Warning(_("Unable to locate package %s"),*I);
	 continue;
      }
      
      cout << Pkg.Name() << ":" << endl;
      
      // Installed version
      cout << _("  Installed: ");
      if (Pkg->CurrentVer == 0)
	 cout << _("(none)") << endl;
      else
	 cout << Pkg.CurrentVer().VerStr() << endl;
      
      // Candidate Version 
      cout << _("  Candidate: ");
      pkgCache::VerIterator V = Plcy.GetCandidateVer(Pkg);
      if (V.end() == true)
	 cout << _("(none)") << endl;
      else
	 cout << V.VerStr() << endl;

      // Pinned version
      if (Plcy.GetPriority(Pkg) != 0)
      {
	 cout << _("  Package Pin: ");
	 V = Plcy.GetMatch(Pkg);
	 if (V.end() == true)
	    cout << _("(not found)") << endl;
	 else
	    cout << V.VerStr() << endl;
      }
      
      // Show the priority tables
      cout << _("  Version Table:") << endl;
      for (V = Pkg.VersionList(); V.end() == false; V++)
      {
	 if (Pkg.CurrentVer() == V)
	    cout << " *** " << V.VerStr();
	 else
	    cout << "     " << V.VerStr();
	 // CNC:2004-05-29
	 if (Plcy.GetCandidateVer(Pkg) == V)
	    cout << " " << Plcy.GetPriority(Pkg) << endl;
	 else
	    cout << " 0" << endl;
	 for (pkgCache::VerFileIterator VF = V.FileList(); VF.end() == false; VF++)
	 {
	    // Locate the associated index files so we can derive a description
	    pkgIndexFile *Indx;
	    if (SrcList->FindIndex(VF.File(),Indx) == false &&
		_system->FindIndex(VF.File(),Indx) == false)
	       return _error->Error(_("Cache is out of sync, can't x-ref a package file"));
	    printf(_("       %4i %s\n"),Plcy.GetPriority(VF.File()),
		   Indx->Describe(true).c_str());
	 }	 
      }      
   }
   
   return true;
}
									/*}}}*/
// GenCaches - Call the main cache generator				/*{{{*/
// ---------------------------------------------------------------------
/* */
bool GenCaches(CommandLine &Cmd)
{
   OpTextProgress Progress(*_config);
   
   pkgSourceList List;
   if (List.ReadMainList() == false)
      return false;   
   return pkgMakeStatusCache(List,Progress);
}

									/*}}}*/
// ShowHelp - Show a help screen					/*{{{*/
// ---------------------------------------------------------------------
/* */
bool ShowHelp(CommandLine &Cmd)
{
   ioprintf(cout,_("%s %s for %s %s compiled on %s %s\n"),PACKAGE,VERSION,
	    COMMON_OS,COMMON_CPU,__DATE__,__TIME__);
   
   if (_config->FindB("version") == true)
     return true;

   cout << 
    _("Usage: apt-cache [options] command\n"
      "       apt-cache [options] add file1 [file2 ...]\n"
      "       apt-cache [options] showpkg pkg1 [pkg2 ...]\n"
      "       apt-cache [options] showsrc pkg1 [pkg2 ...]\n"
      "\n"
      "apt-cache is a low-level tool used to manipulate APT's binary\n"
      "cache files, and query information from them\n"
      "\n"
      "Commands:\n"
      "   add - Add a package file to the source cache\n"
      "   gencaches - Build both the package and source cache\n"
      "   showpkg - Show some general information for a single package\n"
      "   showsrc - Show source records\n"
      "   stats - Show some basic statistics\n"
      "   dump - Show the entire file in a terse form\n"
      "   dumpavail - Print an available file to stdout\n"
      "   unmet - Show unmet dependencies\n"
      "   search - Search the package list for a regex pattern\n"
      "   searchfile - Search the packages for a file\n"
      "   files - Show file list of the package(s)\n"
      "   changelog - Show changelog entries of the package(s)\n"
      "   show - Show a readable record for the package\n"
      "   depends - Show raw dependency information for a package\n"
      "   whatdepends - Show packages depending on given capabilities\n"
      // "   rdepends - Show reverse dependency information for a package\n"
      "   whatprovides - Show packages that provide given capabilities\n"
      "   pkgnames - List the names of all packages\n"
      "   dotty - Generate package graphs for GraphVis\n"
      "   xvcg - Generate package graphs for xvcg\n"
      "   policy - Show policy settings\n"
// CNC:2003-03-16
      );
#ifdef APT_WITH_LUA
      _lua->RunScripts("Scripts::AptCache::Help::Command");
#endif
      cout << _(
      "\n"
      "Options:\n"
      "  -h   This help text.\n"
      "  -p=? The package cache.\n"
      "  -s=? The source cache.\n"
      "  -q   Disable progress indicator.\n"
      "  -i   Show only important deps for the unmet command.\n"
      "  -c=? Read this configuration file\n"
      "  -o=? Set an arbitary configuration option, eg -o dir::cache=/tmp\n"
      "See the apt-cache(8) and apt.conf(5) manual pages for more information.\n");
   return true;
}
									/*}}}*/
// CacheInitialize - Initialize things for apt-cache			/*{{{*/
// ---------------------------------------------------------------------
/* */
void CacheInitialize()
{
   _config->Set("quiet",0);
   _config->Set("help",false);
}
									/*}}}*/

int main(int argc,const char *argv[])
{
   CommandLine::Args Args[] = {
      {'h',"help","help",0},
      {'v',"version","version",0},
      {'p',"pkg-cache","Dir::Cache::pkgcache",CommandLine::HasArg},
      {'s',"src-cache","Dir::Cache::srcpkgcache",CommandLine::HasArg},
      {'q',"quiet","quiet",CommandLine::IntLevel},
      {'i',"important","APT::Cache::Important",0},
      {'f',"full","APT::Cache::ShowFull",0},
      {'g',"generate","APT::Cache::Generate",0},
      {'a',"all-versions","APT::Cache::AllVersions",0},
      {0,"names-only","APT::Cache::NamesOnly",0},
      {'n',"all-names","APT::Cache::AllNames",0},
      {0,"recurse","APT::Cache::RecurseDepends",0},
      {'c',"config-file",0,CommandLine::ConfigFile},
      {'o',"option",0,CommandLine::ArbItem},
      {'n',"installed","APT::Cache::Installed",0},
      {0,0,0,0}};
   CommandLine::Dispatch CmdsA[] = {{"help",&ShowHelp},
                                    {"add",&DoAdd},
                                    {"gencaches",&GenCaches},
                                    {"showsrc",&ShowSrcPackage},
                                    {0,0}};
   CommandLine::Dispatch CmdsB[] = {{"showpkg",&DumpPackage},
                                    {"stats",&Stats},
                                    {"dump",&Dump},
                                    {"dumpavail",&DumpAvail},
                                    {"unmet",&UnMet},
                                    {"search",&Search},
                                    {"searchfile",&SearchFile},
                                    {"depends",&Depends},
                                    {"whatdepends",&WhatDepends},
                                    {"rdepends",&RDepends},
                                    {"whatprovides",&WhatProvides},
                                    {"files",&FileList},
                                    {"changelog",&ChangeLog},
                                    {"dotty",&Dotty},
                                    {"xvcg",&XVcg},
                                    {"show",&ShowPackage},
                                    {"pkgnames",&ShowPkgNames},
                                    {"policy",&Policy},
// CNC:2003-11-23
#ifdef APT_WITH_LUA
				    {"script",&Script},
#endif
                                    {0,0}};

   CacheInitialize();

   // Set up gettext support
   setlocale(LC_ALL,"");
   textdomain(PACKAGE);

   // Parse the command line and initialize the package library
   CommandLine CmdL(Args,_config);
   if (pkgInitConfig(*_config) == false ||
       CmdL.Parse(argc,argv) == false ||
       pkgInitSystem(*_config,_system) == false)
   {
      _error->DumpErrors();
      return 100;
   }

   // See if the help should be shown
   if (_config->FindB("help") == true ||
       CmdL.FileSize() == 0)
   {
      ShowHelp(CmdL);
      return 0;
   }
   
   // Deal with stdout not being a tty
   if (ttyname(STDOUT_FILENO) == 0 && _config->FindI("quiet",0) < 1)
      _config->Set("quiet","1");

   // CNC:2004-02-18
   if (_system->LockRead() == false)
   {
      bool Errors = _error->PendingError();
      _error->DumpErrors();
      return Errors == true?100:0;
   }

   if (CmdL.DispatchArg(CmdsA,false) == false && _error->PendingError() == false)
   { 
      MMap *Map = 0;
      if (_config->FindB("APT::Cache::Generate",true) == false)
      {
	 Map = new MMap(*new FileFd(_config->FindFile("Dir::Cache::pkgcache"),
				    FileFd::ReadOnly),MMap::Public|MMap::ReadOnly);
      }
      else
      {
	 // Open the cache file
	 SrcList = new pkgSourceList;
	 SrcList->ReadMainList();

	 // Generate it and map it
	 OpProgress Prog;
	 pkgMakeStatusCache(*SrcList,Prog,&Map,true);
      }
      
      if (_error->PendingError() == false)
      {
	 pkgCache Cache(Map);   
	 GCache = &Cache;
// CNC:2003-11-23
#ifdef APT_WITH_LUA
	 _lua->SetCache(&Cache);
	 double Consume = 0;
	 if (argc > 1 && _error->PendingError() == false &&
	     _lua->HasScripts("Scripts::AptCache::Command") == true)
	 {
	    _lua->SetGlobal("command_args", CmdL.FileList);
	    _lua->SetGlobal("command_consume", 0.0);
	    _lua->RunScripts("Scripts::AptCache::Command");
	    Consume = _lua->GetGlobalNum("command_consume");
	    _lua->ResetGlobals();
	    _lua->ResetCaches();
	 }

	 if (Consume == 0)
#endif
	 if (_error->PendingError() == false)
	    CmdL.DispatchArg(CmdsB);
      }
      delete Map;
   }
   
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
