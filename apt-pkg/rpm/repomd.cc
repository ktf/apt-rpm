// CNC:2002-07-03

// -*- mode: c++; mode: fold -*-
// Description								/*{{{*/
// $Id: repository.cc,v 1.4 2002/07/29 18:13:52 niemeyer Exp $
/* ######################################################################

   Repository abstraction for 1 or more unique URI+Dist
   
   ##################################################################### */
									/*}}}*/
// Include Files							/*{{{*/
#ifdef __GNUG__
#pragma implementation "apt-pkg/repomd.h"
#endif

#include <config.h>

#ifdef APT_WITH_REPOMD

#include <iostream>
#include <cstring>

#include <apt-pkg/error.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include "xmlutil.h"
#include "repomd.h"

#include <apti18n.h>


using namespace std;

// Parse repomd.xml file for checksums
bool repomdRepository::ParseRelease(string File)
{
   RepoMD = xmlReadFile(File.c_str(), NULL, XML_PARSE_NONET);
   if ((Root = xmlDocGetRootElement(RepoMD)) == NULL) {
      xmlFreeDoc(RepoMD);
      return _error->Error(_("Could not open metadata file '%s'"),File.c_str());
   }

   for (xmlNode *Node = Root->children; Node; Node = Node->next) {
      if (Node->type != XML_ELEMENT_NODE || 
	  strcmp((char*)Node->name, "data") != 0)
	 continue;

      string Hash = "";
      string RealPath = "";
      string Path = "";
      string ChecksumType = "";
      string DataType = "";
      string TimeStamp = "";
      xmlNode *n = NULL;

      xmlChar *type = xmlGetProp(Node, (xmlChar*)"type");
      DataType = (char*)type;
      xmlFree(type);

      // If it's a sqlite _db file, see if we can use it and skip otherwise
      if (DataType.substr(DataType.size()-3, 3) == "_db") {
	 n = XmlFindNode(Node, "database_version");
	 bool db_supported = false;
	 if (n) {
	    xmlChar *x = xmlNodeGetContent(n);
	    int dbver = atoi((char*)x);
	    xmlFree(x);
	    // XXX should ask about supported version from sqlite handler
	    // or something instead of hardcoding in several places...
	    if (dbver == 10) {
	       db_supported = true;
	    }
	 } 
	 if (db_supported == false) {
	    continue;
	 }
      }

      n = XmlFindNode(Node, "location");
      if (n) {
	 xmlChar *href = xmlGetProp(n, (xmlChar*)"href");
	 Path = (char*)href;
	 xmlFree(href);
      }

      n = NULL;
      RealPath = Path;
      if (flExtension(Path) == "gz" || flExtension(Path) == "bz2") {
	 Path = Path.substr(0, Path.size()-flExtension(Path).size()-1);
	 n = XmlFindNode(Node, "open-checksum");
      } else {
	 n = XmlFindNode(Node, "checksum");
      }
      if (n) {
	 xmlChar *hash = xmlNodeGetContent(n);
	 xmlChar *type = xmlGetProp(n, (xmlChar*)"type");
	 Hash = (char*)hash;
	 ChecksumType = (char*)type;
	 xmlFree(hash);
	 xmlFree(type);
      }

      IndexChecksums[Path].MD5 = Hash;
      IndexChecksums[Path].Size = 0;
      RepoFiles[DataType].Path = Path;
      RepoFiles[DataType].RealPath = RealPath;
      RepoFiles[DataType].TimeStamp = TimeStamp;
      if (ChecksumType == "sha") {
	 CheckMethod = "SHA1-Hash";
      } else {
	 CheckMethod = "MDA5-Hash";
      }
   }
   GotRelease = true;

   xmlFreeDoc(RepoMD);
   return true;
}

string repomdRepository::FindURI(string DataType)
{
   string Res = "";
   if (RepoFiles.find(DataType) != RepoFiles.end()) {
        Res = RepoFiles[DataType].Path;
   }
   return Res;
}

string repomdRepository::GetComprMethod(string URI)
{
   string Res = "";
   string Path = string(URI,RootURI.size());
   
   map<string,RepoFile>::iterator I;
   for (I = RepoFiles.begin(); I != RepoFiles.end(); I++) {
      if (Path == flUnCompressed(I->second.RealPath)) {
	 Res = flExtension(I->second.RealPath);
      }
   }
   return Res;
}
   

#endif /* APT_WITH_REPOMD */

// vim:sts=3:sw=3
