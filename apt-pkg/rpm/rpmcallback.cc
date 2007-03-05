#include <map>
#include <stdio.h>
#include <rpm/rpmlib.h>
#include <apti18n.h>

#include <apt-pkg/progress.h>
#include "rpmcallback.h"

#include <iostream>
using namespace std;

static char *copyTags[] = {"name", 
			   "version", 
			   "release", 
			   "arch", 
			   "summary", 
			   NULL};

static void getPackageData(const Header h, map<string,string> &Data)
{
   char **Tag = &copyTags[0];
   char rTag[20];
   Data.clear();
   for (Tag = &copyTags[0]; *Tag != NULL; *Tag++) {
      sprintf(rTag, "%{%s}", *Tag);
      char *s = headerSprintf(h, rTag, rpmTagTable, rpmHeaderFormats, NULL);
      Data[*Tag] = s;
      free(s);
   }

}

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
   InstProgress *Prog = (InstProgress*)data;
   void * rc = NULL;
   const char * filename = (const char *) pkgKey;
   static FD_t fd = NULL;
   static rpmCallbackType state;
   static bool repackage;
   static map<string,string> Data;

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
	 Prog->OverallProgress(0,1,1, "Installing");
	 Prog->SetState(InstProgress::Installing);
      }

      getPackageData(h, Data);
      Prog->SubProgress(total, Data["name"]);
      Prog->Progress(amount);
      break;

   case RPMCALLBACK_TRANS_PROGRESS:
      //cout << "RPMCALLBACK_TRANS_PROGRESS " << amount << " " << total << endl << flush;
   case RPMCALLBACK_INST_PROGRESS:
      //cout << "RPMCALLBACK_INST_PROGRESS " << amount << " " << total << endl << flush;
      Prog->Progress(amount);
      break;

   case RPMCALLBACK_TRANS_START:
      //cout << "RPMCALLBACK_TRANS_START " << amount << " " << total << endl << flush;
      state = what;
      repackage = false;
      Prog->SetState(InstProgress::Preparing);
      Prog->SubProgress(total, "Preparing");
      Prog->SetPackageData(&Data);
   break;

   case RPMCALLBACK_TRANS_STOP:
      Prog->Done();
      break;

   case RPMCALLBACK_REPACKAGE_START:
      repackage = true;
      Prog->OverallProgress(0,1,1, "Repackaging");
      Prog->SetState(InstProgress::Repackaging);
      break;

   case RPMCALLBACK_REPACKAGE_PROGRESS:
      Prog->Progress(amount);
      break;

   case RPMCALLBACK_REPACKAGE_STOP:
      repackage = false;
      break;

   case RPMCALLBACK_UNINST_PROGRESS:
      break;

   case RPMCALLBACK_UNINST_START:
      if (h == NULL) {
	 cout << "uninst start, header null ;(" << endl;
	 break;
      }
      if (state != what) {
	 state = what;
	 Prog->SetState(InstProgress::Removing);
	 Prog->OverallProgress(0,1,1, "Removing");
      }
      getPackageData(h, Data);
      Prog->SubProgress(total, Data["name"]);
      Prog->Progress(total);
      break;

   case RPMCALLBACK_UNINST_STOP:
      Prog->Progress(total);
      if (h == NULL)
	 break;
      getPackageData(h, Data);
      break;
   default: // Fall through
      break;
 
   }
   return rc;
}	

// vim:sts=3:sw=3
