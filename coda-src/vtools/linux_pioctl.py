#! /usr/bin/env python

import os, sys
from util import *

#Linux and BSD version
def _pioctl(path, com, vidata, follow):
    cmd = (com & ~(PIOCPARM_MASK << 16))
    if path == None:
        path = getMountPoint()
    if not hasattr(linux_pioctl, "ctlfile"):
        linux_pioctl.ctlfile = None
    if ctlfile is None:
        mtpt = getMountPoint()
        ctlfile = mtpt + CODA_CONTROL
    data = PioctlData(path, follow, vidata, 0)

    try:
        fd = open(path, "r")
    except:
        print("%s does not exist!" % path, file = sys.stderr)
        return -1

    code = fcntl.ioctl(fd, cmd, data);
    fd.close()

    return code

