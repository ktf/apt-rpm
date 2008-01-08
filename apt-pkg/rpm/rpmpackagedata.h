#ifndef RPMPACKAGEDATA_H
#define RPMPACKAGEDATA_H

#include <apt-pkg/aptconf.h>
#include <apt-pkg/tagfile.h>
#include <apt-pkg/pkgcache.h>
#include "rpmmisc.h"

#include <map>
#include <vector>
#include <regex.h>

using namespace std;

struct LessPred
{
   bool operator()(const char* s1, const char* s2) const
     { return strcmp(s1, s2) < 0; }
};

class RPMPackageData 
{
   protected:

#ifdef APT_WITH_GNU_HASH_MAP
   hash_map<string,pkgCache::State::VerPriority,hash_string> Priorities;
   hash_map<string,pkgCache::Flag::PkgFlags,hash_string> Flags;
   hash_map<string,vector<string>*,hash_string> FakeProvides;
   hash_map<string,int,hash_string> IgnorePackages;
   hash_map<string,int,hash_string> DuplicatedPackages;
   hash_map<string,bool,hash_string> CompatArch;
   typedef map<string,pkgCache::VerIterator> VerMapValueType;
   typedef hash_map<unsigned long,VerMapValueType> VerMapType;
   typedef hash_map<const char*,int,
		    hash<const char*>,cstr_eq_pred> ArchScoresType;
#else
   map<string,pkgCache::State::VerPriority> Priorities;
   map<string,pkgCache::Flag::PkgFlags> Flags;
   map<string,vector<string>*> FakeProvides;
   map<string,int> IgnorePackages;
   map<string,int> DuplicatedPackages;
   map<string,bool> CompatArch;
   typedef map<string,pkgCache::VerIterator> VerMapValueType;
   typedef map<unsigned long,VerMapValueType> VerMapType;
   typedef map<const char*,int,cstr_lt_pred> ArchScoresType;
#endif

   vector<regex_t*> HoldPackages;   
   vector<regex_t*> DuplicatedPatterns;

   struct Translate {
	   regex_t Pattern;
	   string Template;
   };
   
   vector<Translate*> BinaryTranslations;
   vector<Translate*> SourceTranslations;
   vector<Translate*> IndexTranslations;

   VerMapType VerMap;

   void GenericTranslate(vector<Translate*> &TList, string &FullURI,
		   	 map<string,string> &Dict);

   int MinArchScore;

   ArchScoresType ArchScores;
   int RpmArchScore(const char *Arch);

   string BaseArch;
   string PreferredArch;
   string CompatArchSuffix;
   bool MultilibSys;

   public:

   inline pkgCache::State::VerPriority VerPriority(const string &Package) 
   {
      if (Priorities.find(Package) != Priorities.end())
	 return Priorities[Package];
      return pkgCache::State::Standard;
   };
   inline pkgCache::Flag::PkgFlags PkgFlags(const string &Package) 
   	{return Flags[Package];};

   bool HoldPackage(const char *name);
   bool IgnorePackage(const string &Name)
   	{return IgnorePackages.find(Name) != IgnorePackages.end();};

   bool IgnoreDep(pkgVersioningSystem &VS,pkgCache::DepIterator &Dep);

   void TranslateBinary(string &FullURI, map<string,string> &Dict)
   	{return GenericTranslate(BinaryTranslations, FullURI, Dict);};
   void TranslateSource(string &FullURI, map<string,string> &Dict)
   	{return GenericTranslate(SourceTranslations, FullURI, Dict);};
   void TranslateIndex(string &FullURI, map<string,string> &Dict)
   	{return GenericTranslate(IndexTranslations, FullURI, Dict);};

   bool HasBinaryTranslation()
	{return !BinaryTranslations.empty();};
   bool HasSourceTranslation()
	{return !SourceTranslations.empty();};
   bool HasIndexTranslation()
	{return !IndexTranslations.empty();};

   int ArchScore(const char *Arch)
   {
      ArchScoresType::const_iterator I = ArchScores.find(Arch);
      if (I != ArchScores.end())
	 return I->second;
      int Ret = RpmArchScore(Arch);
      // Must iterate and free when deallocating.
      ArchScores[strdup(Arch)] = Ret;
      return Ret;
   }
   void InitMinArchScore();

   bool IsCompatArch(string Arch);
   bool IsMultilibSys() { return MultilibSys; };
   string GetCompatArchSuffix() { return CompatArchSuffix; };

   void SetDupPackage(const string &Name)
   	{DuplicatedPackages[Name] = 1;};
   bool IsDupPackage(const string &Name);

   static RPMPackageData *Singleton();

   void SetVersion(string ID, unsigned long Offset,
		   pkgCache::VerIterator &Version)
   {
      VerMap[Offset][ID] = Version;
   };
   const pkgCache::VerIterator *GetVersion(string ID, unsigned long Offset)
   {
       VerMapType::const_iterator I1 = VerMap.find(Offset);
       if (I1 != VerMap.end()) {
	       VerMapValueType::const_iterator I2 =
		       I1->second.find(ID);
	       if (I2 != I1->second.end())
		       return &I2->second;
       }
       return NULL;
   };

   void CacheBuilt() {VerMap.clear();};

   RPMPackageData();
};


#endif

// vim:sts=3:sw=3
