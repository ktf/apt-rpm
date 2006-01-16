# This file was created automatically by SWIG.
# Don't modify this file, modify the SWIG interface instead.
# This file is compatible with both classic and new-style classes.

import _apt

def _swig_setattr(self,class_type,name,value):
    if (name == "this"):
        if isinstance(value, class_type):
            self.__dict__[name] = value.this
            if hasattr(value,"thisown"): self.__dict__["thisown"] = value.thisown
            del value.thisown
            return
    method = class_type.__swig_setmethods__.get(name,None)
    if method: return method(self,value)
    self.__dict__[name] = value

def _swig_getattr(self,class_type,name):
    method = class_type.__swig_getmethods__.get(name,None)
    if method: return method(self)
    raise AttributeError,name

import types
try:
    _object = types.ObjectType
    _newclass = 1
except AttributeError:
    class _object : pass
    _newclass = 0
del types



pkgInit = _apt.pkgInit
APT_PKG_MAJOR = _apt.APT_PKG_MAJOR
APT_PKG_MINOR = _apt.APT_PKG_MINOR
APT_PKG_RELEASE = _apt.APT_PKG_RELEASE

pkgInitConfig = _apt.pkgInitConfig

pkgInitSystem = _apt.pkgInitSystem
class pkgCache(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, pkgCache, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, pkgCache, name)
    def __repr__(self):
        return "<C pkgCache instance at %s>" % (self.this,)
    DepDepends = _apt.pkgCache_DepDepends
    DepPreDepends = _apt.pkgCache_DepPreDepends
    DepSuggests = _apt.pkgCache_DepSuggests
    DepRecommends = _apt.pkgCache_DepRecommends
    DepConflicts = _apt.pkgCache_DepConflicts
    DepReplaces = _apt.pkgCache_DepReplaces
    DepObsoletes = _apt.pkgCache_DepObsoletes
    DepOr = _apt.pkgCache_DepOr
    DepNoOp = _apt.pkgCache_DepNoOp
    DepLessEq = _apt.pkgCache_DepLessEq
    DepGreaterEq = _apt.pkgCache_DepGreaterEq
    DepLess = _apt.pkgCache_DepLess
    DepGreater = _apt.pkgCache_DepGreater
    DepEquals = _apt.pkgCache_DepEquals
    DepNotEquals = _apt.pkgCache_DepNotEquals
    StateImportant = _apt.pkgCache_StateImportant
    StateRequired = _apt.pkgCache_StateRequired
    StateStandard = _apt.pkgCache_StateStandard
    StateOptional = _apt.pkgCache_StateOptional
    StateExtra = _apt.pkgCache_StateExtra
    StateUnknown = _apt.pkgCache_StateUnknown
    StateInstall = _apt.pkgCache_StateInstall
    StateHold = _apt.pkgCache_StateHold
    StateDeInstall = _apt.pkgCache_StateDeInstall
    StatePurge = _apt.pkgCache_StatePurge
    StateOk = _apt.pkgCache_StateOk
    StateReInstReq = _apt.pkgCache_StateReInstReq
    StateHoldInst = _apt.pkgCache_StateHoldInst
    StateHoldReInstReq = _apt.pkgCache_StateHoldReInstReq
    StateNotInstalled = _apt.pkgCache_StateNotInstalled
    StateUnPacked = _apt.pkgCache_StateUnPacked
    StateHalfConfigured = _apt.pkgCache_StateHalfConfigured
    StateHalfInstalled = _apt.pkgCache_StateHalfInstalled
    StateConfigFiles = _apt.pkgCache_StateConfigFiles
    StateInstalled = _apt.pkgCache_StateInstalled
    FlagAuto = _apt.pkgCache_FlagAuto
    FlagEssential = _apt.pkgCache_FlagEssential
    FlagImportant = _apt.pkgCache_FlagImportant
    FlagNotSource = _apt.pkgCache_FlagNotSource
    FlagNotAutomatic = _apt.pkgCache_FlagNotAutomatic
    __swig_setmethods__["HeaderP"] = _apt.pkgCache_HeaderP_set
    __swig_getmethods__["HeaderP"] = _apt.pkgCache_HeaderP_get
    if _newclass:HeaderP = property(_apt.pkgCache_HeaderP_get, _apt.pkgCache_HeaderP_set)
    __swig_setmethods__["PkgP"] = _apt.pkgCache_PkgP_set
    __swig_getmethods__["PkgP"] = _apt.pkgCache_PkgP_get
    if _newclass:PkgP = property(_apt.pkgCache_PkgP_get, _apt.pkgCache_PkgP_set)
    __swig_setmethods__["VerFileP"] = _apt.pkgCache_VerFileP_set
    __swig_getmethods__["VerFileP"] = _apt.pkgCache_VerFileP_get
    if _newclass:VerFileP = property(_apt.pkgCache_VerFileP_get, _apt.pkgCache_VerFileP_set)
    __swig_setmethods__["PkgFileP"] = _apt.pkgCache_PkgFileP_set
    __swig_getmethods__["PkgFileP"] = _apt.pkgCache_PkgFileP_get
    if _newclass:PkgFileP = property(_apt.pkgCache_PkgFileP_get, _apt.pkgCache_PkgFileP_set)
    __swig_setmethods__["VerP"] = _apt.pkgCache_VerP_set
    __swig_getmethods__["VerP"] = _apt.pkgCache_VerP_get
    if _newclass:VerP = property(_apt.pkgCache_VerP_get, _apt.pkgCache_VerP_set)
    __swig_setmethods__["ProvideP"] = _apt.pkgCache_ProvideP_set
    __swig_getmethods__["ProvideP"] = _apt.pkgCache_ProvideP_get
    if _newclass:ProvideP = property(_apt.pkgCache_ProvideP_get, _apt.pkgCache_ProvideP_set)
    __swig_setmethods__["DepP"] = _apt.pkgCache_DepP_set
    __swig_getmethods__["DepP"] = _apt.pkgCache_DepP_get
    if _newclass:DepP = property(_apt.pkgCache_DepP_get, _apt.pkgCache_DepP_set)
    __swig_setmethods__["StringItemP"] = _apt.pkgCache_StringItemP_set
    __swig_getmethods__["StringItemP"] = _apt.pkgCache_StringItemP_get
    if _newclass:StringItemP = property(_apt.pkgCache_StringItemP_get, _apt.pkgCache_StringItemP_set)
    __swig_setmethods__["StrP"] = _apt.pkgCache_StrP_set
    __swig_getmethods__["StrP"] = _apt.pkgCache_StrP_get
    if _newclass:StrP = property(_apt.pkgCache_StrP_get, _apt.pkgCache_StrP_set)
    def ReMap(*args): return _apt.pkgCache_ReMap(*args)
    def Sync(*args): return _apt.pkgCache_Sync(*args)
    def GetMap(*args): return _apt.pkgCache_GetMap(*args)
    def DataEnd(*args): return _apt.pkgCache_DataEnd(*args)
    def Hash(*args): return _apt.pkgCache_Hash(*args)
    def Priority(*args): return _apt.pkgCache_Priority(*args)
    def FindPkg(*args): return _apt.pkgCache_FindPkg(*args)
    def FindPackage(*args): return _apt.pkgCache_FindPackage(*args)
    def Head(*args): return _apt.pkgCache_Head(*args)
    def PkgIter(*args): return _apt.pkgCache_PkgIter(*args)
    def FileIter(*args): return _apt.pkgCache_FileIter(*args)
    __swig_setmethods__["VS"] = _apt.pkgCache_VS_set
    __swig_getmethods__["VS"] = _apt.pkgCache_VS_get
    if _newclass:VS = property(_apt.pkgCache_VS_get, _apt.pkgCache_VS_set)
    __swig_getmethods__["CompTypeDeb"] = lambda x: _apt.pkgCache_CompTypeDeb
    if _newclass:CompTypeDeb = staticmethod(_apt.pkgCache_CompTypeDeb)
    __swig_getmethods__["CompType"] = lambda x: _apt.pkgCache_CompType
    if _newclass:CompType = staticmethod(_apt.pkgCache_CompType)
    __swig_getmethods__["DepType"] = lambda x: _apt.pkgCache_DepType
    if _newclass:DepType = staticmethod(_apt.pkgCache_DepType)
    def __init__(self, *args):
        _swig_setattr(self, pkgCache, 'this', _apt.new_pkgCache(*args))
        _swig_setattr(self, pkgCache, 'thisown', 1)
    def __del__(self, destroy=_apt.delete_pkgCache):
        try:
            if self.thisown: destroy(self)
        except: pass

class pkgCachePtr(pkgCache):
    def __init__(self, this):
        _swig_setattr(self, pkgCache, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, pkgCache, 'thisown', 0)
        _swig_setattr(self, pkgCache,self.__class__,pkgCache)
_apt.pkgCache_swigregister(pkgCachePtr)
cvar = _apt.cvar

pkgCache_CompTypeDeb = _apt.pkgCache_CompTypeDeb

pkgCache_CompType = _apt.pkgCache_CompType

pkgCache_DepType = _apt.pkgCache_DepType

class pkgCacheHeader(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, pkgCacheHeader, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, pkgCacheHeader, name)
    def __repr__(self):
        return "<C pkgCache::Header instance at %s>" % (self.this,)
    __swig_setmethods__["Signature"] = _apt.pkgCacheHeader_Signature_set
    __swig_getmethods__["Signature"] = _apt.pkgCacheHeader_Signature_get
    if _newclass:Signature = property(_apt.pkgCacheHeader_Signature_get, _apt.pkgCacheHeader_Signature_set)
    __swig_setmethods__["MajorVersion"] = _apt.pkgCacheHeader_MajorVersion_set
    __swig_getmethods__["MajorVersion"] = _apt.pkgCacheHeader_MajorVersion_get
    if _newclass:MajorVersion = property(_apt.pkgCacheHeader_MajorVersion_get, _apt.pkgCacheHeader_MajorVersion_set)
    __swig_setmethods__["MinorVersion"] = _apt.pkgCacheHeader_MinorVersion_set
    __swig_getmethods__["MinorVersion"] = _apt.pkgCacheHeader_MinorVersion_get
    if _newclass:MinorVersion = property(_apt.pkgCacheHeader_MinorVersion_get, _apt.pkgCacheHeader_MinorVersion_set)
    __swig_setmethods__["Dirty"] = _apt.pkgCacheHeader_Dirty_set
    __swig_getmethods__["Dirty"] = _apt.pkgCacheHeader_Dirty_get
    if _newclass:Dirty = property(_apt.pkgCacheHeader_Dirty_get, _apt.pkgCacheHeader_Dirty_set)
    __swig_setmethods__["HasFileDeps"] = _apt.pkgCacheHeader_HasFileDeps_set
    __swig_getmethods__["HasFileDeps"] = _apt.pkgCacheHeader_HasFileDeps_get
    if _newclass:HasFileDeps = property(_apt.pkgCacheHeader_HasFileDeps_get, _apt.pkgCacheHeader_HasFileDeps_set)
    __swig_setmethods__["OptionsHash"] = _apt.pkgCacheHeader_OptionsHash_set
    __swig_getmethods__["OptionsHash"] = _apt.pkgCacheHeader_OptionsHash_get
    if _newclass:OptionsHash = property(_apt.pkgCacheHeader_OptionsHash_get, _apt.pkgCacheHeader_OptionsHash_set)
    __swig_setmethods__["HeaderSz"] = _apt.pkgCacheHeader_HeaderSz_set
    __swig_getmethods__["HeaderSz"] = _apt.pkgCacheHeader_HeaderSz_get
    if _newclass:HeaderSz = property(_apt.pkgCacheHeader_HeaderSz_get, _apt.pkgCacheHeader_HeaderSz_set)
    __swig_setmethods__["PackageSz"] = _apt.pkgCacheHeader_PackageSz_set
    __swig_getmethods__["PackageSz"] = _apt.pkgCacheHeader_PackageSz_get
    if _newclass:PackageSz = property(_apt.pkgCacheHeader_PackageSz_get, _apt.pkgCacheHeader_PackageSz_set)
    __swig_setmethods__["PackageFileSz"] = _apt.pkgCacheHeader_PackageFileSz_set
    __swig_getmethods__["PackageFileSz"] = _apt.pkgCacheHeader_PackageFileSz_get
    if _newclass:PackageFileSz = property(_apt.pkgCacheHeader_PackageFileSz_get, _apt.pkgCacheHeader_PackageFileSz_set)
    __swig_setmethods__["VersionSz"] = _apt.pkgCacheHeader_VersionSz_set
    __swig_getmethods__["VersionSz"] = _apt.pkgCacheHeader_VersionSz_get
    if _newclass:VersionSz = property(_apt.pkgCacheHeader_VersionSz_get, _apt.pkgCacheHeader_VersionSz_set)
    __swig_setmethods__["DependencySz"] = _apt.pkgCacheHeader_DependencySz_set
    __swig_getmethods__["DependencySz"] = _apt.pkgCacheHeader_DependencySz_get
    if _newclass:DependencySz = property(_apt.pkgCacheHeader_DependencySz_get, _apt.pkgCacheHeader_DependencySz_set)
    __swig_setmethods__["ProvidesSz"] = _apt.pkgCacheHeader_ProvidesSz_set
    __swig_getmethods__["ProvidesSz"] = _apt.pkgCacheHeader_ProvidesSz_get
    if _newclass:ProvidesSz = property(_apt.pkgCacheHeader_ProvidesSz_get, _apt.pkgCacheHeader_ProvidesSz_set)
    __swig_setmethods__["VerFileSz"] = _apt.pkgCacheHeader_VerFileSz_set
    __swig_getmethods__["VerFileSz"] = _apt.pkgCacheHeader_VerFileSz_get
    if _newclass:VerFileSz = property(_apt.pkgCacheHeader_VerFileSz_get, _apt.pkgCacheHeader_VerFileSz_set)
    __swig_setmethods__["PackageCount"] = _apt.pkgCacheHeader_PackageCount_set
    __swig_getmethods__["PackageCount"] = _apt.pkgCacheHeader_PackageCount_get
    if _newclass:PackageCount = property(_apt.pkgCacheHeader_PackageCount_get, _apt.pkgCacheHeader_PackageCount_set)
    __swig_setmethods__["VersionCount"] = _apt.pkgCacheHeader_VersionCount_set
    __swig_getmethods__["VersionCount"] = _apt.pkgCacheHeader_VersionCount_get
    if _newclass:VersionCount = property(_apt.pkgCacheHeader_VersionCount_get, _apt.pkgCacheHeader_VersionCount_set)
    __swig_setmethods__["DependsCount"] = _apt.pkgCacheHeader_DependsCount_set
    __swig_getmethods__["DependsCount"] = _apt.pkgCacheHeader_DependsCount_get
    if _newclass:DependsCount = property(_apt.pkgCacheHeader_DependsCount_get, _apt.pkgCacheHeader_DependsCount_set)
    __swig_setmethods__["PackageFileCount"] = _apt.pkgCacheHeader_PackageFileCount_set
    __swig_getmethods__["PackageFileCount"] = _apt.pkgCacheHeader_PackageFileCount_get
    if _newclass:PackageFileCount = property(_apt.pkgCacheHeader_PackageFileCount_get, _apt.pkgCacheHeader_PackageFileCount_set)
    __swig_setmethods__["VerFileCount"] = _apt.pkgCacheHeader_VerFileCount_set
    __swig_getmethods__["VerFileCount"] = _apt.pkgCacheHeader_VerFileCount_get
    if _newclass:VerFileCount = property(_apt.pkgCacheHeader_VerFileCount_get, _apt.pkgCacheHeader_VerFileCount_set)
    __swig_setmethods__["ProvidesCount"] = _apt.pkgCacheHeader_ProvidesCount_set
    __swig_getmethods__["ProvidesCount"] = _apt.pkgCacheHeader_ProvidesCount_get
    if _newclass:ProvidesCount = property(_apt.pkgCacheHeader_ProvidesCount_get, _apt.pkgCacheHeader_ProvidesCount_set)
    __swig_setmethods__["FileList"] = _apt.pkgCacheHeader_FileList_set
    __swig_getmethods__["FileList"] = _apt.pkgCacheHeader_FileList_get
    if _newclass:FileList = property(_apt.pkgCacheHeader_FileList_get, _apt.pkgCacheHeader_FileList_set)
    __swig_setmethods__["StringList"] = _apt.pkgCacheHeader_StringList_set
    __swig_getmethods__["StringList"] = _apt.pkgCacheHeader_StringList_get
    if _newclass:StringList = property(_apt.pkgCacheHeader_StringList_get, _apt.pkgCacheHeader_StringList_set)
    __swig_setmethods__["VerSysName"] = _apt.pkgCacheHeader_VerSysName_set
    __swig_getmethods__["VerSysName"] = _apt.pkgCacheHeader_VerSysName_get
    if _newclass:VerSysName = property(_apt.pkgCacheHeader_VerSysName_get, _apt.pkgCacheHeader_VerSysName_set)
    __swig_setmethods__["Architecture"] = _apt.pkgCacheHeader_Architecture_set
    __swig_getmethods__["Architecture"] = _apt.pkgCacheHeader_Architecture_get
    if _newclass:Architecture = property(_apt.pkgCacheHeader_Architecture_get, _apt.pkgCacheHeader_Architecture_set)
    __swig_setmethods__["MaxVerFileSize"] = _apt.pkgCacheHeader_MaxVerFileSize_set
    __swig_getmethods__["MaxVerFileSize"] = _apt.pkgCacheHeader_MaxVerFileSize_get
    if _newclass:MaxVerFileSize = property(_apt.pkgCacheHeader_MaxVerFileSize_get, _apt.pkgCacheHeader_MaxVerFileSize_set)
    __swig_setmethods__["Pools"] = _apt.pkgCacheHeader_Pools_set
    __swig_getmethods__["Pools"] = _apt.pkgCacheHeader_Pools_get
    if _newclass:Pools = property(_apt.pkgCacheHeader_Pools_get, _apt.pkgCacheHeader_Pools_set)
    __swig_setmethods__["HashTable"] = _apt.pkgCacheHeader_HashTable_set
    __swig_getmethods__["HashTable"] = _apt.pkgCacheHeader_HashTable_get
    if _newclass:HashTable = property(_apt.pkgCacheHeader_HashTable_get, _apt.pkgCacheHeader_HashTable_set)
    def CheckSizes(*args): return _apt.pkgCacheHeader_CheckSizes(*args)
    def __init__(self, *args):
        _swig_setattr(self, pkgCacheHeader, 'this', _apt.new_pkgCacheHeader(*args))
        _swig_setattr(self, pkgCacheHeader, 'thisown', 1)
    def __del__(self, destroy=_apt.delete_pkgCacheHeader):
        try:
            if self.thisown: destroy(self)
        except: pass

