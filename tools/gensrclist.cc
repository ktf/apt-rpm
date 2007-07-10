/*
 * $Id: gensrclist.cc,v 1.8 2003/01/30 17:18:21 niemeyer Exp $
 */
#include <alloca.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <rpm/rpmlib.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <assert.h>

#include <map>
#include <list>
#include <iostream>

#include <apt-pkg/error.h>
#include <apt-pkg/tagfile.h>
#include <apt-pkg/rpmhandler.h>
#include <apt-pkg/configuration.h>
#include <config.h>

#include "cached_md5.h"

#if RPM_VERSION >= 0x040100
#include <rpm/rpmts.h>
#endif
 
using namespace std;

int tags[] =  {
       RPMTAG_NAME,
       RPMTAG_EPOCH,
       RPMTAG_VERSION,
       RPMTAG_RELEASE,
       RPMTAG_GROUP,
       RPMTAG_ARCH,
       RPMTAG_PACKAGER,
       RPMTAG_SOURCERPM,
       RPMTAG_SIZE,
       RPMTAG_VENDOR,
       RPMTAG_OS,
       
       RPMTAG_DESCRIPTION, 
       RPMTAG_SUMMARY, 
       /*RPMTAG_HEADERI18NTABLE*/ HEADER_I18NTABLE,
       
       RPMTAG_REQUIREFLAGS, 
       RPMTAG_REQUIRENAME,
       RPMTAG_REQUIREVERSION
};
int numTags = sizeof(tags) / sizeof(int);

#if defined(__APPLE__) || defined(__FREEBSD__)
int selectDirent(struct dirent *ent)
#else
int selectDirent(const struct dirent *ent)
#endif
{
   int state = 0;
   const char *p = ent->d_name;
   
   while (1) {
      if (*p == '.') {
	  state = 1;
      } else if (state == 1 && *p == 'r')
	  state++;
      else if (state == 2 && *p == 'p')
	  state++;
      else if (state == 3 && *p == 'm')
	  state++;
      else if (state == 4 && *p == '\0')
	  return 1;
      else if (*p == '\0')
	  return 0;
      else
	  state = 0;
      p++;
   }
}

bool readRPMTable(char *file, map<string, list<char*>* > &table)
{
   FILE *indexf;
   char buf[512];
   string srpm;
   
   indexf = fopen(file, "r");
   if (!indexf) {
      cerr << "gensrclist: could not open file " << file << " for reading: "
	  << strerror(errno) << endl;
      return false;
   }
   
   while (fgets(buf, 512, indexf)) {
      char *f;
      
      buf[strlen(buf)-1] = '\0';
      f = strchr(buf, ' ');
      *f = '\0';
      f++;
      
      srpm = string(buf);
      
      if (table.find(srpm) != table.end()) {
	 list<char*> *l = table[srpm];
	 l->push_front(strdup(f));
      } else {
	 list<char*> *l = new list<char*>;
	 l->push_front(strdup(f));
	 table[srpm] = l;
      }
   }
   
   fclose(indexf);
   
   return true;
}


void usage()
{
   cerr << "gensrclist " << VERSION << endl;
   cerr << "usage: gensrclist [<options>] <dir> <suffix> <srpm index>" << endl;
   cerr << "options:" << endl;
//   cerr << " --mapi         ???????????????????" << endl;
   cerr << " --flat          use a flat directory structure, where RPMS and SRPMS"<<endl;
   cerr << "                 are in the same directory level"<<endl;
   cerr << " --meta <suffix> create source package file list with given suffix" << endl;
   cerr << " --append        append to the source package file list, don't overwrite" << endl;
   cerr << " --progress      show a progress bar" << endl;
   cerr << " --cachedir=DIR  use a custom directory for package md5sum cache"<<endl;
}

#if RPM_VERSION >= 0x040000
extern "C" {
// No prototype from rpm after 4.0.
int headerGetRawEntry(Header h, int_32 tag, int_32 * type,
		      void *p, int_32 *c);
}
#endif

