// -*- mode: cpp; mode: fold -*-
// Description								/*{{{*/
// $Id: rpmversion.h,v 1.1 2002/07/23 17:54:51 niemeyer Exp $
/* ######################################################################

   RPM Version - Versioning system for RPM

   This implements the RPM versioning system.
   
   ##################################################################### */
									/*}}}*/
#ifndef PKGLIB_RPMVERSION_H
#define PKGLIB_RPMVERSION_H

#ifdef __GNUG__
#pragma interface "apt-pkg/rpmversion.h"
#endif 

#include <apt-pkg/version.h>
#include <apt-pkg/strutl.h>    
    
class rpmVersioningSystem : public pkgVersioningSystem
{     
   void ParseVersion(const char *V, const char *VEnd, 
		     char **Epoch, char **Version, char **Release);
   
   public:
   
   // Compare versions..
   virtual int DoCmpVersion(const char *A,const char *Aend,
			    const char *B,const char *Bend);
   virtual int DoCmpVersionArch(const char *A,const char *Aend,
		   		const char *AA,const char *AAend,
				const char *B,const char *Bend,
				const char *BA,const char *BAend);
   virtual bool CheckDep(const char *PkgVer,int Op,const char *DepVer);
   virtual bool CheckDep(const char *PkgVer,pkgCache::DepIterator Dep);
   virtual int DoCmpReleaseVer(const char *A,const char *Aend,
			     const char *B,const char *Bend)
   {
      return DoCmpVersion(A,Aend,B,Bend);
   }   
   virtual string UpstreamVersion(const char *A);

   rpmVersioningSystem();
};

extern rpmVersioningSystem rpmVS;

#endif
