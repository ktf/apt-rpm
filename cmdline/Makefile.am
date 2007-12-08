
EXTRA_DIST = indexcopy.cc indexcopy.h

bin_PROGRAMS = apt-get apt-cache apt-cdrom apt-config

if COMPILE_APTSHELL
bin_PROGRAMS += apt-shell
endif
if COMPILE_STATIC
bin_PROGRAMS += apt-get-static apt-cache-static apt-cdrom-static
endif

LDADD = ../apt-pkg/libapt-pkg.la $(RPM_LIBS)

apt_get_SOURCES = apt-get.cc acqprogress.cc acqprogress.h cmdline.cc cmdline.h
apt_cache_SOURCES = apt-cache.cc cmdline.cc cmdline.h
apt_shell_SOURCES = apt-shell.cc acqprogress.cc acqprogress.h cmdline.cc cmdline.h
apt_shell_LDADD = $(LDADD) -lreadline -lncurses
apt_config_SOURCES = apt-config.cc
apt_cdrom_SOURCES = apt-cdrom.cc rpmindexcopy.cc rpmindexcopy.h

apt_get_static_SOURCES = $(apt_get_SOURCES)
apt_get_static_LDFLAGS = -all-static
apt_cache_static_SOURCES = $(apt_cache_SOURCES)
apt_cache_static_LDFLAGS = -all-static
apt_cdrom_static_SOURCES = $(apt_cdrom_SOURCES)
apt_cdrom_static_LDFLAGS = -all-static

