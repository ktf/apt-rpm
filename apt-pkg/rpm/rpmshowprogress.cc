
#include <stdio.h>
#include <rpm/rpmlib.h>
#include <apti18n.h>

#include "rpmshowprogress.h"
//
// This code was originally from rpm 4.0.3.
//
static void printHash(const unsigned long amount, const unsigned long total)
{
    int hashesNeeded;
    int hashesTotal = 50;

    if (isatty (STDOUT_FILENO))
	hashesTotal = 44;

    if (hashesPrinted != hashesTotal) {
	hashesNeeded = (int)(hashesTotal*(total?(((float)amount)/total):1));
	while (hashesNeeded > hashesPrinted) {
	    if (isatty (STDOUT_FILENO)) {
		int i;
		for (i = 0; i < hashesPrinted; i++) (void) putchar ('#');
		for (; i < hashesTotal; i++) (void) putchar (' ');
		fprintf(stdout, "(%3d%%)",
			(int)(100 * (total ? (((float) amount) / total) : 1)));
		for (i = 0; i < (hashesTotal + 6); i++) (void) putchar ('\b');
	    } else
		fprintf(stdout, "#");

	    hashesPrinted++;
	}
	(void) fflush(stdout);
	hashesPrinted = hashesNeeded;

	if (hashesPrinted == hashesTotal) {
	    int i;
	    progressCurrent++;
	    if (isatty(STDOUT_FILENO)) {
	        for (i = 1; i < hashesPrinted; i++) (void) putchar ('#');
		fprintf(stdout, " [%3d%%]", (int)(100 * (progressTotal ?
			(((float) progressCurrent) / progressTotal) : 1)));
	    }
	    fprintf(stdout, "\n");
	}
	(void) fflush(stdout);
    }
}

#if RPM_VERSION < 0x040000
void * rpmpmShowProgress(const Header h,
#else
void * rpmpmShowProgress(const void * arg, 
#endif
			const rpmCallbackType what,
			const unsigned long amount,
			const unsigned long total,
			const void * pkgKey, void * data)
{
#if RPM_VERSION >= 0x040000
    Header h = (Header) arg;
#endif

    char * s;
    int flags = (int) ((long)data);
    void * rc = NULL;
    const char * filename = (const char *) pkgKey;
    static FD_t fd = NULL;
    static rpmCallbackType state;

    switch (what) {
    case RPMCALLBACK_INST_OPEN_FILE:
	if (filename == NULL || filename[0] == '\0')
	    return NULL;
	fd = Fopen(filename, "r.ufdio");
	if (fd)
	    fd = fdLink(fd, "persist (showProgress)");
	return fd;
	/*@notreached@*/ break;

    case RPMCALLBACK_INST_CLOSE_FILE:
	fd = fdFree(fd, "persist (showProgress)");
	if (fd) {
	    (void) Fclose(fd);
	    fd = NULL;
	}
	break;

    case RPMCALLBACK_INST_START:
	hashesPrinted = 0;
	if (h == NULL || !(flags & INSTALL_LABEL))
	    break;

	if (state != what) {
	    state = what;
    	    fprintf(stdout, "%s\n", _("Installing / Updating..."));
	    (void) fflush(stdout);
	}

	if (flags & INSTALL_HASH) {
	    s = headerSprintf(h, "%{NAME}.%{ARCH}",
				rpmTagTable, rpmHeaderFormats, NULL);
	    if (isatty (STDOUT_FILENO))
		fprintf(stdout, "%4d:%-23.23s", progressCurrent + 1, s);
	    else
		fprintf(stdout, "%-28.28s", s);
	    (void) fflush(stdout);
	    free(s);
	    s = NULL;
	} else {
	    s = headerSprintf(h, "%{NAME}-%{VERSION}-%{RELEASE}.%{ARCH}",
				  rpmTagTable, rpmHeaderFormats, NULL);
	    fprintf(stdout, "%s\n", s);
	    (void) fflush(stdout);
	    free(s);
	    s = NULL;
	}
	break;

    case RPMCALLBACK_TRANS_PROGRESS:
    case RPMCALLBACK_INST_PROGRESS:
	if (flags & INSTALL_PERCENT)
	    fprintf(stdout, "%%%% %f\n", (double) (total
				? ((((float) amount) / total) * 100)
				: 100.0));
	else if (flags & INSTALL_HASH)
	    printHash(amount, total);
	(void) fflush(stdout);
	break;

    case RPMCALLBACK_TRANS_START:
	state = what;
	hashesPrinted = 0;
	progressTotal = 1;
	progressCurrent = 0;
	if (!(flags & INSTALL_LABEL))
	    break;
	if (flags & INSTALL_HASH)
	    fprintf(stdout, "%-28s", _("Preparing..."));
	else
	    fprintf(stdout, "%s\n", _("Preparing..."));
	(void) fflush(stdout);
	break;

    case RPMCALLBACK_TRANS_STOP:
	if (flags & INSTALL_HASH)
	    printHash(1, 1);	/* Fixes "preparing..." progress bar */
	progressTotal = packagesTotal;
	progressCurrent = 0;
	break;

    case RPMCALLBACK_UNINST_PROGRESS:
	break;
    case RPMCALLBACK_UNINST_START:
	hashesPrinted = 0;
	if (!(flags & INSTALL_LABEL))
	    break;
	if (state != what) {
	    state = what;
    	    fprintf(stdout, "%s\n", _("Removing / Cleaning up..."));
	    (void) fflush(stdout);
	}
	break;

    case RPMCALLBACK_UNINST_STOP:
	if (h == NULL || !(flags & INSTALL_LABEL))
	    break;
	s = headerSprintf(h, "%{NAME}.%{ARCH}", rpmTagTable, rpmHeaderFormats, NULL);
	if (flags & INSTALL_HASH) {
	    fprintf(stdout, "%4d:%-23.23s", progressCurrent + 1, s);
	} else {
	    fprintf(stdout, "%-28.28s", s);
	}
	printHash(1, 1);
	fflush(stdout);
	s = NULL;
	break;
    }
 
    // TODO: add repackage callbacks

    return rc;
}	

