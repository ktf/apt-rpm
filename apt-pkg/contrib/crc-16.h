// -*- mode: c++; mode: fold -*-
// Description								/*{{{*/
// $Id: crc-16.h,v 1.1 2002/07/23 17:54:51 niemeyer Exp $
/* ######################################################################

   CRC16 - Compute a 16bit crc very quickly
   
   ##################################################################### */
									/*}}}*/
#ifndef APTPKG_CRC16_H
#define APTPKG_CRC16_H

#define INIT_FCS  0xffff
unsigned short AddCRC16(unsigned short fcs, void const *buf,
			unsigned long len);

#endif
