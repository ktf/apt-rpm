// -*- mode: c++; mode: fold -*-
// Description								/*{{{*/
// $Id $
/* ######################################################################

   RPM Index Files
   
   There are three sorts currently
   
   pkglist files
   The system RPM database
   srclist files
   
   ##################################################################### 
 */
									/*}}}*/
#ifndef PKGLIB_RPMINDEXFILE_H
#define PKGLIB_RPMINDEXFILE_H

#ifdef __GNUG__
#pragma interface "apt-pkg/rpmindexfile.h"
#endif

#include <apt-pkg/aptconf.h>
#include <apt-pkg/indexfile.h>
#include <apt-pkg/rpmhandler.h>

class RPMHandler;
class RPMDBHandler;
class pkgRepository;

class rpmIndexFile : public pkgIndexFile
{
   
   public:

   virtual RPMHandler *CreateHandler() const = 0;
   virtual bool HasPackages() const {return false;};

};

class rpmDatabaseIndex : public rpmIndexFile
{
   public:

   virtual const Type *GetType() const;

   // Creates a RPMHandler suitable for usage with this object
   virtual RPMHandler *CreateHandler() const;
   
   // Interface for acquire
   virtual string Describe(bool Short) const {return "RPM Database";};
   
   // Interface for the Cache Generator
   virtual bool Exists() const {return true;};
   virtual unsigned long Size() const;
   virtual bool HasPackages() const {return true;};
   virtual bool Merge(pkgCacheGenerator &Gen,OpProgress &Prog) const;
   virtual bool MergeFileProvides(pkgCacheGenerator &/*Gen*/,
		   		  OpProgress &/*Prog*/) const;
   virtual pkgCache::PkgFileIterator FindInCache(pkgCache &Cache) const;

   rpmDatabaseIndex();
};

class rpmListIndex : public rpmIndexFile
{

   protected:

   string URI;
   string Dist;
   string Section;
   pkgRepository *Repository;
   
   string ReleaseFile(string Type) const;
   string ReleaseURI(string Type) const;   
   string ReleaseInfo(string Type) const;   

   string Info(string Type) const;
   string IndexFile(string Type) const;
   string IndexURI(string Type) const;   

   virtual string MainType() const = 0;
   virtual string IndexPath() const {return IndexFile(MainType());};
   virtual string ReleasePath() const {return IndexFile("release");};

   public:

   virtual bool GetReleases(pkgAcquire *Owner) const;

   // Interface for the Cache Generator
   virtual bool Exists() const;
   virtual unsigned long Size() const;

   // Interface for acquire
   virtual string Describe(bool Short) const;   

   rpmListIndex(string URI,string Dist,string Section,
		pkgRepository *Repository) :
               	URI(URI), Dist(Dist), Section(Section),
   		Repository(Repository)
	{};
};

class rpmPkgListIndex : public rpmListIndex
{
   protected:

   virtual string MainType() const {return "pkglist";}
   virtual string IndexPath() const {return IndexFile(MainType());};

   public:

   virtual const Type *GetType() const;
   
   // Creates a RPMHandler suitable for usage with this object
   virtual RPMHandler *CreateHandler() const
	   { return new RPMFileHandler(IndexPath()); };

   // Stuff for accessing files on remote items
   virtual string ArchiveInfo(pkgCache::VerIterator Ver) const;
   virtual string ArchiveURI(string File) const;
   
   // Interface for acquire
   virtual bool GetIndexes(pkgAcquire *Owner) const;
   
   // Interface for the Cache Generator
   virtual bool HasPackages() const {return true;};
   virtual bool Merge(pkgCacheGenerator &Gen,OpProgress &Prog) const;
   virtual bool MergeFileProvides(pkgCacheGenerator &/*Gen*/,
		   		  OpProgress &/*Prog*/) const;
   virtual pkgCache::PkgFileIterator FindInCache(pkgCache &Cache) const;

   rpmPkgListIndex(string URI,string Dist,string Section,
		   pkgRepository *Repository) :
	   rpmListIndex(URI,Dist,Section,Repository)
      {};
};


class rpmSrcListIndex : public rpmListIndex
{
   protected:

   virtual string MainType() const {return "srclist";}

   public:

   virtual const Type *GetType() const;