class pkgCacheHeaderPtr(pkgCacheHeader):
    def __init__(self, this):
        _swig_setattr(self, pkgCacheHeader, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, pkgCacheHeader, 'thisown', 0)
        _swig_setattr(self, pkgCacheHeader,self.__class__,pkgCacheHeader)
_apt.pkgCacheHeader_swigregister(pkgCacheHeaderPtr)

class pkgCachePackage(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, pkgCachePackage, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, pkgCachePackage, name)
    def __repr__(self):
        return "<C pkgCache::Package instance at %s>" % (self.this,)
    __swig_setmethods__["Name"] = _apt.pkgCachePackage_Name_set
    __swig_getmethods__["Name"] = _apt.pkgCachePackage_Name_get
    if _newclass:Name = property(_apt.pkgCachePackage_Name_get, _apt.pkgCachePackage_Name_set)
    __swig_setmethods__["VersionList"] = _apt.pkgCachePackage_VersionList_set
    __swig_getmethods__["VersionList"] = _apt.pkgCachePackage_VersionList_get
    if _newclass:VersionList = property(_apt.pkgCachePackage_VersionList_get, _apt.pkgCachePackage_VersionList_set)
    __swig_setmethods__["CurrentVer"] = _apt.pkgCachePackage_CurrentVer_set
    __swig_getmethods__["CurrentVer"] = _apt.pkgCachePackage_CurrentVer_get
    if _newclass:CurrentVer = property(_apt.pkgCachePackage_CurrentVer_get, _apt.pkgCachePackage_CurrentVer_set)
    __swig_setmethods__["Section"] = _apt.pkgCachePackage_Section_set
    __swig_getmethods__["Section"] = _apt.pkgCachePackage_Section_get
    if _newclass:Section = property(_apt.pkgCachePackage_Section_get, _apt.pkgCachePackage_Section_set)
    __swig_setmethods__["NextPackage"] = _apt.pkgCachePackage_NextPackage_set
    __swig_getmethods__["NextPackage"] = _apt.pkgCachePackage_NextPackage_get
    if _newclass:NextPackage = property(_apt.pkgCachePackage_NextPackage_get, _apt.pkgCachePackage_NextPackage_set)
    __swig_setmethods__["RevDepends"] = _apt.pkgCachePackage_RevDepends_set
    __swig_getmethods__["RevDepends"] = _apt.pkgCachePackage_RevDepends_get
    if _newclass:RevDepends = property(_apt.pkgCachePackage_RevDepends_get, _apt.pkgCachePackage_RevDepends_set)
    __swig_setmethods__["ProvidesList"] = _apt.pkgCachePackage_ProvidesList_set
    __swig_getmethods__["ProvidesList"] = _apt.pkgCachePackage_ProvidesList_get
    if _newclass:ProvidesList = property(_apt.pkgCachePackage_ProvidesList_get, _apt.pkgCachePackage_ProvidesList_set)
    __swig_setmethods__["SelectedState"] = _apt.pkgCachePackage_SelectedState_set
    __swig_getmethods__["SelectedState"] = _apt.pkgCachePackage_SelectedState_get
    if _newclass:SelectedState = property(_apt.pkgCachePackage_SelectedState_get, _apt.pkgCachePackage_SelectedState_set)
    __swig_setmethods__["InstState"] = _apt.pkgCachePackage_InstState_set
    __swig_getmethods__["InstState"] = _apt.pkgCachePackage_InstState_get
    if _newclass:InstState = property(_apt.pkgCachePackage_InstState_get, _apt.pkgCachePackage_InstState_set)
    __swig_setmethods__["CurrentState"] = _apt.pkgCachePackage_CurrentState_set
    __swig_getmethods__["CurrentState"] = _apt.pkgCachePackage_CurrentState_get
    if _newclass:CurrentState = property(_apt.pkgCachePackage_CurrentState_get, _apt.pkgCachePackage_CurrentState_set)
    __swig_setmethods__["ID"] = _apt.pkgCachePackage_ID_set
    __swig_getmethods__["ID"] = _apt.pkgCachePackage_ID_get
    if _newclass:ID = property(_apt.pkgCachePackage_ID_get, _apt.pkgCachePackage_ID_set)
    __swig_setmethods__["Flags"] = _apt.pkgCachePackage_Flags_set
    __swig_getmethods__["Flags"] = _apt.pkgCachePackage_Flags_get
    if _newclass:Flags = property(_apt.pkgCachePackage_Flags_get, _apt.pkgCachePackage_Flags_set)
    def __init__(self, *args):
        _swig_setattr(self, pkgCachePackage, 'this', _apt.new_pkgCachePackage(*args))
        _swig_setattr(self, pkgCachePackage, 'thisown', 1)
    def __del__(self, destroy=_apt.delete_pkgCachePackage):
        try:
            if self.thisown: destroy(self)
        except: pass

class pkgCachePackagePtr(pkgCachePackage):
    def __init__(self, this):
        _swig_setattr(self, pkgCachePackage, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, pkgCachePackage, 'thisown', 0)
        _swig_setattr(self, pkgCachePackage,self.__class__,pkgCachePackage)
_apt.pkgCachePackage_swigregister(pkgCachePackagePtr)

class pkgCachePackageFile(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, pkgCachePackageFile, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, pkgCachePackageFile, name)
    def __repr__(self):
        return "<C pkgCache::PackageFile instance at %s>" % (self.this,)
    __swig_setmethods__["FileName"] = _apt.pkgCachePackageFile_FileName_set
    __swig_getmethods__["FileName"] = _apt.pkgCachePackageFile_FileName_get
    if _newclass:FileName = property(_apt.pkgCachePackageFile_FileName_get, _apt.pkgCachePackageFile_FileName_set)
    __swig_setmethods__["Archive"] = _apt.pkgCachePackageFile_Archive_set
    __swig_getmethods__["Archive"] = _apt.pkgCachePackageFile_Archive_get
    if _newclass:Archive = property(_apt.pkgCachePackageFile_Archive_get, _apt.pkgCachePackageFile_Archive_set)
    __swig_setmethods__["Component"] = _apt.pkgCachePackageFile_Component_set
    __swig_getmethods__["Component"] = _apt.pkgCachePackageFile_Component_get
    if _newclass:Component = property(_apt.pkgCachePackageFile_Component_get, _apt.pkgCachePackageFile_Component_set)
    __swig_setmethods__["Version"] = _apt.pkgCachePackageFile_Version_set
    __swig_getmethods__["Version"] = _apt.pkgCachePackageFile_Version_get
    if _newclass:Version = property(_apt.pkgCachePackageFile_Version_get, _apt.pkgCachePackageFile_Version_set)
    __swig_setmethods__["Origin"] = _apt.pkgCachePackageFile_Origin_set
    __swig_getmethods__["Origin"] = _apt.pkgCachePackageFile_Origin_get
    if _newclass:Origin = property(_apt.pkgCachePackageFile_Origin_get, _apt.pkgCachePackageFile_Origin_set)
    __swig_setmethods__["Label"] = _apt.pkgCachePackageFile_Label_set
    __swig_getmethods__["Label"] = _apt.pkgCachePackageFile_Label_get
    if _newclass:Label = property(_apt.pkgCachePackageFile_Label_get, _apt.pkgCachePackageFile_Label_set)
    __swig_setmethods__["Architecture"] = _apt.pkgCachePackageFile_Architecture_set
    __swig_getmethods__["Architecture"] = _apt.pkgCachePackageFile_Architecture_get
    if _newclass:Architecture = property(_apt.pkgCachePackageFile_Architecture_get, _apt.pkgCachePackageFile_Architecture_set)
    __swig_setmethods__["Site"] = _apt.pkgCachePackageFile_Site_set
    __swig_getmethods__["Site"] = _apt.pkgCachePackageFile_Site_get
    if _newclass:Site = property(_apt.pkgCachePackageFile_Site_get, _apt.pkgCachePackageFile_Site_set)
    __swig_setmethods__["IndexType"] = _apt.pkgCachePackageFile_IndexType_set
    __swig_getmethods__["IndexType"] = _apt.pkgCachePackageFile_IndexType_get
    if _newclass:IndexType = property(_apt.pkgCachePackageFile_IndexType_get, _apt.pkgCachePackageFile_IndexType_set)
    __swig_setmethods__["Size"] = _apt.pkgCachePackageFile_Size_set
    __swig_getmethods__["Size"] = _apt.pkgCachePackageFile_Size_get
    if _newclass:Size = property(_apt.pkgCachePackageFile_Size_get, _apt.pkgCachePackageFile_Size_set)
    __swig_setmethods__["Flags"] = _apt.pkgCachePackageFile_Flags_set
    __swig_getmethods__["Flags"] = _apt.pkgCachePackageFile_Flags_get
    if _newclass:Flags = property(_apt.pkgCachePackageFile_Flags_get, _apt.pkgCachePackageFile_Flags_set)
    __swig_setmethods__["NextFile"] = _apt.pkgCachePackageFile_NextFile_set
    __swig_getmethods__["NextFile"] = _apt.pkgCachePackageFile_NextFile_get
    if _newclass:NextFile = property(_apt.pkgCachePackageFile_NextFile_get, _apt.pkgCachePackageFile_NextFile_set)
    __swig_setmethods__["ID"] = _apt.pkgCachePackageFile_ID_set
    __swig_getmethods__["ID"] = _apt.pkgCachePackageFile_ID_get
    if _newclass:ID = property(_apt.pkgCachePackageFile_ID_get, _apt.pkgCachePackageFile_ID_set)
    __swig_setmethods__["mtime"] = _apt.pkgCachePackageFile_mtime_set
    __swig_getmethods__["mtime"] = _apt.pkgCachePackageFile_mtime_get
    if _newclass:mtime = property(_apt.pkgCachePackageFile_mtime_get, _apt.pkgCachePackageFile_mtime_set)
    def __init__(self, *args):
        _swig_setattr(self, pkgCachePackageFile, 'this', _apt.new_pkgCachePackageFile(*args))
        _swig_setattr(self, pkgCachePackageFile, 'thisown', 1)
    def __del__(self, destroy=_apt.delete_pkgCachePackageFile):
        try:
            if self.thisown: destroy(self)
        except: pass

class pkgCachePackageFilePtr(pkgCachePackageFile):
    def __init__(self, this):
        _swig_setattr(self, pkgCachePackageFile, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, pkgCachePackageFile, 'thisown', 0)
        _swig_setattr(self, pkgCachePackageFile,self.__class__,pkgCachePackageFile)
_apt.pkgCachePackageFile_swigregister(pkgCachePackageFilePtr)

class pkgCacheVerFile(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, pkgCacheVerFile, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, pkgCacheVerFile, name)
    def __repr__(self):
        return "<C pkgCache::VerFile instance at %s>" % (self.this,)
    __swig_setmethods__["File"] = _apt.pkgCacheVerFile_File_set
    __swig_getmethods__["File"] = _apt.pkgCacheVerFile_File_get
    if _newclass:File = property(_apt.pkgCacheVerFile_File_get, _apt.pkgCacheVerFile_File_set)
    __swig_setmethods__["NextFile"] = _apt.pkgCacheVerFile_NextFile_set
    __swig_getmethods__["NextFile"] = _apt.pkgCacheVerFile_NextFile_get
    if _newclass:NextFile = property(_apt.pkgCacheVerFile_NextFile_get, _apt.pkgCacheVerFile_NextFile_set)
    __swig_setmethods__["Offset"] = _apt.pkgCacheVerFile_Offset_set
    __swig_getmethods__["Offset"] = _apt.pkgCacheVerFile_Offset_get
    if _newclass:Offset = property(_apt.pkgCacheVerFile_Offset_get, _apt.pkgCacheVerFile_Offset_set)
    __swig_setmethods__["Size"] = _apt.pkgCacheVerFile_Size_set
    __swig_getmethods__["Size"] = _apt.pkgCacheVerFile_Size_get
    if _newclass:Size = property(_apt.pkgCacheVerFile_Size_get, _apt.pkgCacheVerFile_Size_set)
    def __init__(self, *args):
        _swig_setattr(self, pkgCacheVerFile, 'this', _apt.new_pkgCacheVerFile(*args))
        _swig_setattr(self, pkgCacheVerFile, 'thisown', 1)
    def __del__(self, destroy=_apt.delete_pkgCacheVerFile):
        try:
            if self.thisown: destroy(self)
        except: pass

