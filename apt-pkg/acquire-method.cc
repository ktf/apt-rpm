// -*- mode: c++; mode: fold -*-
// Description								/*{{{*/
// $Id: acquire-method.cc,v 1.1 2002/07/23 17:54:50 niemeyer Exp $
/* ######################################################################

   Acquire Method

   This is a skeleton class that implements most of the functionality
   of a method and some useful functions to make method implementation
   simpler. The methods all derive this and specialize it. The most
   complex implementation is the http method which needs to provide
   pipelining, it runs the message engine at the same time it is 
   downloading files..
   
   ##################################################################### */
									/*}}}*/
// Include Files							/*{{{*/
#include <apt-pkg/acquire-method.h>
#include <apt-pkg/error.h>
#include <apt-pkg/configuration.h>
#include <apt-pkg/strutl.h>
#include <apt-pkg/fileutl.h>
#include <apt-pkg/hashes.h>

#include <iostream>
#include <sstream>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>
									/*}}}*/

using namespace std;

// AcqMethod::pkgAcqMethod - Constructor				/*{{{*/
// ---------------------------------------------------------------------
/* This constructs the initialization text */
pkgAcqMethod::pkgAcqMethod(const char *Ver,unsigned long Flags)
	: Flags(Flags) // CNC:2002-07-11
{
   char S[300] = "";
   char *End = S;
   strcat(End,"100 Capabilities\n");
   sprintf(End+strlen(End),"Version: %s\n",Ver);

   if ((Flags & SingleInstance) == SingleInstance)
      strcat(End,"Single-Instance: true\n");
   
   if ((Flags & Pipeline) == Pipeline)
      strcat(End,"Pipeline: true\n");
   
   if ((Flags & SendConfig) == SendConfig)
      strcat(End,"Send-Config: true\n");

   if ((Flags & LocalOnly) == LocalOnly)
      strcat(End,"Local-Only: true\n");

   if ((Flags & NeedsCleanup) == NeedsCleanup)
      strcat(End,"Needs-Cleanup: true\n");

   if ((Flags & Removable) == Removable)
      strcat(End,"Removable: true\n");

   // CNC:2004-04-27
   if ((Flags & HasPreferredURI) == HasPreferredURI)
      strcat(End,"Has-Preferred-URI: true\n");
   strcat(End,"\n");

   if (write(STDOUT_FILENO,S,strlen(S)) != (signed)strlen(S))
      exit(100);

   SetNonBlock(STDIN_FILENO,true);

   Queue = 0;
   QueueBack = 0;
}
									/*}}}*/
// AcqMethod::Fail - A fetch has failed					/*{{{*/
// ---------------------------------------------------------------------
/* */
void pkgAcqMethod::Fail(bool Transient)
{
   string Err = "Undetermined Error";
   if (_error->empty() == false)
      _error->PopMessage(Err);   
   _error->Discard();
   Fail(Err,Transient);
}
									/*}}}*/
// AcqMethod::Fail - A fetch has failed					/*{{{*/
// ---------------------------------------------------------------------
/* */
void pkgAcqMethod::Fail(string Err,bool Transient)
{
   // Strip out junk from the error messages
   for (string::iterator I = Err.begin(); I != Err.end(); I++)
   {
      if (*I == '\r') 
	 *I = ' ';
      if (*I == '\n') 
	 *I = ' ';
   }
   
   char S[1024];
   if (Queue != 0)
   {
      snprintf(S,sizeof(S)-50,"400 URI Failure\nURI: %s\n"
	       "Message: %s %s\n",Queue->Uri.c_str(),Err.c_str(),
	       FailExtra.c_str());

      // Dequeue
      FetchItem *Tmp = Queue;
      Queue = Queue->Next;
      delete Tmp;
      if (Tmp == QueueBack)
	 QueueBack = Queue;
   }
   else
      snprintf(S,sizeof(S)-50,"400 URI Failure\nURI: <UNKNOWN>\n"
	       "Message: %s %s\n",Err.c_str(),
	       FailExtra.c_str());
      
   // Set the transient flag 
   if (Transient == true)
      strcat(S,"Transient-Failure: true\n\n");
   else
      strcat(S,"\n");
   
   if (write(STDOUT_FILENO,S,strlen(S)) != (signed)strlen(S))
      exit(100);
}
									/*}}}*/
