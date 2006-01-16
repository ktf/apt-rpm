// -*- mode: cpp; mode: fold -*-
// Description								/*{{{*/
// $Id: pkgcache.h,v 1.2 2003/01/29 13:04:48 niemeyer Exp $
/* ######################################################################
   
   Cache - Structure definitions for the cache file
   
   Please see doc/apt-pkg/cache.sgml for a more detailed description of 
   this format. Also be sure to keep that file up-to-date!!
   
   Clients should always use the CacheIterators classes for access to the
   cache. They provide a simple STL-like method for traversing the links
   of the datastructure.
   
   See pkgcachegen.h for information about generating cache structures.
   
   ##################################################################### */
									/*}}}*/
#ifndef PKGLIB_PKGCACHE_H
#define PKGLIB_PKGCACHE_H

#ifdef __GNUG__
#pragma interface "apt-pkg/pkgcache.h"
#endif 

#include <string>
#include <time.h>
#include <apt-pkg/mmap.h>

using std::string;
    
class pkgVersioningSystem;
class pkgCache
{
   public:
   // Cache element predeclarations
   struct Header;
   struct Package;
   struct PackageFile;
   struct Version;
   struct Provides;
   struct Dependency;
   struct StringItem;
   struct VerFile;
   
   // Iterators
   class PkgIterator;
   class VerIterator;
   class DepIterator;
   class PrvIterator;
   class PkgFileIterator;
   class VerFileIterator;
   friend class PkgIterator;
   friend class VerIterator;
   friend class DepIterator;
   friend class PrvIterator;
   friend class PkgFileIterator;
   friend class VerFileIterator;
   
   class Namespace;
   
   // These are all the constants used in the cache structures
   struct Dep
   {
      enum DepType {Depends=1,PreDepends=2,Suggests=3,Recommends=4,
	 Conflicts=5,Replaces=6,Obsoletes=7};
      enum DepCompareOp {Or=0x10,NoOp=0,LessEq=0x1,GreaterEq=0x2,Less=0x3,
	 Greater=0x4,Equals=0x5,NotEquals=0x6};
   };
   
   struct State
   {
      enum VerPriority {Important=1,Required=2,Standard=3,Optional=4,Extra=5};
      enum PkgSelectedState {Unknown=0,Install=1,Hold=2,DeInstall=3,Purge=4};
      enum PkgInstState {Ok=0,ReInstReq=1,HoldInst=2,HoldReInstReq=3};
      enum PkgCurrentState {NotInstalled=0,UnPacked=1,HalfConfigured=2,
	   HalfInstalled=4,ConfigFiles=5,Installed=6};
   };
   
   struct Flag
   {
      enum PkgFlags {Auto=(1<<0),Essential=(1<<3),Important=(1<<4)};
      enum PkgFFlags {NotSource=(1<<0),NotAutomatic=(1<<1)};
   };

   /* Unnested structures for SWIG. Don't use them for APT internal
    * purposes as this will be dropped as soon as SWIG starts
    * supporting nested structures. Use definitions above instead. */
   enum _DepType {DepDepends=1,DepPreDepends=2,DepSuggests=3,DepRecommends=4,
      DepConflicts=5,DepReplaces=6,DepObsoletes=7};
   enum _DepCompareOp {DepOr=0x10,DepNoOp=0,DepLessEq=0x1,DepGreaterEq=0x2,
      DepLess=0x3,DepGreater=0x4,DepEquals=0x5,DepNotEquals=0x6};
   enum _VerPriority {StateImportant=1,StateRequired=2,StateStandard=3,
      StateOptional=4,StateExtra=5};
   enum _PkgSelectedState {StateUnknown=0,StateInstall=1,StateHold=2,
      StateDeInstall=3,StatePurge=4};
   enum _PkgInstState {StateOk=0,StateReInstReq=1,StateHoldInst=2,
      StateHoldReInstReq=3};
   enum _PkgCurrentState {StateNotInstalled=0,StateUnPacked=1,
      StateHalfConfigured=2,StateHalfInstalled=4,StateConfigFiles=5,
      StateInstalled=6};
   enum _PkgFlags {FlagAuto=(1<<0),FlagEssential=(1<<3),FlagImportant=(1<<4)};
   enum _PkgFFlags {FlagNotSource=(1<<0),FlagNotAutomatic=(1<<1)};
   