   // Creates a RPMHandler suitable for usage with this object
   virtual RPMHandler *CreateHandler() const
	   { return new RPMFileHandler(IndexPath()); };

   // Stuff for accessing files on remote items
   virtual string SourceInfo(pkgSrcRecords::Parser const &Record,
			     pkgSrcRecords::File const &File) const;
   virtual string ArchiveURI(string File) const;
   
   // Interface for acquire
   virtual bool GetIndexes(pkgAcquire *Owner) const;

   // Interface for the record parsers
   virtual pkgSrcRecords::Parser *CreateSrcParser() const;
   

   rpmSrcListIndex(string URI,string Dist,string Section,
		   pkgRepository *Repository) :
	   rpmListIndex(URI,Dist,Section,Repository)
      {};
};

class rpmPkgDirIndex : public rpmPkgListIndex
{
   protected:

   virtual string MainType() const {return "pkgdir";}
   virtual string IndexPath() const;   
   virtual string ReleasePath() const;

   public:

   virtual bool GetReleases(pkgAcquire *Owner) const { return true; }
   virtual bool GetIndexes(pkgAcquire *Owner) const { return true; }

   // Creates a RPMHandler suitable for usage with this object
   virtual RPMHandler *CreateHandler() const
	   { return new RPMDirHandler(IndexPath()); };

   virtual const Type *GetType() const;
   
   // Interface for the Cache Generator
   virtual unsigned long Size() const;

   rpmPkgDirIndex(string URI,string Dist,string Section,
		   pkgRepository *Repository) :
	   rpmPkgListIndex(URI,Dist,Section,Repository)
      {};
};

class rpmSrcDirIndex : public rpmSrcListIndex
{
   protected:

   virtual string MainType() const {return "srcdir";}
   virtual string IndexPath() const;   

   public:

   virtual bool GetReleases(pkgAcquire *Owner) const { return true; }
   virtual bool GetIndexes(pkgAcquire *Owner) const { return true; }

   // Creates a RPMHandler suitable for usage with this object
   virtual RPMHandler *CreateHandler() const
	   { return new RPMDirHandler(IndexPath()); };

   virtual const Type *GetType() const;
   
   // Interface for the Cache Generator
   virtual unsigned long Size() const;

   rpmSrcDirIndex(string URI,string Dist,string Section,
		   pkgRepository *Repository) :
	   rpmSrcListIndex(URI,Dist,Section,Repository)
      {};
};

class rpmSinglePkgIndex : public rpmPkgListIndex
{
   protected:

   string FilePath;

   virtual string MainType() const {return "pkg";}
   virtual string IndexPath() const {return FilePath;}

   public:

   virtual bool GetReleases(pkgAcquire *Owner) const { return true; }
   virtual bool GetIndexes(pkgAcquire *Owner) const { return true; }

   // Creates a RPMHandler suitable for usage with this object
   virtual RPMHandler *CreateHandler() const
	   { return new RPMSingleFileHandler(IndexPath()); };

   virtual string ArchiveURI(string File) const;

   virtual const Type *GetType() const;
   
   rpmSinglePkgIndex(string File) :
	   rpmPkgListIndex("", "", "", NULL), FilePath(File) {};
};

class rpmSingleSrcIndex : public rpmSrcListIndex
{
   protected:

   string FilePath;

   virtual string MainType() const {return "src";}
   virtual string IndexPath() const {return FilePath;}

   public:

   virtual bool GetReleases(pkgAcquire *Owner) const { return true; }
   virtual bool GetIndexes(pkgAcquire *Owner) const { return true; }

   // Creates a RPMHandler suitable for usage with this object
   virtual RPMHandler *CreateHandler() const
	   { return new RPMSingleFileHandler(IndexPath()); };

   virtual string ArchiveURI(string File) const;

   virtual const Type *GetType() const;
   
   rpmSingleSrcIndex(string File) :
	   rpmSrcListIndex("", "", "", NULL), FilePath(File) {};
};

#ifdef APT_WITH_REPOMD
class rpmRepomdIndex : public rpmIndexFile
{
   protected:

   string URI;
   string Dist;
   string Section;
   pkgRepository *Repository;

