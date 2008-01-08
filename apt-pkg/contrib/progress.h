// -*- mode: c++; mode: fold -*-
// Description								/*{{{*/
// $Id: progress.h,v 1.1 2002/07/23 17:54:51 niemeyer Exp $
/* ######################################################################
   
   OpProgress - Operation Progress
   
   This class allows lengthy operations to communicate their progress 
   to the GUI. The progress model is simple and is not designed to handle
   the complex case of the multi-activity aquire class.
   
   The model is based on the concept of an overall operation consisting
   of a series of small sub operations. Each sub operation has it's own
   completion status and the overall operation has it's completion status.
   The units of the two are not mixed and are completely independent.
   
   The UI is expected to subclass this to provide the visuals to the user.
   
   ##################################################################### */
									/*}}}*/
#ifndef PKGLIB_PROGRESS_H
#define PKGLIB_PROGRESS_H

#ifdef __GNUG__
#pragma interface "apt-pkg/progress.h"
#endif 

#include <map>
#include <string>
#include <sys/time.h>

using std::string;
using std::map;

class Configuration;
class OpProgress
{
   off_t Current;
   off_t Total;
   off_t Size;
   off_t SubTotal;
   float LastPercent;
   
   // Change reduction code
   struct timeval LastTime;
   string LastOp;
   string LastSubOp;
   
   protected:
   
   string Op;
   string SubOp;
   float Percent;
   
   bool MajorChange;
   bool SubChange;
   
   bool CheckChange(float Interval = 0.7);		    
   virtual void Update() {};
   
   public:
   
   void Progress(unsigned long Current);
   void SubProgress(unsigned long SubTotal);
   void SubProgress(unsigned long SubTotal,string Op);
   void OverallProgress(unsigned long Current,unsigned long Total,
			unsigned long Size,string Op);
   virtual void Done() {};
   
   OpProgress();
   virtual ~OpProgress() {};
};

class OpTextProgress : public OpProgress
{
   protected:
   
   string OldOp;
   bool NoUpdate;
   bool NoDisplay;
   unsigned long LastLen;
   virtual void Update();
   void Write(const char *S);
   
   public:

   virtual void Done();
   
   OpTextProgress(bool NoUpdate = false) : NoUpdate(NoUpdate), 
                NoDisplay(false), LastLen(0) {};
   OpTextProgress(Configuration &Config);
   virtual ~OpTextProgress() {Done();};
};


class InstProgress : public OpProgress
{
   public:
   enum InstallStates { Preparing, Installing, Repackaging, Removing };

   void SetState(enum InstallStates St) {State = St;};
   void SetPackageData(map<string,string> *PkgData) {PackageData=PkgData;};
   InstProgress(Configuration &Config) : OpProgress(), PackageData(NULL) {};
   virtual ~InstProgress() {};

   protected:
   map<string,string> *PackageData;
   enum InstallStates State;
};

// Progress class that emulates "rpm -Uv --percent" output for Synaptic
// compatibility 
class InstPercentProgress : public InstProgress
{
   protected:
   virtual void Update();

   public:
   virtual void Done();

   InstPercentProgress(Configuration &Config);
   virtual ~InstPercentProgress() {};
};

// Progress class similar to rpm -Uvh but with erasure callbacks and whatnot
// TODO: actually implement it ;)
class InstHashProgress : public InstProgress
{
   protected:
  
   bool Quiet;

   virtual void Update();
   void PrintHashes();

   public:
   virtual void Done();

   InstHashProgress(Configuration &Config);
   virtual ~InstHashProgress() {};
};
#endif

// vim:sts=3:sw=3