// AcqMethod::URIStart - Indicate a download is starting		/*{{{*/
// ---------------------------------------------------------------------
/* */
void pkgAcqMethod::URIStart(FetchResult &Res)
{
   if (Queue == 0)
      abort();

   ostringstream s;

   s << "200 URI Start\nURI: " << Queue->Uri << "\n";
   if (Res.Size != 0)
      s << "Size: " << Res.Size << "\n";

   if (Res.LastModified != 0)
      s << "Last-Modified: " << TimeRFC1123(Res.LastModified) << "\n";

   if (Res.ResumePoint != 0)
      s << "Resume-Point: " << Res.ResumePoint << "\n";

   s << "\n";
   string S = s.str();
   if (write(STDOUT_FILENO,S.c_str(),S.size()) != (ssize_t)S.size())
      exit(100);
}
									/*}}}*/
// AcqMethod::URIDone - A URI is finished				/*{{{*/
// ---------------------------------------------------------------------
/* */
void pkgAcqMethod::URIDone(FetchResult &Res, FetchResult *Alt)
{
   if (Queue == 0)
      abort();
   
   ostringstream s;

   s << "201 URI Done\nURI: " << Queue->Uri << "\n";
   
   if (Res.Filename.empty() == false)
      s << "Filename: " << Res.Filename << "\n";
   
   if (Res.Size != 0)
      s << "Size: " << Res.Size << "\n";
   
   if (Res.LastModified != 0)
      s << "Last-Modified: " << TimeRFC1123(Res.LastModified) << "\n";

   if (Res.MD5Sum.empty() == false)
      s << "MD5-Hash: " << Res.MD5Sum << "\n";
   if (Res.SHA1Sum.empty() == false)
      s << "SHA1-Hash: " << Res.SHA1Sum << "\n";

   // CNC:2002-07-04
   if (Res.SignatureFP.empty() == false)
      s << "Signature-Fingerprint: " << Res.SignatureFP << "\n";

   if (Res.ResumePoint != 0)
      s << "Resume-Point: " << Res.ResumePoint << "\n";

   if (Res.IMSHit == true)
      s << "IMS-Hit: true\n";
      
   if (Alt != 0)
   {
      if (Alt->Filename.empty() == false)
	 s << "Alt-Filename: " << Alt->Filename << "\n";
      
      if (Alt->Size != 0)
	 s << "Alt-Size: " << Alt->Size << "\n";
      
      if (Alt->LastModified != 0)
	 s << "Alt-Last-Modified: " << TimeRFC1123(Alt->LastModified) << "\n";
      
      if (Alt->MD5Sum.empty() == false)
	 s << "Alt-MD5-Hash: " << Alt->MD5Sum << "\n";
      if (Alt->SHA1Sum.empty() == false)
	 s << "Alt-SHA1-Hash: " << Alt->SHA1Sum << "\n";
      
      if (Alt->IMSHit == true)
	 s << "Alt-IMS-Hit: true\n";
   }
   
   s << "\n";
   string S = s.str();
   if (write(STDOUT_FILENO,S.c_str(),S.size()) != (ssize_t)S.size())
      exit(100);

   // Dequeue
   FetchItem *Tmp = Queue;
   Queue = Queue->Next;
   delete Tmp;
   if (Tmp == QueueBack)
      QueueBack = Queue;
}
									/*}}}*/
// AcqMethod::MediaFail - Syncronous request for new media		/*{{{*/
// ---------------------------------------------------------------------
/* This sends a 403 Media Failure message to the APT and waits for it
   to be ackd */
bool pkgAcqMethod::MediaFail(string Required,string Drive)
{
   char S[1024];
   snprintf(S,sizeof(S),"403 Media Failure\nMedia: %s\nDrive: %s\n\n",
	    Required.c_str(),Drive.c_str());

   if (write(STDOUT_FILENO,S,strlen(S)) != (signed)strlen(S))
      exit(100);
   
   vector<string> MyMessages;
   
   /* Here we read messages until we find a 603, each non 603 message is
      appended to the main message list for later processing */
   while (1)
   {
      if (WaitFd(STDIN_FILENO) == false)
	 return false;
      
      if (ReadMessages(STDIN_FILENO,MyMessages) == false)
	 return false;

      string Message = MyMessages.front();
      MyMessages.erase(MyMessages.begin());
      
      // Fetch the message number
      char *End;
      int Number = strtol(Message.c_str(),&End,10);
      if (End == Message.c_str())
      {	 
	 cerr << "Malformed message!" << endl;
	 exit(100);
      }

      // Change ack
      if (Number == 603)
      {
	 while (MyMessages.empty() == false)
	 {
	    Messages.push_back(MyMessages.front());
	    MyMessages.erase(MyMessages.begin());
	 }

	 return !StringToBool(LookupTag(Message,"Fail"),false);
      }
      
      Messages.push_back(Message);
   }   
}
									/*}}}*/
