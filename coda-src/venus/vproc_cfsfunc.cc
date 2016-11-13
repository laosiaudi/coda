/* BLURB gpl

                           Coda File System
                              Release 6

          Copyright (c) 1987-2003 Carnegie Mellon University
                  Additional copyrights listed below

This  code  is  distributed "AS IS" without warranty of any kind under
the terms of the GNU General Public Licence Version 2, as shown in the
file  LICENSE.  The  technical and financial  contributors to Coda are
listed in the file CREDITS.

                        Additional copyrights
                           none currently

#*/

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>

#include <vice.h>
#include <pioctl.h>
#include <coda.h>
#include <prs.h>

#include <errno.h>
#include "coda_assert.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#ifdef  __linux__
#if defined(__GLIBC__) && __GLIBC__ >= 2
#include <dirent.h>
#else
#include <sys/dirent.h>
#endif
#endif

#ifdef __cplusplus
}
#endif

#ifdef  __linux__
#define direct dirent
#define d_namlen d_reclen
#endif
#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif
/* from libal */

/* from venus */
//#include "fso.h"
#include "vproc.h"
char **vproc::cfs_getparameter(char *path, int *argc) {
    FILE *file = fopen(path, "r");
    *argc = 0;

    char line[1000];
    char **argv;
    int cnt = 0;
    while (fgets(line, sizeof(line), file) != NULL) {
        if (!cnt) {
            *argc = atoi(line);
            argv = (char **)malloc(sizeof(char *) * (*argc));
        } else {
            int len = strlen(line);
            argv[cnt - 1] = (char *)malloc(sizeof(char) * len);
            memcpy(argv[cnt - 1], line, len);
            argv[cnt - 1][len - 1] = '\0';
            LOG(0, ("vproc::cfs_listcache, argv is %s len is %d", argv[cnt - 1], len));
        }
        cnt ++;
    }
    fclose(file);

    return argv;
}

void vproc::cfs_error(int fd, char *error) {
    if (fd == 0 || error == NULL)
        return;
    char buffer[CFS_PIOBUFSIZE];
    ssize_t n, bytes_written = 0;
    ssize_t total = strlen(error);

    while (bytes_written < total) {
        int len = CFS_PIOBUFSIZE < total - bytes_written;
        bytes_written += write(fd, error + bytes_written, len);
    }
    return;
}

void vproc::cfs_free(char **argv, int argc) {
    int i = 0;
    for (; i < argc; i ++) {
        free(argv[i]);
        argv[i] = 0;
    }
    free(argv);
}

int vproc::cfs_listcache(int fd) {
    int i, rc = 0;

    int long_format = 0;          /* If == 1, list in a long format. */
    unsigned int valid = 0;       /* list the following fsobjs, if
1: only valid, 2: only non-valid, 3: all */
    int vol_specified = 0;        /* If == 0, all volumes cache status are listed. */
    char *filename = NULL;        /* Specified output file. */

    struct listcache_in {
        uint8_t  long_format;   /* = long_format */
        uint8_t  valid;         /* = valid */
    } data;

    struct ViceIoctl vio;

    char parameter_path[] = "/tmp/.listcache";
    int argc = 0;
    char **argv = cfs_getparameter(parameter_path, &argc);

    int argi = 0;
    if (argc < 3) {
        vol_specified = 0;
        long_format = 0;
        valid = 3;
    } else {
        for (argi = 2; (argi < argc) && (argv[argi][0] == '-'); argi++) {
            if (strcmp(argv[argi], "-f") == 0 || strcmp(argv[argi], "-file") == 0) {
                if ( (argi + 1) < argc ) {
                    filename = argv[++argi];
                } else {                /* filename is not specified as argv[argi+1]. */
                    //exit(-1);
                    cfs_free(argv, argc);
                    return -1;
                }
            }
            else if (strcmp(argv[argi], "-l") == 0)
                long_format = 1;
            else if (strcmp(argv[argi], "-ov") == 0)
                valid |= 1;
            else if (strcmp(argv[argi], "-onv") == 0)
                valid |= 2;
            else if (strcmp(argv[argi], "-a") == 0 ||
                    strcmp(argv[argi], "-all") == 0)
                valid |= 3;
            else {
                //exit(-1);
                LOG(0, ("\n[For debug]vproc::cfs_listcache, should not be here!! %s\n", argv[argi]));
                cfs_free(argv, argc);
                return -1;
            }
        }
        if (argi == argc) vol_specified = 0;
        else vol_specified = 1;
        if (valid == 0) valid = 3;
    }


    if (vol_specified) {
        /* Volumes are specified. List cache for those volumes. */
        for (i = argi; i < argc; i++) {
            /* Set up parms to pioctl */
            VolumeId vol_id;
            char mtptpath[MAXPATHLEN], *path;
            if ( sscanf(argv[i], "%x", (unsigned int *)&vol_id) == 1 ) {
                vio.in = (char *)&vol_id;
                vio.in_size = (int) sizeof(VolumeId);
                vio.out = piobuf;
                vio.out_size = CFS_PIOBUFSIZE;
                memset(piobuf, 0, CFS_PIOBUFSIZE);

                /* Do the pioctl getting mount point pathname */
                do_ioctl(NULL, _VICEIOCTL(_VIOC_GET_MT_PT), &vio);

                if (u.u_error) {
                    cfs_error(fd, "Failed in GetMountPoint.");
                    cfs_free(argv, argc);
                    return -1;
                }
                strcpy(mtptpath, piobuf);
                path = mtptpath;
            } else {
                path = argv[i];
            }

            data.long_format = long_format;
            data.valid = valid;
            vio.in = (char *)&data;
            vio.in_size = (int) sizeof(struct listcache_in);
            vio.out_size = CFS_PIOBUFSIZE;
            vio.out = piobuf;
            memset(piobuf, 0, CFS_PIOBUFSIZE);

            /* Do the pioctl */
            //rc = pioctl(path, _VICEIOCTL(_VIOC_LISTCACHE_VOLUME), &vio, 1);
            do_ioctl(NULL, _VICEIOCTL(_VIOC_LISTCACHE_VOLUME), &vio);
            if (u.u_error) {
                cfs_error(fd, "Failed in Listcache volume.");
                cfs_free(argv, argc);
                return -1;
            }

            if (vio.out_size) {
                cfs_copyout(vio.out, fd);
                unlink(vio.out);
            }
        }
    } else {
        /* No volumes are specified. List all cache volumes. */
        /* Set up parms to pioctl */
        data.long_format = long_format;
        data.valid = valid;
        vio.in = (char *)&data;
        vio.in_size = (int) sizeof(struct listcache_in);

        vio.out_size = CFS_PIOBUFSIZE;
        vio.out = piobuf;
        memset(piobuf, 0, CFS_PIOBUFSIZE);

        /* Do the pioctl */
        do_ioctl(NULL, _VICEIOCTL(_VIOC_LISTCACHE), &vio);
        if (u.u_error) {
            cfs_error(fd, "Failed in ListCache");
            cfs_free(argv, argc);
            return -1;
        }

        if (vio.out_size) {
            cfs_copyout(vio.out, fd);
            unlink(vio.out);
        }
    }
    cfs_free(argv, argc);

}

