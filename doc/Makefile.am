all-local: docs
clean-local: clean-docs

STATIC_MANPAGES = \
		apt.8
SGML_MANPAGES = \
		apt-get.8 \
		apt-cache.8 \
		apt-cdrom.8 \
		apt-config.8 \
		apt.conf.5 \
		sources.list.5 \
		vendors.list.5 \
		apt_preferences.5


EXTRA_DIST = \
		$(STATIC_MANPAGES) \
		$(patsubst %,%.sgml, \
		$(SGML_MANPAGES)) \
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

CLEANFILES = manpage.links manpage.refs

if COMPILE_MANPAGES
man_MANS = $(STATIC_MANPAGES) $(SGML_MANPAGES)

docs: $(man_MANS)

%: %.sgml apt.ent
	@echo Creating man page $@
	@$(DOCBOOK2MAN) $<
else
docs:
endif

clean-docs:
	rm -f $(SGML_MANPAGES)

