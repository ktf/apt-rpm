// -*- mode: c++; mode: fold -*-
#include <apt-pkg/pkgcache.h>
#include <apt-pkg/cachefile.h>
#include <apt-pkg/algorithms.h>
#include <apt-pkg/version.h>
#include <apt-pkg/strutl.h>

#include <fstream>
#include <stdio.h>

using std::ostream;
using std::ofstream;

static ostream c0out(0);
static ostream c1out(0);
static ostream c2out(0);
static ofstream devnull("/dev/null");
static unsigned int ScreenWidth = 80;

bool YnPrompt();
bool AnalPrompt(const char *Text);
void SigWinch(int);

const char *op2str(int op);

class cmdCacheFile : public pkgCacheFile
{
   static pkgCache *SortCache;
   static int NameComp(const void *a,const void *b);

   public:
   pkgCache::Package **List;
   void Sort();

   cmdCacheFile() : List(0) {};
};

bool ShowList(ostream &out,string Title,string List,string VersionsList);
void Stats(ostream &out,pkgDepCache &Dep,pkgDepCache::State *State=NULL);
void ShowBroken(ostream &out,cmdCacheFile &Cache,bool Now, 
	        pkgDepCache::State *State=NULL);
void ShowNew(ostream &out,cmdCacheFile &Cache, pkgDepCache::State *State=NULL);
void ShowDel(ostream &out,cmdCacheFile &Cache, pkgDepCache::State *State=NULL);
void ShowKept(ostream &out,cmdCacheFile &Cache, pkgDepCache::State *State=NULL);
void ShowUpgraded(ostream &out,cmdCacheFile &Cache, 
		  pkgDepCache::State *State=NULL);
bool ShowDowngraded(ostream &out,cmdCacheFile &Cache, 
		    pkgDepCache::State *State=NULL);
bool ShowHold(ostream &out,cmdCacheFile &Cache, pkgDepCache::State *State=NULL);
bool ShowEssential(ostream &out,cmdCacheFile &Cache, pkgDepCache::State *State=NULL);

// vim:sts=3:sw=3