class pkgCacheVerFilePtr(pkgCacheVerFile):
    def __init__(self, this):
        _swig_setattr(self, pkgCacheVerFile, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, pkgCacheVerFile, 'thisown', 0)
        _swig_setattr(self, pkgCacheVerFile,self.__class__,pkgCacheVerFile)
_apt.pkgCacheVerFile_swigregister(pkgCacheVerFilePtr)

class pkgCacheVersion(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, pkgCacheVersion, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, pkgCacheVersion, name)
    def __repr__(self):
        return "<C pkgCache::Version instance at %s>" % (self.this,)
    __swig_setmethods__["VerStr"] = _apt.pkgCacheVersion_VerStr_set
    __swig_getmethods__["VerStr"] = _apt.pkgCacheVersion_VerStr_get
    if _newclass:VerStr = property(_apt.pkgCacheVersion_VerStr_get, _apt.pkgCacheVersion_VerStr_set)
    __swig_setmethods__["Section"] = _apt.pkgCacheVersion_Section_set
    __swig_getmethods__["Section"] = _apt.pkgCacheVersion_Section_get
    if _newclass:Section = property(_apt.pkgCacheVersion_Section_get, _apt.pkgCacheVersion_Section_set)
    __swig_setmethods__["Arch"] = _apt.pkgCacheVersion_Arch_set
    __swig_getmethods__["Arch"] = _apt.pkgCacheVersion_Arch_get
    if _newclass:Arch = property(_apt.pkgCacheVersion_Arch_get, _apt.pkgCacheVersion_Arch_set)
    __swig_setmethods__["FileList"] = _apt.pkgCacheVersion_FileList_set
    __swig_getmethods__["FileList"] = _apt.pkgCacheVersion_FileList_get
    if _newclass:FileList = property(_apt.pkgCacheVersion_FileList_get, _apt.pkgCacheVersion_FileList_set)
    __swig_setmethods__["NextVer"] = _apt.pkgCacheVersion_NextVer_set
    __swig_getmethods__["NextVer"] = _apt.pkgCacheVersion_NextVer_get
    if _newclass:NextVer = property(_apt.pkgCacheVersion_NextVer_get, _apt.pkgCacheVersion_NextVer_set)
    __swig_setmethods__["DependsList"] = _apt.pkgCacheVersion_DependsList_set
    __swig_getmethods__["DependsList"] = _apt.pkgCacheVersion_DependsList_get
    if _newclass:DependsList = property(_apt.pkgCacheVersion_DependsList_get, _apt.pkgCacheVersion_DependsList_set)
    __swig_setmethods__["ParentPkg"] = _apt.pkgCacheVersion_ParentPkg_set
    __swig_getmethods__["ParentPkg"] = _apt.pkgCacheVersion_ParentPkg_get
    if _newclass:ParentPkg = property(_apt.pkgCacheVersion_ParentPkg_get, _apt.pkgCacheVersion_ParentPkg_set)
    __swig_setmethods__["ProvidesList"] = _apt.pkgCacheVersion_ProvidesList_set
    __swig_getmethods__["ProvidesList"] = _apt.pkgCacheVersion_ProvidesList_get
    if _newclass:ProvidesList = property(_apt.pkgCacheVersion_ProvidesList_get, _apt.pkgCacheVersion_ProvidesList_set)
    __swig_setmethods__["Size"] = _apt.pkgCacheVersion_Size_set
    __swig_getmethods__["Size"] = _apt.pkgCacheVersion_Size_get
    if _newclass:Size = property(_apt.pkgCacheVersion_Size_get, _apt.pkgCacheVersion_Size_set)
    __swig_setmethods__["InstalledSize"] = _apt.pkgCacheVersion_InstalledSize_set
    __swig_getmethods__["InstalledSize"] = _apt.pkgCacheVersion_InstalledSize_get
    if _newclass:InstalledSize = property(_apt.pkgCacheVersion_InstalledSize_get, _apt.pkgCacheVersion_InstalledSize_set)
    __swig_setmethods__["Hash"] = _apt.pkgCacheVersion_Hash_set
    __swig_getmethods__["Hash"] = _apt.pkgCacheVersion_Hash_get
    if _newclass:Hash = property(_apt.pkgCacheVersion_Hash_get, _apt.pkgCacheVersion_Hash_set)
    __swig_setmethods__["ID"] = _apt.pkgCacheVersion_ID_set
    __swig_getmethods__["ID"] = _apt.pkgCacheVersion_ID_get
    if _newclass:ID = property(_apt.pkgCacheVersion_ID_get, _apt.pkgCacheVersion_ID_set)
    __swig_setmethods__["Priority"] = _apt.pkgCacheVersion_Priority_set
    __swig_getmethods__["Priority"] = _apt.pkgCacheVersion_Priority_get
    if _newclass:Priority = property(_apt.pkgCacheVersion_Priority_get, _apt.pkgCacheVersion_Priority_set)
    def __init__(self, *args):
        _swig_setattr(self, pkgCacheVersion, 'this', _apt.new_pkgCacheVersion(*args))
        _swig_setattr(self, pkgCacheVersion, 'thisown', 1)
    def __del__(self, destroy=_apt.delete_pkgCacheVersion):
        try:
            if self.thisown: destroy(self)
        except: pass

class pkgCacheVersionPtr(pkgCacheVersion):
    def __init__(self, this):
        _swig_setattr(self, pkgCacheVersion, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, pkgCacheVersion, 'thisown', 0)
        _swig_setattr(self, pkgCacheVersion,self.__class__,pkgCacheVersion)
_apt.pkgCacheVersion_swigregister(pkgCacheVersionPtr)

class pkgCacheDependency(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, pkgCacheDependency, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, pkgCacheDependency, name)
    def __repr__(self):
        return "<C pkgCache::Dependency instance at %s>" % (self.this,)
    __swig_setmethods__["Version"] = _apt.pkgCacheDependency_Version_set
    __swig_getmethods__["Version"] = _apt.pkgCacheDependency_Version_get
    if _newclass:Version = property(_apt.pkgCacheDependency_Version_get, _apt.pkgCacheDependency_Version_set)
    __swig_setmethods__["Package"] = _apt.pkgCacheDependency_Package_set
    __swig_getmethods__["Package"] = _apt.pkgCacheDependency_Package_get
    if _newclass:Package = property(_apt.pkgCacheDependency_Package_get, _apt.pkgCacheDependency_Package_set)
    __swig_setmethods__["NextDepends"] = _apt.pkgCacheDependency_NextDepends_set
    __swig_getmethods__["NextDepends"] = _apt.pkgCacheDependency_NextDepends_get
    if _newclass:NextDepends = property(_apt.pkgCacheDependency_NextDepends_get, _apt.pkgCacheDependency_NextDepends_set)
    __swig_setmethods__["NextRevDepends"] = _apt.pkgCacheDependency_NextRevDepends_set
    __swig_getmethods__["NextRevDepends"] = _apt.pkgCacheDependency_NextRevDepends_get
    if _newclass:NextRevDepends = property(_apt.pkgCacheDependency_NextRevDepends_get, _apt.pkgCacheDependency_NextRevDepends_set)
    __swig_setmethods__["ParentVer"] = _apt.pkgCacheDependency_ParentVer_set
    __swig_getmethods__["ParentVer"] = _apt.pkgCacheDependency_ParentVer_get
    if _newclass:ParentVer = property(_apt.pkgCacheDependency_ParentVer_get, _apt.pkgCacheDependency_ParentVer_set)
    __swig_setmethods__["ID"] = _apt.pkgCacheDependency_ID_set
    __swig_getmethods__["ID"] = _apt.pkgCacheDependency_ID_get
    if _newclass:ID = property(_apt.pkgCacheDependency_ID_get, _apt.pkgCacheDependency_ID_set)
    __swig_setmethods__["Type"] = _apt.pkgCacheDependency_Type_set
    __swig_getmethods__["Type"] = _apt.pkgCacheDependency_Type_get
    if _newclass:Type = property(_apt.pkgCacheDependency_Type_get, _apt.pkgCacheDependency_Type_set)
    __swig_setmethods__["CompareOp"] = _apt.pkgCacheDependency_CompareOp_set
    __swig_getmethods__["CompareOp"] = _apt.pkgCacheDependency_CompareOp_get
    if _newclass:CompareOp = property(_apt.pkgCacheDependency_CompareOp_get, _apt.pkgCacheDependency_CompareOp_set)
    def __init__(self, *args):
        _swig_setattr(self, pkgCacheDependency, 'this', _apt.new_pkgCacheDependency(*args))
        _swig_setattr(self, pkgCacheDependency, 'thisown', 1)
    def __del__(self, destroy=_apt.delete_pkgCacheDependency):
        try:
            if self.thisown: destroy(self)
        except: pass

class pkgCacheDependencyPtr(pkgCacheDependency):
    def __init__(self, this):
        _swig_setattr(self, pkgCacheDependency, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, pkgCacheDependency, 'thisown', 0)
        _swig_setattr(self, pkgCacheDependency,self.__class__,pkgCacheDependency)
_apt.pkgCacheDependency_swigregister(pkgCacheDependencyPtr)

class pkgCacheProvides(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, pkgCacheProvides, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, pkgCacheProvides, name)
    def __repr__(self):
        return "<C pkgCache::Provides instance at %s>" % (self.this,)
    __swig_setmethods__["ParentPkg"] = _apt.pkgCacheProvides_ParentPkg_set
    __swig_getmethods__["ParentPkg"] = _apt.pkgCacheProvides_ParentPkg_get
    if _newclass:ParentPkg = property(_apt.pkgCacheProvides_ParentPkg_get, _apt.pkgCacheProvides_ParentPkg_set)
    __swig_setmethods__["Version"] = _apt.pkgCacheProvides_Version_set
    __swig_getmethods__["Version"] = _apt.pkgCacheProvides_Version_get
    if _newclass:Version = property(_apt.pkgCacheProvides_Version_get, _apt.pkgCacheProvides_Version_set)
    __swig_setmethods__["ProvideVersion"] = _apt.pkgCacheProvides_ProvideVersion_set
    __swig_getmethods__["ProvideVersion"] = _apt.pkgCacheProvides_ProvideVersion_get
    if _newclass:ProvideVersion = property(_apt.pkgCacheProvides_ProvideVersion_get, _apt.pkgCacheProvides_ProvideVersion_set)
    __swig_setmethods__["NextProvides"] = _apt.pkgCacheProvides_NextProvides_set
    __swig_getmethods__["NextProvides"] = _apt.pkgCacheProvides_NextProvides_get
    if _newclass:NextProvides = property(_apt.pkgCacheProvides_NextProvides_get, _apt.pkgCacheProvides_NextProvides_set)
    __swig_setmethods__["NextPkgProv"] = _apt.pkgCacheProvides_NextPkgProv_set
    __swig_getmethods__["NextPkgProv"] = _apt.pkgCacheProvides_NextPkgProv_get
    if _newclass:NextPkgProv = property(_apt.pkgCacheProvides_NextPkgProv_get, _apt.pkgCacheProvides_NextPkgProv_set)
    def __init__(self, *args):
        _swig_setattr(self, pkgCacheProvides, 'this', _apt.new_pkgCacheProvides(*args))
        _swig_setattr(self, pkgCacheProvides, 'thisown', 1)
    def __del__(self, destroy=_apt.delete_pkgCacheProvides):
        try:
            if self.thisown: destroy(self)
        except: pass

class pkgCacheProvidesPtr(pkgCacheProvides):
    def __init__(self, this):
        _swig_setattr(self, pkgCacheProvides, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, pkgCacheProvides, 'thisown', 0)
        _swig_setattr(self, pkgCacheProvides,self.__class__,pkgCacheProvides)
_apt.pkgCacheProvides_swigregister(pkgCacheProvidesPtr)

class pkgCacheStringItem(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, pkgCacheStringItem, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, pkgCacheStringItem, name)
    def __repr__(self):
        return "<C pkgCache::StringItem instance at %s>" % (self.this,)
    __swig_setmethods__["String"] = _apt.pkgCacheStringItem_String_set
    __swig_getmethods__["String"] = _apt.pkgCacheStringItem_String_get
    if _newclass:String = property(_apt.pkgCacheStringItem_String_get, _apt.pkgCacheStringItem_String_set)
    __swig_setmethods__["NextItem"] = _apt.pkgCacheStringItem_NextItem_set
    __swig_getmethods__["NextItem"] = _apt.pkgCacheStringItem_NextItem_get
    if _newclass:NextItem = property(_apt.pkgCacheStringItem_NextItem_get, _apt.pkgCacheStringItem_NextItem_set)
    def __init__(self, *args):
        _swig_setattr(self, pkgCacheStringItem, 'this', _apt.new_pkgCacheStringItem(*args))
        _swig_setattr(self, pkgCacheStringItem, 'thisown', 1)
    def __del__(self, destroy=_apt.delete_pkgCacheStringItem):
        try:
            if self.thisown: destroy(self)
        except: pass

class pkgCacheStringItemPtr(pkgCacheStringItem):
    def __init__(self, this):
        _swig_setattr(self, pkgCacheStringItem, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, pkgCacheStringItem, 'thisown', 0)
        _swig_setattr(self, pkgCacheStringItem,self.__class__,pkgCacheStringItem)
_apt.pkgCacheStringItem_swigregister(pkgCacheStringItemPtr)

