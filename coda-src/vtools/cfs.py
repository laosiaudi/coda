#! /usr/bin/env python
import sys
import util
import socket
import struct
import ctypes
from pioctl import pioctl
import array


def fail():
    print 'Bogus or missing opcode: type "cfs help" for list'
    sys.exit(1)

def brave(cmd):
    print '\tDANGER:    %s\n' % cmd_dict[cmd]["danger"]
    '''
    Only works for Python 2! May change later
    '''
    op = raw_input('\tDo you really want to do this? [n] ')

    if op == 'y':
        print '\tFools rush in where angels fear to tread ........\n'
        return 1
    else:
        print '\tDiscretion is the better part of valor!\n'
        return 0

def parseHost(name_or_ip):
    addr = None

    #translate to ipv4 format first
    try:
        addr = socket.gethostbyname(name_or_ip)
    except:
        return None

    return socket.inet_aton(addr)

def CheckServers(argv, cmd):
    global piobuf
    global CFS_PIOBUFSIZE
    if len(argv) < 2 or len(argv) > 10:
        print 'Usage: %s' % cmd_dict[cmd]["usetxt"]
        sys.exit(1)

    MAXHOSTS = 8
    vio =util.ViceIoctl(0, ctypes.addressof(piobuf), 0, CFS_PIOBUFSIZE)

    host_str = ""
    cnt = 0
    for host in sys.argv[2:]:
        addr = parseHost(host)
        if not addr:
            continue
        host_str += addr
        cnt += 1

    if cnt:
	tmp = struct.pack("i%ds" % len(host_str), cnt, host_str)
        vio.in_data = tmp
        vio.in_size = len(tmp)
    print 'Contacting Server .....\n'
    rc = pioctl(None, util._VICEIOCTL(util._VIOCCKSERV), vio, 1)
    if rc < 0:
        print 'VIOCCKSERV Error'
        sys.exit(1)

    #See if there are any dead servers
    num_of_down_srv = struct.unpack('<I', (piobuf)[0:4])[0]
    downsrvarray = [socket.inet_ntoa(piobuf[i:i + 4]) for i in range(4, len(piobuf) - 1, 4)]
    if not num_of_down_srv:
        print 'All servers up!'
        return

    #Print out names of dead servers
    print 'These servers still down: '
    for srv in downsrvarray:
	print srv
        hent = socket.gethostbyaddr(srv)
        if hent:
            print hent[0]
        else:
            print srv
    return

def BeginRepair():
    pass

def CheckPointML():
    pass

def CheckVolumes():
    pass

def ClearPriorities():
    pass

def DisableASR():
    pass

def EnableASR():
    pass

def EndRepair():
    pass

def ExamineClosure():
    pass

def At_CPU():
    pass

def At_SYS():
    pass

def FlushASR():
    pass

def FlushCache():
    pass

def FlushObject():
    pass

def FlushVolume():
    pass

def GetFid():
    pass

def GetPFid():
    pass

def GetPath():
    pass

def GetMountPoint():
    pass

def Help():
    pass

def ListACL():
    pass

def ListCache():
    pass

def ListVolume():
    pass

def LookAside():
    pass

def LsMount():
    pass

def MarkFidIncon():
    pass

def MkMount():
    pass

def PurgeML():
    pass

def ReplayClosure():
    pass

def RmMount():
    pass

def SetACL():
    pass

def SetQuota():
    pass

def SetVolume():
    pass

def TruncateLog():
    pass

def UnloadKernel():
    pass

def WaitForever():
    pass

def WhereIs():
    pass

def Redir():
    pass

def Disconnect():
    pass

def Reconnect():
    pass

def WriteDisconnect():
    pass

def Adaptive():
    pass

def Strong():
    pass

def ForceReintegrate():
    pass

def ExpandObject():
    pass

def CollapseObject():
    pass

def ListLocal():
    pass

def CheckLocal():
    pass

def DiscardLocal():
    pass

def PreserveLocal():
    pass


