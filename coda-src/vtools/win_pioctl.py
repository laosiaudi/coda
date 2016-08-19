#! /usr/bin/env python

import os, sys
from util import *
from win32file import CreateFile
from win32file import DeviceIoControl

def _pioctl(path, com, vidata, follow):
    if not hasattr(win_pioctl, "set_flag"):
        win_pioctl.set_flag = True
        win_pioctl.cygdrive = "/cygdrive/./"
        win_pioctl.driveletter = None
        win_pioctl.ctl_file = ""
        win_pioctl.hdev = None

    mountPoint = getMountPoint()
    if len(mountPoint) == 2 and mountPoint[-1] == ':':
        win_pioctl.driveletter = mountPoint[0].lower()
        win_pioctl.cygdrive = win_pioctl.cygdrive[:10]
            + win_pioctl.driveletter

    win_pioctl.ctl_file = mountPoint + "\\" + CODA_CONTROL
    driveletter = win_pioctl.driveletter
    cygdrive = win_pioctl.cygdrive

    if not path:
        path = getMountPoint()

    if path:
        if driveletter and path.startswith(driveletter):
            path = path[2:]
        elif path.startswith(mountPoint):
            path = path[len(mountPoint):]
        elif path.startswith("/coda/"):
            path = path[6:]
        elif path.startswith(cygdrive):
            path = path[12:]
        elif path[0] != '/':
            cwd = os.getcwd()
            if cwd.startswith(mountPoint):
                prefix = cwd[len(mountPoint):]
            elif cwd.startswith(cygdrive):
                prefix = cwd[11:]
            else:
                #set error code?
                #does not look like a coda path
                return -1
            prefix = prefix + "/"
        else:
            #does not look like a coda path
            return -1;


    data = PioctlData(path, follow, vidata, cmd) =
    #send_data = prefix + path + data.vidata.in + data.pack()
    if not win_pioctl.hdev:
        try:
            win_pioctl.hdev = CreateFile(win_pioctl.ctl_file, 0, 0, None,
                                         OPEN_EXISTING, 0, None)
        except pywintypes.error:
            print 'Unable to open .CONTROL file'
            return -1
    hdev = win_pioctl.hdev
    out_size = max(data.vidata.out_size, ctypes.sizeof(ctypes.c_ulong))
    try:
        outbuf = DeviceIoControl(hdev, CODA_FSCTL_PIOCTL,
                                 data.vi.in_data, out_size, None)
        data.vi.out_data = outbuf
        return 0
    except:
        print 'DeviceIoControl failed with error'
        return -1