void vproc::cfs_copyout(char *path, int fd) {
    ftruncate(fd, 0);
    ssize_t total = 0;
    char buffer[CFS_PIOBUFSIZE];
    ssize_t n, bytes_written;
    int src_fd;

    src_fd = ::open(path, O_RDONLY);
    if (src_fd == -1) {
        fprintf(stderr, "Failed to open %s for reading\n", path);
        return;
    }

    while ((n = read(src_fd, buffer, CFS_PIOBUFSIZE)) > 0) {
        bytes_written = write(fd, buffer, n);
        total += bytes_written;
        CODA_ASSERT(bytes_written == n);
    }
    ::close(src_fd);
    unlink(path);
    return;
}

int vproc::do_ioctl_op(int fd, int idx) {
    int long_format = 0;
    unsigned int valid = 3;
    //int vol_specified = 0;
    char *filename = NULL;

    struct listcache_in {
        uint8_t long_format;
        uint8_t valid;
    } _data;

    struct ViceIoctl vio;

    _data.long_format = long_format;
    _data.valid = valid;
    vio.in = (char *)&_data;
    vio.in_size = (int) sizeof(struct listcache_in);

    char piobuf[2048];
    //vio.out_size = CFS_PIOBUFSIZE;
    vio.out_size = 2048;
    vio.out = piobuf;
    memset(piobuf, 0, CFS_PIOBUFSIZE);

    switch(idx) {
        case 0: {
            cfs_listcache(fd);
            break;
        }
        case 1: {
            vio.in = 0;
            vio.in_size = 0;
            char up_msg[] = "All servers up\n";
            char down_msg[] = "These servers still down: ";
            do_ioctl(NULL, _VICEIOCTL(_VIOCCKSERV), &vio);
            struct in_addr *downsrvarray;
            downsrvarray = (struct in_addr *)piobuf;
            if (downsrvarray[0].s_addr == 0) {
                write(fd, up_msg, strlen(up_msg));
                return 0;
            } else {
                write(fd, down_msg, strlen(down_msg));
                int idx = 0;
                for (; downsrvarray[idx].s_addr != 0; idx ++) {
                    struct hostent *hent;

                    hent = gethostbyaddr((char *)&downsrvarray[idx],
                            sizeof(long), AF_INET);
                    if (hent)
                        write(fd, hent->h_name, strlen(hent->h_name));
                    else
                        write(fd, inet_ntoa(downsrvarray[idx]),
                                strlen(inet_ntoa(downsrvarray[idx])));
                }
                write(fd, "\n", 1);
                return 0;
            }
        }
        case 2: {
            vio.in = 0;
            vio.in_size = 0;
            vio.out = 0;
            vio.out_size = 0;
            do_ioctl(NULL, _VICEIOCTL(_VIOCCKBACK), &vio);
            return 0;
        }
        case 3: {
            vio.in = 0;
            vio.in_size = 0;
            vio.out = 0;
            vio.out_size = 0;
            do_ioctl(NULL, _VICEIOCTL(_VIOC_CLEARPRIORITIES), &vio);
            return 0;
        }
        case 4: {
            vio.in = 0;
            vio.in_size = 0;
            vio.out = 0;
            vio.out_size = 0;
            do_ioctl(NULL, _VICEIOCTL(_VIOC_FLUSHCACHE), &vio);
            return 0;
        }
        case 5:
            return 0;
        case 6:
            return 0;
        case 7:
            return 0;
    }

    //int fd = f->GetContainerFD();
    return 0;

}
