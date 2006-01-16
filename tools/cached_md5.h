/*
 * $Id: cached_md5.h,v 1.4 2003/01/29 13:47:31 niemeyer Exp $
 *
 * $Log: cached_md5.h,v $
 * Revision 1.4  2003/01/29 13:47:31  niemeyer
 * Patch by Dmitry V. Levin <ldv@altlinux.org>
 *
 * * tools/cached_md5.h(CachedMD5::CachedMD5):
 *   Add additional argument (Domain).
 * * tools/cached_md5.cc(CachedMD5::CachedMD5):
 *   Use passed argument (Domain) instead of __progname.
 * * tools/genpkglist.cc(main): Pass additional argument to CachedMD5.
 * * tools/gensrclist.cc(main): Likewise.
 *
 * CachedMD5 constructor uses __progname which is wrong in some cases (e.g.
 * genbasedir renamed to smth else). I'm guilty of introducing this code in
 * apt-rpm, so I suggest to fix it.
 *
 * Revision 1.3  2002/07/26 23:22:27  niemeyer
 * Use APT's MD5 implementation.
 *
 * Revision 1.2  2002/07/26 17:39:28  niemeyer
 * Changes for GCC 3.1 and RPM 4.1 support (merged patch from Enrico Scholz).
 *
 * Revision 1.1  2002/07/23 17:54:53  niemeyer
 * Added to CVS.
 *
 * Revision 1.1  2001/08/07 20:46:03  kojima
 * Alexander Bokovoy <a.bokovoy@sam-solutions.net>'s patch for cleaning
 * up genpkglist
 *
 *
 */

#ifndef	__CACHED_MD5_H__
#define	__CACHED_MD5_H__

#include <sys/types.h>
#include <string>
#include <map>

using namespace std;

class CachedMD5
{
   string CacheFileName;
   struct FileData
   {
      string MD5;
      time_t TimeStamp;
   };
   map<string, FileData> MD5Table;

   public:

   void MD5ForFile(string FileName, time_t TimeStamp, char *buf);

   CachedMD5(string DirName, string Domain);
   ~CachedMD5();
};

#endif	/* __CACHED_MD5_H__ */

// vim:sts=3:sw=3
