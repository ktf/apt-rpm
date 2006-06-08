dist_man_MANS = apt.8
MAINTAINERCLEANFILES =
EXTRA_DIST =

EXTRA_DIST += apt-cache.8.sgml
dist_man_MANS += apt-cache.8
if COMPILE_MANPAGES
apt-cache.8: apt-cache.8.sgml apt.ent
	(cd $(srcdir) && $(DOCBOOK2MAN) apt-cache.8.sgml)
	@rm -f $(srcdir)/manpage.*
endif
MAINTAINERCLEANFILES += apt-cache.8

EXTRA_DIST += apt-cdrom.8.sgml
dist_man_MANS += apt-cdrom.8
if COMPILE_MANPAGES
apt-cdrom.8: apt-cdrom.8.sgml apt.ent
	(cd $(srcdir) && $(DOCBOOK2MAN) apt-cdrom.8.sgml)
	@rm -f $(srcdir)/manpage.*
endif
MAINTAINERCLEANFILES += apt-cdrom.8

EXTRA_DIST += apt.conf.5.sgml
dist_man_MANS += apt.conf.5
if COMPILE_MANPAGES
apt.conf.5: apt.conf.5.sgml apt.ent
	(cd $(srcdir) && $(DOCBOOK2MAN) apt.conf.5.sgml)
	@rm -f $(srcdir)/manpage.*
endif
MAINTAINERCLEANFILES += apt.conf.5

EXTRA_DIST += apt-config.8.sgml
dist_man_MANS += apt-config.8
if COMPILE_MANPAGES
apt-config.8: apt-config.8.sgml apt.ent
	(cd $(srcdir) && $(DOCBOOK2MAN) apt-config.8.sgml)
	@rm -f $(srcdir)/manpage.*
endif
MAINTAINERCLEANFILES += apt-config.8

EXTRA_DIST += apt-get.8.sgml
dist_man_MANS += apt-get.8
if COMPILE_MANPAGES
apt-get.8: apt-get.8.sgml apt.ent
	(cd $(srcdir) && $(DOCBOOK2MAN) apt-get.8.sgml)
	@rm -f $(srcdir)/manpage.*
endif
MAINTAINERCLEANFILES += apt-get.8

EXTRA_DIST += apt_preferences.5.sgml
dist_man_MANS += apt_preferences.5
if COMPILE_MANPAGES
apt_preferences.5: apt_preferences.5.sgml apt.ent
	(cd $(srcdir) && $(DOCBOOK2MAN) apt_preferences.5.sgml)
	@rm -f $(srcdir)/manpage.*
endif
MAINTAINERCLEANFILES += apt_preferences.5

EXTRA_DIST += sources.list.5.sgml
dist_man_MANS += sources.list.5
if COMPILE_MANPAGES
sources.list.5: sources.list.5.sgml apt.ent
	(cd $(srcdir) && $(DOCBOOK2MAN) sources.list.5.sgml)
	@rm -f $(srcdir)/manpage.*
endif
MAINTAINERCLEANFILES += sources.list.5

EXTRA_DIST += vendors.list.5.sgml
dist_man_MANS += vendors.list.5
if COMPILE_MANPAGES
vendors.list.5: vendors.list.5.sgml apt.ent
	(cd $(srcdir) && $(DOCBOOK2MAN) vendors.list.5.sgml)
	@rm -f $(srcdir)/manpage.*
endif
MAINTAINERCLEANFILES += vendors.list.5

EXTRA_DIST += \
		apt.ent \
		style.txt \
		cache.sgml \
		files.sgml \
		guide.sgml \
		offline.sgml \
		design.sgml \
		method.sgml \
		examples/apt.conf \
		examples/configure-index \
		examples/sources.list
