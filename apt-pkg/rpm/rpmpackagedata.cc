
#include <config.h>

#ifdef HAVE_RPM

#include <apt-pkg/error.h>
#include <apt-pkg/rpmpackagedata.h>
#include <apt-pkg/fileutl.h>
#include <apt-pkg/strutl.h>
#include <apt-pkg/configuration.h>
#include <apt-pkg/version.h>

#include <apti18n.h>

#include <rpm/rpmlib.h>

RPMPackageData::RPMPackageData()
   : MinArchScore(-1)
#ifdef WITH_HASH_MAP
   , ArchScores(31), VerMap(517)
#endif
{
   // Populate priorities
   string FileName = _config->FindFile("Dir::Etc::rpmpriorities");
   FileFd F(FileName, FileFd::ReadOnly);
   if (_error->PendingError()) 
   {
      _error->Error(_("could not open package priority file %s"),
		    FileName.c_str());
      return;
   }
   pkgTagFile Tags(&F);
   pkgTagSection Section;

   if (!Tags.Step(Section)) 
   {
      _error->Error(_("no data in %s"), FileName.c_str());
       return;
   }
   
   for (int i = 0; i != 6; i++) 
   {
      const char *priorities[] = 
      {
	 "Essential", "Important", "Required", "Standard", "Optional", "Extra"
      };
      pkgCache::State::VerPriority states[] = {
	     pkgCache::State::Important,
	     pkgCache::State::Important,
	     pkgCache::State::Required,
	     pkgCache::State::Standard,
	     pkgCache::State::Optional,
	     pkgCache::State::Extra
      };
      pkgCache::Flag::PkgFlags flags[] = {
	     pkgCache::Flag::Essential,
	     pkgCache::Flag::Important,
	     pkgCache::Flag::Important,
	     (pkgCache::Flag::PkgFlags)0,
	     (pkgCache::Flag::PkgFlags)0,
	     (pkgCache::Flag::PkgFlags)0
      };

      string Packages = Section.FindS(priorities[i]);
      if (Packages.empty()) 
	 continue;

      const char *C = Packages.c_str();
      while (*C != 0)
      {
	 string pkg;
	 if (ParseQuoteWord(C,pkg))
	 {
	    Priorities[pkg] = states[i];
	    Flags[pkg] = flags[i];
	 }
      }
   }

   // Populate holding packages
   const Configuration::Item *Top = _config->Tree("RPM::Hold");
   for (Top = (Top == 0?0:Top->Child); Top != 0; Top = Top->Next)
   {
      regex_t *ptrn = new regex_t;
      if (regcomp(ptrn,Top->Value.c_str(),REG_EXTENDED|REG_ICASE|REG_NOSUB) != 0)
      {
	 _error->Warning(_("Bad regular expression '%s' in option RPM::Hold."),
			 Top->Value.c_str());
	 delete ptrn;
      }
      else
	  HoldPackages.push_back(ptrn);
   }

   // Populate ignored packages
   Top = _config->Tree("RPM::Ignore");
   for (Top = (Top == 0?0:Top->Child); Top != 0; Top = Top->Next)
      IgnorePackages[Top->Value] = 1;

   // Populate Allow-Duplicated packages.
   Top = _config->Tree("RPM::Allow-Duplicated");
   for (Top = (Top == 0?0:Top->Child); Top != 0; Top = Top->Next)
   {
      regex_t *ptrn = new regex_t;
      if (regcomp(ptrn,Top->Value.c_str(),REG_EXTENDED|REG_ICASE|REG_NOSUB) != 0)
      {
	 _error->Warning(_("Bad regular expression '%s' in option RPM::Allow-Duplicated."),
			 Top->Value.c_str());
	 delete ptrn;
      }
      else
	  DuplicatedPatterns.push_back(ptrn);
   }

   // Populate fake provides
   Top = _config->Tree("RPM::Fake-Provides");
   for (Top = (Top == 0?0:Top->Child); Top != 0; Top = Top->Next)
   {
      string Name = "";
      string Version = "";
      const char *C = Top->Value.c_str();
      if (ParseQuoteWord(C,Name) == false)
      {
	 _error->Warning(_("Bad entry '%s' in option RPM::FakeProvides."),
			 Top->Value.c_str());
	 continue;
      }
      ParseQuoteWord(C,Version);

      if (Version.empty() == true)
      {
	 delete FakeProvides[Name];
	 FakeProvides[Name] = NULL;
      }
      else
      {
	 if (FakeProvides.find(Name) != FakeProvides.end())
	 {
	    // If it's NULL, it was provided with an empty version
	    if (FakeProvides[Name] != NULL)
	       FakeProvides[Name]->push_back(Version);
	 }
	 else
	 {
	    vector<string> *VerList = new vector<string>;
	    VerList->push_back(Version);
	    FakeProvides[Name] = VerList;
	 }
      }
   }


   // Populate translations
   Configuration Cnf;
   string CnfFile = _config->FindDir("Dir::Etc::translateparts");
   if (FileExists(CnfFile) == true)
      if (ReadConfigDir(Cnf,CnfFile,true) == false)
	 return;
   CnfFile = _config->FindFile("Dir::Etc::translatelist");
   if (FileExists(CnfFile) == true)
      if (ReadConfigFile(Cnf,CnfFile,true) == false)
	 return;

   const char *TString[] = {"translate-binary",
			    "translate-source",
			    "translate-index"};
   vector<Translate*> *TList[] = {&BinaryTranslations,
				  &SourceTranslations,
				  &IndexTranslations};
   for (int i = 0; i != 3; i++)
   {
      Top = Cnf.Tree(TString[i]);
      for (Top = (Top == 0?0:Top->Child); Top != 0; Top = Top->Next)
      {
	 Translate *t = new Translate();
	 if (regcomp(&t->Pattern,Top->Tag.c_str(),REG_EXTENDED|REG_ICASE|REG_NOSUB) != 0)
	 {
	    _error->Warning(_("Bad regular expression '%s' in URI translation"),
			    Top->Tag.c_str());
	    delete t;
	 }
	 else
	 {
	    Configuration Block(Top);
	    t->Template = Block.Find("Template");
	    TList[i]->push_back(t);
	 }
      }
   }
}

