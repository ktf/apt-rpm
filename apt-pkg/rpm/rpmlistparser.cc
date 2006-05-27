// Description
// $Id: rpmlistparser.cc,v 1.7 2003/01/29 18:55:03 niemeyer Exp $
// 
/* ######################################################################
 * 
 * Package Cache Generator - Generator for the cache structure.
 * This builds the cache structure from the abstract package list parser. 
 * 
 ##################################################################### 
 */


// Include Files
#include <config.h>

#ifdef HAVE_RPM

#include <apt-pkg/rpmlistparser.h>
#include <apt-pkg/rpmhandler.h>
#include <apt-pkg/rpmpackagedata.h>
#include <apt-pkg/rpmsystem.h>
#include <apt-pkg/error.h>
#include <apt-pkg/configuration.h>
#include <apt-pkg/strutl.h>
#include <apt-pkg/crc-16.h>
#include <apt-pkg/tagfile.h>
#include <apt-pkg/error.h>

#include <apti18n.h>

#include <algorithm>

#define WITH_VERSION_CACHING 1

string MultilibArchs[] = {"x86_64", "ia64", "ppc64", "sparc64"};

// ListParser::rpmListParser - Constructor				/*{{{*/
// ---------------------------------------------------------------------
/* */
rpmListParser::rpmListParser(RPMHandler *Handler)
	: Handler(Handler), VI(0)
{
   Handler->Rewind();
   if (Handler->IsDatabase() == true)
   {
#ifdef WITH_HASH_MAP
      SeenPackages = new SeenPackagesType(517);
#else
      SeenPackages = new SeenPackagesType;
#endif
   }
   else
   {
      SeenPackages = NULL;
   }
   RpmData = RPMPackageData::Singleton();
}
                                                                        /*}}}*/

rpmListParser::~rpmListParser()
{
   delete SeenPackages;
}

                                                                        /*}}}*/
// ListParser::Package - Return the package name			/*{{{*/
// ---------------------------------------------------------------------
/* This is to return the name of the package this section describes */
string rpmListParser::Package()
{
   if (CurrentName.empty() == false)
      return CurrentName;

#ifdef WITH_VERSION_CACHING
   if (VI != NULL) {
      CurrentName = VI->ParentPkg().Name();
      return CurrentName;
   }
#endif

   string Name = Handler->Name();
   
   Duplicated = false;
   
   
   if (Name.empty() == true)
   {
      _error->Error(_("Corrupt pkglist: no RPMTAG_NAME in header entry"));
      return "";
   } 

   bool IsDup = false;

   if (RpmData->IsMultilibSys() && RpmData->IsCompatArch(Architecture())) {
	 Name += ".32bit";	 
	 CurrentName = Name;
   }

   
   // If this package can have multiple versions installed at
   // the same time, then we make it so that the name of the
   // package is NAME+"#"+VERSION and also add a provides
   // with the original name and version, to satisfy the 
   // dependencies.
   if (RpmData->IsDupPackage(Name) == true)
      IsDup = true;
   else if (SeenPackages != NULL) {
      if (SeenPackages->find(Name.c_str()) != SeenPackages->end())
      {
	 if (_config->FindB("RPM::Allow-Duplicated-Warning", true) == true)
	    _error->Warning(
   _("There are multiple versions of \"%s\" in your system.\n"
     "\n"
     "This package won't be cleanly updated, unless you leave\n"
     "only one version. To leave multiple versions installed,\n"
     "you may remove that warning by setting the following\n"
     "option in your configuration file:\n"
     "\n"
     "RPM::Allow-Duplicated { \"^%s$\"; };\n"
     "\n"
     "To disable these warnings completely set:\n"
     "\n"
     "RPM::Allow-Duplicated-Warning \"false\";\n")
			      , Name.c_str(), Name.c_str());
	 RpmData->SetDupPackage(Name);
	 VirtualizePackage(Name);
	 IsDup = true;
      }
   }
   if (IsDup == true)
   {
      Name += "#"+Version();
      Duplicated = true;
   } 
   CurrentName = Name;
   return Name;
}

                                                                        /*}}}*/
