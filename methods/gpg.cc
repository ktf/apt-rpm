
#include <apt-pkg/error.h>
#include <apt-pkg/acquire-method.h>
#include <apt-pkg/strutl.h>
#include <apt-pkg/fileutl.h>

#include <sys/stat.h>
#include <unistd.h>
#include <utime.h>
#include <stdio.h>
#include <sys/wait.h>

#include <apti18n.h>

class GPGMethod : public pkgAcqMethod
{
   virtual bool Fetch(FetchItem *Itm);
   
 public:
   
   GPGMethod() : pkgAcqMethod("1.0",SingleInstance | SendConfig) {};
};


#define STRCMP(buf, conststr) strncmp(buf, conststr, sizeof(conststr)-1)
/*
 * extractSignedFile - Extract parts of a gpg signed file in the format
 *         described below.
 *    file: The file to extract
 *    targetPrefix: Prefix of the extracted files, containing the directory
 *       name and prefix of the filename. Directory must not be writable
 *       by anyone else and must be empty.
 *    targetFile: Path of the file where the signed data will be stored.
 *    oldStyle: Set to true if the filename is not in the format specified
 *       below and is probably an armoured signed file.
 *    sigCount: Number of signatures found in the file.
 * 
 * Returns false if there was an error processing the file.
 * 
 * Extracted signatures will be named
 * targetPrefix+".sig"{1,2,...,sigCount}
 * 
 * File format for the signed file is:
 *<BOF>
 * <ascii data that was signed by gpg>
 * -----BEGIN PGP SIGNATURE-----\n
 * <gpg signature data>
 * -----END PGP SIGNATURE-----\n
 * <Repeat the above n times, depending
 * on how many people have signed the file.>
 * <EOF>
 * 
 * Ie: the original file cat'enated with the signatures generated
 * through gpg -s --armor --detach <yourfile>
 */
bool extractSignedFile(string file, string targetPrefix, string targetFile,
		       bool &oldStyle, int &sigCount)
{
   FILE *fin;
   FILE *fout;
   char buffer[256];
   string tmps;

   fin = fopen(file.c_str(), "r");
   if (fin == NULL)
      return _error->Errno("open", "could not open gpg signed file %s",
			   file.c_str());
   
   oldStyle = false;

   bool Failed = false;

   // store the signed file in a separate file
   tmps = targetFile;

   fout = fopen(tmps.c_str(), "w+");
   if (fout == NULL)
   {
      fclose(fin);
      return _error->Errno("fopen", "could not create file %s",
			   tmps.c_str());
   }
   while (1)
   {
      if (fgets(buffer, sizeof(buffer)-1, fin) == NULL)
      {
	 Failed = true;
	 _error->Error("no signatures in file %s", file.c_str());
	 break;	
      }

      if (STRCMP(buffer, "-----BEGIN") == 0)
	 break;

      if (fputs(buffer, fout) < 0)
      {
	 Failed = true;
	 _error->Errno("fputs", "error writing to %s", tmps.c_str());
	 break;
      }
   }
   fclose(fout);

   if (Failed) 
   {
      fclose(fin);
      return false;
   }
   
   sigCount = 0;
   // now store each of the signatures in a file, separately
   while (1) 
   {
      char buf[32];

      if (STRCMP(buffer, "-----BEGIN PGP SIGNATURE-----") != 0)
      {
	 Failed = true;
	 _error->Error("unexpected data in gpg signed file %s",
		       tmps.c_str());
	 break;
      }
      
      sigCount++;
      snprintf(buf, sizeof(buf), "%i", sigCount);

      tmps = targetPrefix+"sig"+string(buf);
      
      fout = fopen(tmps.c_str(), "w+");
      if (fout == NULL) 
      {
	 fclose(fin);
	 return _error->Errno("fopen", "could not create signature file %s",
			      tmps.c_str());
      }
      while (1)
      {
	 if (fputs(buffer, fout) < 0)
	 {
	    Failed = true;
	    _error->Errno("fputs", "error writing to %s", tmps.c_str());
	    break;
	 }

	 if (STRCMP(buffer, "-----END PGP SIGNATURE-----") == 0)
	    break;

	 if (fgets(buffer, sizeof(buffer)-1, fin) == NULL)
	 {
	    Failed = true;
	    _error->Errno("fgets", "error reading from %s", file.c_str());
	    break;	 
	 }
      }
      fclose(fout);

      if (Failed) 
      {
	 fclose(fin);
	 return false;
      }
      
      if (fgets(buffer, sizeof(buffer)-1, fin) == NULL)
	 break;
      
      if (buffer[0] == '\n')
	 break;      
   }

   fclose(fin);

   return true;
}
#undef STRCMP



