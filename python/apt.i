%module apt
%include std_string.i
%include std_vector.i

%{
#include <apt-pkg/init.h>
#include <apt-pkg/pkgcache.h>
#include <apt-pkg/depcache.h>
#include <apt-pkg/cachefile.h>
#include <apt-pkg/mmap.h>
#include <apt-pkg/cacheiterators.h>
#include <apt-pkg/algorithms.h>
#include <apt-pkg/pkgsystem.h>
#include <apt-pkg/contrib/configuration.h>
#include <apt-pkg/contrib/progress.h>
#include <apt-pkg/version.h>
#include <apt-pkg/pkgrecords.h>
#include <apt-pkg/acquire-item.h>
#include <apt-pkg/acquire.h>
#include <apt-pkg/packagemanager.h>
#include <apt-pkg/sourcelist.h>
#include <apt-pkg/contrib/error.h>
#include <apt-pkg/luaiface.h>

#include <string>
#include <time.h>
%}

%inline %{
typedef pkgCache::VerIterator VerIterator;
typedef pkgCache::PkgIterator PkgIterator;
typedef pkgCache::DepIterator DepIterator;
typedef pkgCache::PrvIterator PrvIterator;
typedef pkgCache::PkgFileIterator PkgFileIterator;
typedef pkgCache::VerFileIterator VerFileIterator;
typedef pkgCache::Header Header;
typedef pkgCache::Package Package;
typedef pkgCache::PackageFile PackageFile;
typedef pkgCache::VerFile VerFile;
typedef pkgCache::Version Version;
typedef pkgCache::Dependency Dependency;
typedef pkgCache::Provides Provides;
typedef pkgCache::StringItem StringItem;
typedef pkgCache::Dep Dep;
typedef pkgCache::State State;
typedef pkgCache::Flag Flag;
typedef pkgDepCache::Policy Policy;
typedef pkgDepCache::StateCache StateCache;
typedef Configuration::Item Item;
typedef pkgRecords::Parser Parser;
%}

/* Fix some operators. */
%rename(next) operator++;
%rename(assign) operator=;
%rename(pkgCache) operator pkgCache *;
%rename(pkgDepCache) operator pkgDepCache *;
%rename(Package) operator Package *;
%rename(Version) operator Version *;
%rename(Dependency) operator Dependency *;
%rename(Provides) operator Provides *;
%rename(PackageFile) operator PackageFile *;
%rename(VerFile) operator VerFile *;
%rename(__getitem__) operator[];
%ignore operator pkgCache &;
%ignore operator pkgDepCache &;

/* Set some data as immutable. */
%immutable pkgVersion;
%immutable pkgLibVersion;
%immutable pkgOS;
%immutable pkgCPU;
%immutable pkgSystem::Label;
%immutable pkgVersioningSystem::Label;
%immutable pkgDepCache::StateCache::CandVersion;
%immutable pkgDepCache::StateCache::CurVersion;

/* One-shot initialization function. */
%inline %{
inline bool pkgInit() 
{
   return pkgInitConfig(*_config) && pkgInitSystem(*_config,_system);
}
%}

/* No suport for nested classes yet. */
%rename(pkgCacheHeader) pkgCache::Header;
%rename(pkgCachePackage) pkgCache::Package;
%rename(pkgCachePackageFile) pkgCache::PackageFile;
%rename(pkgCacheVerFile) pkgCache::VerFile;
%rename(pkgCacheVersion) pkgCache::Version;
%rename(pkgCacheDependency) pkgCache::Dependency;
%rename(pkgCacheProvides) pkgCache::Provides;
%rename(pkgCacheStringItem) pkgCache::StringItem;
%rename(pkgCacheDep) pkgCache::Dep;
%rename(pkgCacheState) pkgCache::State;
%rename(pkgCacheFlag) pkgCache::Flag;
%rename(pkgCacheVerIterator) pkgCache::VerIterator;
%rename(pkgCachePkgIterator) pkgCache::PkgIterator;
%rename(pkgCacheDepIterator) pkgCache::DepIterator;
%rename(pkgCachePrvIterator) pkgCache::PrvIterator;
%rename(pkgCachePkgFileIterator) pkgCache::PkgFileIterator;
%rename(pkgCacheVerFileIterator) pkgCache::VerFileIterator;
%rename(pkgDepCacheStateCache) pkgDepCache::StateCache;
%rename(pkgRecordsParser) pkgRecords::Parser;
%rename(pkgAcquireItem) pkgAcquire::Item;
%rename(ConfigurationItem) Configuration::Item;

