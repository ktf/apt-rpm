// -*- mode: cpp; mode: fold -*-
// Description								/*{{{*/
// $Id: rpmsrcrecords.h,v 1.5 2002/08/08 20:07:33 niemeyer Exp $
/* ######################################################################
   
   SRPM Records - Parser implementation for RPM style source indexes
   
   ##################################################################### 
 */
									/*}}}*/
#ifndef PKGLIB_RPMSRCRECORDS_H
#define PKGLIB_RPMSRCRECORDS_H

#ifdef __GNUG__
#pragma interface "apt-pkg/rpmsrcrecords.h"
#endif 

#include <apt-pkg/srcrecords.h>
#include <apt-pkg/fileutl.h>
#include <rpm/rpmlib.h>

  
class RPMHandler;

class rpmSrcRecordParser : public pkgSrcRecords::Parser
{
   RPMHandler *Handler;
   Header HeaderP;

   const char *StaticBinList[400];

   char *Buffer;
   unsigned int BufSize;
   unsigned int BufUsed;
   
   void BufCat(char *text);
   void BufCat(char *begin, char *end);
   void BufCatTag(char *tag, char *value);
   void BufCatDep(char *pkg, char *version, int flags);
   void BufCatDescr(char *descr);

public:
   virtual bool Restart();
   virtual bool Step(); 
   virtual bool Jump(unsigned long Off);

   virtual string Package() const;
   virtual string Version() const;
   virtual string Maintainer() const;
   virtual string Section() const;
   virtual const char **Binaries();
   virtual unsigned long Offset();
   virtual string AsStr();
   virtual bool Files(vector<pkgSrcRecords::File> &F);
   virtual bool BuildDepends(vector<BuildDepRec> &BuildDeps, bool ArchOnly);

   rpmSrcRecordParser(string File,pkgIndexFile const *Index);
   ~rpmSrcRecordParser();
};

#endif