char *getFileSigner(const char *file, const char *sigfile,
		    const char *outfile, string &signerKeyID)
{
   pid_t pid;
   int fd[2];
   char buffer[1024];
   FILE *f;
   char keyid[64];
   int status;
   bool goodsig = false;
   bool badsig = false;

   if (pipe(fd) < 0)
      return "could not create pipe";  

   pid = fork();
   if (pid < 0)
      return "could not spawn new process";
   else if (pid == 0) 
   {
      string path = _config->Find("Dir::Bin::gpg", "/usr/bin/gpg");
      string pubring = "";
      const char *argv[16];
      int argc = 0;
      
      close(fd[0]);
      close(STDERR_FILENO);
      close(STDOUT_FILENO);
      dup2(fd[1], STDOUT_FILENO);
      dup2(fd[1], STDERR_FILENO);
      
      unsetenv("LANG");
      unsetenv("LC_ALL");
      unsetenv("LC_MESSAGES");

      argv[argc++] = "gpg";
      argv[argc++] = "--batch";
      argv[argc++] = "--no-secmem-warning";
      pubring = _config->Find("APT::GPG::Pubring");
      if (pubring.empty() == false)
      {
	 argv[argc++] = "--keyring"; argv[argc++] = pubring.c_str();
      }
      argv[argc++] = "--status-fd"; argv[argc++] = "2";
      
      if (outfile != NULL)
      {
	 argv[argc++] = "-o"; argv[argc++] = outfile;
      }
      else
      {
	 argv[argc++] = "--verify"; argv[argc++] = sigfile;
      }
      argv[argc++] = file;
      argv[argc] = NULL;
      
      execvp(path.c_str(), (char**)argv);
      
      exit(111);
   }
   close(fd[1]);
   keyid[0] = 0;
   goodsig = false;
   
   f = fdopen(fd[0], "r");
   
   while (1) {
      char *ptr, *ptr1;
      
      if (!fgets(buffer, 1024, f))
	 break;
      
      if (goodsig && keyid[0])
	 continue;     
      
#define SIGPACK "[GNUPG:] VALIDSIG"
      if ((ptr1 = strstr(buffer, SIGPACK)) != NULL) 
      {
	 char *sig;
	 ptr = sig = ptr1 + sizeof(SIGPACK);
	 while (isxdigit(*ptr) && (ptr-sig) < sizeof(keyid)) ptr++;
	 *ptr = 0;
	 strcpy(keyid, sig);
      }
#undef SIGPACK
      
#define GOODSIG "[GNUPG:] GOODSIG"
      if ((ptr1 = strstr(buffer, GOODSIG)) != NULL)
	 goodsig = true;
#undef GOODSIG

#define BADSIG "[GNUPG:] BADSIG"
      if ((ptr1 = strstr(buffer, BADSIG)) != NULL)
	 badsig = true;
#undef BADSIG
   }
   fclose(f);
   
   waitpid(pid, &status, 0);

   if (WEXITSTATUS(status) == 0) 
   {
      signerKeyID = string(keyid);
      return NULL;
   }
   else if (WEXITSTATUS(status) == 111) 
   {
      return "Could not execute gpg to verify signature";
   }
   else 
   {
      if (badsig)
	 return "File has bad signature, it might have been corrupted or tampered.";

      if (!keyid[0] || !goodsig)
	 return "File was not signed with a known key. Check if the proper gpg key was imported to your keyring.";
      
      return "File could not be authenticated";
   }
}


