localedir = $(datadir)/locale

lib_LTLIBRARIES = libapt-pkg.la

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libapt-pkg.pc

libapt_pkg_la_LIBADD = @RPM_LIBS@
libapt_pkg_la_LDFLAGS = -version-info 3:0:0

AM_CPPFLAGS = -DLIBDIR=\"$(libdir)\" -DPKGDATADIR=\"$(pkgdatadir)\"
AM_CPPFLAGS += -DLOCALEDIR=\"$(localedir)\" -DAPT_DOMAIN=\"$(PACKAGE)\"
AM_CPPFLAGS += -I$(top_srcdir)/lua/include -I$(top_srcdir)/lua/local

if WITH_LUA
libapt_pkg_la_LIBADD += $(top_builddir)/lua/liblua.la
endif

libapt_pkg_la_SOURCES = \
	contrib/cdromutl.cc \
	contrib/cdromutl.h \
	contrib/cmndline.cc \
	contrib/cmndline.h \
	contrib/configuration.cc \
	contrib/configuration.h \
	contrib/crc-16.cc \
	contrib/crc-16.h \
	contrib/error.cc \
	contrib/error.h \
	contrib/fileutl.cc \
	contrib/fileutl.h \
	contrib/hashes.cc \
	contrib/hashes.h \
	contrib/md5.cc \
	contrib/md5.h \
	contrib/mmap.cc \
	contrib/mmap.h \
	contrib/progress.cc \
	contrib/progress.h \
	contrib/sha1.cc \
	contrib/sha1.h \
	contrib/sptr.h \
	contrib/sqlite.h \
	contrib/sqlite.cc \
	contrib/strutl.cc \
	contrib/strutl.h \
	contrib/system.h \
	rpm/repomd.cc \
	rpm/repomd.h \
	rpm/aptcallback.h \
	rpm/aptcallback.cc \
	rpm/rpmhandler.cc \
	rpm/rpmhandler.h \
	rpm/rpmindexfile.cc \
	rpm/rpmindexfile.h \
	rpm/rpmlistparser.cc \
	rpm/rpmlistparser.h \
	rpm/rpmmisc.h \
	rpm/rpmpackagedata.cc \
	rpm/rpmpackagedata.h \
	rpm/rpmpm.cc \
	rpm/rpmpm.h \
	rpm/rpmrecords.cc \
	rpm/rpmrecords.h \
	rpm/rpmsrcrecords.cc \
	rpm/rpmsrcrecords.h \
	rpm/rpmsystem.cc \
	rpm/rpmsystem.h \
	rpm/rpmversion.cc \
	rpm/rpmversion.h \
	rpm/xmlutil.cc \
	rpm/xmlutil.h \
	acquire.cc \
	acquire.h \
	acquire-item.cc \
	acquire-item.h \
	acquire-method.cc \
	acquire-method.h \
	acquire-worker.cc \
	acquire-worker.h \
	algorithms.cc \
	algorithms.h \
	cachefile.cc \
	cachefile.h \
	cacheiterators.h \
	clean.cc \
	clean.h \
	depcache.cc \
	depcache.h \
	indexfile.cc \
	indexfile.h \
	init.cc \
	init.h \
	luaiface.cc \
	luaiface.h \
	orderlist.cc \
	orderlist.h \
	packagemanager.cc \
	packagemanager.h \
	pkgcache.cc \
	pkgcachegen.cc \
	pkgcachegen.h \
	pkgcache.h \
	pkgrecords.cc \
	pkgrecords.h \
	pkgsystem.cc \
	pkgsystem.h \
	policy.cc \
	policy.h \
	repository.cc \
	repository.h \
	sourcelist.cc \
	sourcelist.h \
	srcrecords.cc \
	srcrecords.h \
	tagfile.cc \
	tagfile.h \
	version.cc \
	version.h \
	versionmatch.cc \
	versionmatch.h

PRIVATEHEADERS = \
	contrib/system.h \
	contrib/sqlite.h \
	rpm/repomd.h \
	rpm/aptcallback.h \
	rpm/rpmhandler.h \
	rpm/rpmindexfile.h \
	rpm/rpmlistparser.h \
	rpm/rpmmisc.h \
	rpm/rpmpackagedata.h \
	rpm/rpmpm.h \
	rpm/rpmrecords.h \
	rpm/rpmsrcrecords.h \
	rpm/rpmsystem.h \
	rpm/rpmversion.h \
	rpm/xmlutil.h 

includedir=${prefix}/include/apt-pkg
include_HEADERS = $(filter-out \
		    $(PRIVATEHEADERS), \
		    $(filter %.h,$(libapt_pkg_la_SOURCES)))

# This is a hack to allow us to maintain the original
# APT system of a a flat apt-pkg/ header directory.
PRIVREALHEADERS = $(PRIVATEHEADERS)
PRIVCOPYHEADERS = $(patsubst %.h, \
		    $(top_builddir)/include/%.h, \
		    $(notdir $(PRIVREALHEADERS)))
REALHEADERS = $(filter-out \
		$(PRIVREALHEADERS), \
		$(filter %.h,$(libapt_pkg_la_SOURCES)))
COPYHEADERS = $(patsubst %.h, \
		$(top_builddir)/include/apt-pkg/%.h, \
		$(notdir $(REALHEADERS)))
REALOBJECTS = $(patsubst %.cc,%.lo,\
	      	$(filter %.cc,$(libapt_pkg_la_SOURCES)))
$(REALOBJECTS): $(COPYHEADERS) $(PRIVCOPYHEADERS)

$(top_builddir)/include/apt-pkg/%: %
	@$(mkinstalldirs) $(top_builddir)/include/apt-pkg
	cp -f -p $< $@
$(top_builddir)/include/apt-pkg/%: contrib/%
	@$(mkinstalldirs) $(top_builddir)/include/apt-pkg
	cp -f -p $< $@
$(top_builddir)/include/apt-pkg/%: rpm/%
	@$(mkinstalldirs) $(top_builddir)/include/apt-pkg
	cp -f -p $< $@
$(top_builddir)/include/%: %
	@$(mkinstalldirs) $(top_builddir)/include/apt-pkg
	cp -f -p $< $@
$(top_builddir)/include/%: contrib/%
	@$(mkinstalldirs) $(top_builddir)/include/apt-pkg
	cp -f -p $< $@
$(top_builddir)/include/%: rpm/%
	@$(mkinstalldirs) $(top_builddir)/include/apt-pkg
	cp -f -p $< $@

DISTCLEANFILES = libapt-pkg.pc
EXTRA_DIST = libapt-pkg.pc.in

clean-local:
	rm -f $(PRIVCOPYHEADERS)
	rm -rf $(top_builddir)/include/apt-pkg

