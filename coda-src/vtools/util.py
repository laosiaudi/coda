#! /usr/bin/env python
from ctypes import Structure, c_char_p, c_int, c_void_p, c_uint, sizeof
from codaconf import codaconf_init, codaconf_lookup
import platform
CODA_CONTROL = '.CONTROL'
mountPoint = None
_VIOCCKSERV = 10
PIOCPARM_MASK = 0x0000ffff

def getMountPoint():
    global mountPoint
    if mountPoint is None:
        codaconf_init("venus.conf")
        if mountPoint == None or mountPoint == "":
            mountPoint = codaconf_lookup("mountpoint", "/coda")
    return mountPoint

class ViceIoctl(Structure):
    _fields_ = [("in_data", c_char_p),
                ("out_data", c_char_p),
                ("in_size", c_uint),
                ("out_size", c_uint)]

class PioctlData(Structure):
    _fields_ = [("path", c_char_p),
                ("follow", c_int),
                ("vi", ViceIoctl),
                ("cmd", c_int)]


_IOC_NRBITS = 8
_IOC_TYPEBITS = 2
_IOC_SIZEBITS = 14
_IOC_DIRBITS = 2

_IOC_NRMASK = (1 << _IOC_NRBITS) - 1
if platform.system() == 'Linux':
    _IOC_NRMASK = 0xff
_IOC_TYPEMASK = (1 << _IOC_TYPEBITS) - 1
if platform.system() == 'Linux':
    _IOC_TYPEMASK = 0xff

_IOC_SIZEMASK = (1 << _IOC_SIZEBITS) - 1
_IOC_DIRMASK = (1 << _IOC_DIRBITS) - 1

_IOC_NRSHIFT = 0
if platform.system() == 'Linux':
    _IOC_NRSHIFT = 0
_IOC_TYPESHIFT = (_IOC_NRSHIFT + _IOC_NRBITS)
if platform.system() == 'Linux':
    _IOC_TYPESHIFT = 8
_IOC_SIZESHIFT = (_IOC_TYPESHIFT+_IOC_TYPEBITS)
_IOC_DIRSHIFT = (_IOC_SIZESHIFT+_IOC_SIZEBITS)

def _IOC_DIR(nr): return (((nr) >> _IOC_DIRSHIFT) & _IOC_DIRMASK)
def _IOC_TYPE(nr): return (((nr) >> _IOC_TYPESHIFT) & _IOC_TYPEMASK)
def _IOC_NR(nr): return (((nr) >> _IOC_NRSHIFT) & _IOC_NRMASK)
def _IOC_SIZE(nr): return (((nr) >> _IOC_SIZESHIFT) & _IOC_SIZEMASK)

_IOC_NONE = 0
_IOC_WRITE = 1
_IOC_READ = 2

def _IOC(dir, type, nr, size):
    return dir  << _IOC_DIRSHIFT  | \
            type << _IOC_TYPESHIFT | \
            nr   << _IOC_NRSHIFT   | \
            size << _IOC_SIZESHIFT

def _VICEIOCTL(id):
    return _IOW(ord('V'), id, sizeof(ViceIoctl))

def _VALIDVICEIOCTL(com):
    return com >= _VICEIOCTL(0) and com <= _VICEIOCT(255)

def _IOC_TYPE(nr):
    return (nr >> _IOC_TYPESHIFT) & _IOC_TYPEMASK

def _IOC_NR(nr):
    return (nr >> _IOC_NRSHIFT) & _IOC_NRMASK

def _IO(type, nr):
    return _IOC(_IOC_NONE, type, nr, 0)

def _IOR(type, nr, size):
    return _IOC(_IOC_READ, type, nr, size)

def _IOW(type, nr, size):
    return _IOC(_IOC_WRITE, type, nr, size)

def _IOWR(type, nr, size):
    return _IOC(_IOC_READ | _IOC_WRITE, type, nr, size)








