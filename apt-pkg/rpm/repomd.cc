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
      return _error->Error(_("could not open Release file '%s'"),File.c_str());
   }

   for (xmlNode *Node = Root->children; Node; Node = Node->next) {
      if (Node->type != XML_ELEMENT_NODE || 
	  strcmp((char*)Node->name, "data") != 0)
	 continue;

      string Hash = "";
      string Path = "";
      string Type = "";
      string Timestamp = "";
      xmlNode *n = NULL;

      n = FindNode(Node, "location");
      if (n) {
	 xmlChar *href = xmlGetProp(n, (xmlChar*)"href");
	 Path = (char*)href;
	 xmlFree(href);
      }

      n = NULL;
      if (flExtension(Path) == "gz") {
	 Path = Path.substr(0, Path.size()-3);
	 n = FindNode(Node, "open-checksum");
      } else {
	 n = FindNode(Node, "checksum");
      }
      if (n) {
	 xmlChar *hash = xmlNodeGetContent(n);
	 xmlChar *type = xmlGetProp(n, (xmlChar*)"type");
	 Hash = (char*)hash;
	 Type = (char*)type;
	 xmlFree(hash);
	 xmlFree(type);
      }

      IndexChecksums[Path].MD5 = Hash;
      IndexChecksums[Path].Size = 0;
      if (Type == "sha") {
	 CheckMethod = "SHA1-Hash";
      } else {
	 CheckMethod = "MDA5-Hash";
      }
   }
   
   GotRelease = true;

   xmlFreeDoc(RepoMD);
   return true;
}

#endif /* APT_WITH_REPOMD */

// vim:sts=3:sw=3
