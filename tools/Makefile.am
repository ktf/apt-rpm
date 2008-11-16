
bin_PROGRAMS = genpkglist gensrclist countpkglist
bin_SCRIPTS = genbasedir

EXTRA_DIST = genbasedir

LDADD = ../apt-pkg/libapt-pkg.la $(RPM_LIBS)

genpkglist_SOURCES = genpkglist.cc cached_md5.cc cached_md5.h genutil.h
gensrclist_SOURCES = gensrclist.cc cached_md5.cc cached_md5.h genutil.h
countpkglist_SOURCES = countpkglist.cc
