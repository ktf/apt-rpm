// -*- mode: c++; mode: fold -*-
// Description								/*{{{*/
// $Id: pkgcachegen.h,v 1.3 2002/07/25 18:07:18 niemeyer Exp $
/* ######################################################################
   
   Package Cache Generator - Generator for the cache structure.
   
   This builds the cache structure from the abstract package list parser. 
   Each archive source has it's own list parser that is instantiated by
   the caller to provide data for the generator. 
   
   Parts of the cache are created by this generator class while other
   parts are created by the list parser. The list parser is responsible
   for creating version, depends and provides structures, and some of
   their contents
   
   ##################################################################### */
									/*}}}*/
#ifndef PKGLIB_PKGCACHEGEN_H
#define PKGLIB_PKGCACHEGEN_H

#include <apt-pkg/pkgcache.h>

class pkgSourceList;
class OpProgress;
class MMap;
class pkgIndexFile;

class pkgCacheGenerator
{
   private:
   
   pkgCache::StringItem *UniqHash[26];
   
   public:
   
   class ListParser;
   friend class ListParser;
   
   protected:
   
   DynamicMMap &Map;
   pkgCache Cache;
   OpProgress *Progress;
   
   string PkgFileName;
   pkgCache::PackageFile *CurrentFile;

   // Flag file dependencies
   bool FoundFileDeps;
   
   bool NewFileVer(pkgCache::VerIterator &Ver,ListParser &List);
   unsigned long NewVersion(pkgCache::VerIterator &Ver,string VerStr,unsigned long Next);

   public:

   // CNC:2003-02-27 - We need this in rpmListParser.
   bool NewPackage(pkgCache::PkgIterator &Pkg,string Name);

   unsigned long WriteUniqString(const char *S,unsigned int Size);
   inline unsigned long WriteUniqString(string S) {return WriteUniqString(S.c_str(),S.length());}

   void DropProgress() {Progress = 0;}
   bool SelectFile(string File,string Site,pkgIndexFile const &Index,
		   unsigned long Flags = 0);
   bool MergeList(ListParser &List,pkgCache::VerIterator *Ver = 0);
   inline pkgCache &GetCache() {return Cache;}
   inline pkgCache::PkgFileIterator GetCurFile() 
         {return pkgCache::PkgFileIterator(Cache,CurrentFile);}

   bool HasFileDeps() {return FoundFileDeps;}
   bool MergeFileProvides(ListParser &List);

   // CNC:2003-03-18
   inline void ResetFileDeps() {FoundFileDeps = false;}
      
   pkgCacheGenerator(DynamicMMap *Map,OpProgress *Progress);
   ~pkgCacheGenerator();
};

// This is the abstract package list parser class.
class pkgCacheGenerator::ListParser
{
   // Some cache items
   pkgCache::VerIterator OldDepVer;
   map_ptrloc *OldDepLast;

   // Flag file dependencies
   bool FoundFileDeps;
      
   protected:

   // CNC:2003-02-27 - We need Owner in rpmListParser.
   pkgCacheGenerator *Owner;
   friend class pkgCacheGenerator;

   inline unsigned long WriteUniqString(string S) {return Owner->WriteUniqString(S);}
   inline unsigned long WriteUniqString(const char *S,unsigned int Size) {return Owner->WriteUniqString(S,Size);}
   inline unsigned long WriteString(string S) {return Owner->Map.WriteString(S);}
   inline unsigned long WriteString(const char *S,unsigned int Size) {return Owner->Map.WriteString(S,Size);}
   bool NewDepends(pkgCache::VerIterator Ver,string Package,
		   string Version,unsigned int Op,
		   unsigned int Type);
   bool NewProvides(pkgCache::VerIterator Ver,string Package,string Version);
   
   public:
   
   // These all operate against the current section
   virtual string Package() = 0;
   virtual string Version() = 0;
   // CNC:2002-07-09
   virtual string Architecture() {return string();}
   virtual bool NewVersion(pkgCache::VerIterator Ver) = 0;
   virtual unsigned short VersionHash() = 0;
   virtual bool UsePackage(pkgCache::PkgIterator Pkg,
			   pkgCache::VerIterator Ver) = 0;
   virtual off_t Offset() = 0;
   virtual off_t Size() = 0;

   // CNC:2003-02-16 - If this is false, the Size of the pkgIndexFile must
   // 		       provide the number of elements, since a sequential
   // 		       counter will be used to verify progress.
   virtual bool OrderedOffset() {return true;}

   // CNC:2003-02-20 - This method will help on package ordering tasks,
   // 		       ensuring that if a package with the same version
   // 		       is installed, it won't be unexpectedly downloaded,
   // 		       even if with a "better" architecture or different
   // 		       dependencies.
   virtual bool IsDatabase() {return false;}
   
   virtual bool Step() = 0;
   
   inline bool HasFileDeps() {return FoundFileDeps;}
   virtual bool CollectFileProvides(pkgCache &Cache,
				    pkgCache::VerIterator Ver) {return true;}

   ListParser() : FoundFileDeps(false) {}
   virtual ~ListParser() {}
};

bool pkgMakeStatusCache(pkgSourceList &List,OpProgress &Progress,
			MMap **OutMap = 0,bool AllowMem = false);
bool pkgMakeOnlyStatusCache(OpProgress &Progress,DynamicMMap **OutMap);

#ifdef APT_COMPATIBILITY
#if APT_COMPATIBILITY != 986
#warning "Using APT_COMPATIBILITY"
#endif
MMap *pkgMakeStatusCacheMem(pkgSourceList &List,OpProgress &Progress)
{
   MMap *Map = 0;
   if (pkgMakeStatusCache(List,Progress,&Map,true) == false)
      return 0;
   return Map;
}
#endif

#endif