abbreviation = {
    "br": "beginrepair",
    "cs": "checkservers",
    "ck": "checkpointml",
    "cp": "clearpriorities",
    "dasr": "disableasr",
    "easr": "endbleasr",
    "er": "endrepair",
    "ec": "examineclosure",
    "@cpu": "cpuname",
    "@sys": "sysname",
    "fasr": "flushasr",
    "fl": "flushobject",
    "gf": "getfid",
    "gp": "getpath",
    "gmt": "getmountpoint",
    "la": "listacl",
    "lc": "listcache",
    "lv": "listvol",
    "lka": "lookaside",
    "mkm": "mkmount",
    "rc": "replayclosure",
    "rmm": "rmmount",
    "sa": "setacl",
    "sq": "setquota",
    "sv": "setvol",
    "tl": "truncatelog",
    "uk": "unloadkernel",
    "wf": "waitforever",
    "wd": "writedisconnect",
    "fr": "forcereintegrate",
}

cmd_dict = {
    "beginrepair": {"handler": BeginRepair,
                    "usetext": "cfs beginrepair <inc-obj-name>",
                    "helptxt": "Expose replicas of inc. objects",
                    "danger": None
        },
    "checkservers": {"handler": CheckServers,
                     "usetext": "cfs checkservers <servernames>",
                     "helptxt": "Check up/down status of servers",
                     "danger":  None
        },
    "checkpointml": {"handler": CheckPointML,
                     "usetext": "cfs checkpointml <dir> <checkpoint-dir>",
                     "helptxt": "Checkpoint volume modify log",
                     "danger": None
        },
    "checkvolumes": {"handler": CheckVolumes,
                     "usetext": "cfs checkvolumes",
                     "helptxt": "Check volume/name mappings",
                     "danger": None
        },
    "clearpriorities": {"handler": ClearPriorities,
                        "usetext": "cfs clearpriorities",
                        "helptxt": "Clear short-term priorities (DANGEROUS)",
                        "danger": "important files may be lost, if disconnected"
        },
    "disableasr": {"handler": DisableASR,
                   "usetext": "cfs disableasr <dir/file>",
                   "helptxt": "Disable ASR execution in object's volume",
                   "danger": None
        },
    "enableasr": {"handler": EnableASR,
                       "usetext": "cfs enableasr <dir/file>",
                       "helptxt": "Enable ASR execution in this volume",
                       "danger": None
        },
    "endrepair": {"handler": EndRepair,
                  "usetext": "cfs endrepair <inc-obj-name>",
                  "helptxt": "Hide individual replicas of inc objects",
                  "danger": None
        },
    "examineclosure": {"handler": ExamineClosure,
                       "usetext": "cfs ec [-c] [<closure> <closure> ...]",
                       "helptxt": "Examine reintegration closure",
                       "danger": None
        },
    "cpuname": {"handler": At_CPU,
                "usetext": "cfs {cpuname|@cpu}",
                "helptxt": "print the @cpu expansion for the current platform",
                "danger": None
        },
    "sysname": {"handler": At_SYS,
                "usetext": "cfs {sysname|@sys}",
                "helptxt": "print the @sys expansion for the current platform",
                "danger": None
        },
    "flushasr": {"handler": FlushASR,
                 "usetext": "cfs fasr <file-name>",
                 "helptxt": "force the asr to get executed on next access",
                 "danger": None
        },
    "flushcache": {"handler": FlushCache,
                   "usetext": "cfs flushcache",
                   "helptxt": "Flush entire cache (DANGEROUS)",
                   "danger": "all files will be lost, if disconnected"
        },
    "flushobject": {"handler": FlushObject,
                    "usetext": "cfs flushobject <obj>  [<obj> <obj> ...]",
                    "helptxt": "Flush objects from cache ",
                    "danger": "these files will be lost, if disconnected"
        },
    "flushvolume": {"handler": FlushVolume,
                    "usetext": "cfs flushvolume  <dir> [<dir> <dir> ...]",
                    "helptxt": "Flush all data in volumes (DANGEROUS)",
                    "danger": "important files may be lost, if disconnected"
        },
    "getfid": {"handler": GetFid,
               "usetext": "cfs getfid <path> [<path> <path> ...]",
               "helptxt": "Map path to fid",
               "danger": None
        },
    "getpfid": {"handler": GetPFid,
                "usetext": "cfs getpfid <fid> [<fid> <fid> ...]",
                "helptxt": "Map fid to parent fid",
                "danger": None
        },
    "getpath": {"handler": GetPath,
                "usetext": "cfs getpath <fid> [<fid> <fid> ...]",
                "helptxt": "Map fid to volume-relative path",
                "danger": None
        },
    "getmountpoint": {"handler": GetMountPoint,
                      "usetext": "cfs getmountpoint <volid> [<volid> <volid> ...]",
                      "helptxt": "Get mount point pathname for specified volume",
                      "danger": None
        },
    "help": {"handler": Help,
             "usetext": "cfs help [opcode]",
             "helptxt": "Type \"cfs help <opcode>\" for specific help on <opcode>",
             "danger": None
        },
    "listacl": {"handler": ListACL,
                "usetext": "cfs listacl <dir> [<dir> <dir> ...]",
                "helptxt": "List access control list ",
                "danger": None
        },
    "listcache": {"handler": ListCache,
                  "usetext": "cfs listcache [-f <file>] [-l] [-ov] [-onv] [-all] [<vol> <vol> ...]",
                  "helptxt": "List cached fsobjs",
                  "danger": None
        },
    "listvol": {"handler": ListVolume,
                "usetext": "cfs listvol [-local] <dir> [<dir> <dir> ...]",
                "helptxt": "Display volume status (-local avoids querying server)",
                "danger": None
        },
    "lookaside": {"handler": LookAside,
                  "usetext": "cfs lookaside [--clear] +/-<db1> +/-<db2> +/-<db3> ....\n       cfs lookaside --list\n",
                  "helptxt": "Add, remove or list cache lookaside databases",
                  "danger": None
        },
    "lsmount": {"handler": LsMount,
                "usetext": "cfs lsmount <dir> [<dir> <dir> ...]",
                "helptxt": "List mount point",
                "danger": None
        },
    "markincon": {"handler": MarkFidIncon,
                  "usetext": "cfs markincon <path> [<path> <path> ...]",
                  "helptxt": "Mark object in conflict",
                  "danger": "this meddles with the version vector and can trash the object"
        },
    "mkmount": {"handler": MkMount,
                "usetext": "cfs mkmount <directory> [<volume name>]",
                "helptxt": "Make mount point",
                "danger": None
        },
    "purgeml": {"handler": PurgeML,
                "usetext": "cfs purgeml <dir>",
                "helptxt": "Purge volume modify log (DANGEROUS)",
                "danger": "will destroy all changes made while disconnected"
        },
    "replayclosure": {"handler": ReplayClosure,
                      "usetext": "cfs replayclosure [-i] [-r] [<closure> <closure> ...]",
                      "helptxt": "Replay reintegration closure",
                      "danger": None
        },
    "rmmount": {"handler": RmMount,
                "usetext": "cfs rmmount <dir> [<dir> <dir> ...]",
                "helptxt": "Remove mount point ",
                "danger": None
        },
    "setacl": {"handler": SetACL,
               "usetext": "cfs setacl [-clear] [-negative] <dir> <name> <rights> [<name> <rights> ....]",
               "helptxt": "Set access control list",
               "danger": None
        },
    "setquota": {"handler": SetQuota,
	         "usetext": "cfs setquota <dir> <blocks>",
                 "helptxt": "Set maximum disk quota",
                 "danger": None
        },
    "setvol": {"handler": SetVolume,
               "usetext": "cfs setvol <dir> [-max <disk space quota in 1K units>] [-min <disk space guaranteed>] [-motd <message of the day>] [-offlinemsg <offline message>]",
               "helptxt": "Set volume status ",
               "danger": None
        },
    "truncatelog": {"handler": TruncateLog,
                    "usetext": "cfs truncatelog",
                    "helptxt": "Truncate the RVM log at this instant",
                    "danger": None
        },
    "unloadkernel": {"handler": UnloadKernel,
                     "usetext": "cfs unloadkernel",
                     "helptxt": "Unloads the kernel module",
                     "danger": None
        },
    "waitforever": {"handler": WaitForever,
                    "usetext": "cfs waitforever [-on] [-off]",
                    "helptxt": "Control waitforever behavior",
                    "danger": None
        },
    "whereis": {"handler": WhereIs,
                "usetext": "cfs whereis <dir> [<dir> <dir> ...]",
                "helptxt": "List location of object",
                "danger": None
        },
    "redir": {"handler": Redir,
              "usetext": "cfs redir <dir> <ip-address>",
              "helptxt": "Redirect volume to a staging server",
              "danger": None
        },
    "disconnect": {"handler": Disconnect,
                   "usetext": "cfs disconnect <servernames>",
                   "helptxt": "Partition from file servers (A LITTLE RISKY)",
                   "danger": None
        },
    "reconnect": {"handler": Reconnect,
                  "usetext": "cfs reconnect <servernames>",
                  "helptxt": "Heal partition to servers from cfs disconnect",
                  "danger": None
        },
    "writedisconnect": {"handler": WriteDisconnect,
                        "usetext": "cfs writedisconnect [-age <sec>] [-time <sec>] [<dir> <dir> <dir> ...]",
                        "helptxt": "Set write-disconnection parameters for specified volumes.\n\t\t\t(default parameters -age 60 -time 30.0)",
                        "danger": None
        },
    "adaptive": {"handler": Adaptive,
	         "usetext": "cfs adaptive [<dir> <dir> <dir> ...]",
	         "helptxt": "Reintegrate quickly, but adapt for low bandwidth.\n\t\t\talias for 'cfs writedisconnect -age 0 -time 15.0'",
                 "danger": None
        },
    "strong": {"handler": Strong,
	       "usetext": "cfs strong [<dir> <dir> <dir> ...]",
	       "helptxt": "Force reintegration as each operation completes.\n\t\t\talias for 'cfs writedisconnect -age 0 -time 0'",
	       "danger": None
        },
    "forcereintegrate": {"handler": ForceReintegrate,
	                 "usetext": "cfs forcereintegrate [<dir> <dir> <dir> ...]",
	                 "helptxt": "Force modifications in specified volumes to the server",
                         "danger": None
	},
    "expand": {"handler": ExpandObject,
               "usetext": "cfs expand <path>",
               "helptxt": "Expand object into a directory containing all replicas",
               "danger": None
        },
    "collapse": {"handler": CollapseObject,
                 "usetext": "cfs collapse <path>",
                 "helptxt": "Collapse expanded object (see cfs expand)",
                 "danger": None
        },
    "listlocal": {"handler": ListLocal,
                  "usetext": "cfs listlocal <path>",
                  "helptxt": "List the head record of a volume's CML",
                  "danger": None
        },
    "checklocal": {"handler": CheckLocal,
                   "usetext": "cfs checklocal <path>",
                   "helptxt": "Check why the head of a volume's CML failed reintegration",
                   "danger": None
        },
    "discardlocal": {"handler": DiscardLocal,
                     "usetext": "cfs discardlocal [-force] [-all] <path>",
                     "helptxt": "Discard the inconsistent head record of a volume's CML",
                     "danger": None
        },
    "preservelocal": {"handler": PreserveLocal,
                      "usetext": "cfs preservelocal [-all] <path>",
                      "helptxt": "Preserve the inconsistent head record of a volume's CML",
                      "danger": None
        },
}
CFS_PIOBUFSIZE = 2048
piobuf = ctypes.create_string_buffer('\0' * CFS_PIOBUFSIZE)

if __name__ == '__main__':
    if len(sys.argv) < 2:
        fail()
    op = sys.argv[1]
    if op not in abbreviation and op not in cmd_dict:
        fail()

    if op in abbreviation:
        cmd = abbreviation[op]
    else:
        cmd = op

    if cmd_dict[cmd]["danger"]:
        if not brave(cmd):
            sys.exit(0)

    cmd_dict[cmd]["handler"](sys.argv, cmd)

    

    sys.exit(0)
