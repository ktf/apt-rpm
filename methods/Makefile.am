
methodsdir=${libdir}/apt/methods
methods_PROGRAMS = cdrom copy file ftp gpg gzip bzip2 http rsh ssh

LDADD = ../apt-pkg/libapt-pkg.la

cdrom_SOURCES = cdrom.cc
copy_SOURCES = copy.cc
file_SOURCES = file.cc
gpg_SOURCES = gpg.cc
gzip_SOURCES = gzip.cc
bzip2_SOURCES = $(gzip_SOURCES)
rsh_SOURCES = rsh.cc rsh.h
ssh_SOURCES = $(rsh_SOURCES)

http_SOURCES = \
	       http.cc \
	       http.h \
	       rfc2553emu.cc \
	       rfc2553emu.h \
	       connect.cc \
	       connect.h
http_LDADD = $(LDADD) @SOCKETLIBS@

ftp_SOURCES = \
	       ftp.cc \
	       ftp.h \
	       rfc2553emu.cc \
	       rfc2553emu.h \
	       connect.cc \
	       connect.h
ftp_LDADD = $(LDADD) @SOCKETLIBS@