class pkgDepCache(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, pkgDepCache, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, pkgDepCache, name)
    def __repr__(self):
        return "<C pkgDepCache instance at %s>" % (self.this,)
    DepNow = _apt.pkgDepCache_DepNow
    DepInstall = _apt.pkgDepCache_DepInstall
    DepCVer = _apt.pkgDepCache_DepCVer
    DepGNow = _apt.pkgDepCache_DepGNow
    DepGInstall = _apt.pkgDepCache_DepGInstall
    DepGCVer = _apt.pkgDepCache_DepGCVer
    DepNowPolicy = _apt.pkgDepCache_DepNowPolicy
    DepNowMin = _apt.pkgDepCache_DepNowMin
    DepInstPolicy = _apt.pkgDepCache_DepInstPolicy
    DepInstMin = _apt.pkgDepCache_DepInstMin
    DepCandPolicy = _apt.pkgDepCache_DepCandPolicy
    DepCandMin = _apt.pkgDepCache_DepCandMin
    AutoKept = _apt.pkgDepCache_AutoKept
    Purge = _apt.pkgDepCache_Purge
    ReInstall = _apt.pkgDepCache_ReInstall
    NowVersion = _apt.pkgDepCache_NowVersion
    InstallVersion = _apt.pkgDepCache_InstallVersion
    CandidateVersion = _apt.pkgDepCache_CandidateVersion
    ModeDelete = _apt.pkgDepCache_ModeDelete
    ModeKeep = _apt.pkgDepCache_ModeKeep
    ModeInstall = _apt.pkgDepCache_ModeInstall
    def BuildGroupOrs(*args): return _apt.pkgDepCache_BuildGroupOrs(*args)
    def UpdateVerState(*args): return _apt.pkgDepCache_UpdateVerState(*args)
    def Head(*args): return _apt.pkgDepCache_Head(*args)
    def PkgIter(*args): return _apt.pkgDepCache_PkgIter(*args)
    def FindPkg(*args): return _apt.pkgDepCache_FindPkg(*args)
    def GetCache(*args): return _apt.pkgDepCache_GetCache(*args)
    def VS(*args): return _apt.pkgDepCache_VS(*args)
    def GetCandidateVer(*args): return _apt.pkgDepCache_GetCandidateVer(*args)
    def IsImportantDep(*args): return _apt.pkgDepCache_IsImportantDep(*args)
    def GetPolicy(*args): return _apt.pkgDepCache_GetPolicy(*args)
    def GetPkgPriority(*args): return _apt.pkgDepCache_GetPkgPriority(*args)
    def __getitem__(*args): return _apt.pkgDepCache___getitem__(*args)
    def MarkKeep(*args): return _apt.pkgDepCache_MarkKeep(*args)
    def MarkDelete(*args): return _apt.pkgDepCache_MarkDelete(*args)
    def MarkInstall(*args): return _apt.pkgDepCache_MarkInstall(*args)
    def SetReInstall(*args): return _apt.pkgDepCache_SetReInstall(*args)
    def SetCandidateVersion(*args): return _apt.pkgDepCache_SetCandidateVersion(*args)
    def Update(*args): return _apt.pkgDepCache_Update(*args)
    def UsrSize(*args): return _apt.pkgDepCache_UsrSize(*args)
    def DebSize(*args): return _apt.pkgDepCache_DebSize(*args)
    def DelCount(*args): return _apt.pkgDepCache_DelCount(*args)
    def KeepCount(*args): return _apt.pkgDepCache_KeepCount(*args)
    def InstCount(*args): return _apt.pkgDepCache_InstCount(*args)
    def BrokenCount(*args): return _apt.pkgDepCache_BrokenCount(*args)
    def BadCount(*args): return _apt.pkgDepCache_BadCount(*args)
    def Init(*args): return _apt.pkgDepCache_Init(*args)
    def __init__(self, *args):
        _swig_setattr(self, pkgDepCache, 'this', _apt.new_pkgDepCache(*args))
        _swig_setattr(self, pkgDepCache, 'thisown', 1)
    def __del__(self, destroy=_apt.delete_pkgDepCache):
        try:
            if self.thisown: destroy(self)
        except: pass

class pkgDepCachePtr(pkgDepCache):
    def __init__(self, this):
        _swig_setattr(self, pkgDepCache, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, pkgDepCache, 'thisown', 0)
        _swig_setattr(self, pkgDepCache,self.__class__,pkgDepCache)
_apt.pkgDepCache_swigregister(pkgDepCachePtr)

class pkgDepCacheStateCache(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, pkgDepCacheStateCache, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, pkgDepCacheStateCache, name)
    def __repr__(self):
        return "<C pkgDepCache::StateCache instance at %s>" % (self.this,)
    __swig_getmethods__["CandVersion"] = _apt.pkgDepCacheStateCache_CandVersion_get
    if _newclass:CandVersion = property(_apt.pkgDepCacheStateCache_CandVersion_get)
    __swig_getmethods__["CurVersion"] = _apt.pkgDepCacheStateCache_CurVersion_get
    if _newclass:CurVersion = property(_apt.pkgDepCacheStateCache_CurVersion_get)
    __swig_setmethods__["CandidateVer"] = _apt.pkgDepCacheStateCache_CandidateVer_set
    __swig_getmethods__["CandidateVer"] = _apt.pkgDepCacheStateCache_CandidateVer_get
    if _newclass:CandidateVer = property(_apt.pkgDepCacheStateCache_CandidateVer_get, _apt.pkgDepCacheStateCache_CandidateVer_set)
    __swig_setmethods__["InstallVer"] = _apt.pkgDepCacheStateCache_InstallVer_set
    __swig_getmethods__["InstallVer"] = _apt.pkgDepCacheStateCache_InstallVer_get
    if _newclass:InstallVer = property(_apt.pkgDepCacheStateCache_InstallVer_get, _apt.pkgDepCacheStateCache_InstallVer_set)
    __swig_setmethods__["Flags"] = _apt.pkgDepCacheStateCache_Flags_set
    __swig_getmethods__["Flags"] = _apt.pkgDepCacheStateCache_Flags_get
    if _newclass:Flags = property(_apt.pkgDepCacheStateCache_Flags_get, _apt.pkgDepCacheStateCache_Flags_set)
    __swig_setmethods__["iFlags"] = _apt.pkgDepCacheStateCache_iFlags_set
    __swig_getmethods__["iFlags"] = _apt.pkgDepCacheStateCache_iFlags_get
    if _newclass:iFlags = property(_apt.pkgDepCacheStateCache_iFlags_get, _apt.pkgDepCacheStateCache_iFlags_set)
    __swig_setmethods__["Status"] = _apt.pkgDepCacheStateCache_Status_set
    __swig_getmethods__["Status"] = _apt.pkgDepCacheStateCache_Status_get
    if _newclass:Status = property(_apt.pkgDepCacheStateCache_Status_get, _apt.pkgDepCacheStateCache_Status_set)
    __swig_setmethods__["Mode"] = _apt.pkgDepCacheStateCache_Mode_set
    __swig_getmethods__["Mode"] = _apt.pkgDepCacheStateCache_Mode_get
    if _newclass:Mode = property(_apt.pkgDepCacheStateCache_Mode_get, _apt.pkgDepCacheStateCache_Mode_set)
    __swig_setmethods__["DepState"] = _apt.pkgDepCacheStateCache_DepState_set
    __swig_getmethods__["DepState"] = _apt.pkgDepCacheStateCache_DepState_get
    if _newclass:DepState = property(_apt.pkgDepCacheStateCache_DepState_get, _apt.pkgDepCacheStateCache_DepState_set)
    def StripEpoch(*args): return _apt.pkgDepCacheStateCache_StripEpoch(*args)
    def Update(*args): return _apt.pkgDepCacheStateCache_Update(*args)
    def NewInstall(*args): return _apt.pkgDepCacheStateCache_NewInstall(*args)
    def Delete(*args): return _apt.pkgDepCacheStateCache_Delete(*args)
    def Keep(*args): return _apt.pkgDepCacheStateCache_Keep(*args)
    def Upgrade(*args): return _apt.pkgDepCacheStateCache_Upgrade(*args)
    def Upgradable(*args): return _apt.pkgDepCacheStateCache_Upgradable(*args)
    def Downgrade(*args): return _apt.pkgDepCacheStateCache_Downgrade(*args)
    def Held(*args): return _apt.pkgDepCacheStateCache_Held(*args)
    def NowBroken(*args): return _apt.pkgDepCacheStateCache_NowBroken(*args)
    def InstBroken(*args): return _apt.pkgDepCacheStateCache_InstBroken(*args)
    def Install(*args): return _apt.pkgDepCacheStateCache_Install(*args)
    def InstVerIter(*args): return _apt.pkgDepCacheStateCache_InstVerIter(*args)
    def CandidateVerIter(*args): return _apt.pkgDepCacheStateCache_CandidateVerIter(*args)
    def __init__(self, *args):
        _swig_setattr(self, pkgDepCacheStateCache, 'this', _apt.new_pkgDepCacheStateCache(*args))
        _swig_setattr(self, pkgDepCacheStateCache, 'thisown', 1)
    def __del__(self, destroy=_apt.delete_pkgDepCacheStateCache):
        try:
            if self.thisown: destroy(self)
        except: pass

class pkgDepCacheStateCachePtr(pkgDepCacheStateCache):
    def __init__(self, this):
        _swig_setattr(self, pkgDepCacheStateCache, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, pkgDepCacheStateCache, 'thisown', 0)
        _swig_setattr(self, pkgDepCacheStateCache,self.__class__,pkgDepCacheStateCache)
_apt.pkgDepCacheStateCache_swigregister(pkgDepCacheStateCachePtr)

class pkgCachePkgIterator(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, pkgCachePkgIterator, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, pkgCachePkgIterator, name)
    def __repr__(self):
        return "<C pkgCache::PkgIterator instance at %s>" % (self.this,)
    def __nonzero__(*args): return _apt.pkgCachePkgIterator___nonzero__(*args)
    def next(*args): return _apt.pkgCachePkgIterator_next(*args)
    def __iter__(*args): return _apt.pkgCachePkgIterator___iter__(*args)
    NeedsNothing = _apt.pkgCachePkgIterator_NeedsNothing
    NeedsUnpack = _apt.pkgCachePkgIterator_NeedsUnpack
    NeedsConfigure = _apt.pkgCachePkgIterator_NeedsConfigure
    def end(*args): return _apt.pkgCachePkgIterator_end(*args)
    def __eq__(*args): return _apt.pkgCachePkgIterator___eq__(*args)
    def __ne__(*args): return _apt.pkgCachePkgIterator___ne__(*args)
    def __deref__(*args): return _apt.pkgCachePkgIterator___deref__(*args)
    def __mul__(*args): return _apt.pkgCachePkgIterator___mul__(*args)
    def Package(*args): return _apt.pkgCachePkgIterator_Package(*args)
    def Cache(*args): return _apt.pkgCachePkgIterator_Cache(*args)
    def Name(*args): return _apt.pkgCachePkgIterator_Name(*args)
    def Section(*args): return _apt.pkgCachePkgIterator_Section(*args)
    def Purge(*args): return _apt.pkgCachePkgIterator_Purge(*args)
    def VersionList(*args): return _apt.pkgCachePkgIterator_VersionList(*args)
    def CurrentVer(*args): return _apt.pkgCachePkgIterator_CurrentVer(*args)
    def RevDependsList(*args): return _apt.pkgCachePkgIterator_RevDependsList(*args)
    def ProvidesList(*args): return _apt.pkgCachePkgIterator_ProvidesList(*args)
    def Index(*args): return _apt.pkgCachePkgIterator_Index(*args)
    def State(*args): return _apt.pkgCachePkgIterator_State(*args)
    def __init__(self, *args):
        _swig_setattr(self, pkgCachePkgIterator, 'this', _apt.new_pkgCachePkgIterator(*args))
        _swig_setattr(self, pkgCachePkgIterator, 'thisown', 1)
    __swig_getmethods__["NextPackage"] = _apt.pkgCachePkgIterator_NextPackage_get
    if _newclass:NextPackage = property(_apt.pkgCachePkgIterator_NextPackage_get)
    __swig_getmethods__["RevDepends"] = _apt.pkgCachePkgIterator_RevDepends_get
    if _newclass:RevDepends = property(_apt.pkgCachePkgIterator_RevDepends_get)
    __swig_getmethods__["SelectedState"] = _apt.pkgCachePkgIterator_SelectedState_get
    if _newclass:SelectedState = property(_apt.pkgCachePkgIterator_SelectedState_get)
    __swig_getmethods__["InstState"] = _apt.pkgCachePkgIterator_InstState_get
    if _newclass:InstState = property(_apt.pkgCachePkgIterator_InstState_get)
    __swig_getmethods__["CurrentState"] = _apt.pkgCachePkgIterator_CurrentState_get
    if _newclass:CurrentState = property(_apt.pkgCachePkgIterator_CurrentState_get)
    __swig_getmethods__["ID"] = _apt.pkgCachePkgIterator_ID_get
    if _newclass:ID = property(_apt.pkgCachePkgIterator_ID_get)
    __swig_getmethods__["Flags"] = _apt.pkgCachePkgIterator_Flags_get
    if _newclass:Flags = property(_apt.pkgCachePkgIterator_Flags_get)
    def __del__(self, destroy=_apt.delete_pkgCachePkgIterator):
        try:
            if self.thisown: destroy(self)
        except: pass

class pkgCachePkgIteratorPtr(pkgCachePkgIterator):
    def __init__(self, this):
        _swig_setattr(self, pkgCachePkgIterator, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, pkgCachePkgIterator, 'thisown', 0)
        _swig_setattr(self, pkgCachePkgIterator,self.__class__,pkgCachePkgIterator)
_apt.pkgCachePkgIterator_swigregister(pkgCachePkgIteratorPtr)

class pkgCacheVerIterator(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, pkgCacheVerIterator, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, pkgCacheVerIterator, name)
    def __repr__(self):
        return "<C pkgCache::VerIterator instance at %s>" % (self.this,)
    def __nonzero__(*args): return _apt.pkgCacheVerIterator___nonzero__(*args)
    def next(*args): return _apt.pkgCacheVerIterator_next(*args)
    def __iter__(*args): return _apt.pkgCacheVerIterator___iter__(*args)
    def end(*args): return _apt.pkgCacheVerIterator_end(*args)
    def assign(*args): return _apt.pkgCacheVerIterator_assign(*args)
    def __eq__(*args): return _apt.pkgCacheVerIterator___eq__(*args)
    def __ne__(*args): return _apt.pkgCacheVerIterator___ne__(*args)
    def CompareVer(*args): return _apt.pkgCacheVerIterator_CompareVer(*args)
    def __deref__(*args): return _apt.pkgCacheVerIterator___deref__(*args)
    def __mul__(*args): return _apt.pkgCacheVerIterator___mul__(*args)
    def Version(*args): return _apt.pkgCacheVerIterator_Version(*args)
    def Cache(*args): return _apt.pkgCacheVerIterator_Cache(*args)
    def VerStr(*args): return _apt.pkgCacheVerIterator_VerStr(*args)
    def Section(*args): return _apt.pkgCacheVerIterator_Section(*args)
    def Arch(*args): return _apt.pkgCacheVerIterator_Arch(*args)
    def ParentPkg(*args): return _apt.pkgCacheVerIterator_ParentPkg(*args)
    def DependsList(*args): return _apt.pkgCacheVerIterator_DependsList(*args)
    def ProvidesList(*args): return _apt.pkgCacheVerIterator_ProvidesList(*args)
    def FileList(*args): return _apt.pkgCacheVerIterator_FileList(*args)
    def Index(*args): return _apt.pkgCacheVerIterator_Index(*args)
    def Downloadable(*args): return _apt.pkgCacheVerIterator_Downloadable(*args)
    def PriorityType(*args): return _apt.pkgCacheVerIterator_PriorityType(*args)
    def RelStr(*args): return _apt.pkgCacheVerIterator_RelStr(*args)
    def Automatic(*args): return _apt.pkgCacheVerIterator_Automatic(*args)
    def NewestFile(*args): return _apt.pkgCacheVerIterator_NewestFile(*args)
    def __init__(self, *args):
        _swig_setattr(self, pkgCacheVerIterator, 'this', _apt.new_pkgCacheVerIterator(*args))
        _swig_setattr(self, pkgCacheVerIterator, 'thisown', 1)
    __swig_getmethods__["NextVer"] = _apt.pkgCacheVerIterator_NextVer_get
    if _newclass:NextVer = property(_apt.pkgCacheVerIterator_NextVer_get)
    __swig_getmethods__["Size"] = _apt.pkgCacheVerIterator_Size_get
    if _newclass:Size = property(_apt.pkgCacheVerIterator_Size_get)
    __swig_getmethods__["InstalledSize"] = _apt.pkgCacheVerIterator_InstalledSize_get
    if _newclass:InstalledSize = property(_apt.pkgCacheVerIterator_InstalledSize_get)
    __swig_getmethods__["Hash"] = _apt.pkgCacheVerIterator_Hash_get
    if _newclass:Hash = property(_apt.pkgCacheVerIterator_Hash_get)
    __swig_getmethods__["ID"] = _apt.pkgCacheVerIterator_ID_get
    if _newclass:ID = property(_apt.pkgCacheVerIterator_ID_get)
    __swig_getmethods__["Priority"] = _apt.pkgCacheVerIterator_Priority_get
    if _newclass:Priority = property(_apt.pkgCacheVerIterator_Priority_get)
    def __del__(self, destroy=_apt.delete_pkgCacheVerIterator):
        try:
            if self.thisown: destroy(self)
        except: pass

