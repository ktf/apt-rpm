#ifndef APTPKG_SQLITE_H
#define APTPKG_SQLITE_H

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
   char **curptr;
   int nrow, ncol;
   int cur;

   map<string,int> ColNames;

   public:
   bool Exec(const string SQL);
   int Size() { return nrow; }
   //bool FetchOne(map<string,string> &Row);

   // XXX size_t'ize these..
   bool Jump(unsigned long Pos);
   bool Rewind();
   bool Step();
   unsigned long inline Offset() {return cur-1;}

   string GetCol(const string ColName);
   unsigned long GetColI(const string ColName);

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
   bool Exclusive(bool mode);

   SqliteDB(string DBPath);
   ~SqliteDB();
};

#endif /* WITH_SQLITE3 */

#endif
// vim:sts=3:sw=3
