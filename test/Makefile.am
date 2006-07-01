noinst_PROGRAMS =
EXTRA_PROGRAMS =

# Program for testing methods
noinst_PROGRAMS += mthdcat
mthdcat_SOURCES = mthdcat.cc

# Program for testing methods
noinst_PROGRAMS += uritest
uritest_SOURCES = uri.cc
uritest_LDADD = ../apt-pkg/libapt-pkg.la

# Scratch program to test incomplete code fragments in
noinst_PROGRAMS += scratch-test
scratch_test_SOURCES = scratch.cc
scratch_test_LDADD = ../apt-pkg/libapt-pkg.la

# Version compare tester
noinst_PROGRAMS +=versiontest
versiontest_SOURCES = versiontest.cc
versiontest_LDADD = ../apt-pkg/libapt-pkg.la

# Program for testing the config file parser
noinst_PROGRAMS += conftest
conftest_SOURCES = conf.cc
conftest_LDADD = ../apt-pkg/libapt-pkg.la

# Program for testing hashes
noinst_PROGRAMS += hash
hash_SOURCES = hash.cc
hash_LDADD = ../apt-pkg/libapt-pkg.la

EXTRA_DIST = versions.lst
