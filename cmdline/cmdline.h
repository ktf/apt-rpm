// -*- mode: c++; mode: fold -*-

#include <apt-pkg/algorithms.h>

#include <fstream>
#include <stdio.h>

using std::ostream;
using std::ofstream;

static ostream c0out(0);
static ostream c1out(0);
static ostream c2out(0);
static ofstream devnull("/dev/null");
static unsigned int ScreenWidth = 80;

bool YnPrompt();
bool AnalPrompt(const char *Text);
void SigWinch(int);
bool ShowList(ostream &out,string Title,string List,string VersionsList);

const char *op2str(int op);

