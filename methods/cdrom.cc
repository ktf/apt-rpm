// -*- mode: cpp; mode: fold -*-
// Description								/*{{{*/
// $Id: cdrom.cc,v 1.20 2003/02/10 07:34:41 doogie Exp $
/* ######################################################################

   CDROM URI method for APT
   
   ##################################################################### */
									/*}}}*/
// Include Files							/*{{{*/
#include <apt-pkg/acquire-method.h>
#include <apt-pkg/cdromutl.h>
#include <apt-pkg/error.h>
#include <apt-pkg/configuration.h>
#include <apt-pkg/fileutl.h>

#include <sys/stat.h>
#include <unistd.h>

#include <iostream>

// CNC:2003-02-20 - Moved header to fix compilation error when
// 		    --disable-nls is used.
#include <apti18n.h>
									/*}}}*/
// CNC:2002-10-18
#include <utime.h>  

using namespace std;

class CDROMMethod : public pkgAcqMethod
{
   bool DatabaseLoaded;
   ::Configuration Database;
   string CurrentID;
   string CDROM;
   bool Mounted;
   
   virtual bool Fetch(FetchItem *Itm);
   string GetID(string Name);
   virtual void Exit();
   virtual string PreferredURI();

   // CNC:2002-10-18
   bool Copy(string Src, string Dest);
   
   public:
   
   CDROMMethod();
};

// CDROMMethod::CDROMethod - Constructor				/*{{{*/
// ---------------------------------------------------------------------
/* */
CDROMMethod::CDROMMethod() : pkgAcqMethod("1.0",SingleInstance | LocalOnly |
					  SendConfig | NeedsCleanup |
					  Removable | HasPreferredURI), 
                                          DatabaseLoaded(false), 
                                          Mounted(false)
{
};
									/*}}}*/
// CNC:2004-04-27
// CDROMMethod::PreferredURI() -					/*{{{*/
// ---------------------------------------------------------------------
/* */
string CDROMMethod::PreferredURI()
{
   CDROM = _config->FindDir("Acquire::cdrom::mount","/cdrom/");
   if (CDROM[0] == '.')
      CDROM= SafeGetCWD() + '/' + CDROM;
   string ID;
   MountCdrom(CDROM);
   if (IdentCdrom(CDROM,ID,2) == true) {
      if (DatabaseLoaded == false)
      {
	 string DFile = _config->FindFile("Dir::State::cdroms");
	 if (FileExists(DFile) == true)
	 {
	    if (ReadConfigFile(Database,DFile) == false) {
	       _error->Error(_("Unable to read the cdrom database %s"),
			     DFile.c_str());
	       return "";
	    }
	 }
	 DatabaseLoaded = true;
      }
      string Name = Database.Find("CD::"+ID);
      if (Name.empty() == false)
	 return "cdrom:[" + Name + "]";
   }
   return "";
}
									/*}}}*/
// CDROMMethod::Exit - Unmount the disc if necessary			/*{{{*/
// ---------------------------------------------------------------------
/* */
void CDROMMethod::Exit()
{
   if (Mounted == true)
      UnmountCdrom(CDROM);
}
									/*}}}*/
// CDROMMethod::GetID - Search the database for a matching string	/*{{{*/
// ---------------------------------------------------------------------
/* */
string CDROMMethod::GetID(string Name)
{
   // Search for an ID
   const Configuration::Item *Top = Database.Tree("CD");
   if (Top != 0)
      Top = Top->Child;
   
   for (; Top != 0;)
   {      
      if (Top->Value == Name)
	 return Top->Tag;
      
      Top = Top->Next;
   }
   return string();
}
									/*}}}*/