/* Wonderful SWIG magic to turn APT iterators into Python iterators. */
%exception next {
	$action
	/* Pass ahead the StopIteration exception. */
	if (!result) return NULL;
}
#define EXTEND_ITERATOR(IterClass) \
%newobject IterClass::next; \
%newobject IterClass::__iter__; \
%ignore IterClass::operator++; \
%extend IterClass { \
	inline bool __nonzero__() { return self->end() == false; }; \
	inline IterClass *next() { \
		if (self->end() == true) { \
			PyErr_SetObject(PyExc_StopIteration, Py_None); \
			return NULL; \
		} \
		IterClass *ret = new IterClass(*self); \
		(*self)++; \
		return ret; \
	}; \
	/* We must return a copy here, otherwise the original object
	 * might be deallocated and the returned pointer would become
	 * invalid */ \
	inline IterClass *__iter__() { return new IterClass(*self); }; \
}
EXTEND_ITERATOR(pkgCache::PkgIterator);
EXTEND_ITERATOR(pkgCache::VerIterator);
EXTEND_ITERATOR(pkgCache::DepIterator);
EXTEND_ITERATOR(pkgCache::PrvIterator);
EXTEND_ITERATOR(pkgCache::PkgFileIterator);
EXTEND_ITERATOR(pkgCache::VerFileIterator);

/* Now transform the functions returning iterators into ones with more
 * meaningful names for that purpose. */
%rename(PkgIter) pkgDepCache::PkgBegin;
%rename(PkgIter) pkgCache::PkgBegin;
%rename(FileIter) pkgCache::FileBegin;
%ignore pkgCache::PkgEnd;
%ignore pkgCache::FileEnd;

/* That's the best way I found to access ItemsBegin/ItemsEnd, for now.
 * It may be changed to behave like the iterators above in the future,
 * so don't expect a list to be returned. */
%ignore pkgAcquire::ItemsBegin;
%ignore pkgAcquire::ItemsEnd;
%extend pkgAcquire {
PyObject *
ItemsIter()
{
	static swig_type_info *ItemDescr = NULL;
	PyObject *list, *o;
	pkgAcquire::ItemIterator I;
	if (!ItemDescr) {
		ItemDescr = SWIG_TypeQuery("pkgAcquire::Item *");
		assert(ItemDescr);
	}
	list = PyList_New(0);
	if (list == NULL)
		return NULL;
	for (I = self->ItemsBegin(); I != self->ItemsEnd(); I++) {
		o = SWIG_NewPointerObj((void *)(*I), ItemDescr, 0);
		if (!o || PyList_Append(list, o) == -1) {
			Py_XDECREF(o);
			Py_DECREF(list);
			return NULL;
	    	}
		Py_DECREF(o);
	}
	return list;
}
}

/* Wrap string members. */
%immutable pkgAcquire::Item::DestFile;
%immutable pkgAcquire::Item::ErrorText;
%extend pkgAcquire::Item {
	const char *DestFile;
	const char *ErrorText;
}
%ignore pkgAcquire::Item::DestFile;
%ignore pkgAcquire::Item::ErrorText;
%{
#define pkgAcquire_Item_DestFile_get(x) ((x)->DestFile.c_str())
#define pkgAcquire_Item_ErrorText_get(x) ((x)->ErrorText.c_str())
%}

/* Also from Configuration::Item. */
%extend Configuration::Item {
	const char *Tag;
	const char *Value;
}
%ignore Configuration::Item::Tag;
%ignore Configuration::Item::Value;
%{
#define Configuration_Item_Tag_get(x) ((x)->Tag.c_str())
#define Configuration_Item_Value_get(x) ((x)->Value.c_str())
#define Configuration_Item_Tag_set(x,y) ((x)->Tag = (y))
#define Configuration_Item_Value_set(x,y) ((x)->Value = (y))
%}

/* Typemap to present map_ptrloc in a better way */
%apply int { map_ptrloc };

/* That should be enough for our usage, but _error is indeed an alias
 * for a function which returns an statically alocated GlobalError object. */
%immutable _error;
GlobalError *_error;

%immutable _lua;
Lua *_lua;

/* Undefined reference!? */
%ignore pkgCache::PkgIterator::TargetVer;

/* There's a struct and a function with the same name. */
%ignore SubstVar;

/* Allow threads to run while doing DoInstall() */
%exception pkgPackageManager::DoInstall {
Py_BEGIN_ALLOW_THREADS
$function
Py_END_ALLOW_THREADS
}