class pkgCacheVerIteratorPtr(pkgCacheVerIterator):
    def __init__(self, this):
        _swig_setattr(self, pkgCacheVerIterator, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, pkgCacheVerIterator, 'thisown', 0)
        _swig_setattr(self, pkgCacheVerIterator,self.__class__,pkgCacheVerIterator)
_apt.pkgCacheVerIterator_swigregister(pkgCacheVerIteratorPtr)

class pkgCacheDepIterator(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, pkgCacheDepIterator, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, pkgCacheDepIterator, name)
    def __repr__(self):
        return "<C pkgCache::DepIterator instance at %s>" % (self.this,)
    def __nonzero__(*args): return _apt.pkgCacheDepIterator___nonzero__(*args)
    def next(*args): return _apt.pkgCacheDepIterator_next(*args)
    def __iter__(*args): return _apt.pkgCacheDepIterator___iter__(*args)
    def end(*args): return _apt.pkgCacheDepIterator_end(*args)
    def __eq__(*args): return _apt.pkgCacheDepIterator___eq__(*args)
    def __ne__(*args): return _apt.pkgCacheDepIterator___ne__(*args)
    def __deref__(*args): return _apt.pkgCacheDepIterator___deref__(*args)
    def __mul__(*args): return _apt.pkgCacheDepIterator___mul__(*args)
    def Dependency(*args): return _apt.pkgCacheDepIterator_Dependency(*args)
    def Cache(*args): return _apt.pkgCacheDepIterator_Cache(*args)
    def TargetVer(*args): return _apt.pkgCacheDepIterator_TargetVer(*args)
    def TargetPkg(*args): return _apt.pkgCacheDepIterator_TargetPkg(*args)
    def ParentVer(*args): return _apt.pkgCacheDepIterator_ParentVer(*args)
    def ParentPkg(*args): return _apt.pkgCacheDepIterator_ParentPkg(*args)
    def Reverse(*args): return _apt.pkgCacheDepIterator_Reverse(*args)
    def Index(*args): return _apt.pkgCacheDepIterator_Index(*args)
    def IsCritical(*args): return _apt.pkgCacheDepIterator_IsCritical(*args)
    def GlobOr(*args): return _apt.pkgCacheDepIterator_GlobOr(*args)
    def AllTargets(*args): return _apt.pkgCacheDepIterator_AllTargets(*args)
    def SmartTargetPkg(*args): return _apt.pkgCacheDepIterator_SmartTargetPkg(*args)
    def CompType(*args): return _apt.pkgCacheDepIterator_CompType(*args)
    def DepType(*args): return _apt.pkgCacheDepIterator_DepType(*args)
    def __init__(self, *args):
        _swig_setattr(self, pkgCacheDepIterator, 'this', _apt.new_pkgCacheDepIterator(*args))
        _swig_setattr(self, pkgCacheDepIterator, 'thisown', 1)
    __swig_getmethods__["Version"] = _apt.pkgCacheDepIterator_Version_get
    if _newclass:Version = property(_apt.pkgCacheDepIterator_Version_get)
    __swig_getmethods__["Package"] = _apt.pkgCacheDepIterator_Package_get
    if _newclass:Package = property(_apt.pkgCacheDepIterator_Package_get)
    __swig_getmethods__["NextDepends"] = _apt.pkgCacheDepIterator_NextDepends_get
    if _newclass:NextDepends = property(_apt.pkgCacheDepIterator_NextDepends_get)
    __swig_getmethods__["NextRevDepends"] = _apt.pkgCacheDepIterator_NextRevDepends_get
    if _newclass:NextRevDepends = property(_apt.pkgCacheDepIterator_NextRevDepends_get)
    __swig_getmethods__["ID"] = _apt.pkgCacheDepIterator_ID_get
    if _newclass:ID = property(_apt.pkgCacheDepIterator_ID_get)
    __swig_getmethods__["Type"] = _apt.pkgCacheDepIterator_Type_get
    if _newclass:Type = property(_apt.pkgCacheDepIterator_Type_get)
    __swig_getmethods__["CompareOp"] = _apt.pkgCacheDepIterator_CompareOp_get
    if _newclass:CompareOp = property(_apt.pkgCacheDepIterator_CompareOp_get)
    def __del__(self, destroy=_apt.delete_pkgCacheDepIterator):
        try:
            if self.thisown: destroy(self)
        except: pass

class pkgCacheDepIteratorPtr(pkgCacheDepIterator):
    def __init__(self, this):
        _swig_setattr(self, pkgCacheDepIterator, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, pkgCacheDepIterator, 'thisown', 0)
        _swig_setattr(self, pkgCacheDepIterator,self.__class__,pkgCacheDepIterator)
_apt.pkgCacheDepIterator_swigregister(pkgCacheDepIteratorPtr)

class pkgCachePrvIterator(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, pkgCachePrvIterator, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, pkgCachePrvIterator, name)
    def __repr__(self):
        return "<C pkgCache::PrvIterator instance at %s>" % (self.this,)
    def __nonzero__(*args): return _apt.pkgCachePrvIterator___nonzero__(*args)
    def next(*args): return _apt.pkgCachePrvIterator_next(*args)
    def __iter__(*args): return _apt.pkgCachePrvIterator___iter__(*args)
    def end(*args): return _apt.pkgCachePrvIterator_end(*args)
    def __eq__(*args): return _apt.pkgCachePrvIterator___eq__(*args)
    def __ne__(*args): return _apt.pkgCachePrvIterator___ne__(*args)
    def __deref__(*args): return _apt.pkgCachePrvIterator___deref__(*args)
    def __mul__(*args): return _apt.pkgCachePrvIterator___mul__(*args)
    def Provides(*args): return _apt.pkgCachePrvIterator_Provides(*args)
    def Cache(*args): return _apt.pkgCachePrvIterator_Cache(*args)
    def Name(*args): return _apt.pkgCachePrvIterator_Name(*args)
    def ProvideVersion(*args): return _apt.pkgCachePrvIterator_ProvideVersion(*args)
    def ParentPkg(*args): return _apt.pkgCachePrvIterator_ParentPkg(*args)
    def OwnerVer(*args): return _apt.pkgCachePrvIterator_OwnerVer(*args)
    def OwnerPkg(*args): return _apt.pkgCachePrvIterator_OwnerPkg(*args)
    def Index(*args): return _apt.pkgCachePrvIterator_Index(*args)
    def __init__(self, *args):
        _swig_setattr(self, pkgCachePrvIterator, 'this', _apt.new_pkgCachePrvIterator(*args))
        _swig_setattr(self, pkgCachePrvIterator, 'thisown', 1)
    __swig_getmethods__["Version"] = _apt.pkgCachePrvIterator_Version_get
    if _newclass:Version = property(_apt.pkgCachePrvIterator_Version_get)
    __swig_getmethods__["NextProvides"] = _apt.pkgCachePrvIterator_NextProvides_get
    if _newclass:NextProvides = property(_apt.pkgCachePrvIterator_NextProvides_get)
    __swig_getmethods__["NextPkgProv"] = _apt.pkgCachePrvIterator_NextPkgProv_get
    if _newclass:NextPkgProv = property(_apt.pkgCachePrvIterator_NextPkgProv_get)
    def __del__(self, destroy=_apt.delete_pkgCachePrvIterator):
        try:
            if self.thisown: destroy(self)
        except: pass

class pkgCachePrvIteratorPtr(pkgCachePrvIterator):
    def __init__(self, this):
        _swig_setattr(self, pkgCachePrvIterator, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, pkgCachePrvIterator, 'thisown', 0)
        _swig_setattr(self, pkgCachePrvIterator,self.__class__,pkgCachePrvIterator)
_apt.pkgCachePrvIterator_swigregister(pkgCachePrvIteratorPtr)

class pkgCachePkgFileIterator(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, pkgCachePkgFileIterator, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, pkgCachePkgFileIterator, name)
    def __repr__(self):
        return "<C pkgCache::PkgFileIterator instance at %s>" % (self.this,)
    def __nonzero__(*args): return _apt.pkgCachePkgFileIterator___nonzero__(*args)
    def next(*args): return _apt.pkgCachePkgFileIterator_next(*args)
    def __iter__(*args): return _apt.pkgCachePkgFileIterator___iter__(*args)
    def end(*args): return _apt.pkgCachePkgFileIterator_end(*args)
    def __eq__(*args): return _apt.pkgCachePkgFileIterator___eq__(*args)
    def __ne__(*args): return _apt.pkgCachePkgFileIterator___ne__(*args)
    def __deref__(*args): return _apt.pkgCachePkgFileIterator___deref__(*args)
    def __mul__(*args): return _apt.pkgCachePkgFileIterator___mul__(*args)
    def PackageFile(*args): return _apt.pkgCachePkgFileIterator_PackageFile(*args)
    def Cache(*args): return _apt.pkgCachePkgFileIterator_Cache(*args)
    def FileName(*args): return _apt.pkgCachePkgFileIterator_FileName(*args)
    def Archive(*args): return _apt.pkgCachePkgFileIterator_Archive(*args)
    def Component(*args): return _apt.pkgCachePkgFileIterator_Component(*args)
    def Version(*args): return _apt.pkgCachePkgFileIterator_Version(*args)
    def Origin(*args): return _apt.pkgCachePkgFileIterator_Origin(*args)
    def Label(*args): return _apt.pkgCachePkgFileIterator_Label(*args)
    def Site(*args): return _apt.pkgCachePkgFileIterator_Site(*args)
    def Architecture(*args): return _apt.pkgCachePkgFileIterator_Architecture(*args)
    def IndexType(*args): return _apt.pkgCachePkgFileIterator_IndexType(*args)
    def Index(*args): return _apt.pkgCachePkgFileIterator_Index(*args)
    def IsOk(*args): return _apt.pkgCachePkgFileIterator_IsOk(*args)
    def RelStr(*args): return _apt.pkgCachePkgFileIterator_RelStr(*args)
    def __init__(self, *args):
        _swig_setattr(self, pkgCachePkgFileIterator, 'this', _apt.new_pkgCachePkgFileIterator(*args))
        _swig_setattr(self, pkgCachePkgFileIterator, 'thisown', 1)
    __swig_getmethods__["Size"] = _apt.pkgCachePkgFileIterator_Size_get
    if _newclass:Size = property(_apt.pkgCachePkgFileIterator_Size_get)
    __swig_getmethods__["Flags"] = _apt.pkgCachePkgFileIterator_Flags_get
    if _newclass:Flags = property(_apt.pkgCachePkgFileIterator_Flags_get)
    __swig_getmethods__["NextFile"] = _apt.pkgCachePkgFileIterator_NextFile_get
    if _newclass:NextFile = property(_apt.pkgCachePkgFileIterator_NextFile_get)
    __swig_getmethods__["ID"] = _apt.pkgCachePkgFileIterator_ID_get
    if _newclass:ID = property(_apt.pkgCachePkgFileIterator_ID_get)
    __swig_getmethods__["mtime"] = _apt.pkgCachePkgFileIterator_mtime_get
    if _newclass:mtime = property(_apt.pkgCachePkgFileIterator_mtime_get)
    def __del__(self, destroy=_apt.delete_pkgCachePkgFileIterator):
        try:
            if self.thisown: destroy(self)
        except: pass

class pkgCachePkgFileIteratorPtr(pkgCachePkgFileIterator):
    def __init__(self, this):
        _swig_setattr(self, pkgCachePkgFileIterator, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, pkgCachePkgFileIterator, 'thisown', 0)
        _swig_setattr(self, pkgCachePkgFileIterator,self.__class__,pkgCachePkgFileIterator)
_apt.pkgCachePkgFileIterator_swigregister(pkgCachePkgFileIteratorPtr)

class pkgCacheVerFileIterator(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, pkgCacheVerFileIterator, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, pkgCacheVerFileIterator, name)
    def __repr__(self):
        return "<C pkgCache::VerFileIterator instance at %s>" % (self.this,)
    def __nonzero__(*args): return _apt.pkgCacheVerFileIterator___nonzero__(*args)
    def next(*args): return _apt.pkgCacheVerFileIterator_next(*args)
    def __iter__(*args): return _apt.pkgCacheVerFileIterator___iter__(*args)
    def end(*args): return _apt.pkgCacheVerFileIterator_end(*args)
    def __eq__(*args): return _apt.pkgCacheVerFileIterator___eq__(*args)
    def __ne__(*args): return _apt.pkgCacheVerFileIterator___ne__(*args)
    def __deref__(*args): return _apt.pkgCacheVerFileIterator___deref__(*args)
    def __mul__(*args): return _apt.pkgCacheVerFileIterator___mul__(*args)
    def VerFile(*args): return _apt.pkgCacheVerFileIterator_VerFile(*args)
    def Cache(*args): return _apt.pkgCacheVerFileIterator_Cache(*args)
    def File(*args): return _apt.pkgCacheVerFileIterator_File(*args)
    def Index(*args): return _apt.pkgCacheVerFileIterator_Index(*args)
    def __init__(self, *args):
        _swig_setattr(self, pkgCacheVerFileIterator, 'this', _apt.new_pkgCacheVerFileIterator(*args))
        _swig_setattr(self, pkgCacheVerFileIterator, 'thisown', 1)
    __swig_getmethods__["NextFile"] = _apt.pkgCacheVerFileIterator_NextFile_get
    if _newclass:NextFile = property(_apt.pkgCacheVerFileIterator_NextFile_get)
    __swig_getmethods__["Offset"] = _apt.pkgCacheVerFileIterator_Offset_get
    if _newclass:Offset = property(_apt.pkgCacheVerFileIterator_Offset_get)
    __swig_getmethods__["Size"] = _apt.pkgCacheVerFileIterator_Size_get
    if _newclass:Size = property(_apt.pkgCacheVerFileIterator_Size_get)
    def __del__(self, destroy=_apt.delete_pkgCacheVerFileIterator):
        try:
            if self.thisown: destroy(self)
        except: pass

class pkgCacheVerFileIteratorPtr(pkgCacheVerFileIterator):
    def __init__(self, this):
        _swig_setattr(self, pkgCacheVerFileIterator, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, pkgCacheVerFileIterator, 'thisown', 0)
        _swig_setattr(self, pkgCacheVerFileIterator,self.__class__,pkgCacheVerFileIterator)
