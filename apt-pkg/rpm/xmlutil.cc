
#include "xmlutil.h"
#include <string>

#ifdef APT_WITH_REPOMD
xmlNode *XmlFindNode(xmlNode *Node, const string Name)
{
   for (xmlNode *n = Node->children; n; n = n->next) {
      if (xmlStrcmp(n->name, (xmlChar*)Name.c_str()) == 0) {
         return n;
      }
   }
   return NULL;
}

string XmlFindNodeContent(xmlNode *Node, string Name)
{
   xmlNode *n = XmlFindNode(Node, Name);
   return XmlGetContent(n);
}

string XmlGetContent(xmlNode *Node)
{
   string str = "";
   if (Node) {
      xmlChar *content = xmlNodeGetContent(Node);
      if (content) {
         str = (char*)content;
         xmlFree(content);
      }
   }
   return str;
}

string XmlGetProp(xmlNode *Node, string Prop)
{
   string str = "";
   if (Node) {
      xmlChar *prop = xmlGetProp(Node, (xmlChar*)Prop.c_str());
      if (prop) {
         str = (char*)prop;
         xmlFree(prop);
      }
   }
   return str;
}

#endif

// vim:sts=3:sw=3