// AcqMethod::NeedAuth - Request authentication				/*{{{*/
// ---------------------------------------------------------------------
/* This sends a 404 Authenticate message to the APT and waits for it
   to be ackd */
bool pkgAcqMethod::NeedAuth(string Description,string &User,string &Pass)
{
   char S[1024];
   snprintf(S,sizeof(S),"404 Authenticate\nDescription: %s\n\n",
	    Description.c_str());

   if (write(STDOUT_FILENO,S,strlen(S)) != (signed)strlen(S))
      exit(100);
   
   vector<string> MyMessages;
   
   /* Here we read messages until we find a 604, each non 604 message is
      appended to the main message list for later processing */
   while (1)
   {
      if (WaitFd(STDIN_FILENO) == false)
	 return false;
      
      if (ReadMessages(STDIN_FILENO,MyMessages) == false)
	 return false;

      string Message = MyMessages.front();
      MyMessages.erase(MyMessages.begin());
      
      // Fetch the message number
      char *End;
      int Number = strtol(Message.c_str(),&End,10);
      if (End == Message.c_str())
      {	 
	 cerr << "Malformed message!" << endl;
	 exit(100);
      }

      // Change ack
      if (Number == 604)
      {
	 while (MyMessages.empty() == false)
	 {
	    Messages.push_back(MyMessages.front());
	    MyMessages.erase(MyMessages.begin());
	 }

	 if (StringToBool(LookupTag(Message,"Fail"),false) == false)
	 {
	    User = LookupTag(Message,"User");
	    Pass = LookupTag(Message,"Password");
	    return true;
	 }
	 else
	    return false;
      }
      
      Messages.push_back(Message);
   }   
}
									/*}}}*/
// AcqMethod::Configuration - Handle the configuration message		/*{{{*/
// ---------------------------------------------------------------------
/* This parses each configuration entry and puts it into the _config 
   Configuration class. */
bool pkgAcqMethod::Configuration(string Message)
{
   ::Configuration &Cnf = *_config;
   
   const char *I = Message.c_str();
   const char *MsgEnd = I + Message.length();
   
   unsigned int Length = strlen("Config-Item");
   for (; I + Length < MsgEnd; I++)
   {
      // Not a config item
      if (I[Length] != ':' || stringcasecmp(I,I+Length,"Config-Item") != 0)
	 continue;
      
      I += Length + 1;
      
      for (; I < MsgEnd && *I == ' '; I++);
      const char *Equals = I;
      for (; Equals < MsgEnd && *Equals != '='; Equals++);
      const char *End = Equals;
      for (; End < MsgEnd && *End != '\n'; End++);
      if (End == Equals)
	 return false;
      
      Cnf.Set(DeQuoteString(string(I,Equals-I)),
	      DeQuoteString(string(Equals+1,End-Equals-1)));
      I = End;
   }
   
   return true;
}
									/*}}}*/
// AcqMethod::Run - Run the message engine				/*{{{*/
// ---------------------------------------------------------------------
/* Fetch any messages and execute them. In single mode it returns 1 if
   there are no more available messages - any other result is a 
   fatal failure code! */
int pkgAcqMethod::Run(bool Single)
{
   while (1)
   {
      // Block if the message queue is empty
      if (Messages.empty() == true)
      {
	 if (Single == false)
	    if (WaitFd(STDIN_FILENO) == false)
	       break;
	 if (ReadMessages(STDIN_FILENO,Messages) == false)
	    break;
      }
            
      // Single mode exits if the message queue is empty
      if (Single == true && Messages.empty() == true)
	 return -1;
      
      string Message = Messages.front();
      Messages.erase(Messages.begin());
      
      // Fetch the message number
      char *End;
      int Number = strtol(Message.c_str(),&End,10);
      if (End == Message.c_str())
      {	 
	 cerr << "Malformed message!" << endl;
	 return 100;
      }

      switch (Number)
      {	 
	 case 601:
	 if (Configuration(Message) == false)
	    return 100;
	 break;
	 
	 case 600:
	 {
	    FetchItem *Tmp = new FetchItem;
	    
	    Tmp->Uri = LookupTag(Message,"URI");
	    Tmp->DestFile = LookupTag(Message,"FileName");
	    if (StrToTime(LookupTag(Message,"Last-Modified"),Tmp->LastModified) == false)
	       Tmp->LastModified = 0;
	    Tmp->IndexFile = StringToBool(LookupTag(Message,"Index-File"),false);
	    Tmp->Next = 0;

	    // CNC:2002-07-11
	    if (StringToBool(LookupTag(Message,"Local-Only-IMS"),false) == true
	        && (Flags & LocalOnly) == 0)
	       Tmp->LastModified = 0;
	    
	    // Append it to the list
	    FetchItem **I = &Queue;
	    for (; *I != 0; I = &(*I)->Next);
	    *I = Tmp;
	    if (QueueBack == 0)
	       QueueBack = Tmp;
	    
	    // Notify that this item is to be fetched.
	    if (Fetch(Tmp) == false)
	       Fail();
	    
	    break;					     
	 }   

	 // CNC:2004-04-27
	 case 679:
	 {
	    char S[1024];
	    snprintf(S,sizeof(S),"179 Preferred URI\nPreferredURI: %s\n\n",
		     PreferredURI().c_str());
	    if (write(STDOUT_FILENO,S,strlen(S)) != (signed)strlen(S))
	       exit(100);
	    break;

	 }
      }      
   }

   Exit();
   return 0;
}
									/*}}}*/
