#include <apt-pkg/error.h>
#include <apt-pkg/configuration.h>
#include <apt-pkg/progress.h>
#include <apt-pkg/init.h>
#include <apt-pkg/fileutl.h>

using namespace std;

int main(int argc,char *argv[])
{
   pkgInitConfig(*_config);
   pkgInitSystem(*_config,_system);

   // do something... 

   _error->DumpErrors();
}
