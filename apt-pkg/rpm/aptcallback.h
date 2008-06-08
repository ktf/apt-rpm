#ifndef _APTRPM_RPMCALLBACK_H
#define _APTRPM_RPMCALLBACK_H

#include <apt-pkg/progress.h>
#include <rpm/rpmcli.h>
#include "rapttypes.h"

#if RPM_VERSION < 0x040000
void * rpmCallback(const Header h,
#else
void * rpmCallback(const void * arg, 
#endif
			const rpmCallbackType what,
                        const raptOffset amount,
                        const raptOffset total,
			const void * pkgKey, void * data);

#endif /* _APTRPM_RPMCALLBACK_H */
// vim:sts=3:sw=3