// CNC:2002-10-18
bool CDROMMethod::Copy(string Src, string Dest)
{
   // See if the file exists
   FileFd From(Src,FileFd::ReadOnly);
   FileFd To(Dest,FileFd::WriteEmpty);
   To.EraseOnFailure();
   if (_error->PendingError() == true)
   {
      To.OpFail();
      return false;
   }
   
   // Copy the file
   if (CopyFile(From,To) == false)
   {
      To.OpFail();
      return false;
   }

   From.Close();
   To.Close();
 
   struct stat Buf;
   if (stat(Src.c_str(),&Buf) != 0)
       return _error->Error("File not found");      
   
   // Transfer the modification times
   struct utimbuf TimeBuf;
   TimeBuf.actime = Buf.st_atime;
   TimeBuf.modtime = Buf.st_mtime;
   if (utime(Dest.c_str(),&TimeBuf) != 0)
   {
      To.OpFail();
      return _error->Errno("utime","Failed to set modification time");
   }
   return true;
}

// CDROMMethod::Fetch - Fetch a file					/*{{{*/
// ---------------------------------------------------------------------
/* */
bool CDROMMethod::Fetch(FetchItem *Itm)
{
   URI Get = Itm->Uri;
   string File = Get.Path;
   FetchResult Res;

   bool Debug = _config->FindB("Debug::Acquire::cdrom",false);

   /* All IMS queries are returned as a hit, CDROMs are readonly so 
      time stamps never change */
   if (Itm->LastModified != 0)
   {
      Res.LastModified = Itm->LastModified;
      Res.IMSHit = true;
      Res.Filename = File;
      URIDone(Res);
      return true;
   }

   // Load the database
   if (DatabaseLoaded == false)
   {
      // Read the database
      string DFile = _config->FindFile("Dir::State::cdroms");
      if (FileExists(DFile) == true)
      {
	 if (ReadConfigFile(Database,DFile) == false)
	    return _error->Error(_("Unable to read the cdrom database %s"),
			  DFile.c_str());
      }
      DatabaseLoaded = true;
   }
       
   // All non IMS queries for package files fail.
   if (Itm->IndexFile == true || GetID(Get.Host).empty() == true)
   {
      Fail(_("Please use apt-cdrom to make this CD recognized by APT."
	   " apt-get update cannot be used to add new CDs"));
      return true;
   }

   // We already have a CD inserted, but it is the wrong one
   if (CurrentID.empty() == false && Database.Find("CD::" + CurrentID) != Get.Host)
   {
      Fail(_("Wrong CD"),true);
      return true;
   }
   
   CDROM = _config->FindDir("Acquire::cdrom::mount","/cdrom/");
   if (CDROM[0] == '.')
      CDROM= SafeGetCWD() + '/' + CDROM;
   string NewID;
   while (CurrentID.empty() == true)
   {
      bool Hit = false;
      Mounted = MountCdrom(CDROM);
      for (unsigned int Version = 2; Version != 0; Version--)
      {
	 if (IdentCdrom(CDROM,NewID,Version) == false)
	    return false;
	 
	 if (Debug == true)
	    clog << "ID " << Version << " " << NewID << endl;
      
	 // A hit
	 if (Database.Find("CD::" + NewID) == Get.Host)
	 {
	    Hit = true;
	    break;
	 }	 
      }

      if (Hit == true)
	 break;
	 
      // I suppose this should prompt somehow?
      if (UnmountCdrom(CDROM) == false)
	 return _error->Error(_("Unable to unmount the CD-ROM in %s, it may still be in use."),
			      CDROM.c_str());
      if (MediaFail(Get.Host,CDROM) == false)
      {
	 CurrentID = "FAIL";
	 Fail(_("Wrong CD"),true);
	 return true;
      }
   }
   
   // CNC:2002-10-18
   // Found a CD
   if (_config->FindB("Acquire::CDROM::Copy-All", false) == true ||
       _config->FindB("Acquire::CDROM::Copy", false) == true) {
      Res.Filename = Queue->DestFile;
      URIStart(Res);
      Copy(CDROM+File, Queue->DestFile);
   } else {
      Res.Filename = CDROM + File;
   }
   
   struct stat Buf;
   if (stat(Res.Filename.c_str(),&Buf) != 0)
      return _error->Error(_("File not found"));
   
   if (NewID.empty() == false)
      CurrentID = NewID;
   Res.LastModified = Buf.st_mtime;
   Res.Size = Buf.st_size;
   URIDone(Res);
   return true;
}
									/*}}}*/

int main()
{
   CDROMMethod Mth;
   return Mth.Run();
}