_apt.pkgCacheVerFileIterator_swigregister(pkgCacheVerFileIteratorPtr)

class pkgCacheFile(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, pkgCacheFile, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, pkgCacheFile, name)
    def __repr__(self):
        return "<C pkgCacheFile instance at %s>" % (self.this,)
    __swig_setmethods__["Policy"] = _apt.pkgCacheFile_Policy_set
    __swig_getmethods__["Policy"] = _apt.pkgCacheFile_Policy_get
    if _newclass:Policy = property(_apt.pkgCacheFile_Policy_get, _apt.pkgCacheFile_Policy_set)
    def pkgCache(*args): return _apt.pkgCacheFile_pkgCache(*args)
    def pkgDepCache(*args): return _apt.pkgCacheFile_pkgDepCache(*args)
    def __deref__(*args): return _apt.pkgCacheFile___deref__(*args)
    def __mul__(*args): return _apt.pkgCacheFile___mul__(*args)
    def __getitem__(*args): return _apt.pkgCacheFile___getitem__(*args)
    def BuildCaches(*args): return _apt.pkgCacheFile_BuildCaches(*args)
    def Open(*args): return _apt.pkgCacheFile_Open(*args)
    def Close(*args): return _apt.pkgCacheFile_Close(*args)
    def __init__(self, *args):
        _swig_setattr(self, pkgCacheFile, 'this', _apt.new_pkgCacheFile(*args))
        _swig_setattr(self, pkgCacheFile, 'thisown', 1)
    def __del__(self, destroy=_apt.delete_pkgCacheFile):
        try:
            if self.thisown: destroy(self)
        except: pass
    def BuildGroupOrs(*args): return _apt.pkgCacheFile_BuildGroupOrs(*args)
    def UpdateVerState(*args): return _apt.pkgCacheFile_UpdateVerState(*args)
    def Head(*args): return _apt.pkgCacheFile_Head(*args)
    def PkgIter(*args): return _apt.pkgCacheFile_PkgIter(*args)
    def FindPkg(*args): return _apt.pkgCacheFile_FindPkg(*args)
    def GetCache(*args): return _apt.pkgCacheFile_GetCache(*args)
    def VS(*args): return _apt.pkgCacheFile_VS(*args)
    def GetCandidateVer(*args): return _apt.pkgCacheFile_GetCandidateVer(*args)
    def IsImportantDep(*args): return _apt.pkgCacheFile_IsImportantDep(*args)
    def GetPolicy(*args): return _apt.pkgCacheFile_GetPolicy(*args)
    def GetPkgPriority(*args): return _apt.pkgCacheFile_GetPkgPriority(*args)
    def MarkKeep(*args): return _apt.pkgCacheFile_MarkKeep(*args)
    def MarkDelete(*args): return _apt.pkgCacheFile_MarkDelete(*args)
    def MarkInstall(*args): return _apt.pkgCacheFile_MarkInstall(*args)
    def SetReInstall(*args): return _apt.pkgCacheFile_SetReInstall(*args)
    def SetCandidateVersion(*args): return _apt.pkgCacheFile_SetCandidateVersion(*args)
    def Update(*args): return _apt.pkgCacheFile_Update(*args)
    def UsrSize(*args): return _apt.pkgCacheFile_UsrSize(*args)
    def DebSize(*args): return _apt.pkgCacheFile_DebSize(*args)
    def DelCount(*args): return _apt.pkgCacheFile_DelCount(*args)
    def KeepCount(*args): return _apt.pkgCacheFile_KeepCount(*args)
    def InstCount(*args): return _apt.pkgCacheFile_InstCount(*args)
    def BrokenCount(*args): return _apt.pkgCacheFile_BrokenCount(*args)
    def BadCount(*args): return _apt.pkgCacheFile_BadCount(*args)
    def Init(*args): return _apt.pkgCacheFile_Init(*args)

class pkgCacheFilePtr(pkgCacheFile):
    def __init__(self, this):
        _swig_setattr(self, pkgCacheFile, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, pkgCacheFile, 'thisown', 0)
        _swig_setattr(self, pkgCacheFile,self.__class__,pkgCacheFile)
_apt.pkgCacheFile_swigregister(pkgCacheFilePtr)

class pkgProblemResolver(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, pkgProblemResolver, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, pkgProblemResolver, name)
    def __repr__(self):
        return "<C pkgProblemResolver instance at %s>" % (self.this,)
    def Protect(*args): return _apt.pkgProblemResolver_Protect(*args)
    def Remove(*args): return _apt.pkgProblemResolver_Remove(*args)
    def Clear(*args): return _apt.pkgProblemResolver_Clear(*args)
    def Resolve(*args): return _apt.pkgProblemResolver_Resolve(*args)
    def ResolveByKeep(*args): return _apt.pkgProblemResolver_ResolveByKeep(*args)
    def InstallProtect(*args): return _apt.pkgProblemResolver_InstallProtect(*args)
    def RemoveDepends(*args): return _apt.pkgProblemResolver_RemoveDepends(*args)
    def __init__(self, *args):
        _swig_setattr(self, pkgProblemResolver, 'this', _apt.new_pkgProblemResolver(*args))
        _swig_setattr(self, pkgProblemResolver, 'thisown', 1)
    def __del__(self, destroy=_apt.delete_pkgProblemResolver):
        try:
            if self.thisown: destroy(self)
        except: pass

class pkgProblemResolverPtr(pkgProblemResolver):
    def __init__(self, this):
        _swig_setattr(self, pkgProblemResolver, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, pkgProblemResolver, 'thisown', 0)
        _swig_setattr(self, pkgProblemResolver,self.__class__,pkgProblemResolver)
_apt.pkgProblemResolver_swigregister(pkgProblemResolverPtr)


pkgDistUpgrade = _apt.pkgDistUpgrade

pkgApplyStatus = _apt.pkgApplyStatus

pkgFixBroken = _apt.pkgFixBroken

pkgAllUpgrade = _apt.pkgAllUpgrade

pkgMinimizeUpgrade = _apt.pkgMinimizeUpgrade

pkgPrioSortList = _apt.pkgPrioSortList
class pkgSystem(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, pkgSystem, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, pkgSystem, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C pkgSystem instance at %s>" % (self.this,)
    __swig_getmethods__["GetSystem"] = lambda x: _apt.pkgSystem_GetSystem
    if _newclass:GetSystem = staticmethod(_apt.pkgSystem_GetSystem)
    __swig_getmethods__["Label"] = _apt.pkgSystem_Label_get
    if _newclass:Label = property(_apt.pkgSystem_Label_get)
    __swig_setmethods__["VS"] = _apt.pkgSystem_VS_set
    __swig_getmethods__["VS"] = _apt.pkgSystem_VS_get
    if _newclass:VS = property(_apt.pkgSystem_VS_get, _apt.pkgSystem_VS_set)
    def Lock(*args): return _apt.pkgSystem_Lock(*args)
    def UnLock(*args): return _apt.pkgSystem_UnLock(*args)
    def LockRead(*args): return _apt.pkgSystem_LockRead(*args)
    def CacheBuilt(*args): return _apt.pkgSystem_CacheBuilt(*args)
    def CreatePM(*args): return _apt.pkgSystem_CreatePM(*args)
    def Initialize(*args): return _apt.pkgSystem_Initialize(*args)
    def ArchiveSupported(*args): return _apt.pkgSystem_ArchiveSupported(*args)
    def AddStatusFiles(*args): return _apt.pkgSystem_AddStatusFiles(*args)
    def AddSourceFiles(*args): return _apt.pkgSystem_AddSourceFiles(*args)
    def FindIndex(*args): return _apt.pkgSystem_FindIndex(*args)
    def Score(*args): return _apt.pkgSystem_Score(*args)
    def PreProcess(*args): return _apt.pkgSystem_PreProcess(*args)
    def ProcessCache(*args): return _apt.pkgSystem_ProcessCache(*args)
    def IgnoreDep(*args): return _apt.pkgSystem_IgnoreDep(*args)
    def OptionsHash(*args): return _apt.pkgSystem_OptionsHash(*args)
    def __del__(self, destroy=_apt.delete_pkgSystem):
        try:
            if self.thisown: destroy(self)
        except: pass

class pkgSystemPtr(pkgSystem):
    def __init__(self, this):
        _swig_setattr(self, pkgSystem, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, pkgSystem, 'thisown', 0)
        _swig_setattr(self, pkgSystem,self.__class__,pkgSystem)
_apt.pkgSystem_swigregister(pkgSystemPtr)

pkgSystem_GetSystem = _apt.pkgSystem_GetSystem

class Configuration(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, Configuration, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, Configuration, name)
    def __repr__(self):
        return "<C Configuration instance at %s>" % (self.this,)
    def Find(*args): return _apt.Configuration_Find(*args)
    def FindFile(*args): return _apt.Configuration_FindFile(*args)
    def FindDir(*args): return _apt.Configuration_FindDir(*args)
    def FindI(*args): return _apt.Configuration_FindI(*args)
    def FindB(*args): return _apt.Configuration_FindB(*args)
    def FindAny(*args): return _apt.Configuration_FindAny(*args)
    def CndSet(*args): return _apt.Configuration_CndSet(*args)
    def Set(*args): return _apt.Configuration_Set(*args)
    def Exists(*args): return _apt.Configuration_Exists(*args)
    def ExistsAny(*args): return _apt.Configuration_ExistsAny(*args)
    def Clear(*args): return _apt.Configuration_Clear(*args)
    def Tree(*args): return _apt.Configuration_Tree(*args)
    def Dump(*args): return _apt.Configuration_Dump(*args)
    def __init__(self, *args):
        _swig_setattr(self, Configuration, 'this', _apt.new_Configuration(*args))
        _swig_setattr(self, Configuration, 'thisown', 1)
    def __del__(self, destroy=_apt.delete_Configuration):
        try:
            if self.thisown: destroy(self)
        except: pass

class ConfigurationPtr(Configuration):
    def __init__(self, this):
        _swig_setattr(self, Configuration, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, Configuration, 'thisown', 0)
        _swig_setattr(self, Configuration,self.__class__,Configuration)
_apt.Configuration_swigregister(ConfigurationPtr)


ReadConfigFile = _apt.ReadConfigFile

ReadConfigDir = _apt.ReadConfigDir
class ConfigurationItem(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, ConfigurationItem, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, ConfigurationItem, name)
    def __repr__(self):
        return "<C Configuration::Item instance at %s>" % (self.this,)
    __swig_setmethods__["Tag"] = _apt.ConfigurationItem_Tag_set
    __swig_getmethods__["Tag"] = _apt.ConfigurationItem_Tag_get
    if _newclass:Tag = property(_apt.ConfigurationItem_Tag_get, _apt.ConfigurationItem_Tag_set)
    __swig_setmethods__["Value"] = _apt.ConfigurationItem_Value_set
    __swig_getmethods__["Value"] = _apt.ConfigurationItem_Value_get
    if _newclass:Value = property(_apt.ConfigurationItem_Value_get, _apt.ConfigurationItem_Value_set)
    __swig_setmethods__["Parent"] = _apt.ConfigurationItem_Parent_set
    __swig_getmethods__["Parent"] = _apt.ConfigurationItem_Parent_get
    if _newclass:Parent = property(_apt.ConfigurationItem_Parent_get, _apt.ConfigurationItem_Parent_set)
    __swig_setmethods__["Child"] = _apt.ConfigurationItem_Child_set
    __swig_getmethods__["Child"] = _apt.ConfigurationItem_Child_get
    if _newclass:Child = property(_apt.ConfigurationItem_Child_get, _apt.ConfigurationItem_Child_set)
    __swig_setmethods__["Next"] = _apt.ConfigurationItem_Next_set
    __swig_getmethods__["Next"] = _apt.ConfigurationItem_Next_get
    if _newclass:Next = property(_apt.ConfigurationItem_Next_get, _apt.ConfigurationItem_Next_set)
    def FullTag(*args): return _apt.ConfigurationItem_FullTag(*args)
    def __init__(self, *args):
        _swig_setattr(self, ConfigurationItem, 'this', _apt.new_ConfigurationItem(*args))
        _swig_setattr(self, ConfigurationItem, 'thisown', 1)
    def __del__(self, destroy=_apt.delete_ConfigurationItem):
        try:
            if self.thisown: destroy(self)
        except: pass

class ConfigurationItemPtr(ConfigurationItem):
    def __init__(self, this):
        _swig_setattr(self, ConfigurationItem, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, ConfigurationItem, 'thisown', 0)
        _swig_setattr(self, ConfigurationItem,self.__class__,ConfigurationItem)
_apt.ConfigurationItem_swigregister(ConfigurationItemPtr)

class OpProgress(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, OpProgress, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, OpProgress, name)
    def __repr__(self):
        return "<C OpProgress instance at %s>" % (self.this,)
    def Progress(*args): return _apt.OpProgress_Progress(*args)
    def SubProgress(*args): return _apt.OpProgress_SubProgress(*args)
    def OverallProgress(*args): return _apt.OpProgress_OverallProgress(*args)
    def Done(*args): return _apt.OpProgress_Done(*args)
    def __init__(self, *args):
        _swig_setattr(self, OpProgress, 'this', _apt.new_OpProgress(*args))
        _swig_setattr(self, OpProgress, 'thisown', 1)
    def __del__(self, destroy=_apt.delete_OpProgress):
        try:
            if self.thisown: destroy(self)
        except: pass

class OpProgressPtr(OpProgress):
    def __init__(self, this):
        _swig_setattr(self, OpProgress, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, OpProgress, 'thisown', 0)
        _swig_setattr(self, OpProgress,self.__class__,OpProgress)
_apt.OpProgress_swigregister(OpProgressPtr)

class OpTextProgress(OpProgress):
    __swig_setmethods__ = {}
    for _s in [OpProgress]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, OpTextProgress, name, value)
    __swig_getmethods__ = {}
    for _s in [OpProgress]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, OpTextProgress, name)
    def __repr__(self):
        return "<C OpTextProgress instance at %s>" % (self.this,)
    def Done(*args): return _apt.OpTextProgress_Done(*args)
    def __init__(self, *args):
        _swig_setattr(self, OpTextProgress, 'this', _apt.new_OpTextProgress(*args))
        _swig_setattr(self, OpTextProgress, 'thisown', 1)
    def __del__(self, destroy=_apt.delete_OpTextProgress):
        try:
            if self.thisown: destroy(self)
        except: pass

class OpTextProgressPtr(OpTextProgress):
    def __init__(self, this):
        _swig_setattr(self, OpTextProgress, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, OpTextProgress, 'thisown', 0)
        _swig_setattr(self, OpTextProgress,self.__class__,OpTextProgress)
_apt.OpTextProgress_swigregister(OpTextProgressPtr)

