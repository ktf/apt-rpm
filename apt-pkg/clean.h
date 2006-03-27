// -*- mode: cpp; mode: fold -*-
// Description								/*{{{*/
// $Id: clean.h,v 1.1 2002/07/23 17:54:50 niemeyer Exp $
/* ######################################################################

   Clean - Clean out downloaded directories
   
   ##################################################################### */
									/*}}}*/
#ifndef APTPKG_CLEAN_H
#define APTPKG_CLEAN_H

#ifdef __GNUG__
#pragma interface "apt-pkg/clean.h"
#endif 

#include <apt-pkg/pkgcache.h>

class pkgArchiveCleaner
{
   protected:
   
   virtual void Erase(const char * /*File*/,string /*Pkg*/,string /*Ver*/,struct stat & /*St*/) {};
   virtual ~pkgArchiveCleaner() {};

   public:   
   
   bool Go(string Dir,pkgCache &Cache);
};

#endif
