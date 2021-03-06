// -*- mode: c++; mode: fold -*-
// Description								/*{{{*/
// $Id: hashes.cc,v 1.1 2002/07/23 17:54:51 niemeyer Exp $
/* ######################################################################

   Hashes - Simple wrapper around the hash functions
   
   This is just used to make building the methods simpler, this is the
   only interface required..
   
   ##################################################################### */
									/*}}}*/
// Include Files							/*{{{*/
#include <apt-pkg/hashes.h>
    
#include <unistd.h>    
#include <system.h>
#include <algorithm>
									/*}}}*/

// Hashes::AddFD - Add the contents of the FD				/*{{{*/
// ---------------------------------------------------------------------
/* */
bool Hashes::AddFD(int Fd,unsigned long Size)
{
   unsigned char Buf[64*64];
   int Res = 0;
   while (Size != 0)
   {
      Res = read(Fd,Buf,std::min(Size,(unsigned long)sizeof(Buf)));
      if (Res < 0 || (unsigned)Res != std::min(Size,(unsigned long)sizeof(Buf)))
	 return false;
      Size -= Res;
      MD5.Add(Buf,Res);
      SHA1.Add(Buf,Res);
   }
   return true;
}
									/*}}}*/

