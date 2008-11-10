// -*- mode: c++; mode: fold -*-
// Description								/*{{{*/
// $Id: cdromutl.h,v 1.1 2002/07/23 17:54:51 niemeyer Exp $
/* ######################################################################

   CDROM Utilities - Some functions to manipulate CDROM mounts.
   
   ##################################################################### */
									/*}}}*/
#ifndef PKGLIB_CDROMUTL_H
#define PKGLIB_ACQUIRE_METHOD_H

#include <string>

using std::string;

bool MountCdrom(string Path);
bool UnmountCdrom(string Path);
bool IdentCdrom(string CD,string &Res,unsigned int Version = 2);

#endif
