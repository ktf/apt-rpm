#ifndef APTPKG_SQLITE_H
#define APTPKG_SQLITE_H

#ifdef __GNUG__
#pragma interface "apt-pkg/sqlite.h"
#endif

#ifdef WITH_SQLITE3

#include <sqlite3.h>
#include <string>
#include <map>

using std::string;
using std::map;

typedef map<string,string> SqliteRow; 

class SqliteQuery
{
   protected:
   sqlite3 *DB;
   char **res;
   int nrow, ncol;
   int cur;

   map<string,int> ColNames;

   public:
   bool Exec(string SQL);
   int Size() { return nrow; };
   //bool FetchOne(map<string,string> &Row);

   // XXX size_t'ize these..
   bool Jump(unsigned long Pos);
   bool Rewind();
   bool Step();
   unsigned long inline Offset() {return cur-1;};

   string GetCol(string ColName);
   unsigned long GetColI(string ColName);

   SqliteQuery(sqlite3 *DB);
   ~SqliteQuery();
};

class SqliteDB
{
   protected:
   sqlite3 *DB;
   string DBPath;

   public:
   SqliteQuery *Query();

   SqliteDB(string DBPath);
   ~SqliteDB();
};

#endif /* WITH_SQLITE3 */

#endif
// vim:sts=3:sw=3