// ListParser::Arch - Return the architecture string			/*{{{*/
// ---------------------------------------------------------------------
string rpmListParser::Architecture()
{
#ifdef WITH_VERSION_CACHING
   if (VI != NULL)
      return VI->Arch();
#endif
   return Handler->Arch();
}
                                                                        /*}}}*/
// ListParser::Version - Return the version string			/*{{{*/
// ---------------------------------------------------------------------
/* This is to return the string describing the version in RPM form,
 version-release. If this returns the blank string then the
 entry is assumed to only describe package properties */
string rpmListParser::Version()
{
#ifdef WITH_VERSION_CACHING
   if (VI != NULL)
      return VI->VerStr();
#endif

   return Handler->EVR();
}
                                                                        /*}}}*/
// ListParser::NewVersion - Fill in the version structure		/*{{{*/
// ---------------------------------------------------------------------
/* */
bool rpmListParser::NewVersion(pkgCache::VerIterator Ver)
{
#if WITH_VERSION_CACHING
   // Cache it for future usage.
   RpmData->SetVersion(Handler->GetID(), Offset(), Ver);
#endif
   
   // Parse the section
   Ver->Section = WriteUniqString(Handler->Group());
   Ver->Arch = WriteUniqString(Handler->Arch());
   
   // Archive Size
   Ver->Size = Handler->FileSize();
   
   // Unpacked Size (in kbytes)
   Ver->InstalledSize = Handler->InstalledSize();
     
   if (ParseDepends(Ver,pkgCache::Dep::Depends) == false)
       return false;
   if (ParseDepends(Ver,pkgCache::Dep::Conflicts) == false)
       return false;
   if (ParseDepends(Ver,pkgCache::Dep::Obsoletes) == false)
       return false;

   if (ParseProvides(Ver) == false)
       return false;

   if (Handler->ProvideFileName() &&
       NewProvides(Ver, Handler->FileName(), "") == false)
	 return false;

   return true;
}
									/*}}}*/
// ListParser::UsePackage - Update a package structure			/*{{{*/
// ---------------------------------------------------------------------
/* This is called to update the package with any new information
   that might be found in the section */
bool rpmListParser::UsePackage(pkgCache::PkgIterator Pkg,
			       pkgCache::VerIterator Ver)
{
   if (SeenPackages != NULL)
      (*SeenPackages)[Pkg.Name()] = true;
   if (Pkg->Section == 0)
      Pkg->Section = WriteUniqString(Handler->Group());
   if (_error->PendingError()) 
       return false;
   string PkgName = Pkg.Name();
   string::size_type HashPos = PkgName.find('#');
   if (HashPos != string::npos)
      PkgName = PkgName.substr(0, HashPos);
   Ver->Priority = RpmData->VerPriority(PkgName);
   Pkg->Flags |= RpmData->PkgFlags(PkgName);
   if (HashPos != string::npos && (Pkg->Flags & pkgCache::Flag::Essential))
      Pkg->Flags = pkgCache::Flag::Important;
   if (ParseStatus(Pkg,Ver) == false)
       return false;
   return true;
}
                                                                        /*}}}*/
// ListParser::VersionHash - Compute a unique hash for this version	/*{{{*/
// ---------------------------------------------------------------------
/* */

static bool depsort(const Dependency *a, const Dependency *b)
{   
   return a->Name < b->Name;
}

