#! /usr/bin/env python

import os, sys
import fcntl
import platform

if sys.platform.startswith("win"):
    from win_pioctl import _pioctl
elif sys.platform.startswith("linux"):
    from linux_pioctl import _pioctl
else:
    raise ImportError("my module doesn't support this system")

def pioctl(path, com, vidata, follow):
    return _pioctl(path, com, vidata, follow)

