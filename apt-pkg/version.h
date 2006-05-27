// -*- mode: c++; mode: fold -*-
// Description								/*{{{*/
// $Id: version.h,v 1.2 2002/10/03 16:46:15 niemeyer Exp $
/* ######################################################################

   Version - Versioning system..

   The versioning system represents how versions are compared, represented
   and how dependencies are evaluated. As a general rule versioning
   systems are not compatible unless specifically allowed by the 
   TestCompatibility query.
   
   The versions are stored in a global list of versions, but that is just
   so that they can be queried when someone does 'apt-get -v'. 
   pkgSystem provides the proper means to access the VS for the active
   system.
   
   ##################################################################### */
									/*}}}*/
#ifndef PKGLIB_VERSION_H
#define PKGLIB_VERSION_H

#ifdef __GNUG__
#pragma interface "apt-pkg/version.h"
#endif 

#include <apt-pkg/strutl.h>    
#include <string>

// CNC:2002-07-10
#include <apt-pkg/pkgcache.h>

using std::string;

class pkgVersioningSystem
{
   public:
   // Global list of VS's
   static pkgVersioningSystem **GlobalList;
   static unsigned long GlobalListLen;
   static pkgVersioningSystem *GetVS(const char *Label);
   
   const char *Label;
   
   // Compare versions..
   virtual int DoCmpVersion(const char *A,const char *Aend,
			  const char *B,const char *Bend) = 0;   

   // CNC:2002-07-08
   virtual int DoCmpVersionArch(const char *A,const char *Aend,
		   		const char *AA,const char *AAend,
				const char *B,const char *Bend,
				const char *BA,const char *BAend)
	{return DoCmpVersion(A,Aend,B,Bend);};
   virtual int CmpVersionArch(string A,string AA,
		   	      const char *B,const char *BA)
	{
	   if (AA.length() == 0 || BA == NULL || *BA == 0)
	      return DoCmpVersion(A.c_str(),A.c_str()+A.length(),
				  B,B+strlen(B));
	   else
	      return DoCmpVersionArch(A.c_str(),A.c_str()+A.length(),
				      AA.c_str(),AA.c_str()+AA.length(),
				      B,B+strlen(B),BA,BA+strlen(BA));
	};
   virtual bool CheckDep(const char *PkgVer,pkgCache::DepIterator Dep)
   	{return CheckDep(PkgVer,Dep->CompareOp,Dep.TargetVer());};
   

   virtual bool CheckDep(const char *PkgVer,int Op,const char *DepVer) = 0;
   virtual int DoCmpReleaseVer(const char *A,const char *Aend,
			       const char *B,const char *Bend) = 0;
   virtual string UpstreamVersion(const char *A) = 0;
   
   // See if the given VS is compatible with this one.. 
   virtual bool TestCompatibility(pkgVersioningSystem const &Against) 
                {return this == &Against;};

   // Shortcuts
   APT_MKSTRCMP(CmpVersion,DoCmpVersion);
   APT_MKSTRCMP(CmpReleaseVer,DoCmpReleaseVer);
   
   pkgVersioningSystem();
   virtual ~pkgVersioningSystem() {};
};


#ifdef APT_COMPATIBILITY
// CNC:2003-02-21 - We're not compiling the deb subsystem.
//#include <apt-pkg/debversion.h>
#endif

#endif

// vim:sts=3:sw=3