static bool depuniq(const Dependency *a, const Dependency *b)
{   
   return a->Name == b->Name;
}
unsigned short rpmListParser::VersionHash()
{
#ifdef WITH_VERSION_CACHING
   if (VI != NULL)
      return (*VI)->Hash;
#endif

   unsigned long Result = INIT_FCS;
   Result = AddCRC16(Result, Package().c_str(), Package().length());
   Result = AddCRC16(Result, Version().c_str(), Version().length());
   Result = AddCRC16(Result, Architecture().c_str(), Architecture().length());

   int DepSections[] = { 
      pkgCache::Dep::Depends,
      pkgCache::Dep::Conflicts,
      pkgCache::Dep::Obsoletes,
   };
   
   for (size_t i = 0; i < sizeof(DepSections)/sizeof(int); i++) {
      vector<Dependency*> Deps;
      if (Handler->Depends(DepSections[i], Deps) == false) continue;

      sort(Deps.begin(), Deps.end(), depsort);
      // rpmdb can give out dupes for scriptlet dependencies, filter them out
      vector<Dependency*>::iterator DepEnd = unique(Deps.begin(), Deps.end(), depuniq);
      vector<Dependency*>::iterator I = Deps.begin();
      for (; I != DepEnd; I++) { 
	 Result = AddCRC16(Result, (*I)->Name.c_str(), (*I)->Name.length());
      }
   }
   return Result;
}
                                                                        /*}}}*/
// ListParser::ParseStatus - Parse the status field			/*{{{*/
// ---------------------------------------------------------------------
bool rpmListParser::ParseStatus(pkgCache::PkgIterator Pkg,
				pkgCache::VerIterator Ver)
{   
   if (!Handler->IsDatabase())  // this means we're parsing an hdlist, so it's not installed
      return true;
   
   // if we're reading from the rpmdb, then it's installed
   // 
   Pkg->SelectedState = pkgCache::State::Install;
   Pkg->InstState = pkgCache::State::Ok;
   Pkg->CurrentState = pkgCache::State::Installed;
   
   Pkg->CurrentVer = Ver.Index();
   
   return true;
}

                                                                        /*}}}*/
// ListParser::ParseDepends - Parse a dependency list			/*{{{*/
// ---------------------------------------------------------------------
/* This is the higher level depends parser. It takes a tag and generates
 a complete depends tree for the given version. */
bool rpmListParser::ParseDepends(pkgCache::VerIterator Ver,
				 unsigned int Type)
{
   vector<Dependency*> Deps;

   if (Handler->Depends(Type, Deps) == false)
      return false;
   
   for (vector<Dependency*>::iterator I = Deps.begin(); I != Deps.end(); I++) {
      if (NewDepends(Ver,(*I)->Name,(*I)->Version,
		    (*I)->Op,(*I)->Type) == false) {
	 return false;
      }
   }
   return true;

}
                                                                        /*}}}*/
bool rpmListParser::CollectFileProvides(pkgCache &Cache,
					pkgCache::VerIterator Ver)
{
   vector<string> Files;
   bool ret = true;

   if (Handler->FileProvides(Files) == false) {
      return false;
   }
   for (vector<string>::iterator I = Files.begin(); I != Files.end(); I++) {
      const char *FileName = (*I).c_str();
      pkgCache::Package *P = Cache.FindPackage(FileName);
      if (P != NULL) {
	 // Check if this is already provided.
	 bool Found = false;
	 for (pkgCache::PrvIterator Prv = Ver.ProvidesList();
	      Prv.end() == false; Prv++) {
	    if (strcmp(Prv.Name(), FileName) == 0) {
	       Found = true;
	       break;
	    }
	 }
	 if (Found == false && NewProvides(Ver, FileName, "") == false) {
	    ret = false;
	    break;
	 }
      }
   }
   return ret;
}

// ListParser::ParseProvides - Parse the provides list			/*{{{*/
// ---------------------------------------------------------------------
/* */
bool rpmListParser::ParseProvides(pkgCache::VerIterator Ver)
{
   vector<Dependency*> Provs;

   if (Handler->Provides(Provs) == false) {
      return false;
   }
   for (vector<Dependency*>::iterator I = Provs.begin(); I != Provs.end(); I++) {
      if (NewProvides(Ver,(*I)->Name,(*I)->Version) == false) {
	 return false;
      }
   }
   return true;

}
                                                                        /*}}}*/
