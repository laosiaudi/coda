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

    data = PioctlData(ctypes.c_char_p(path), follow, vidata)

    try:
        fd = open(ctlfile, "rb")
    except:
        sys.stderr.write("%s does not exist!" % ctlfile)
        return -1

    tmp = array.array('B', data.send())
    code = fcntl.ioctl(fd, cmd, tmp, 1);
    fd.close()
    return code