   protected:
   
   // Memory mapped cache file
   string CacheFile;
   MMap &Map;

   // CNC:2003-02-16 - Inlined here.
   inline unsigned long sHash(const char *S) const;
   inline unsigned long sHash(string S) const {return sHash(S.c_str());};
   
   public:
   
   // Pointers to the arrays of items
   Header *HeaderP;
   Package *PkgP;
   VerFile *VerFileP;
   PackageFile *PkgFileP;
   Version *VerP;
   Provides *ProvideP;
   Dependency *DepP;
   StringItem *StringItemP;
   char *StrP;

   virtual bool ReMap();
   inline bool Sync() {return Map.Sync();};
   inline MMap &GetMap() {return Map;};
   inline void *DataEnd() {return ((unsigned char *)Map.Data()) + Map.Size();};
      
   // String hashing function (512 range)
   inline unsigned long Hash(string S) const {return sHash(S);};
   inline unsigned long Hash(const char *S) const {return sHash(S);};

   // Usefull transformation things
   const char *Priority(unsigned char Priority);
   
   // Accessors
   PkgIterator FindPkg(string Name);
   // CNC:2003-02-17 - A slightly changed FindPkg(), hacked for performance.
   Package *FindPackage(const char *Name);
   Header &Head() {return *HeaderP;};
   inline PkgIterator PkgBegin();
   inline PkgIterator PkgEnd();
   inline PkgFileIterator FileBegin();
   inline PkgFileIterator FileEnd();

   // Make me a function
   pkgVersioningSystem *VS;
   
   // Converters
   static const char *CompTypeDeb(unsigned char Comp);
   static const char *CompType(unsigned char Comp);
   static const char *DepType(unsigned char Dep);
   
   pkgCache(MMap *Map,bool DoMap = true);
   virtual ~pkgCache() {};
};

// Header structure
struct pkgCache::Header
{
   // Signature information
   unsigned long Signature;
   short MajorVersion;
   short MinorVersion;
   bool Dirty;

   // CNC:2003-03-18
   bool HasFileDeps;

   // CNC:2003-11-24
   unsigned long OptionsHash;
   
   // Size of structure values
   unsigned short HeaderSz;
   unsigned short PackageSz;
   unsigned short PackageFileSz;
   unsigned short VersionSz;
   unsigned short DependencySz;
   unsigned short ProvidesSz;
   unsigned short VerFileSz;
   
   // Structure counts
   unsigned long PackageCount;
   unsigned long VersionCount;
   unsigned long DependsCount;
   unsigned long PackageFileCount;
   unsigned long VerFileCount;
   unsigned long ProvidesCount;
   
   // Offsets
   map_ptrloc FileList;              // struct PackageFile
   map_ptrloc StringList;            // struct StringItem
   map_ptrloc VerSysName;            // StringTable
   map_ptrloc Architecture;          // StringTable
   unsigned long MaxVerFileSize;

   /* Allocation pools, there should be one of these for each structure
      excluding the header */
   DynamicMMap::Pool Pools[7];
   
   // Rapid package name lookup
   map_ptrloc HashTable[8*1048];

   bool CheckSizes(Header &Against) const;
   Header();
};

struct pkgCache::Package
{
   // Pointers
   map_ptrloc Name;              // Stringtable
   map_ptrloc VersionList;       // Version
   map_ptrloc CurrentVer;        // Version
   map_ptrloc Section;           // StringTable (StringItem)
      
   // Linked list 
   map_ptrloc NextPackage;       // Package
   map_ptrloc RevDepends;        // Dependency
   map_ptrloc ProvidesList;      // Provides
   
   // Install/Remove/Purge etc
   unsigned char SelectedState;     // What
   unsigned char InstState;         // Flags
   unsigned char CurrentState;      // State
   
   unsigned short ID;
   unsigned long Flags;
};

