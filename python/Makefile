
MODULES = apt.py

CC = g++
SWIG = swig
#SWIGFLAGS = -w312 -w315 -w508 -w509
SWIGFLAGS = -w312 -w315 -w508
#SWIG = /var/tmp/swig/BUILD/SWIG-1.3.16/swig
LUAFLAGS = -DWITH_LUA

PYTHON = python
PYTHONFLAGS = $(shell $(PYTHON) -c "from distutils.sysconfig import get_python_inc; print '-I' + get_python_inc(1)")

all: $(MODULES)

$(MODULES): %.py: _%.so

_%.so: %_wrap.cxx
	$(CC) -fPIC -shared -I.. $(PYTHONFLAGS) $(LUAFLAGS) -L../bin -L../build/bin -lapt-pkg -o $@ $<

apt_wrap.cxx: apt.i
	$(SWIG) $(SWIGFLAGS) -c++ -I.. $(LUAFLAGS) -python $<

clean:
	rm -f *.cxx

distclean: clean
	rm -f $(MODULES) *.pyc *.pyo *.so