int main(int argc, char ** argv) 
{
   char buf[300];
   char cwd[PATH_MAX];
   string srpmdir;
   FD_t outfd, fd;
   struct dirent **dirEntries;
   int rc, i;
   Header h;
   int_32 size[1];
   int entry_no, entry_cur;
   CachedMD5 *md5cache;
   map<string, list<char*>* > rpmTable; // table that maps srpm -> generated rpm
   bool mapi = false;
   bool progressBar = false;
   bool flatStructure = false;
   char *arg_dir, *arg_suffix, *arg_srpmindex;
   const char *srcListSuffix = NULL;
   bool srcListAppend = false;

   putenv("LC_ALL="); // Is this necessary yet (after i18n was supported)?
   for (i = 1; i < argc; i++) {
      if (strcmp(argv[i], "--mapi") == 0) {
	 mapi = true;
      } else if (strcmp(argv[i], "--flat") == 0) {
	 flatStructure = true;
      } else if (strcmp(argv[i], "--progress") == 0) {
	 progressBar = true;
      } else if (strcmp(argv[i], "--append") == 0) {
	 srcListAppend = true;
      } else if (strcmp(argv[i], "--meta") == 0) {
	 i++;
	 if (i < argc) {
	    srcListSuffix = argv[i];
	 } else {
	    cout << "gensrclist: argument missing for option --meta"<<endl;
	    exit(1);
	 }
      } else if (strcmp(argv[i], "--cachedir") == 0) {
	 i++;
	 if (i < argc) {
            _config->Set("Dir::Cache", argv[i]);
	 } else {
            cout << "genpkglist: argument missing for option --cachedir"<<endl;
	    exit(1);
	 }
      } else {
	 break;
      }
   }
   if (argc - i == 3) {
      arg_dir = argv[i++];
      arg_suffix = argv[i++];
      arg_srpmindex = argv[i++];
   }
   else {
      usage();
      exit(1);
   }
   
   if (!readRPMTable(arg_srpmindex, rpmTable))
       exit(1);
   
   md5cache = new CachedMD5(string(arg_dir)+string(arg_suffix), "gensrclist");

   if(getcwd(cwd, PATH_MAX) == 0)
   {
      cerr << argv[0] << ":" << strerror(errno) << endl;
      exit(1);
   }
   
   if (*arg_dir != '/') {
      strcpy(buf, cwd);
      strcat(buf, "/");
      strcat(buf, arg_dir);
   } else
       strcpy(buf, arg_dir);
   
   strcat(buf, "/SRPMS.");
   strcat(buf, arg_suffix);
   
   srpmdir = "SRPMS." + string(arg_suffix);
#ifdef OLD_FLATSCHEME
   if (flatStructure) {
      // add the last component of the directory to srpmdir
      // that will cancel the effect of the .. used in sourcelist.cc
      // when building the directory from where to fetch srpms in apt
      char *prefix;
      prefix = strrchr(arg_dir, '/');
      if (prefix == NULL)
	 prefix = arg_dir;
      else
	 prefix++;
      if (*prefix != 0 && *(prefix+strlen(prefix)-1) == '/')
	 srpmdir = string(prefix) + srpmdir;
      else
	 srpmdir = string(prefix) + "/" + srpmdir;
   }
#else
   if (!flatStructure)
      srpmdir = "../"+srpmdir;
#ifndef REMOVE_THIS_SOMEDAY
   /* This code is here just so that code in rpmsrcrecords.cc in versions
    * prior to 0.5.15cnc4 is able to detect if that's a "new" style SRPM
    * directory scheme, or an old style. Someday, when 0.5.15cnc4 will be
    * history, this code may be safely removed. */
   else
      srpmdir = "./"+srpmdir;
#endif
#endif
   
   entry_no = scandir(buf, &dirEntries, selectDirent, alphasort);
   if (entry_no < 0) { 
      cerr << "gensrclist: error opening directory " << buf << ":"
	  << strerror(errno) << endl;
      return 1;
   }

   if(chdir(buf) != 0)
   {
      cerr << argv[0] << ":" << strerror(errno) << endl;
      exit(1);
   }
   
   if (srcListSuffix != NULL)
      sprintf(buf, "%s/srclist.%s", cwd, srcListSuffix);
   else
      sprintf(buf, "%s/srclist.%s", cwd, arg_suffix);
   
   if (srcListAppend == true && FileExists(buf)) {
      outfd = fdOpen(buf, O_WRONLY|O_APPEND, 0644);
   } else {
      unlink(buf);
      outfd = fdOpen(buf, O_WRONLY|O_TRUNC|O_CREAT, 0644);
   }
   if (!outfd) {
      cerr << "gensrclist: error creating file" << buf << ":"
	  << strerror(errno);
      return 1;
   }

#if RPM_VERSION >= 0x040100
   rpmReadConfigFiles(NULL, NULL);
   rpmts ts = rpmtsCreate();
   rpmtsSetVSFlags(ts, (rpmVSFlags_e)-1);
#else
   Header sigs;
#endif   
  
   for (entry_cur = 0; entry_cur < entry_no; entry_cur++) {
      struct stat sb;

      if (progressBar) {
         if (entry_cur)
            printf("\b\b\b\b\b\b\b\b\b\b");
         printf(" %04i/%04i", entry_cur + 1, entry_no);
         fflush(stdout);
      }

      if (stat(dirEntries[entry_cur]->d_name, &sb) < 0) {
	 cerr << "\nWarning: " << strerror(errno) << ": " << 
	         dirEntries[entry_cur]->d_name << endl;
	 continue;
      }
      
      fd = fdOpen(dirEntries[entry_cur]->d_name, O_RDONLY, 0666);
	 
      if (!fd) {
	 cerr << "\nWarning: " << strerror(errno) << ": " <<
	         dirEntries[entry_cur]->d_name << endl;
	 continue;
      }

      size[0] = sb.st_size;
	 
#if RPM_VERSION >= 0x040100
      rc = rpmReadPackageFile(ts, fd, dirEntries[entry_cur]->d_name, &h);
      if (rc == RPMRC_OK || rc == RPMRC_NOTTRUSTED || rc == RPMRC_NOKEY) {
#else
      rc = rpmReadPackageInfo(fd, &sigs, &h);
      if (rc == 0) {
#endif
	    Header newHeader;
	    int i;
	    bool foundInIndex;
	    
	    newHeader = headerNew();
	    
	    // the std tags
	    for (i = 0; i < numTags; i++) {
	       int type, count;
	       void *data;
	       int res;
	       
	       // Copy raw entry, so that internationalized strings
	       // will get copied correctly.
	       res = headerGetRawEntry(h, tags[i], &type, &data, &count);
	       if (res != 1)
		  continue;
	       headerAddEntry(newHeader, tags[i], type, data, count);
	    }
	    
	    
	    // our additional tags
	    headerAddEntry(newHeader, CRPMTAG_DIRECTORY, RPM_STRING_TYPE,
			   srpmdir.c_str(), 1);
	    
	    headerAddEntry(newHeader, CRPMTAG_FILENAME, RPM_STRING_TYPE, 
			   dirEntries[entry_cur]->d_name, 1);
	    headerAddEntry(newHeader, CRPMTAG_FILESIZE, RPM_INT32_TYPE,
			   size, 1);
	    
	    {
	       char md5[34];
	       
	       md5cache->MD5ForFile(dirEntries[entry_cur]->d_name, sb.st_mtime, md5);
	       
	       headerAddEntry(newHeader, CRPMTAG_MD5, RPM_STRING_TYPE,
			      md5, 1);
	    }
	    
	    foundInIndex = false;
	    {
	       int count = 0;
	       char **l = NULL;
	       list<char*> *rpmlist = rpmTable[string(dirEntries[entry_cur]->d_name)];
	       
	       if (rpmlist) {
		  l = new char *[rpmlist->size()];
		  
		  foundInIndex = true;
		  
		  for (list<char*>::const_iterator i = rpmlist->begin();
		       i != rpmlist->end();
		       i++) {
		     l[count++] = *i;
		  }
	       }
	       
	       if (count) {
		  headerAddEntry(newHeader, CRPMTAG_BINARY,
				 RPM_STRING_ARRAY_TYPE, l, count);
	       }
	    }
	    if (foundInIndex || !mapi)
		headerWrite(outfd, newHeader, HEADER_MAGIC_YES);
	    
	    headerFree(newHeader);
	    headerFree(h);
#if RPM_VERSION < 0x040100
	    rpmFreeSignature(sigs);
#endif
      } else {
	 cerr << "\nWarning: Skipping malformed RPM: " <<
	         dirEntries[entry_cur]->d_name << endl;
      }
      Fclose(fd);
   } 
   
   Fclose(outfd);

#if RPM_VERSION >= 0x040100
   ts = rpmtsFree(ts);
#endif   
   
   delete md5cache;
   
   return 0;
}

// vim:sts=3:sw=3