// ListParser::Step - Move to the next section in the file		/*{{{*/
// ---------------------------------------------------------------------
/* This has to be carefull to only process the correct architecture */
bool rpmListParser::Step()
{
   while (Handler->Skip() == true)
   {
      CurrentName = "";

#ifdef WITH_VERSION_CACHING
      VI = RpmData->GetVersion(Handler->GetID(), Offset());
      if (VI != NULL)
	 return true;
#endif
      
      string RealName = Package();

      if (Duplicated == true)
	 RealName = RealName.substr(0,RealName.find('#'));
      if (RpmData->IgnorePackage(RealName) == true)
	 continue;
 
      if (Handler->IsDatabase() == true ||
	  RpmData->ArchScore(Architecture().c_str()) > 0)
	 return true;
   }
   return false;
}
                                                                        /*}}}*/
// ListParser::LoadReleaseInfo - Load the release information		/*{{{*/
// ---------------------------------------------------------------------
/* */
bool rpmListParser::LoadReleaseInfo(pkgCache::PkgFileIterator FileI,
				    FileFd &File)
{
   pkgTagFile Tags(&File);
   pkgTagSection Section;
   if (!Tags.Step(Section))
       return false;
   
   const char *Start;
   const char *Stop;
   if (Section.Find("Archive",Start,Stop))
       FileI->Archive = WriteUniqString(Start,Stop - Start);
   if (Section.Find("Component",Start,Stop))
       FileI->Component = WriteUniqString(Start,Stop - Start);
   if (Section.Find("Version",Start,Stop))
       FileI->Version = WriteUniqString(Start,Stop - Start);
   if (Section.Find("Origin",Start,Stop))
       FileI->Origin = WriteUniqString(Start,Stop - Start);
   if (Section.Find("Label",Start,Stop))
       FileI->Label = WriteUniqString(Start,Stop - Start);
   if (Section.Find("Architecture",Start,Stop))
       FileI->Architecture = WriteUniqString(Start,Stop - Start);
   
   if (Section.FindFlag("NotAutomatic",FileI->Flags,
			pkgCache::Flag::NotAutomatic) == false)
       _error->Warning(_("Bad NotAutomatic flag"));
   
   return !_error->PendingError();
}
                                                                        /*}}}*/

unsigned long rpmListParser::Size() 
{
   return (Handler->InstalledSize()+512)/1024;
}

// This is a slightly complex operation. It must take a package, and
// move every version to new packages, named accordingly to
// Allow-Duplicated rules.
void rpmListParser::VirtualizePackage(string Name)
{
   pkgCache::PkgIterator FromPkgI = Owner->GetCache().FindPkg(Name);

   // Should always be false
   if (FromPkgI.end() == true)
      return;

   pkgCache::VerIterator FromVerI = FromPkgI.VersionList();
   while (FromVerI.end() == false) {
      string MangledName = Name+"#"+string(FromVerI.VerStr());

      // Get the new package.
      pkgCache::PkgIterator ToPkgI = Owner->GetCache().FindPkg(MangledName);
      if (ToPkgI.end() == true) {
	 // Theoretically, all packages virtualized should pass here at least
	 // once for each new version in the list, since either the package was
	 // already setup as Allow-Duplicated (and this method would never be
	 // called), or the package doesn't exist before getting here. If
	 // we discover that this assumption is false, then we must do
	 // something to order the version list correctly, since the package
	 // could already have some other version there.
	 Owner->NewPackage(ToPkgI, MangledName);

	 // Should it get the flags from the original package? Probably not,
	 // or automatic Allow-Duplicated would work differently than
	 // hardcoded ones.
	 ToPkgI->Flags |= RpmData->PkgFlags(MangledName);
	 ToPkgI->Section = FromPkgI->Section;
      }
      
      // Move the version to the new package.
      FromVerI->ParentPkg = ToPkgI.Index();

      // Put it at the end of the version list (about ordering,
      // read the comment above).
      map_ptrloc *ToVerLast = &ToPkgI->VersionList;
      for (pkgCache::VerIterator ToVerLastI = ToPkgI.VersionList();
	   ToVerLastI.end() == false; ToVerLastI++)
	   ToVerLast = &ToVerLastI->NextVer;

      *ToVerLast = FromVerI.Index();

      // Provide the real package name with the current version.
      NewProvides(FromVerI, Name, FromVerI.VerStr());

      // Is this the current version of the old package? If yes, set it
      // as the current version of the new package as well.
      if (FromVerI == FromPkgI.CurrentVer()) {
	 ToPkgI->CurrentVer = FromVerI.Index();
	 ToPkgI->SelectedState = pkgCache::State::Install;
	 ToPkgI->InstState = pkgCache::State::Ok;
	 ToPkgI->CurrentState = pkgCache::State::Installed;
      }

      // Move the iterator before reseting the NextVer.
      pkgCache::Version *FromVer = (pkgCache::Version*)FromVerI;
      FromVerI++;
      FromVer->NextVer = 0;
   }

   // Reset original package data.
   FromPkgI->CurrentVer = 0;
   FromPkgI->VersionList = 0;
   FromPkgI->Section = 0;
   FromPkgI->SelectedState = 0;
   FromPkgI->InstState = 0;
   FromPkgI->CurrentState = 0;
}