bool RPMPackageData::HoldPackage(const char *name)
{
   for (vector<regex_t*>::iterator I = HoldPackages.begin();
	I != HoldPackages.end(); I++)
      if (regexec(*I,name,0,0,0) == 0)
	 return true;
   return false;
}

bool RPMPackageData::IgnoreDep(pkgVersioningSystem &VS,
			       pkgCache::DepIterator &Dep)
{
   const char *name = Dep.TargetPkg().Name();
   if (FakeProvides.find(name) != FakeProvides.end()) {
      vector<string> *VerList = FakeProvides[name];
      if (VerList == NULL)
	 return true;
      for (vector<string>::iterator I = VerList->begin();
	   I != VerList->end(); I++)
      {
	 if (VS.CheckDep(I->c_str(),Dep->CompareOp,Dep.TargetVer()) == true)
	    return true;
      }
   }
   return false;
}

static void ParseTemplate(string &Template, map<string,string> &Dict)
{
   string::size_type start(string::npos);
   string::size_type end(string::npos);
   string::size_type last_start;
   while (true)
   {
      last_start = start;
      start = Template.rfind("%(", end);
      if (start == string::npos)
	 break;
      end = Template.find(")", start);
      if (end < last_start)
      {
	 string Key(Template,start+2,end-(start+2));
	 map<string,string>::const_iterator I = Dict.find(Key);
	 if (I != Dict.end())
	    Template.replace(start, end-start+1, I->second);
      }
      if (start == 0)
	 break;
      end = start-1;
   }
}

void RPMPackageData::GenericTranslate(vector<Translate*> &TList,
				      string &FullURI,
				      map<string,string> &Dict)
{
   const char *fulluri = FullURI.c_str();
   for (vector<Translate*>::iterator I = TList.begin(); I != TList.end(); I++)
   {
      if (regexec(&(*I)->Pattern,fulluri,0,0,0) == 0)
      {
	 FullURI = (*I)->Template;
	 ParseTemplate(FullURI, Dict);
	 break;
      }
   }
}

void RPMPackageData::InitMinArchScore()
{
   if (MinArchScore != -1)
      return;
   string Arch = _config->Find("RPM::Architecture", "");
   if (Arch.empty() == false)
      MinArchScore = rpmMachineScore(RPM_MACHTABLE_INSTARCH, Arch.c_str());
   else
      MinArchScore = 0;
}

int RPMPackageData::RpmArchScore(const char *Arch)
{
   int Score = rpmMachineScore(RPM_MACHTABLE_INSTARCH, Arch);
   if (Score >= MinArchScore)
      return Score;
   return 0;
}

bool RPMPackageData::IsDupPackage(const string &Name)
{
   if (DuplicatedPackages.find(Name) != DuplicatedPackages.end())
      return true;
   const char *name = Name.c_str();
   for (vector<regex_t*>::iterator I = DuplicatedPatterns.begin();
	I != DuplicatedPatterns.end(); I++) {
      if (regexec(*I,name,0,0,0) == 0) {
	 SetDupPackage(Name);
	 return true;
      }
   }
   return false;
}

RPMPackageData *RPMPackageData::Singleton()
{
   static RPMPackageData *data = NULL;
   if (!data)
      data = new RPMPackageData();
   return data;
}

#endif /* HAVE_RPM */
// vim:sw=3:sts=3