struct pkgCache::PackageFile
{
   // Names
   map_ptrloc FileName;        // Stringtable
   map_ptrloc Archive;         // Stringtable
   map_ptrloc Component;       // Stringtable
   map_ptrloc Version;         // Stringtable
   map_ptrloc Origin;          // Stringtable
   map_ptrloc Label;           // Stringtable
   map_ptrloc Architecture;    // Stringtable
   map_ptrloc Site;            // Stringtable
   map_ptrloc IndexType;       // Stringtable
   unsigned long Size;            
   unsigned long Flags;
   
   // Linked list
   map_ptrloc NextFile;        // PackageFile
   unsigned short ID;
   time_t mtime;                  // Modification time for the file
};

struct pkgCache::VerFile
{
   map_ptrloc File;           // PackageFile
   map_ptrloc NextFile;       // PkgVerFile
   map_ptrloc Offset;         // File offset
   unsigned short Size;
};

struct pkgCache::Version
{
   map_ptrloc VerStr;            // Stringtable
   map_ptrloc Section;           // StringTable (StringItem)
   map_ptrloc Arch;              // StringTable
      
   // Lists
   map_ptrloc FileList;          // VerFile
   map_ptrloc NextVer;           // Version
   map_ptrloc DependsList;       // Dependency
   map_ptrloc ParentPkg;         // Package
   map_ptrloc ProvidesList;      // Provides
   
   map_ptrloc Size;              // These are the .deb size
   map_ptrloc InstalledSize;
   unsigned short Hash;
   unsigned short ID;
   unsigned char Priority;
};

struct pkgCache::Dependency
{
   map_ptrloc Version;         // Stringtable
   map_ptrloc Package;         // Package
   map_ptrloc NextDepends;     // Dependency
   map_ptrloc NextRevDepends;  // Dependency
   map_ptrloc ParentVer;       // Version
   
   // Specific types of depends
   map_ptrloc ID;   
   unsigned char Type;
   unsigned char CompareOp;
};

struct pkgCache::Provides
{
   map_ptrloc ParentPkg;        // Pacakge
   map_ptrloc Version;          // Version
   map_ptrloc ProvideVersion;   // Stringtable
   map_ptrloc NextProvides;     // Provides
   map_ptrloc NextPkgProv;      // Provides
};

struct pkgCache::StringItem
{
   map_ptrloc String;        // Stringtable
   map_ptrloc NextItem;      // StringItem
};

#include <apt-pkg/cacheiterators.h>

// CNC:2003-02-16 - Inlined here.
#include <ctype.h>
#define hash_count(a) (sizeof(a)/sizeof(a[0]))
inline unsigned long pkgCache::sHash(const char *Str) const
{
   unsigned long Hash = 0;
   for (const char *I = Str; *I != 0; I++)
      //Hash = 5*Hash + tolower(*I);
      Hash = 5*Hash + *I;
   return Hash % hash_count(HeaderP->HashTable);
}
#undef hash_count

inline pkgCache::PkgIterator pkgCache::PkgBegin() 
       {return PkgIterator(*this);};
inline pkgCache::PkgIterator pkgCache::PkgEnd() 
       {return PkgIterator(*this,PkgP);};
inline pkgCache::PkgFileIterator pkgCache::FileBegin()
       {return PkgFileIterator(*this,PkgFileP + HeaderP->FileList);};
inline pkgCache::PkgFileIterator pkgCache::FileEnd()
       {return PkgFileIterator(*this,PkgFileP);};

// Oh I wish for Real Name Space Support
class pkgCache::Namespace
{   
   public:

   typedef pkgCache::PkgIterator PkgIterator;
   typedef pkgCache::VerIterator VerIterator;
   typedef pkgCache::DepIterator DepIterator;
   typedef pkgCache::PrvIterator PrvIterator;
   typedef pkgCache::PkgFileIterator PkgFileIterator;
   typedef pkgCache::VerFileIterator VerFileIterator;   
   typedef pkgCache::Version Version;
   typedef pkgCache::Package Package;
   typedef pkgCache::Header Header;
   typedef pkgCache::Dep Dep;
   typedef pkgCache::Flag Flag;
};

#endif

// vim:sts=3:sw=3
