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
		guide.it.sgml \
		method.sgml \
		examples/apt.conf \
		examples/configure-index \
		examples/sources.list \
		fr/apt-cache.fr.8.sgml \
		fr/apt-cdrom.fr.8.sgml \
		fr/apt-config.fr.8.sgml \
		fr/apt-extracttemplates.fr.1.sgml \
		fr/apt-ftparchive.fr.1.sgml \
		fr/apt-get.fr.8.sgml \
		fr/apt-sortpkgs.fr.1.sgml \
		fr/apt.conf.fr.5.sgml \
		fr/apt.ent.fr \
		fr/apt_preferences.fr.5.sgml \
		fr/makefile \
		fr/sources.list.fr.5.sgml \
		fr/vendors.list.fr.5.sgml \
		es/apt-cache.es.8.sgml \
		es/apt-cdrom.es.8.sgml \
		es/apt-config.es.8.sgml \
		es/apt-get.es.8.sgml \
		es/apt.conf.es.5.sgml \
		es/apt.ent.es \
		es/apt.es.8 \
		es/apt_preferences.es.5.sgml \
		es/makefile \
		es/sources.list.es.5.sgml \
		es/vendors.list.es.5.sgml \
		pt_BR/apt.ent.pt_BR \
		pt_BR/apt_preferences.pt_BR.5.sgml \
		pt_BR/makefile
