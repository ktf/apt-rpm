
MODULES = apt.py

CC = g++
SWIG = swig
#SWIGFLAGS = -w312 -w315 -w508 -w509
SWIGFLAGS = -w312 -w315 -w508
#SWIG = /var/tmp/swig/BUILD/SWIG-1.3.16/swig
LUAFLAGS = -DWITH_LUA

PYTHONVER = $(shell python -c "import sys; v=sys.version_info[:2]; print '%d.%d'%v" 2>/dev/null || echo NONE)
PYTHONFLAGS=-I/usr/include/python$(PYTHONVER)

all: $(MODULES)

$(MODULES): %.py: _%.so

_%.so: %_wrap.cxx
	$(CC) -fPIC -shared -I.. $(PYTHONFLAGS) $(LUAFLAGS) -L../bin -L../build/bin -lapt-pkg -o $@ $<

apt_wrap.cxx: apt.i
	$(SWIG) $(SWIGFLAGS) -c++ -I.. $(LUAFLAGS) -python $<

clean:
	rm -f *.cxx

distclean: clean
	rm -f $(MODULES) *.pyc _*.so
