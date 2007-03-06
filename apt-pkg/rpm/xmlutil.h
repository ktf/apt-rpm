#ifndef XMLUTIL_H
#define XMLUTIL_H

#include <apt-pkg/aptconf.h>

#ifdef APT_WITH_REPOMD
#include <string>
#include <libxml/parser.h>
#include <libxml/tree.h>

using std::string;

xmlNode *XmlFindNode(xmlNode *Node, const string Name);
string XmlFindNodeContent(xmlNode *Node, string Name);
string XmlGetContent(xmlNode *Node);
string XmlGetProp(xmlNode *Node, string Prop);
#endif

#endif

// vim:sts=3:sw=3
