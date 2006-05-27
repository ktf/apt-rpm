// -*- mode: c++; mode: fold -*-
// Description								/*{{{*/
// $Id: version.cc,v 1.1 2002/07/23 17:54:50 niemeyer Exp $
/* ######################################################################

   Version - Versioning system..
   
   ##################################################################### */
									/*}}}*/
// Include Files							/*{{{*/
#ifdef __GNUG__
#pragma implementation "apt-pkg/version.h"
#endif 

#include <apt-pkg/version.h>
#include <apt-pkg/pkgcache.h>

#include <stdlib.h>
									/*}}}*/
    
static pkgVersioningSystem *VSList[10];
pkgVersioningSystem **pkgVersioningSystem::GlobalList = VSList;
unsigned long pkgVersioningSystem::GlobalListLen = 0;

// pkgVS::pkgVersioningSystem - Constructor				/*{{{*/
// ---------------------------------------------------------------------
/* Link to the global list of versioning systems supported */
pkgVersioningSystem::pkgVersioningSystem()
{
   VSList[GlobalListLen] = this;
   GlobalListLen++;
}
									/*}}}*/
// pkgVS::GetVS - Find a VS by name					/*{{{*/
// ---------------------------------------------------------------------
/* */
pkgVersioningSystem *pkgVersioningSystem::GetVS(const char *Label)
{
   for (unsigned I = 0; I != GlobalListLen; I++)
      if (strcmp(VSList[I]->Label,Label) == 0)
	 return VSList[I];
   return 0;
}
									/*}}}*/
// vim:sts=3:sw=3
