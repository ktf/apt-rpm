
extern int packagesTotal;

#if RPM_VERSION < 0x040000
void * rpmpmShowProgress(const Header h,
#else
void * rpmpmShowProgress(const void * arg, 
#endif
			const rpmCallbackType what,
#if RPM_VERSION >= 0x040405
                        const unsigned long long amount,
                        const unsigned long long total,
#else
                        const unsigned long amount,
                        const unsigned long total,
#endif
			const void * pkgKey, void * data);