bool makeTmpDir(string dir, string &path)
{
   char *buf;

   path = dir+"/apt-gpg.XXXXXX";
   buf = new char[path.length()+1];
   if (buf == NULL)
      return _error->Error(_("Could not allocate memory"));
   strcpy(buf, path.c_str());

   if (mkdtemp(buf) == NULL)
      return _error->Errno("mkdtemp", _("Could not create temporary directory"));
   path = buf;

   delete [] buf;

   return true;
}


void removeTmpDir(string path, int sigCount)
{
   while (sigCount > 0)
   {
      char buf[32];
      snprintf(buf, sizeof(buf)-1, "%i", sigCount--);
      unlink(string(path+"/sig"+string(buf)).c_str());
   }
   
   rmdir(path.c_str());
}




bool GPGMethod::Fetch(FetchItem *Itm)
{
   URI Get = Itm->Uri;
   string Path = Get.Host + Get.Path; // To account for relative paths
   string KeyList;

   FetchResult Res;
   Res.Filename = Itm->DestFile;
   URIStart(Res);
   
   string TempDir;
   const char *SysTempDir;

   SysTempDir = getenv("TMPDIR");
   if (SysTempDir == NULL || !FileExists(SysTempDir)) {
      SysTempDir = getenv("TMP");
      if (SysTempDir == NULL || !FileExists(SysTempDir))
         SysTempDir = "/tmp";
   }
   if (makeTmpDir(SysTempDir, TempDir) == false)
      return false;
   
   int SigCount = 0;
   bool OldStyle = true;

   if (extractSignedFile(Path, TempDir+"/", Itm->DestFile, OldStyle,
			 SigCount) == false)
      return false;

   if (OldStyle == true) 
   {
      // Run GPG on file, extract contents and get the key ID of the signer
      char *msg = getFileSigner(Path.c_str(), NULL,
		      		Itm->DestFile.c_str(), KeyList);
      if (msg != NULL) 
      {
	 removeTmpDir(TempDir, SigCount);
	 return _error->Error(msg);
      }
   }
   else 
   {
      char *msg;
      int i;
      char buf[32];
      string KeyID;
      
      // Check fingerprint for each signature
      for (i = 1; i <= SigCount; i++) 
      {
	 snprintf(buf, sizeof(buf)-1, "%i", i);
	 
	 string SigFile = TempDir+"/sig"+string(buf);

	 
	 // Run GPG on file and get the key ID of the signer
	 msg = getFileSigner(Itm->DestFile.c_str(), SigFile.c_str(), 
			     NULL, KeyID);
	 if (msg != NULL)
	 {
	    removeTmpDir(TempDir, SigCount);	       
	    return _error->Error(msg);
	 }
	 if (KeyList.empty())
	    KeyList = KeyID;
	 else
	    KeyList = KeyList+","+KeyID;
      }
   }
   
   removeTmpDir(TempDir, SigCount);
   
   // Transfer the modification times
   struct stat Buf;
   if (stat(Path.c_str(),&Buf) != 0)
      return _error->Errno("stat","Failed to stat %s", Path.c_str());
   
   struct utimbuf TimeBuf;
   TimeBuf.actime = Buf.st_atime;
   TimeBuf.modtime = Buf.st_mtime;
   if (utime(Itm->DestFile.c_str(),&TimeBuf) != 0)
      return _error->Errno("utime","Failed to set modification time");
   
   if (stat(Itm->DestFile.c_str(),&Buf) != 0)
      return _error->Errno("stat","Failed to stat %s", Itm->DestFile.c_str());
   
   // Return a Done response
   Res.LastModified = Buf.st_mtime;
   Res.Size = Buf.st_size;
   Res.SignatureFP = KeyList;
   URIDone(Res);

   return true;
}


int main()
{
   GPGMethod Mth;

   return Mth.Run();
}