class pkgVersioningSystem(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, pkgVersioningSystem, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, pkgVersioningSystem, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C pkgVersioningSystem instance at %s>" % (self.this,)
    __swig_getmethods__["GetVS"] = lambda x: _apt.pkgVersioningSystem_GetVS
    if _newclass:GetVS = staticmethod(_apt.pkgVersioningSystem_GetVS)
    __swig_getmethods__["Label"] = _apt.pkgVersioningSystem_Label_get
    if _newclass:Label = property(_apt.pkgVersioningSystem_Label_get)
    def DoCmpVersion(*args): return _apt.pkgVersioningSystem_DoCmpVersion(*args)
    def DoCmpVersionArch(*args): return _apt.pkgVersioningSystem_DoCmpVersionArch(*args)
    def CmpVersionArch(*args): return _apt.pkgVersioningSystem_CmpVersionArch(*args)
    def CheckDep(*args): return _apt.pkgVersioningSystem_CheckDep(*args)
    def DoCmpReleaseVer(*args): return _apt.pkgVersioningSystem_DoCmpReleaseVer(*args)
    def UpstreamVersion(*args): return _apt.pkgVersioningSystem_UpstreamVersion(*args)
    def TestCompatibility(*args): return _apt.pkgVersioningSystem_TestCompatibility(*args)
    def CmpVersion(*args): return _apt.pkgVersioningSystem_CmpVersion(*args)
    def CmpReleaseVer(*args): return _apt.pkgVersioningSystem_CmpReleaseVer(*args)
    def __del__(self, destroy=_apt.delete_pkgVersioningSystem):
        try:
            if self.thisown: destroy(self)
        except: pass

class pkgVersioningSystemPtr(pkgVersioningSystem):
    def __init__(self, this):
        _swig_setattr(self, pkgVersioningSystem, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, pkgVersioningSystem, 'thisown', 0)
        _swig_setattr(self, pkgVersioningSystem,self.__class__,pkgVersioningSystem)
_apt.pkgVersioningSystem_swigregister(pkgVersioningSystemPtr)

pkgVersioningSystem_GetVS = _apt.pkgVersioningSystem_GetVS

class pkgRecords(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, pkgRecords, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, pkgRecords, name)
    def __repr__(self):
        return "<C pkgRecords instance at %s>" % (self.this,)
    def Lookup(*args): return _apt.pkgRecords_Lookup(*args)
    def __init__(self, *args):
        _swig_setattr(self, pkgRecords, 'this', _apt.new_pkgRecords(*args))
        _swig_setattr(self, pkgRecords, 'thisown', 1)
    def __del__(self, destroy=_apt.delete_pkgRecords):
        try:
            if self.thisown: destroy(self)
        except: pass

class pkgRecordsPtr(pkgRecords):
    def __init__(self, this):
        _swig_setattr(self, pkgRecords, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, pkgRecords, 'thisown', 0)
        _swig_setattr(self, pkgRecords,self.__class__,pkgRecords)
_apt.pkgRecords_swigregister(pkgRecordsPtr)

class pkgRecordsParser(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, pkgRecordsParser, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, pkgRecordsParser, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C pkgRecords::Parser instance at %s>" % (self.this,)
    def FileName(*args): return _apt.pkgRecordsParser_FileName(*args)
    def MD5Hash(*args): return _apt.pkgRecordsParser_MD5Hash(*args)
    def SHA1Hash(*args): return _apt.pkgRecordsParser_SHA1Hash(*args)
    def SourcePkg(*args): return _apt.pkgRecordsParser_SourcePkg(*args)
    def Maintainer(*args): return _apt.pkgRecordsParser_Maintainer(*args)
    def ShortDesc(*args): return _apt.pkgRecordsParser_ShortDesc(*args)
    def LongDesc(*args): return _apt.pkgRecordsParser_LongDesc(*args)
    def Name(*args): return _apt.pkgRecordsParser_Name(*args)
    def GetRec(*args): return _apt.pkgRecordsParser_GetRec(*args)
    def HasFile(*args): return _apt.pkgRecordsParser_HasFile(*args)
    def __del__(self, destroy=_apt.delete_pkgRecordsParser):
        try:
            if self.thisown: destroy(self)
        except: pass

class pkgRecordsParserPtr(pkgRecordsParser):
    def __init__(self, this):
        _swig_setattr(self, pkgRecordsParser, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, pkgRecordsParser, 'thisown', 0)
        _swig_setattr(self, pkgRecordsParser,self.__class__,pkgRecordsParser)
_apt.pkgRecordsParser_swigregister(pkgRecordsParserPtr)

class pkgAcquireItem(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, pkgAcquireItem, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, pkgAcquireItem, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C pkgAcquire::Item instance at %s>" % (self.this,)
    __swig_getmethods__["DestFile"] = _apt.pkgAcquireItem_DestFile_get
    if _newclass:DestFile = property(_apt.pkgAcquireItem_DestFile_get)
    __swig_getmethods__["ErrorText"] = _apt.pkgAcquireItem_ErrorText_get
    if _newclass:ErrorText = property(_apt.pkgAcquireItem_ErrorText_get)
    StatIdle = _apt.pkgAcquireItem_StatIdle
    StatFetching = _apt.pkgAcquireItem_StatFetching
    StatDone = _apt.pkgAcquireItem_StatDone
    StatError = _apt.pkgAcquireItem_StatError
    __swig_setmethods__["Status"] = _apt.pkgAcquireItem_Status_set
    __swig_getmethods__["Status"] = _apt.pkgAcquireItem_Status_get
    if _newclass:Status = property(_apt.pkgAcquireItem_Status_get, _apt.pkgAcquireItem_Status_set)
    __swig_setmethods__["FileSize"] = _apt.pkgAcquireItem_FileSize_set
    __swig_getmethods__["FileSize"] = _apt.pkgAcquireItem_FileSize_get
    if _newclass:FileSize = property(_apt.pkgAcquireItem_FileSize_get, _apt.pkgAcquireItem_FileSize_set)
    __swig_setmethods__["PartialSize"] = _apt.pkgAcquireItem_PartialSize_set
    __swig_getmethods__["PartialSize"] = _apt.pkgAcquireItem_PartialSize_get
    if _newclass:PartialSize = property(_apt.pkgAcquireItem_PartialSize_get, _apt.pkgAcquireItem_PartialSize_set)
    __swig_setmethods__["Mode"] = _apt.pkgAcquireItem_Mode_set
    __swig_getmethods__["Mode"] = _apt.pkgAcquireItem_Mode_get
    if _newclass:Mode = property(_apt.pkgAcquireItem_Mode_get, _apt.pkgAcquireItem_Mode_set)
    __swig_setmethods__["ID"] = _apt.pkgAcquireItem_ID_set
    __swig_getmethods__["ID"] = _apt.pkgAcquireItem_ID_get
    if _newclass:ID = property(_apt.pkgAcquireItem_ID_get, _apt.pkgAcquireItem_ID_set)
    __swig_setmethods__["Complete"] = _apt.pkgAcquireItem_Complete_set
    __swig_getmethods__["Complete"] = _apt.pkgAcquireItem_Complete_get
    if _newclass:Complete = property(_apt.pkgAcquireItem_Complete_get, _apt.pkgAcquireItem_Complete_set)
    __swig_setmethods__["Local"] = _apt.pkgAcquireItem_Local_set
    __swig_getmethods__["Local"] = _apt.pkgAcquireItem_Local_get
    if _newclass:Local = property(_apt.pkgAcquireItem_Local_get, _apt.pkgAcquireItem_Local_set)
    __swig_setmethods__["QueueCounter"] = _apt.pkgAcquireItem_QueueCounter_set
    __swig_getmethods__["QueueCounter"] = _apt.pkgAcquireItem_QueueCounter_get
    if _newclass:QueueCounter = property(_apt.pkgAcquireItem_QueueCounter_get, _apt.pkgAcquireItem_QueueCounter_set)
    def Failed(*args): return _apt.pkgAcquireItem_Failed(*args)
    def Done(*args): return _apt.pkgAcquireItem_Done(*args)
    def Start(*args): return _apt.pkgAcquireItem_Start(*args)
    def Custom600Headers(*args): return _apt.pkgAcquireItem_Custom600Headers(*args)
    def DescURI(*args): return _apt.pkgAcquireItem_DescURI(*args)
    def Finished(*args): return _apt.pkgAcquireItem_Finished(*args)
    def MD5Sum(*args): return _apt.pkgAcquireItem_MD5Sum(*args)
    def GetOwner(*args): return _apt.pkgAcquireItem_GetOwner(*args)
    def __del__(self, destroy=_apt.delete_pkgAcquireItem):
        try:
            if self.thisown: destroy(self)
        except: pass

class pkgAcquireItemPtr(pkgAcquireItem):
    def __init__(self, this):
        _swig_setattr(self, pkgAcquireItem, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, pkgAcquireItem, 'thisown', 0)
        _swig_setattr(self, pkgAcquireItem,self.__class__,pkgAcquireItem)
_apt.pkgAcquireItem_swigregister(pkgAcquireItemPtr)

class pkgAcqIndex(pkgAcquireItem):
    __swig_setmethods__ = {}
    for _s in [pkgAcquireItem]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, pkgAcqIndex, name, value)
    __swig_getmethods__ = {}
    for _s in [pkgAcquireItem]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, pkgAcqIndex, name)
    def __repr__(self):
        return "<C pkgAcqIndex instance at %s>" % (self.this,)
    def Done(*args): return _apt.pkgAcqIndex_Done(*args)
    def Custom600Headers(*args): return _apt.pkgAcqIndex_Custom600Headers(*args)
    def DescURI(*args): return _apt.pkgAcqIndex_DescURI(*args)
    def __init__(self, *args):
        _swig_setattr(self, pkgAcqIndex, 'this', _apt.new_pkgAcqIndex(*args))
        _swig_setattr(self, pkgAcqIndex, 'thisown', 1)
    def __del__(self, destroy=_apt.delete_pkgAcqIndex):
        try:
            if self.thisown: destroy(self)
        except: pass

class pkgAcqIndexPtr(pkgAcqIndex):
    def __init__(self, this):
        _swig_setattr(self, pkgAcqIndex, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, pkgAcqIndex, 'thisown', 0)
        _swig_setattr(self, pkgAcqIndex,self.__class__,pkgAcqIndex)
_apt.pkgAcqIndex_swigregister(pkgAcqIndexPtr)

class pkgAcqIndexRel(pkgAcquireItem):
    __swig_setmethods__ = {}
    for _s in [pkgAcquireItem]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, pkgAcqIndexRel, name, value)
    __swig_getmethods__ = {}
    for _s in [pkgAcquireItem]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, pkgAcqIndexRel, name)
    def __repr__(self):
        return "<C pkgAcqIndexRel instance at %s>" % (self.this,)
    def Failed(*args): return _apt.pkgAcqIndexRel_Failed(*args)
    def Done(*args): return _apt.pkgAcqIndexRel_Done(*args)
    def Custom600Headers(*args): return _apt.pkgAcqIndexRel_Custom600Headers(*args)
    def DescURI(*args): return _apt.pkgAcqIndexRel_DescURI(*args)
    def __init__(self, *args):
        _swig_setattr(self, pkgAcqIndexRel, 'this', _apt.new_pkgAcqIndexRel(*args))
        _swig_setattr(self, pkgAcqIndexRel, 'thisown', 1)
    def __del__(self, destroy=_apt.delete_pkgAcqIndexRel):
        try:
            if self.thisown: destroy(self)
        except: pass

class pkgAcqIndexRelPtr(pkgAcqIndexRel):
    def __init__(self, this):
        _swig_setattr(self, pkgAcqIndexRel, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, pkgAcqIndexRel, 'thisown', 0)
        _swig_setattr(self, pkgAcqIndexRel,self.__class__,pkgAcqIndexRel)
_apt.pkgAcqIndexRel_swigregister(pkgAcqIndexRelPtr)

class pkgAcqArchive(pkgAcquireItem):
    __swig_setmethods__ = {}
    for _s in [pkgAcquireItem]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, pkgAcqArchive, name, value)
    __swig_getmethods__ = {}
    for _s in [pkgAcquireItem]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, pkgAcqArchive, name)
    def __repr__(self):
        return "<C pkgAcqArchive instance at %s>" % (self.this,)
    def Failed(*args): return _apt.pkgAcqArchive_Failed(*args)
    def Done(*args): return _apt.pkgAcqArchive_Done(*args)
    def MD5Sum(*args): return _apt.pkgAcqArchive_MD5Sum(*args)
    def DescURI(*args): return _apt.pkgAcqArchive_DescURI(*args)
    def Finished(*args): return _apt.pkgAcqArchive_Finished(*args)
    def __init__(self, *args):
        _swig_setattr(self, pkgAcqArchive, 'this', _apt.new_pkgAcqArchive(*args))
        _swig_setattr(self, pkgAcqArchive, 'thisown', 1)
    def __del__(self, destroy=_apt.delete_pkgAcqArchive):
        try:
            if self.thisown: destroy(self)
        except: pass

class pkgAcqArchivePtr(pkgAcqArchive):
    def __init__(self, this):
        _swig_setattr(self, pkgAcqArchive, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, pkgAcqArchive, 'thisown', 0)
        _swig_setattr(self, pkgAcqArchive,self.__class__,pkgAcqArchive)
_apt.pkgAcqArchive_swigregister(pkgAcqArchivePtr)

class pkgAcqFile(pkgAcquireItem):
    __swig_setmethods__ = {}
    for _s in [pkgAcquireItem]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, pkgAcqFile, name, value)
    __swig_getmethods__ = {}
    for _s in [pkgAcquireItem]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, pkgAcqFile, name)
    def __repr__(self):
        return "<C pkgAcqFile instance at %s>" % (self.this,)
    def Failed(*args): return _apt.pkgAcqFile_Failed(*args)
    def Done(*args): return _apt.pkgAcqFile_Done(*args)
    def MD5Sum(*args): return _apt.pkgAcqFile_MD5Sum(*args)
    def DescURI(*args): return _apt.pkgAcqFile_DescURI(*args)
    def __init__(self, *args):
        _swig_setattr(self, pkgAcqFile, 'this', _apt.new_pkgAcqFile(*args))
        _swig_setattr(self, pkgAcqFile, 'thisown', 1)
    def __del__(self, destroy=_apt.delete_pkgAcqFile):
        try:
            if self.thisown: destroy(self)
        except: pass

class pkgAcqFilePtr(pkgAcqFile):
    def __init__(self, this):
        _swig_setattr(self, pkgAcqFile, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, pkgAcqFile, 'thisown', 0)
        _swig_setattr(self, pkgAcqFile,self.__class__,pkgAcqFile)
_apt.pkgAcqFile_swigregister(pkgAcqFilePtr)

class pkgAcquire(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, pkgAcquire, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, pkgAcquire, name)
    def __repr__(self):
        return "<C pkgAcquire instance at %s>" % (self.this,)
    def ItemsIter(*args): return _apt.pkgAcquire_ItemsIter(*args)
    def GetConfig(*args): return _apt.pkgAcquire_GetConfig(*args)
    Continue = _apt.pkgAcquire_Continue
    Failed = _apt.pkgAcquire_Failed
    Cancelled = _apt.pkgAcquire_Cancelled
    def Run(*args): return _apt.pkgAcquire_Run(*args)
    def Shutdown(*args): return _apt.pkgAcquire_Shutdown(*args)
    def WorkersBegin(*args): return _apt.pkgAcquire_WorkersBegin(*args)
    def WorkerStep(*args): return _apt.pkgAcquire_WorkerStep(*args)
    def UriBegin(*args): return _apt.pkgAcquire_UriBegin(*args)
    def UriEnd(*args): return _apt.pkgAcquire_UriEnd(*args)
    def Clean(*args): return _apt.pkgAcquire_Clean(*args)
    def TotalNeeded(*args): return _apt.pkgAcquire_TotalNeeded(*args)
    def FetchNeeded(*args): return _apt.pkgAcquire_FetchNeeded(*args)
    def PartialPresent(*args): return _apt.pkgAcquire_PartialPresent(*args)
    def __init__(self, *args):
        _swig_setattr(self, pkgAcquire, 'this', _apt.new_pkgAcquire(*args))
        _swig_setattr(self, pkgAcquire, 'thisown', 1)
    def __del__(self, destroy=_apt.delete_pkgAcquire):
        try:
            if self.thisown: destroy(self)
        except: pass

