dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([apt],[0.5.15lorg3.92],[apt-rpm@lists.laiskiainen.org])
dnl Check the archs, we want the target type.
AC_CONFIG_AUX_DIR([buildlib])
AC_CANONICAL_HOST
AC_CONFIG_SRCDIR([AUTHORS.RPM])
AM_INIT_AUTOMAKE([foreign subdir-objects dist-bzip2 1.9.5])
AM_MAINTAINER_MODE

AC_CONFIG_HEADER([include/config.h:buildlib/config.h.in])
AC_CONFIG_HEADER([include/apt-pkg/aptconf.h:apt-pkg/aptconf.h.in])
AC_PROG_LIBTOOL

dnl Check our C compiler
AC_PROG_CC
AM_PROG_CC_C_O
AC_ISC_POSIX

dnl Check for other programs
AC_PROG_CXX
AC_PROG_CPP

dnl Checks for sockets
SAVE_LIBS="$LIBS"
LIBS=""
AC_SEARCH_LIBS(gethostbyname,nsl)
AC_SEARCH_LIBS(connect,socket)
SOCKETLIBS="$LIBS"
AC_SUBST(SOCKETLIBS)
LIBS="$SAVE_LIBS"
 
dnl Checks for pthread -- disabled due to glibc bugs jgg
dnl AC_CHECK_LIB(pthread, pthread_create,[AC_DEFINE(HAVE_PTHREAD) PTHREADLIB="-lpthread"])
AC_SUBST(PTHREADLIB)
dnl if test "$PTHREADLIB" != "-lpthread"; then
dnl   AC_MSG_ERROR(failed: I need posix threads, pthread)
dnl fi

dnl Check for RPM executable path
AC_PATH_PROG(RPM_PATH,rpm,none)
if test "$RPM_PATH" = "none"; then
   AC_MSG_ERROR([RPM executable was not found in your system])