xmlNode *rpmRepomdParser::FindNode(xmlNode *n, const string Name)
{
   for (n = n->children; n; n = n->next) {
      if (strcmp((char*)n->name, Name.c_str()) == 0) {
	 return n;
      }
   }
   return NULL;
}

bool rpmRepomdParser::LoadReleaseInfo(pkgCache::PkgFileIterator FileI,
                                      const string File, const string Dist)
{
   size_t start, stop, size;
   string comp;

   // Munge sources.list distribution into something that can be used 
   // as Component to allow repository pinning with repomd
   start = Dist.find_first_not_of("/");
   size = Dist.length();
   while ((start >= 0) && (start < size)) {
      stop = Dist.find_first_of("/", start);
      string part = Dist.substr(start, stop - start);
      if (comp.empty()) {
	 comp = part;
      } else {
	 comp += "-" + part;
      }
      if ((stop < 0) || (stop > size)) stop = size;
      start = Dist.find_first_not_of("/", stop + 1);
   }
   FileI->Component = WriteUniqString(comp);
   // Should these be populated with something (what?) a well?
   //FileI->Archive = WriteUniqString("Unknown");
   //FileI->Version = WriteUniqString("Unknown");
   //FileI->Origin = WriteUniqString("Unknown");
   //FileI->Label = WriteUniqString("Unknown");

   xmlDocPtr RepoMD = NULL;
   xmlNode *Root = NULL;

   RepoMD = xmlReadFile(File.c_str(), NULL, XML_PARSE_NONET);
   if ((Root = xmlDocGetRootElement(RepoMD)) == NULL) {
      xmlFreeDoc(RepoMD);
      return _error->Error(_("could not open Release file '%s'"),File.c_str());
   }

   /* Parse primary, filelists and other location from here */
   for (xmlNode *n = Root->children; n; n = n->next) {
      if (n->type == XML_ELEMENT_NODE && strcmp((char*)n->name, "data") == 0) {
        string type = (char*)xmlGetProp(n, (xmlChar*)"type");
        if (type == "primary") {
           xmlNode *loc = FindNode(n, "location");
           if (loc) {
              Primary = (char*)xmlGetProp(loc, (xmlChar*)"href");
           }
       } else if (type == "filelists") {
           xmlNode *loc = FindNode(n, "location");
           if (loc) {
              Filelist = (char*)xmlGetProp(loc, (xmlChar*)"href");
           }
        }
      }

   }

   xmlFreeDoc(RepoMD);
   return true;
}

#endif /* HAVE_RPM */

// vim:sts=3:sw=3