class pkgAcquirePtr(pkgAcquire):
    def __init__(self, this):
        _swig_setattr(self, pkgAcquire, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, pkgAcquire, 'thisown', 0)
        _swig_setattr(self, pkgAcquire,self.__class__,pkgAcquire)
_apt.pkgAcquire_swigregister(pkgAcquirePtr)

class pkgAcquireStatus(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, pkgAcquireStatus, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, pkgAcquireStatus, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C pkgAcquireStatus instance at %s>" % (self.this,)
    __swig_setmethods__["Update"] = _apt.pkgAcquireStatus_Update_set
    __swig_getmethods__["Update"] = _apt.pkgAcquireStatus_Update_get
    if _newclass:Update = property(_apt.pkgAcquireStatus_Update_get, _apt.pkgAcquireStatus_Update_set)
    __swig_setmethods__["MorePulses"] = _apt.pkgAcquireStatus_MorePulses_set
    __swig_getmethods__["MorePulses"] = _apt.pkgAcquireStatus_MorePulses_get
    if _newclass:MorePulses = property(_apt.pkgAcquireStatus_MorePulses_get, _apt.pkgAcquireStatus_MorePulses_set)
    def Fetched(*args): return _apt.pkgAcquireStatus_Fetched(*args)
    def MediaChange(*args): return _apt.pkgAcquireStatus_MediaChange(*args)
    def IMSHit(*args): return _apt.pkgAcquireStatus_IMSHit(*args)
    def Fetch(*args): return _apt.pkgAcquireStatus_Fetch(*args)
    def Done(*args): return _apt.pkgAcquireStatus_Done(*args)
    def Fail(*args): return _apt.pkgAcquireStatus_Fail(*args)
    def Pulse(*args): return _apt.pkgAcquireStatus_Pulse(*args)
    def Start(*args): return _apt.pkgAcquireStatus_Start(*args)
    def Stop(*args): return _apt.pkgAcquireStatus_Stop(*args)
    def __del__(self, destroy=_apt.delete_pkgAcquireStatus):
        try:
            if self.thisown: destroy(self)
        except: pass

class pkgAcquireStatusPtr(pkgAcquireStatus):
    def __init__(self, this):
        _swig_setattr(self, pkgAcquireStatus, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, pkgAcquireStatus, 'thisown', 0)
        _swig_setattr(self, pkgAcquireStatus,self.__class__,pkgAcquireStatus)
_apt.pkgAcquireStatus_swigregister(pkgAcquireStatusPtr)

class pkgPackageManager(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, pkgPackageManager, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, pkgPackageManager, name)
    def __repr__(self):
        return "<C pkgPackageManager instance at %s>" % (self.this,)
    Completed = _apt.pkgPackageManager_Completed
    Failed = _apt.pkgPackageManager_Failed
    Incomplete = _apt.pkgPackageManager_Incomplete
    def GetArchives(*args): return _apt.pkgPackageManager_GetArchives(*args)
    def DoInstall(*args): return _apt.pkgPackageManager_DoInstall(*args)
    def FixMissing(*args): return _apt.pkgPackageManager_FixMissing(*args)
    def __init__(self, *args):
        _swig_setattr(self, pkgPackageManager, 'this', _apt.new_pkgPackageManager(*args))
        _swig_setattr(self, pkgPackageManager, 'thisown', 1)
    def __del__(self, destroy=_apt.delete_pkgPackageManager):
        try:
            if self.thisown: destroy(self)
        except: pass

class pkgPackageManagerPtr(pkgPackageManager):
    def __init__(self, this):
        _swig_setattr(self, pkgPackageManager, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, pkgPackageManager, 'thisown', 0)
        _swig_setattr(self, pkgPackageManager,self.__class__,pkgPackageManager)
_apt.pkgPackageManager_swigregister(pkgPackageManagerPtr)

class pkgSourceList(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, pkgSourceList, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, pkgSourceList, name)
    def __repr__(self):
        return "<C pkgSourceList instance at %s>" % (self.this,)
    def ReadMainList(*args): return _apt.pkgSourceList_ReadMainList(*args)
    def Read(*args): return _apt.pkgSourceList_Read(*args)
    def ReadVendors(*args): return _apt.pkgSourceList_ReadVendors(*args)
    def Reset(*args): return _apt.pkgSourceList_Reset(*args)
    def ReadAppend(*args): return _apt.pkgSourceList_ReadAppend(*args)
    def ReadSourceDir(*args): return _apt.pkgSourceList_ReadSourceDir(*args)
    def begin(*args): return _apt.pkgSourceList_begin(*args)
    def end(*args): return _apt.pkgSourceList_end(*args)
    def size(*args): return _apt.pkgSourceList_size(*args)
    def empty(*args): return _apt.pkgSourceList_empty(*args)
    def FindIndex(*args): return _apt.pkgSourceList_FindIndex(*args)
    def GetIndexes(*args): return _apt.pkgSourceList_GetIndexes(*args)
    def GetReleases(*args): return _apt.pkgSourceList_GetReleases(*args)
    def __init__(self, *args):
        _swig_setattr(self, pkgSourceList, 'this', _apt.new_pkgSourceList(*args))
        _swig_setattr(self, pkgSourceList, 'thisown', 1)
    def __del__(self, destroy=_apt.delete_pkgSourceList):
        try:
            if self.thisown: destroy(self)
        except: pass

class pkgSourceListPtr(pkgSourceList):
    def __init__(self, this):
        _swig_setattr(self, pkgSourceList, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, pkgSourceList, 'thisown', 0)
        _swig_setattr(self, pkgSourceList,self.__class__,pkgSourceList)
_apt.pkgSourceList_swigregister(pkgSourceListPtr)

class GlobalError(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, GlobalError, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, GlobalError, name)
    def __repr__(self):
        return "<C GlobalError instance at %s>" % (self.this,)
    def PushState(*args): return _apt.GlobalError_PushState(*args)
    def PopState(*args): return _apt.GlobalError_PopState(*args)
    def PopBackState(*args): return _apt.GlobalError_PopBackState(*args)
    def Errno(*args): return _apt.GlobalError_Errno(*args)
    def WarningE(*args): return _apt.GlobalError_WarningE(*args)
    def Error(*args): return _apt.GlobalError_Error(*args)
    def Warning(*args): return _apt.GlobalError_Warning(*args)
    def PendingError(*args): return _apt.GlobalError_PendingError(*args)
    def empty(*args): return _apt.GlobalError_empty(*args)
    def PopMessage(*args): return _apt.GlobalError_PopMessage(*args)
    def Discard(*args): return _apt.GlobalError_Discard(*args)
    def DumpErrors(*args): return _apt.GlobalError_DumpErrors(*args)
    def __init__(self, *args):
        _swig_setattr(self, GlobalError, 'this', _apt.new_GlobalError(*args))
        _swig_setattr(self, GlobalError, 'thisown', 1)
    def __del__(self, destroy=_apt.delete_GlobalError):
        try:
            if self.thisown: destroy(self)
        except: pass

class GlobalErrorPtr(GlobalError):
    def __init__(self, this):
        _swig_setattr(self, GlobalError, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, GlobalError, 'thisown', 0)
        _swig_setattr(self, GlobalError,self.__class__,GlobalError)
_apt.GlobalError_swigregister(GlobalErrorPtr)


_GetErrorObj = _apt._GetErrorObj
class LuaCacheControl(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, LuaCacheControl, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, LuaCacheControl, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C LuaCacheControl instance at %s>" % (self.this,)
    def Open(*args): return _apt.LuaCacheControl_Open(*args)
    def Close(*args): return _apt.LuaCacheControl_Close(*args)
    def __del__(self, destroy=_apt.delete_LuaCacheControl):
        try:
            if self.thisown: destroy(self)
        except: pass

class LuaCacheControlPtr(LuaCacheControl):
    def __init__(self, this):
        _swig_setattr(self, LuaCacheControl, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, LuaCacheControl, 'thisown', 0)
        _swig_setattr(self, LuaCacheControl,self.__class__,LuaCacheControl)
_apt.LuaCacheControl_swigregister(LuaCacheControlPtr)

class Lua(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, Lua, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, Lua, name)
    def __repr__(self):
        return "<C Lua instance at %s>" % (self.this,)
    def RunInteractive(*args): return _apt.Lua_RunInteractive(*args)
    def RunScript(*args): return _apt.Lua_RunScript(*args)
    def RunScripts(*args): return _apt.Lua_RunScripts(*args)
    def HasScripts(*args): return _apt.Lua_HasScripts(*args)
    def ResetScript(*args): return _apt.Lua_ResetScript(*args)
    def SetGlobal(*args): return _apt.Lua_SetGlobal(*args)
    def ResetGlobals(*args): return _apt.Lua_ResetGlobals(*args)
    def GetGlobalStr(*args): return _apt.Lua_GetGlobalStr(*args)
    def GetGlobalStrList(*args): return _apt.Lua_GetGlobalStrList(*args)
    def GetGlobalNum(*args): return _apt.Lua_GetGlobalNum(*args)
    def GetGlobalBool(*args): return _apt.Lua_GetGlobalBool(*args)
    def GetGlobalPtr(*args): return _apt.Lua_GetGlobalPtr(*args)
    def GetGlobalPkg(*args): return _apt.Lua_GetGlobalPkg(*args)
    def GetGlobalPkgList(*args): return _apt.Lua_GetGlobalPkgList(*args)
    def SetDepCache(*args): return _apt.Lua_SetDepCache(*args)
    def SetCache(*args): return _apt.Lua_SetCache(*args)
    def SetCacheControl(*args): return _apt.Lua_SetCacheControl(*args)
    def SetProblemResolver(*args): return _apt.Lua_SetProblemResolver(*args)
    def SetDontFix(*args): return _apt.Lua_SetDontFix(*args)
    def ResetCaches(*args): return _apt.Lua_ResetCaches(*args)
    def GetDepCache(*args): return _apt.Lua_GetDepCache(*args)
    def GetCache(*args): return _apt.Lua_GetCache(*args)
    def GetCacheControl(*args): return _apt.Lua_GetCacheControl(*args)
    def GetProblemResolver(*args): return _apt.Lua_GetProblemResolver(*args)
    def GetDontFix(*args): return _apt.Lua_GetDontFix(*args)
    def __init__(self, *args):
        _swig_setattr(self, Lua, 'this', _apt.new_Lua(*args))
        _swig_setattr(self, Lua, 'thisown', 1)
    def __del__(self, destroy=_apt.delete_Lua):
        try:
            if self.thisown: destroy(self)
        except: pass

class LuaPtr(Lua):
    def __init__(self, this):
        _swig_setattr(self, Lua, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, Lua, 'thisown', 0)
        _swig_setattr(self, Lua,self.__class__,Lua)
_apt.Lua_swigregister(LuaPtr)


_GetLuaObj = _apt._GetLuaObj
class pkgAcquireStatusDumb(pkgAcquireStatus):
    __swig_setmethods__ = {}
    for _s in [pkgAcquireStatus]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, pkgAcquireStatusDumb, name, value)
    __swig_getmethods__ = {}
    for _s in [pkgAcquireStatus]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, pkgAcquireStatusDumb, name)
    def __repr__(self):
        return "<C pkgAcquireStatusDumb instance at %s>" % (self.this,)
    def __init__(self, *args):
        _swig_setattr(self, pkgAcquireStatusDumb, 'this', _apt.new_pkgAcquireStatusDumb(*args))
        _swig_setattr(self, pkgAcquireStatusDumb, 'thisown', 1)
    def __del__(self, destroy=_apt.delete_pkgAcquireStatusDumb):
        try:
            if self.thisown: destroy(self)
        except: pass

class pkgAcquireStatusDumbPtr(pkgAcquireStatusDumb):
    def __init__(self, this):
        _swig_setattr(self, pkgAcquireStatusDumb, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, pkgAcquireStatusDumb, 'thisown', 0)
        _swig_setattr(self, pkgAcquireStatusDumb,self.__class__,pkgAcquireStatusDumb)
_apt.pkgAcquireStatusDumb_swigregister(pkgAcquireStatusDumbPtr)

class ROpPyProgress(OpProgress):
    __swig_setmethods__ = {}
    for _s in [OpProgress]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, ROpPyProgress, name, value)
    __swig_getmethods__ = {}
    for _s in [OpProgress]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, ROpPyProgress, name)
    def __repr__(self):
        return "<C ROpPyProgress instance at %s>" % (self.this,)
    __swig_setmethods__["Op"] = _apt.ROpPyProgress_Op_set
    __swig_getmethods__["Op"] = _apt.ROpPyProgress_Op_get
    if _newclass:Op = property(_apt.ROpPyProgress_Op_get, _apt.ROpPyProgress_Op_set)
    __swig_setmethods__["SubOp"] = _apt.ROpPyProgress_SubOp_set
    __swig_getmethods__["SubOp"] = _apt.ROpPyProgress_SubOp_get
    if _newclass:SubOp = property(_apt.ROpPyProgress_SubOp_get, _apt.ROpPyProgress_SubOp_set)
    __swig_setmethods__["Percent"] = _apt.ROpPyProgress_Percent_set
    __swig_getmethods__["Percent"] = _apt.ROpPyProgress_Percent_get
    if _newclass:Percent = property(_apt.ROpPyProgress_Percent_get, _apt.ROpPyProgress_Percent_set)
    __swig_setmethods__["MajorChange"] = _apt.ROpPyProgress_MajorChange_set
    __swig_getmethods__["MajorChange"] = _apt.ROpPyProgress_MajorChange_get
    if _newclass:MajorChange = property(_apt.ROpPyProgress_MajorChange_get, _apt.ROpPyProgress_MajorChange_set)
    def CheckChange(*args): return _apt.ROpPyProgress_CheckChange(*args)
    def __init__(self, *args):
        _swig_setattr(self, ROpPyProgress, 'this', _apt.new_ROpPyProgress(*args))
        _swig_setattr(self, ROpPyProgress, 'thisown', 1)
    def __del__(self, destroy=_apt.delete_ROpPyProgress):
        try:
            if self.thisown: destroy(self)
        except: pass

class ROpPyProgressPtr(ROpPyProgress):
    def __init__(self, this):
        _swig_setattr(self, ROpPyProgress, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, ROpPyProgress, 'thisown', 0)
        _swig_setattr(self, ROpPyProgress,self.__class__,ROpPyProgress)
_apt.ROpPyProgress_swigregister(ROpPyProgressPtr)

class OpPyProgress(ROpPyProgress):
	def __init__(self):
		ROpPyProgress.__init__(self, self)
	def __repr__(self):
		return "<C OpPyProgress instance at %s>" % self.this


