
extern int packagesTotal;

#if RPM_VERSION < 0x040000
void * rpmpmShowProgress(const Header h,
#else
void * rpmpmShowProgress(const void * arg, 
#endif
			const rpmCallbackType what,
			const unsigned long amount,
			const unsigned long total,
			const void * pkgKey, void * data);
