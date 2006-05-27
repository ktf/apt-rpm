// -*- mode: c++; mode: fold -*-
// Description								/*{{{*/
// $Id: connect.h,v 1.1 2002/07/23 17:54:53 niemeyer Exp $
/* ######################################################################

   Connect - Replacement connect call
   
   ##################################################################### */
									/*}}}*/
#ifndef CONNECT_H
#define CONNECT_H

#include <string>
#include <apt-pkg/acquire-method.h>

bool Connect(string To,int Port,const char *Service,int DefPort,
	     int &Fd,unsigned long TimeOut,pkgAcqMethod *Owner);
void RotateDNS();

#endif