/* Preprocess string macros (note that we're %import'ing). */
%import <apt-pkg/contrib/strutl.h>

%include <apt-pkg/init.h>
%include <apt-pkg/pkgcache.h>
%include <apt-pkg/depcache.h>
%include <apt-pkg/cacheiterators.h>
%include <apt-pkg/cachefile.h>
%include <apt-pkg/algorithms.h>
%include <apt-pkg/pkgsystem.h>
%include <apt-pkg/contrib/configuration.h>
%include <apt-pkg/contrib/progress.h>
%include <apt-pkg/version.h>
%include <apt-pkg/pkgrecords.h>
%include <apt-pkg/acquire-item.h>
%include <apt-pkg/acquire.h>
%include <apt-pkg/packagemanager.h>
%include <apt-pkg/sourcelist.h>
%include <apt-pkg/contrib/error.h>
%include <apt-pkg/luaiface.h>

/* Create a dumb status class which can be instantiated. pkgAcquireStatus
 * has fully abstract methods. */
%inline %{
class pkgAcquireStatusDumb : public pkgAcquireStatus
{
   virtual bool MediaChange(string Media,string Drive) {};
};
%}

/* That's the real class we use for Python inheritance. */
%{
class ROpPyProgress : public OpProgress {
	PyObject *PyObj;

	public:
	OpProgress::Op;
	OpProgress::SubOp;
	OpProgress::Percent;
	OpProgress::MajorChange;
	OpProgress::CheckChange;

	virtual void Update()
	{
		if (PyObject_HasAttrString(PyObj, "Update")) {
			PyObject *Ret;
			Ret = PyObject_CallMethod(PyObj, "Update", NULL);
			Py_XDECREF(Ret);
		}
	};

	virtual void Done()
	{
		if (PyObject_HasAttrString(PyObj, "Done")) {
			PyObject *Ret;
			Ret = PyObject_CallMethod(PyObj, "Done", NULL);
			Py_XDECREF(Ret);
		}
	};
	
	ROpPyProgress(PyObject *PyObj) : PyObj(PyObj) {Py_INCREF(PyObj);};
	~ROpPyProgress() {Py_DECREF(PyObj);};
};
%}

/* That's how we want SWIG to see our class. */
%extend ROpPyProgress {
	const char *Op;
	const char *SubOp;
}
%ignore ROpPyProgress::Op;
%ignore ROpPyProgress::SubOp;
%{
#define ROpPyProgress_Op_get(x) ((x)->Op.c_str())
#define ROpPyProgress_Op_set(x,y) ((x)->Op = (y))
#define ROpPyProgress_SubOp_get(x) ((x)->SubOp.c_str())
#define ROpPyProgress_SubOp_set(x,y) ((x)->SubOp = (y))
%}
class ROpPyProgress : public OpProgress {
	public:
	float Percent;
	bool MajorChange;
	bool CheckChange(float Interval=0.7);		    
	ROpPyProgress(PyObject *PyObj);
};

/* That's the proxy class the user sees. This exists only to pass
 * "self" as the first parameter of ROpPyProgress. */
%pythoncode %{
class OpPyProgress(ROpPyProgress):
	def __init__(self):
		ROpPyProgress.__init__(self, self)
	def __repr__(self):
		return "<C OpPyProgress instance at %s>" % self.this
%}

