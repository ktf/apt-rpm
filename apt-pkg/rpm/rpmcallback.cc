
#include <stdio.h>
#include <rpm/rpmlib.h>
#include <apti18n.h>

#include <apt-pkg/progress.h>
#include "rpmcallback.h"

#include <iostream>
using namespace std;

static int progressCurrent = 0;
static int progressTotal = 0;
int packagesTotal = 0;

#if RPM_VERSION < 0x040000
void * rpmCallback(const Header h,
#else
void * rpmCallback(const void * arg, 
#endif
		   const rpmCallbackType what,
                   const rpmCallbackSize_t amount,
                   const rpmCallbackSize_t total,
		   const void * pkgKey, void * data)

{
#if RPM_VERSION >= 0x040000
   Header h = (Header) arg;
#endif

   char * s;
   OpProgress *Prog = (OpProgress*)data;
   void * rc = NULL;
   const char * filename = (const char *) pkgKey;
   static FD_t fd = NULL;
   static rpmCallbackType state;
   static bool repackage;

   switch (what) {
   case RPMCALLBACK_INST_OPEN_FILE:
      if (filename == NULL || filename[0] == '\0')
	 return NULL;
      fd = Fopen(filename, "r.ufdio");
      if (fd)
	 fd = fdLink(fd, "persist (showProgress)");
      return fd;
      break;

   case RPMCALLBACK_INST_CLOSE_FILE:
      fd = fdFree(fd, "persist (showProgress)");
      if (fd) {
	 (void) Fclose(fd);
	 fd = NULL;
      }
      break;

   case RPMCALLBACK_INST_START:
      if (state != what && repackage == false) {
	 state = what;
	 Prog->Progress(100);
	 Prog->OverallProgress(0,1,1, "Installing");
      }

      s = headerSprintf(h, "%{NAME}-%{VERSION}-%{RELEASE}.%{ARCH}",
				  rpmTagTable, rpmHeaderFormats, NULL);
      Prog->SubProgress(100, s);
      break;

   case RPMCALLBACK_TRANS_PROGRESS:
   case RPMCALLBACK_INST_PROGRESS:
      Prog->Progress((int) (total ? ((((float) amount) / total) * 100): 100.0));
      break;

   case RPMCALLBACK_TRANS_START:
      state = what;
      repackage = false;
      progressTotal = 1;
      progressCurrent = 0;
      Prog->OverallProgress(0,100,1, "Preparing");
   break;

   case RPMCALLBACK_TRANS_STOP:
      Prog->Progress(100);
      Prog->Done();
      progressTotal = packagesTotal;
      progressCurrent = 0;
      break;

   case RPMCALLBACK_REPACKAGE_START:
      progressCurrent = 0;
      repackage = true;
      Prog->OverallProgress(0,1,1, "Repackaging");
      break;

   case RPMCALLBACK_REPACKAGE_PROGRESS:
      Prog->Progress(100);
      break;

   case RPMCALLBACK_REPACKAGE_STOP:
      progressTotal = total;
      progressCurrent = total;
      progressTotal = packagesTotal;
      repackage = false;
      break;

   case RPMCALLBACK_UNINST_PROGRESS:
      break;

   case RPMCALLBACK_UNINST_START:
      if (state != what) {
	 state = what;
	 Prog->OverallProgress(0,1,1, "Removing");
      }
      break;

   case RPMCALLBACK_UNINST_STOP:
      if (h == NULL)
	 break;
      s = headerSprintf(h, "%{NAME}-%{VERSION}-%{RELEASE}.%{ARCH}",
				  rpmTagTable, rpmHeaderFormats, NULL);
      Prog->SubProgress(1, s);
      break;
   default: // Fall through
      break;
 
   }
   return rc;
}	

// vim:sts=3:sw=3
