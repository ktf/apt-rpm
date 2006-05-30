// -*- mode: c++; mode: fold -*-
#include <apt-pkg/configuration.h>
#include <apt-pkg/error.h>

#include <config.h>
#include <apti18n.h>

#include "cmdline.h"

#include <regex.h>

using namespace std;

// YnPrompt - Yes No Prompt.						/*{{{*/
// ---------------------------------------------------------------------
/* Returns true on a Yes.*/
bool YnPrompt()
{
   if (_config->FindB("APT::Get::Assume-Yes",false) == true)
   {
      c1out << _("Y") << endl;
      return true;
   }

   char response[1024] = "";
   cin.getline(response, sizeof(response));

   if (!cin)
      return false;

   if (strlen(response) == 0)
      return true;

   regex_t Pattern;
   int Res;

   Res = regcomp(&Pattern, nl_langinfo(YESEXPR),
                 REG_EXTENDED|REG_ICASE|REG_NOSUB);

   if (Res != 0) {
      char Error[300];        
      regerror(Res,&Pattern,Error,sizeof(Error));
      return _error->Error(_("Regex compilation error - %s"),Error);
   }
   
   Res = regexec(&Pattern, response, 0, NULL, 0);
   if (Res == 0)
      return true;
   return false;
}
									/*}}}*/
// AnalPrompt - Annoying Yes No Prompt.					/*{{{*/
// ---------------------------------------------------------------------
/* Returns true on a Yes.*/
bool AnalPrompt(const char *Text)
{
   char Buf[1024];
   cin.getline(Buf,sizeof(Buf));
   if (strcmp(Buf,Text) == 0)
      return true;
   return false;
}
									/*}}}*/
// ShowList - Show a list						/*{{{*/
// ---------------------------------------------------------------------
/* This prints out a string of space separated words with a title and 
   a two space indent line wraped to the current screen width. */
bool ShowList(ostream &out,string Title,string List,string VersionsList)
{
   if (List.empty() == true)
      return true;
   // trim trailing space
   int NonSpace = List.find_last_not_of(' ');
   if (NonSpace != -1)
   {
      List = List.erase(NonSpace + 1);
      if (List.empty() == true)
	 return true;
   }

   // Acount for the leading space
   int ScreenWidth = ::ScreenWidth - 3;
      
   out << Title << endl;
   string::size_type Start = 0;
   string::size_type VersionsStart = 0;
   while (Start < List.size())
   {
      if(_config->FindB("APT::Get::Show-Versions",false) == true &&
         VersionsList.size() > 0) {
         string::size_type End;
         string::size_type VersionsEnd;
         
         End = List.find(' ',Start);
         VersionsEnd = VersionsList.find('\n', VersionsStart);

         out << "   " << string(List,Start,End - Start) << " (" << 
            string(VersionsList,VersionsStart,VersionsEnd - VersionsStart) << 
            ")" << endl;

	 if (End == string::npos || End < Start)
	    End = Start + ScreenWidth;

         Start = End + 1;
         VersionsStart = VersionsEnd + 1;
      } else {
         string::size_type End;

         if (Start + ScreenWidth >= List.size())
            End = List.size();
         else
            End = List.rfind(' ',Start+ScreenWidth);

         if (End == string::npos || End < Start)
            End = Start + ScreenWidth;
         out << "  " << string(List,Start,End - Start) << endl;
         Start = End + 1;
      }
   }   

   return false;
}

const char *op2str(int op)
{
   switch (op & 0x0f)
   {
      case pkgCache::Dep::LessEq: return "<=";
      case pkgCache::Dep::GreaterEq: return ">=";
      case pkgCache::Dep::Less: return "<";
      case pkgCache::Dep::Greater: return ">";
      case pkgCache::Dep::Equals: return "=";
      case pkgCache::Dep::NotEquals: return "!";
      default: return "";
   }
}

// SigWinch - Window size change signal handler                         /*{{{*/
// ---------------------------------------------------------------------
/* */
void SigWinch(int)
{
   // Riped from GNU ls
#ifdef TIOCGWINSZ
   struct winsize ws;

   if (ioctl(1, TIOCGWINSZ, &ws) != -1 && ws.ws_col >= 5)
      ScreenWidth = ws.ws_col - 1;
#endif
}

// vim:sts=3:sw=3