#if 0
/* The same scheme as above for pkgAcquireStatus. */
%{
class pkgRPyAcquireStatus : public pkgAcquireStatus
{
	PyObject *PyObj;

	void ItemDescMethod(const char *Name, pkgAcquire::ItemDesc &Itm) {
		static swig_type_info *SwigType = 0;
		if (!SwigType) {
			SwigType = SWIG_TypeQuery("pkgAcquire::ItemDesc *");
			assert(SwigType);
		}
		PyObject *attr = PyObject_GetAttrString(PyObj, (char*)Name);
		if (attr != NULL) {
			PyObject *Arg;
			Arg = SWIG_NewPointerObj(&Itm, SwigType, 0);
			if (!Arg) return;
			PyObject *Ret;
			Ret = PyObject_CallFunction(attr, "(O)", Arg);
			Py_XDECREF(Arg);
			Py_XDECREF(Ret);
		} else {
			PyErr_Clear();
		}
	};

	public:
	/* No wrapping yet. */
	//struct timeval Time;
	//struct timeval StartTime;
	pkgAcquireStatus::LastBytes;
	pkgAcquireStatus::CurrentCPS;
	pkgAcquireStatus::CurrentBytes;
	pkgAcquireStatus::TotalBytes;
	pkgAcquireStatus::FetchedBytes;
	pkgAcquireStatus::ElapsedTime;
	pkgAcquireStatus::TotalItems;
	pkgAcquireStatus::CurrentItems;
   
   	/* Call only Python method, if existent, or parent method. */
	void Fetched(unsigned long long Size,unsigned long long ResumePoint)
	{
		PyObject *attr = PyObject_GetAttrString(PyObj, "Fetched");
		if (attr != NULL) {
			PyObject *Ret;
			Ret = PyObject_CallFunction(attr, "(ii)",
						    Size, ResumePoint);
			Py_XDECREF(Ret);
		} else {
			PyErr_Clear();
			pkgAcquireStatus::Fetched(Size, ResumePoint);
		}
	};

	bool MediaChange(string Media,string Drive)
	{
		PyObject *attr = PyObject_GetAttrString(PyObj, "MediaChange");
		if (attr != NULL) {
			PyObject *Ret;
			bool RealRet = false;
			Ret = PyObject_CallFunction(attr, "(ss)",
						    Media.c_str(),
						    Drive.c_str());
			if (Ret) {
				RealRet = PyObject_IsTrue(Ret);
				Py_DECREF(Ret);
			}
			return RealRet;
		} else {
			PyErr_Clear();
		}
	};
   
	void IMSHit(pkgAcquire::ItemDesc &Itm)
		{ ItemDescMethod("IMSHit", Itm); };
	void Fetch(pkgAcquire::ItemDesc &Itm)
		{ ItemDescMethod("Fetch", Itm); };
	void Done(pkgAcquire::ItemDesc &Itm)
		{ ItemDescMethod("Done", Itm); };
	void Fail(pkgAcquire::ItemDesc &Itm)
		{ ItemDescMethod("Fail", Itm); };

	bool Pulse(pkgAcquire *Owner) {
		pkgAcquireStatus::Pulse(Owner);
		static swig_type_info *SwigType = 0;
		if (!SwigType) {
			SwigType = SWIG_TypeQuery("pkgAcquire *");
			assert(SwigType);
		}
		PyObject *attr = PyObject_GetAttrString(PyObj, "Pulse");
		if (attr != NULL) {
			PyObject *Arg;
			Arg = SWIG_NewPointerObj(Owner, SwigType, 0);
			if (!Arg) return false;
			PyObject *Ret;
			Ret = PyObject_CallFunction(attr, "(O)", Arg);
			Py_XDECREF(Arg);
			bool RealRet = false;
			if (Ret != NULL) {
				RealRet = PyObject_IsTrue(Ret);
				Py_DECREF(Ret);
			}
			return RealRet;
		} else {
			PyErr_Clear();
		}
	};

	void Start() {
		pkgAcquireStatus::Start();
		PyObject *attr = PyObject_GetAttrString(PyObj, "Start");
		if (attr != NULL) {
			PyObject *Ret;
			Ret = PyObject_CallFunction(attr, NULL);
			Py_XDECREF(Ret);
		} else {
			PyErr_Clear();
		}
	};
	void Stop() {
		pkgAcquireStatus::Stop();
		PyObject *attr = PyObject_GetAttrString(PyObj, "Stop");
		if (attr != NULL) {
			PyObject *Ret;
			Ret = PyObject_CallFunction(attr, NULL);
			Py_XDECREF(Ret);
		} else {
			PyErr_Clear();
		}
	};
   
	pkgRPyAcquireStatus(PyObject *PyObj) : PyObj(PyObj)
		{ Py_INCREF(PyObj); };
	~pkgRPyAcquireStatus()
		{ Py_DECREF(PyObj); };
};
%}
class pkgRPyAcquireStatus : public pkgAcquireStatus {
	public:
	pkgRPyAcquireStatus(PyObject *PyObj);
	double LastBytes;
	double CurrentCPS;
	double CurrentBytes;
	double TotalBytes;
	double FetchedBytes;
	unsigned long ElapsedTime;
	unsigned long TotalItems;
	unsigned long CurrentItems;
};
%pythoncode %{
class pkgPyAcquireStatus(pkgRPyAcquireStatus):
	def __init__(self):
		pkgRPyAcquireStatus.__init__(self, self)
	def __repr__(self):
		return "<C pkgPyAcquireStatus instance at %s>" % self.this
%}
#endif

// vim:ft=swig
