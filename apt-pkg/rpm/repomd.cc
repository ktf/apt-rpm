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
#include <apt-pkg/repomd.h>
#include <apt-pkg/error.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

#include <apti18n.h>

using namespace std;

xmlNode *FindNode(xmlNode *Node, const string Name)
{
   for (xmlNode *n = Node->children; n; n = n->next) {
      if (strcmp((char*)n->name, Name.c_str()) == 0) {
         return n;
      }
   }
   return NULL;
}


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
      string Path = "";
      string ChecksumType = "";
      string DataType = "";
      string TimeStamp = "";
      xmlNode *n = NULL;

      xmlChar *type = xmlGetProp(Node, (xmlChar*)"type");
      DataType = (char*)type;

      n = FindNode(Node, "location");
      if (n) {
	 xmlChar *href = xmlGetProp(n, (xmlChar*)"href");
	 Path = (char*)href;
	 xmlFree(href);
      }

      n = NULL;
      if (flExtension(Path) == "gz" || flExtension(Path) == "bz2") {
	 Path = Path.substr(0, Path.size()-flExtension(Path).size()-1);
	 n = FindNode(Node, "open-checksum");
      } else {
	 n = FindNode(Node, "checksum");
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
      RepoFiles[DataType].TimeStamp = TimeStamp;
      if (ChecksumType == "sha") {
	 CheckMethod = "SHA1-Hash";
      } else {
	 CheckMethod = "MDA5-Hash";
      }
      xmlFree(type);
   }
   
   GotRelease = true;
   if (RepoFiles.find("primary_db") != RepoFiles.end()) {
      ComprMethod = "bz2";
   } else {
      ComprMethod = "gz";
   }

   xmlFreeDoc(RepoMD);
   return true;
}

bool repomdRepository::FindURI(string DataType, string &URI)
{
   bool found = false;
   if (RepoFiles.find(DataType) != RepoFiles.end()) {
        URI = RepoFiles[DataType].Path;
        found = true;
   }
   return found;
}


#endif /* APT_WITH_REPOMD */

// vim:sts=3:sw=3
