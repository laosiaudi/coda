#! /usr/bin/env python

import os, sys
from util import *
import fcntl
import ctypes
import array

#Linux and BSD version
def _pioctl(path, com, vidata, follow):
    cmd = (com & ~(PIOCPARM_MASK << 16))
    size = ((com >> 16) & PIOCPARM_MASK) + ctypes.sizeof(ctypes.c_char_p) + ctypes.sizeof(ctypes.c_int)
    cmd = cmd | ((size & PIOCPARM_MASK) << 16)

    if path == None:
        path = getMountPoint()
    if not hasattr(_pioctl, "ctlfile"):
        _pioctl.ctlfile = None
    ctlfile = _pioctl.ctlfile
    if ctlfile is None:
        mtpt = getMountPoint()
        ctlfile = mtpt + "/" + CODA_CONTROL
    data = PioctlData(ctypes.c_char_p(path), follow, vidata, 0)

    try:
        fd = open(ctlfile, "r")
    except:
        sys.stderr.write("%s does not exist!" % ctlfile)
        return -1
    print "fd is " + str(fd) + " cmd is " + str(cmd)
    buf = array.array("B", data)
    code = fcntl.ioctl(fd, cmd, buf);
    fd.close()

    return code