// AcqMethod::Log - Send a log message					/*{{{*/
// ---------------------------------------------------------------------
/* */
void pkgAcqMethod::Log(const char *Format,...)
{
   string CurrentURI = "<UNKNOWN>";
   if (Queue != 0)
      CurrentURI = Queue->Uri;
   
   va_list args;
   va_start(args,Format);

   // sprintf the description
   char S[1024];
   unsigned int Len = snprintf(S,sizeof(S)-4,"101 Log\nURI: %s\n"
			       "Message: ",CurrentURI.c_str());

   vsnprintf(S+Len,sizeof(S)-4-Len,Format,args);
   strcat(S,"\n\n");
   
   if (write(STDOUT_FILENO,S,strlen(S)) != (signed)strlen(S))
      exit(100);
}
									/*}}}*/
// AcqMethod::Status - Send a status message				/*{{{*/
// ---------------------------------------------------------------------
/* */
void pkgAcqMethod::Status(const char *Format,...)
{
   string CurrentURI = "<UNKNOWN>";
   if (Queue != 0)
      CurrentURI = Queue->Uri;
   
   va_list args;
   va_start(args,Format);

   ostringstream s;
   s << "102 Status\nURI: " << CurrentURI << "\nMessage: ";

   // sprintf the description
   char Buf[1024];
   vsnprintf(Buf,sizeof(Buf)-4,Format,args);
   s << Buf << "\n\n";

   string S = s.str();
   if (write(STDOUT_FILENO,S.c_str(),S.size()) != (ssize_t)S.size())
      exit(100);
}
									/*}}}*/
// AcqMethod::Redirect - Send a redirect message			/*{{{*/
// ---------------------------------------------------------------------
/* This method sends the redirect message and also manipulates the queue
   to keep the pipeline synchronized. */
void pkgAcqMethod::Redirect(const string &NewURI)
{
   string CurrentURI = "<UNKNOWN>";
   if (Queue != 0)
      CurrentURI = Queue->Uri;

   ostringstream s;
   s << "103 Redirect\nURI: " << CurrentURI << "\nNew-URI: " << NewURI 
     << "\n\n";

   string S = s.str();
   if (write(STDOUT_FILENO,S.c_str(),S.size()) != (ssize_t)S.size())
      exit(100);

   // Change the URI for the request.
   Queue->Uri = NewURI;

   /* To keep the pipeline synchronized, move the current request to
      the end of the queue, past the end of the current pipeline. */
   FetchItem *I;
   for (I = Queue; I->Next != 0; I = I->Next) ;
   I->Next = Queue;
   Queue = Queue->Next;
   I->Next->Next = 0;
   if (QueueBack == 0)
      QueueBack = I->Next;
}
									/*}}}*/

// AcqMethod::FetchResult::FetchResult - Constructor			/*{{{*/
// ---------------------------------------------------------------------
/* */
pkgAcqMethod::FetchResult::FetchResult() : LastModified(0),
                                   IMSHit(false), Size(0), ResumePoint(0)
{
}
									/*}}}*/
// AcqMethod::FetchResult::TakeHashes - Load hashes			/*{{{*/
// ---------------------------------------------------------------------
/* This hides the number of hashes we are supporting from the caller. 
   It just deals with the hash class. */
void pkgAcqMethod::FetchResult::TakeHashes(Hashes &Hash)
{
   MD5Sum = Hash.MD5.Result();
   SHA1Sum = Hash.SHA1.Result();
}
									/*}}}*/
// vim:sts=3:sw=3
