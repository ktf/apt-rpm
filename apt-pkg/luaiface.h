
#ifndef LUAIFACE_H
#define LUAIFACE_H

#include <apt-pkg/aptconf.h>

#ifdef APT_WITH_LUA

#include <map>
#include <vector>

#include <apt-pkg/pkgcache.h>

using namespace std;

class pkgDepCache;
class pkgProblemResolver;
class lua_State;
typedef int (*lua_CFunction)(struct lua_State*);

class LuaCacheControl {
   public:

   virtual pkgDepCache *Open();
   virtual pkgDepCache *Open(bool Write) = 0;
   virtual void Close() = 0;

   virtual ~LuaCacheControl() {}
};

class Lua {

   protected:

   lua_State *L;

   vector<string> Globals;

   pkgDepCache *DepCache;
   pkgCache *Cache;

   LuaCacheControl *CacheControl;

   pkgProblemResolver *Fix;
   bool DontFix;

   void InternalRunScript();

   public:

   void RunInteractive(const char *PlaceHint=NULL);
   void RunInteractive(const string &PlaceHint)
	 { RunInteractive(PlaceHint.c_str()); }

   bool RunScript(const string &Script, const string &ChunkCacheKey="")
	 { return RunScript(Script.c_str(), (ChunkCacheKey.length() == 0) ?
				      NULL : ChunkCacheKey.c_str()); }
   bool RunScript(const char *Script, const char *ChunkCacheKey=NULL);
   bool RunScripts(const char *ConfListKey, bool CacheChunks=false);
   bool HasScripts(const char *ConfListKey);
   void ResetScript(const string &ChunkCacheKey)
	 { ResetScript(ChunkCacheKey.c_str()); }
   void ResetScript(const char *ChunkCacheKey);

   void SetGlobal(const char *Name);
   void SetGlobal(const char *Name, const char *Value);
   void SetGlobal(const char *Name, bool Value);
   void SetGlobal(const char *Name, double Value);
   void SetGlobal(const char *Name, void *Value);
   void SetGlobal(const char *Name, const string &Value)
	 { SetGlobal(Name, Value.c_str()); }
   void SetGlobal(const char *Name, int Value)
	 { SetGlobal(Name, (double)Value); }
   void SetGlobal(const char *Name, lua_CFunction Value);
   void SetGlobal(const char *Name, const char **Value, int Total=-1);
   void SetGlobal(const char *Name, pkgCache::Package *Value);
   void SetGlobal(const char *Name, vector<const char *> &Value,
		  int Total=-1);
   void SetGlobal(const char *Name, vector<string> &Value,
		  int Total=-1);
   void SetGlobal(const char *Name, vector<pkgCache::Package*> &Value,
		  int Total=-1);
   void ResetGlobals();

   const char *GetGlobalStr(const char *Name);
   vector<string> GetGlobalStrList(const char *Name);
   double GetGlobalNum(const char *Name);
   bool GetGlobalBool(const char *Name);
   void *GetGlobalPtr(const char *Name);
   pkgCache::Package *GetGlobalPkg(const char *Name);
   vector<pkgCache::Package*> GetGlobalPkgList(const char *Name);

   void SetDepCache(pkgDepCache *DepCache_);
   void SetCache(pkgCache *Cache_) { Cache = Cache_; }
   void SetCacheControl(LuaCacheControl *CacheControl_);
   void SetProblemResolver(pkgProblemResolver *Fix_) { Fix = Fix_; }
   void SetDontFix() { DontFix = true; }
   void ResetCaches()
      { DepCache = NULL; Cache = NULL; Fix = NULL; DontFix = false; }

   // For API functions
   pkgDepCache *GetDepCache(lua_State *L=NULL);
   pkgCache *GetCache(lua_State *L=NULL);
   LuaCacheControl *GetCacheControl() { return CacheControl; }
   pkgProblemResolver *GetProblemResolver() { return Fix; }
   bool GetDontFix() { return DontFix; }

   Lua();
   ~Lua();
};

// The same system used with _error
Lua *_GetLuaObj();
#define _lua _GetLuaObj()

#endif // APT_WITH_LUA

#endif

// vim:sts=3:sw=3
