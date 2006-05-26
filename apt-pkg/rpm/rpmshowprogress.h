
static int hashesTotal = 0;
static int hashesCurrent = 0;
static int progressTotal = 0;
static int progressCurrent = 0;
extern int packagesTotal;

static void printHash(const unsigned long amount, const unsigned long total);

#if RPM_VERSION < 0x040000
void * rpmpmShowProgress(const Header h,
#else
void * rpmpmShowProgress(const void * arg, 
#endif
			const rpmCallbackType what,
			const unsigned long amount,
			const unsigned long total,
			const void * pkgKey, void * data);