   string ReleaseFile(string Type) const;
   string ReleaseURI(string Type) const;
   string ReleaseInfo(string Type) const;

   string Info(string Type) const;
   string IndexFile(string Type) const;
   string IndexURI(string Type) const;

   virtual string MainType() const = 0;
   virtual string IndexPath() const {return IndexFile("primary");};
   virtual string ReleasePath() const;

   public:

   // Creates a RPMHandler suitable for usage with this object
   virtual RPMHandler *CreateHandler() const
          { return new RPMRepomdHandler(IndexFile("primary")); };

   virtual bool GetReleases(pkgAcquire *Owner) const;

   // Interface for the Cache Generator
   virtual bool Exists() const;
   virtual unsigned long Size() const;

   // Interface for acquire
   virtual string Describe(bool Short) const;
   virtual bool GetIndexes(pkgAcquire *Owner) const;
   virtual string ChecksumType() {return "SHA1-Hash";};

   virtual string ArchiveInfo(pkgCache::VerIterator Ver) const;
   virtual string ArchiveURI(string File) const;

   virtual bool Merge(pkgCacheGenerator &Gen,OpProgress &Prog) const;
   virtual bool MergeFileProvides(pkgCacheGenerator &/*Gen*/,
		   		  OpProgress &/*Prog*/) const;
   virtual pkgCache::PkgFileIterator FindInCache(pkgCache &Cache) const;

   // Interface for the source record parsers - repomd can have both binary
   // and source packages in the same repository!
   virtual pkgSrcRecords::Parser *CreateSrcParser() const;

   rpmRepomdIndex(string URI,string Dist,string Section,
               pkgRepository *Repository);

};

class rpmRepomdPkgIndex : public rpmRepomdIndex
{
   protected:

   virtual string MainType() const {return "repomd";};

   public:

   virtual bool HasPackages() const {return true;};
   virtual const Type *GetType() const;


   rpmRepomdPkgIndex(string URI,string Dist,string Section,
                     pkgRepository *Repository) :
          rpmRepomdIndex(URI,Dist,Section,Repository) {};

};

class rpmRepomdSrcIndex : public rpmRepomdIndex
{
   protected:

   virtual string MainType() const {return "repomd-src";};

   public:

   virtual const Type *GetType() const;

   // Stuff for accessing files on remote items
   virtual string SourceInfo(pkgSrcRecords::Parser const &Record,
			     pkgSrcRecords::File const &File) const;

   rpmRepomdSrcIndex(string URI,string Dist,string Section,
                     pkgRepository *Repository) :
          rpmRepomdIndex(URI,Dist,Section,Repository) {};

};

class rpmRepomdDBIndex : public rpmRepomdIndex
{
   protected:

   virtual string IndexPath() const {return IndexFile("primary_db");};

   public:

   // Creates a RPMHandler suitable for usage with this object
   virtual RPMHandler *CreateHandler() const
          { return new RPMSqliteHandler(IndexFile("primary_db")); };
   virtual bool GetIndexes(pkgAcquire *Owner) const;
   virtual unsigned long Size() const;
   virtual bool MergeFileProvides(pkgCacheGenerator &/*Gen*/,
		   		  OpProgress &/*Prog*/) const;


   rpmRepomdDBIndex(string URI,string Dist,string Section,
                     pkgRepository *Repository) :
          rpmRepomdIndex(URI,Dist,Section,Repository) {};
};

class rpmRepomdDBPkgIndex : public rpmRepomdDBIndex
{
   protected:

   virtual string MainType() const {return "repodb";};

   public:

   virtual bool HasPackages() const {return true;};
   virtual const Type *GetType() const;


   rpmRepomdDBPkgIndex(string URI,string Dist,string Section,
                     pkgRepository *Repository) :
          rpmRepomdDBIndex(URI,Dist,Section,Repository) {};

};

class rpmRepomdDBSrcIndex : public rpmRepomdDBIndex
{
   protected:

   virtual string MainType() const {return "repodb-src";};

   public:

   virtual const Type *GetType() const;

   rpmRepomdDBSrcIndex(string URI,string Dist,string Section,
                     pkgRepository *Repository) :
          rpmRepomdDBIndex(URI,Dist,Section,Repository) {};

};
#endif /* APT_WITH_REPOMD */

#endif