else
   AC_DEFINE_UNQUOTED([HAVE_RPM],1,[Define if RPM backend is available])

   dnl Check for RPM version
   AC_MSG_CHECKING(for RPM version)
   RPM_VERSION_RAW=`LANG= LC_ALL= $RPM_PATH --version | sed 's/^RPM version //'`
   RPM_VERSION_MAJOR=`echo $RPM_VERSION_RAW | cut -d. -f1`
   RPM_VERSION_MINOR=`echo $RPM_VERSION_RAW | cut -d. -f2`
   RPM_VERSION_MICRO=`echo $RPM_VERSION_RAW | cut -d. -f3`
   if test -z "$RPM_VERSION_MAJOR" -o -z "$RPM_VERSION_MINOR"; then
      AC_MSG_ERROR([can't parse RPM version])
   fi
   if test -z "$RPM_VERSION_MICRO"; then
      RPM_VERSION_MICRO=0
   fi
   AC_DEFINE_UNQUOTED([RPM_VERSION_MAJOR],$RPM_VERSION_MAJOR,[RPM major version])
   AC_DEFINE_UNQUOTED([RPM_VERSION_MINOR],$RPM_VERSION_MINOR,[RPM minor version])
   AC_DEFINE_UNQUOTED([RPM_VERSION_MICRO],$RPM_VERSION_MICRO,[RPM micro version])
   AC_DEFINE_UNQUOTED([RPM_VERSION],
      [((RPM_VERSION_MAJOR<<16)|(RPM_VERSION_MINOR<<8)|RPM_VERSION_MICRO)],
      [Define to the RPM version])
   AC_DEFINE_UNQUOTED([RPM_VERSION_RAW],"$RPM_VERSION_RAW",[RPM raw version])
   AC_MSG_RESULT($RPM_VERSION_RAW)

   dnl Check for RPM libraries and headers

   AC_CHECK_LIB(bz2,BZ2_bzopen, [],
		[AC_MSG_ERROR([Can't find libbz2 library])])
   AC_CHECK_LIB(z,gzopen, [],
		[AC_MSG_ERROR([Can't find libz library])])
   AC_CHECK_HEADERS(db1/db.h)

   CPPFLAGS="$CPPFLAGS -I/usr/include/rpm"
   AC_CHECK_HEADER(rpm/rpmlib.h, [],
		   [AC_MSG_ERROR([Can't find rpmlib.h])])

   if test $RPM_VERSION_MAJOR -lt 4; then
      RPMLIBS="-lrpm -lz -lbz2 -lpopt"
   else
      AC_CHECK_LIB(rpmdb,rpmdbOpen,
         [RPMDBLIBS="-lrpmdb"],
         [RPMDBLIBS="-ldb-3.1"],
         [-lrpm -lrpmio -lz -lbz2 -lpopt])
      RPMLIBS="-lrpm $RPMDBLIBS -lrpmio -lz -lbz2 -lpopt"
   fi
   AC_MSG_CHECKING(for RPM libraries)
   AC_MSG_RESULT($RPMLIBS)
   AC_SUBST(RPMLIBS)
fi

dnl Can we use g++ hash_map?
AH_TEMPLATE(APT_WITH_GNU_HASH_MAP,
	    [Define to 1 if you want to use the g++ hash_map extension])
AC_MSG_CHECKING(for g++ hash_map extension)
AC_LANG_PUSH([C++])
AC_TRY_LINK(
[
#include <ext/hash_map>
using namespace __gnu_cxx;
],[hash_map<char*,hash<char*> > HashMap;
],[AC_DEFINE_UNQUOTED(APT_WITH_GNU_HASH_MAP, 1) AC_MSG_RESULT(yes)
],[AC_MSG_RESULT(no)])
AC_LANG_POP([C++])

dnl sqlite3
SAVE_LIBS="$LIBS"
LIBS=""
AC_SEARCH_LIBS([sqlite3_open], [sqlite3], [], [have_sqlite3="no"])
if test "x$have_sqlite3" != "xno"; then
   AC_DEFINE([WITH_SQLITE3], [1], [yes])
   SQLITE3LIBS="$LIBS"
fi
LIBS="$SAVE_LIBS"

AM_CONDITIONAL([WITH_SQLITE3], [test ! "x$have_sqlite3" = "no"])
AC_SUBST(SQLITE3LIBS)

dnl CNC:2003-03-20
AC_MSG_CHECKING(for --disable-scripts)
AC_ARG_ENABLE([scripts],
	      AS_HELP_STRING([--disable-scripts],[disable the lua script extension system]),
	      [enable_scripts="$enableval"],[enable_scripts="yes"])
if test "$enable_scripts" != "no"; then
  AC_MSG_RESULT(no)
  AC_DEFINE(APT_WITH_LUA, 1, 
	    [Define if you want to enable the extension system]) 
else
  AC_MSG_RESULT(yes)
fi
AM_CONDITIONAL(WITH_LUA, test "$enable_scripts" != "no")

dnl SELinux support
AC_MSG_CHECKING(for --disable-selinux)
AC_ARG_ENABLE([selinux],
	      AS_HELP_STRING([--disable-selinux],
			     [disable SELinux support]),
	      [enable_selinux="$enableval"],[enable_selinux="yes"])
if test "$enable_selinux" != "no"; then
  AC_MSG_RESULT(no)
  AC_DEFINE(WITH_SELINUX, 1, 
	    [Define if you want to enable SELinux support]) 
else
  AC_MSG_RESULT(yes)
fi
AM_CONDITIONAL(WITH_SELINUX, test "$enable_selinux" != "no")


dnl Repomd support
AC_MSG_CHECKING(for --disable-repomd)
AC_ARG_ENABLE([repomd],
	      AS_HELP_STRING([--disable-repomd],
			     [disable repomd support]),
	      [enable_repomd="$enableval"],[enable_repomd="yes"])
if test "$enable_repomd" != "no"; then
  AC_MSG_RESULT(no)
  AC_DEFINE(APT_WITH_REPOMD, 1, [Define if you want to enable repomd support]) 
  ## xmlReadFile is not available before 2.6.0
  PKG_CHECK_MODULES([LIBXML2],[libxml-2.0 >= 2.6])
  RPMLIBS="$LIBXML2_LIBS $RPMLIBS"
  CPPFLAGS="$LIBXML2_CFLAGS $CPPFLAGS"
  ## FIXME: Conditionalize this properly. We can support xml repomd without 
  ## sqlite and repodb with libxml2 older than 2.6. In addition, rollback
  ## will be using sqlite as well, we only want to check for it once.
  RPMLIBS="$RPMLIBS $SQLITE3LIBS"
else
  AC_MSG_RESULT(yes)
fi
AM_CONDITIONAL(WITH_REPOMD, test "$enable_repomd" != "no")

dnl Before configuring libtool check for --enable-static-progs option
AC_MSG_CHECKING(for --enable-static-progs)
AC_ARG_ENABLE(static-progs,
	      AS_HELP_STRING([--enable-static-progs],
			     [build completely static (standalone) binaries]),
	      [enable_static_progs="$enableval"],[enable_static_progs="no"])
AM_CONDITIONAL(COMPILE_STATIC, test "$enable_static_progs" = "yes")
if test "$enable_static_progs" = "yes" ; then
  AC_MSG_RESULT(yes)
else
  AC_MSG_RESULT(no)
fi

dnl Check for apt-shell dependencies
AC_CHECK_LIB(ncurses,tputs,
  [AC_CHECK_HEADER(readline/readline.h,
     [AC_CHECK_LIB(readline,rl_completion_matches,
        [compile_aptshell=yes],,[-lncurses])])])
AM_CONDITIONAL(COMPILE_APTSHELL, test "$compile_aptshell" = "yes")

dnl Converts the ARCH to be something singular for this general CPU family
dnl This is often the dpkg architecture string.
AH_TEMPLATE(COMMON_CPU, [Define the common CPU])
AH_TEMPLATE(COMMON_OS, [Define the common OS])
AC_MSG_CHECKING(system architecture)
archset="`awk \" ! /^#|^\\\$/ { if(match(\\\"$host_cpu\\\",\\\"^\\\"\\\$1\\\"\\\$\\\")) {print \\\$2; exit}}\" $srcdir/buildlib/archtable`"
if test "x$archset" = "x"; then
  AC_MSG_ERROR(failed: use --host=)
fi
AC_MSG_RESULT($archset)
AC_DEFINE_UNQUOTED(COMMON_CPU,"$archset")

dnl Get a common name for the host OS - this is primarily only for HURD and is
dnl non fatal if it fails
AC_MSG_CHECKING(system OS)
osset="`awk \" ! /^#|^\\\$/ {if (match(\\\"$host_vendor-$host_os\\\",\\\$1)) {print \\\$2; exit}}\" $srcdir/buildlib/ostable`"
AC_MSG_RESULT($osset)
AC_DEFINE_UNQUOTED(COMMON_OS,"$osset")

dnl We use C99 types if at all possible
AC_CACHE_CHECK([for C99 integer types],c9x_ints,[
    AC_TRY_COMPILE([#include <inttypes.h>],
                   [uint8_t Foo1;uint16_t Foo2;uint32_t Foo3;],
		   c9x_ints=yes,c9x_ints=no)])

AC_C_BIGENDIAN

dnl Check the sizes etc. of the architecture
dnl Cross compilers can either get a real C library or preload the cache
dnl with their size values.
AC_CHECK_SIZEOF(char)
AC_CHECK_SIZEOF(int)
AC_CHECK_SIZEOF(short)
AC_CHECK_SIZEOF(long)

dnl Single Unix Spec statvfs
AC_CHECK_FUNC(statvfs,[have_statvfs=yes])

dnl Arg, linux and bsd put their statfs function in different places
AH_TEMPLATE(HAVE_SYS_VFS_H, [Define to 1 if sys/vfs.h is available])
AH_TEMPLATE(HAVE_SYS_MOUNT_H, [Define to 1 if sys/mount.h is available])
if test "$have_statvfs" != "yes"; then
   AC_EGREP_HEADER(statfs,sys/vfs.h,[AC_DEFINE(HAVE_SYS_VFS_H)],[
   	AC_EGREP_HEADER(statfs,sys/mount.h,[AC_DEFINE(HAVE_SYS_MOUNT_H)],
   			[AC_MSG_ERROR(failed: Need statvfs)])
   ])
fi

dnl We should use the real timegm function if we have it.
AH_TEMPLATE(HAVE_TIMEGM, [Define to 1 if timegm is available])
AC_CHECK_FUNC(timegm,AC_DEFINE(HAVE_TIMEGM))
AC_SUBST(HAVE_TIMEGM)

dnl HP-UX sux..
AH_TEMPLATE(NEED_SOCKLEN_T_DEFINE, [Define to 1 if socket_t is missing])
AC_MSG_CHECKING(for missing socklen_t)
AC_EGREP_HEADER(socklen_t, sys/socket.h,[AC_MSG_RESULT(no)],[
	AC_DEFINE(NEED_SOCKLEN_T_DEFINE)
	NEED_SOCKLEN_T_DEFINE=yes
        AC_MSG_RESULT(missing.)])
AC_SUBST(NEED_SOCKLEN_T_DEFINE)

dnl HP-UX needs -d_XOPEN_SOURCE_EXTENDED for h_errno
AC_MSG_CHECKING(for h_errno)
AC_EGREP_HEADER(h_errno, netdb.h, [AC_MSG_RESULT(normal)],
   [CPPFLAGS="$CPPFLAGS -D_XOPEN_SOURCE_EXTENDED"
    AC_EGREP_HEADER(h_errno, netdb.h, 
       [AC_MSG_RESULT(needs _XOPEN_SOURCE_EXTENDED)],
       [AC_MSG_ERROR("not found.")])
   ])

dnl Check for debiandoc
dnl AC_PATH_PROG(DEBIANDOC_HTML,debiandoc2html)
dnl AC_PATH_PROG(DEBIANDOC_TEXT,debiandoc2text)

AC_PATH_PROG([GPG],[gpg],[/usr/bin/gpg])

AM_GNU_GETTEXT_VERSION([0.14.5])
AM_GNU_GETTEXT([external])

dnl ah_NUM_PROCS
dnl rc_GLIBC_VER
dnl rc_LIBSTDCPP_VER
dnl ah_GCC3DEP

AC_CONFIG_FILES([
	  Makefile
	  lua/Makefile
	  apt-pkg/Makefile
	  apt-pkg/libapt-pkg.pc
	  methods/Makefile
	  cmdline/Makefile
	  tools/Makefile
	  doc/Makefile
	  test/Makefile
	  po/Makefile.in
	  ])
AC_CONFIG_LINKS([include/apti18n.h:buildlib/gettext.h])

AS_IF([test "$c9x_ints" = "no"],[
AC_CONFIG_LINKS([include/inttypes.h:buildlib/inttypes.h])
])

AS_IF([test "$have_statvfs" != "yes"],[
AC_CONFIG_LINKS([include/statvfs.h:buildlib/statvfs.h])
])

AS_IF([test "$NEED_SOCKLEN_T_DEFINE" = "yes"],[
AC_CONFIG_LINKS([include/netdb.h:buildlib/netdb.h])
])

AC_OUTPUT
