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
#define PERROR(desc) do { fflush(stdout); perror(desc); } while(0)
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

void vproc::cfs_output(int fd, char *error) {
    if (fd == 0 || error == NULL)
        return;
    ssize_t bytes_written = 0;
    ssize_t total = strlen(error);

    while (bytes_written < total) {
        int len = (CFS_PIOBUFSIZE < total - bytes_written) ?
            CFS_PIOBUFSIZE : (total - bytes_written);
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
int vproc::cfs_parseHost(char *name_or_ip, struct in_addr *addr)
{
    struct hostent *h;

    if (inet_aton(name_or_ip, addr))
        return 1;

    h = gethostbyname(name_or_ip);
    if (!h) {
        addr->s_addr = INADDR_ANY;
        return 0;
    }

    *addr = *(struct in_addr *)h->h_addr;
    return 1;
}

int vproc::cfs_getlongest(int argc, char *argv[]) {
/* Return length of longest argument; for use in aligning printf() output */
    int i, max, next;

    max = 0;
    for (i = 2; i < argc; i++)
    {
        next = (int) strlen(argv[i]);
        if (max < next) max = next;
    }
    return(max);
}

int vproc::cfs_listcache(int fd) {
    int i;

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
                    cfs_output(fd, "Failed in GetMountPoint.");
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
                cfs_output(fd, "Failed in Listcache volume.");
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
            cfs_output(fd, "Failed in ListCache");
            cfs_free(argv, argc);
            return -1;
        }

        if (vio.out_size) {
            cfs_copyout(vio.out, fd);
            unlink(vio.out);
        }
    }
    cfs_free(argv, argc);
    return 0;

}

#define MAXHOSTS 8  /* from venus.private.h, should be in vice.h! */
int vproc::cfs_checkservers(int fd) {
    int rc, i;
    struct in_addr *downsrvarray;
    char *insrv=0;
    struct ViceIoctl vio;
    char parameter_path[] = "/tmp/.checkservers";
    int argc = 0;
    char **argv = cfs_getparameter(parameter_path, &argc);


    vio.in = 0;
    vio.in_size = 0;
    vio.out = piobuf;
    vio.out_size = CFS_PIOBUFSIZE;

    /* pack server host ids, if any */
    /* format of vio.in is #servers, hostid, hostid, ... */
    if (argc > 2)
        insrv = (char *) malloc(sizeof(int) + sizeof(struct in_addr) * MAXHOSTS);

    int hcount = 0;
    for (i = 2; i < argc; i++) {
        int ix = (int) (hcount * sizeof(struct in_addr) + sizeof(int));
        struct in_addr host;

        if (!cfs_parseHost(argv[i], &host)) continue;

        *((struct in_addr *) &insrv[ix]) = host;
        hcount++;
    }
    if (hcount) {
        ((int *) insrv)[0] = hcount;
        vio.in = insrv;
        vio.in_size = (int) (hcount * sizeof(struct in_addr) + sizeof(int));
    }

    //printf("Contacting servers .....\n"); /* say something so Puneet knows something is going on */
    //rc = pioctl(NULL, _VICEIOCTL(_VIOCCKSERV), &vio, 1);
    do_ioctl(NULL, _VICEIOCTL(_VIOCCKSERV), &vio);
    //if (rc < 0) { PERROR("VIOCCKSERV"); exit(-1); }
    if (u.u_error) {
        PERROR("VIOCCKSERV");
        cfs_free(argv, argc);
        return -1;
    }

    /* See if there are any dead servers */
    if (insrv) free(insrv); /* free insrv only if it was alloc before */
    downsrvarray = (struct in_addr *) piobuf;
    if (downsrvarray[0].s_addr == 0) {
        cfs_output(fd, "All servers up\n");
        cfs_free(argv, argc);
        return 0;
    }

    /* Print out names of dead servers */
    cfs_output(fd, "These servers still down\n");
    //printf("These servers still down: ");
    for (i = 0; downsrvarray[i].s_addr != 0; i++) {
        struct hostent *hent;

        hent = gethostbyaddr((char *)&downsrvarray[i], sizeof(long), AF_INET);
        char str[80];
        if (hent)
            sprintf(str, " %s", hent->h_name);
        else
            sprintf(str, " %s", inet_ntoa(downsrvarray[i]));
        cfs_output(fd, str);
    }
    cfs_output(fd, "\n");
    return 0;
}

int vproc::cfs_checkvolumes(int fd) {
    struct ViceIoctl vio;

    vio.in = 0;
    vio.in_size = 0;
    vio.out = 0;
    vio.out_size = 0;

    do_ioctl(NULL, _VICEIOCTL(_VIOCCKBACK), &vio);
    if (u.u_error) {
        PERROR("VIOC_VIOCCKBACK");
        return -1;
    }
    return 0;
}

int vproc::cfs_clearpriorities(int fd) {
    struct ViceIoctl vio;

    vio.in = 0;
    vio.in_size = 0;
    vio.out = 0;
    vio.out_size = 0;

    do_ioctl(NULL, _VICEIOCTL(_VIOC_CLEARPRIORITIES), &vio);
    if (u.u_error) {
        PERROR("  VIOC_CLEARPRIORITIES");
        return -1;
    }
    return 0;

}

int vproc::cfs_flushcache(int fd) {
    struct ViceIoctl vio;

    vio.in = 0;
    vio.in_size = 0;
    vio.out = 0;
    vio.out_size = 0;

    do_ioctl(NULL, _VICEIOCTL(_VIOC_FLUSHCACHE), &vio);
    if (u.u_error) {
        PERROR("VIOC_FLUSHCACHE");
        return -1;
    }
    return 0;
}

int vproc::cfs_flushvolume(int fd) {
    char parameter_path[] = "/tmp/.flushvolume";
    int argc = 0;
    char **argv = cfs_getparameter(parameter_path, &argc);
    int w = cfs_getlongest(argc, argv);
    struct ViceIoctl vio;

    vio.in = 0;
    vio.in_size = 0;
    vio.out = 0;
    vio.out_size = 0;

    int i = 2;
    for (; i < argc; i ++) {
        if (argc > 3) {
            char str[80];
            sprintf(str, "  %*s  ", w, argv[i]); /* echo input if more than one fid */
            cfs_output(fd, str);
        }

        do_ioctl(argv[i], _VICEIOCTL(_VIOC_FLUSHVOLUME), &vio);
        if (u.u_error) {
            PERROR("  VIOC_FLUSHVOLUME");
            continue;
        } else {
            if (argc > 3)
                cfs_output(fd, "\n");
        }
    }
    return 0;
}

int vproc::cfs_listacl(int fd) {
    int i, j, rc;
    struct ViceIoctl vio;
    struct acl a;
    char rightsbuf[10];

    char parameter_path[] = "/tmp/.listacl";
    int argc = 0;
    char **argv = cfs_getparameter(parameter_path, &argc);
    char str[CFS_PIOBUFSIZE];

    for (i = 2; i < argc; i++)
    {
        vio.in = 0;
        vio.in_size = 0;
        vio.out_size = CFS_PIOBUFSIZE;
        vio.out = piobuf;
        do_ioctl(argv[i], _VICEIOCTL(_VIOCGETAL), &vio);
        if (u.u_error) {
            PERROR(argv[i]);
            continue;
        }

        if (argc > 3) {
            memset(str, 0, CFS_PIOBUFSIZE);
            sprintf(str, "\n%s:\n", argv[i]);
        }
        rc = parseacl(vio.out, &a);
        if (rc < 0)
        {
            cfs_output(fd, "Venus returned an ill-formed ACL\n");
            return -1;
        }
        for (j = 0; j < a.pluscount; j++)
        {
            memset(str, 0, CFS_PIOBUFSIZE);
            fillrights(a.plusentries[j].rights, rightsbuf);
            sprintf(str, "%20s %-8s\n", a.plusentries[j].id, rightsbuf); /* id */
            cfs_output(fd, str);
        }

        for (j = 0; j < a.minuscount; j++)
        {
            memset(str, 0, CFS_PIOBUFSIZE);
            fillrights(a.minusentries[j].rights, rightsbuf);
            sprintf(str, "%20s %-8s\n", a.plusentries[j].id, rightsbuf); /* id */
            cfs_output(fd, str);
        }
    }

}

void vproc::cfs_lsmount(int fd) {
    int i, rc, w;
    struct ViceIoctl vio;
    char path[MAXPATHLEN+1], tail[MAXNAMLEN+1];
    char *s;
    struct stat stbuf;

    char parameter_path[] = "/tmp/.lsmount";
    int argc = 0;
    char **argv = cfs_getparameter(parameter_path, &argc);
    char str[CFS_PIOBUFSIZE];

    w = getlongest(argc, argv);

    for (i = 2; i < argc; i++)
    {

        if (argc > 3) {
            memset(str, 0, CFS_PIOBUFSIZE);
            sprintf(str, "  %*s  ", w, argv[i]);
            cfs_output(fd, str);
        }

        /* First see if it's a dangling mount point */
        rc = stat(argv[i], &stbuf);
        if (rc < 0 && errno == ENOENT)
        {/* It's a nonexistent name, alright! */
            rc = lstat(argv[i], &stbuf);
            if (stbuf.st_mode & 0xff00 & S_IFLNK)
            {/* It is a sym link; read it */
                rc = readlink(argv[i], piobuf, (int) sizeof(piobuf));
                if (rc < 0) { PERROR("readlink"); continue; }
                /* Check if first char is a '#'; this is a heuristic, but rarely misleads */
                memset(str, 0, CFS_PIOBUFSIZE);
                if (piobuf[0] == '#') {
                    sprintf(str, "Dangling sym link; looks like a mount point for volume \"%s\"\n", &piobuf[1]);
                    cfs_output(fd, str);
                } else {
                    sprintf(str, "!! %s is not a volume mount point\n", argv[i]);
                    cfs_output(fd, str);
                }
                continue;
            }
        }

        /* Next see if you can chdir to the target */
        s = cfs_myrealpath(argv[i], path, sizeof(path));
        if (!s) {
            if (errno == ENOTDIR)
                fprintf(stderr, "%s - Not a mount point\n", argv[i]);
            else
                PERROR("realpath");
            continue;
        }

        /* there must be a slash in what myrealpath() returns */
        s = strrchr(path, '/');
        *s = 0; /* lop off last component */
        strncpy(tail, s+1, sizeof(tail)); /* and copy it */

        /* Ask Venus if this is a mount point */
        vio.in = tail;
        vio.in_size = (int) strlen(tail)+1;
        vio.out = piobuf;
        vio.out_size = CFS_PIOBUFSIZE;
        memset(piobuf, 0, CFS_PIOBUFSIZE);
        do_ioctl(path, _VICEIOCTL(_VIOC_AFS_STAT_MT_PT), &vio);
        if (u.u_error)
        {
            if (errno == EINVAL || errno == ENOTDIR) {
                memset(str, 0, CFS_PIOBUFSIZE);
                sprintf(str, "Not a mount point\n");
                cfs_output(fd, str);
                continue;
            } else { PERROR("VIOC_AFS_STAT_MT_PT"); continue; }
        }

        memset(str, 0, CFS_PIOBUFSIZE);
        sprintf(str, "Mount point for volume \"%s\"\n", &piobuf[1]);
        cfs_output(fd, str);
    }

}
char *vproc::cfs_myrealpath(const char *path, char *buf, size_t size) {
    char curdir[MAXPATHLEN+1], *s;
    int rc;

    s = getcwd(curdir, sizeof(curdir));
    if (!s) return NULL;

    rc = chdir(path);
    if (rc < 0) return NULL;

    s = getcwd(buf, size);

    chdir(curdir);
    return s;

}
int vproc::cfs_parseacl(char *s, struct acl *a) {
    int i;
    char *c, *r;

    translate(s, '\n', '\0');

    c = s;
    sscanf(c, "%d", &a->pluscount);
    c += strlen(c) + 1;
    if (a->pluscount > 0)
    {
        a->plusentries = (struct aclentry *) calloc(a->pluscount, sizeof(struct aclentry));
        CODA_ASSERT(a->plusentries);
    }
    sscanf(c, "%d", &a->minuscount);
    c += strlen(c) + 1;
    if (a->minuscount > 0)
    {
        a->minusentries = (struct aclentry *) calloc(a->minuscount, sizeof(struct aclentry));
        CODA_ASSERT(a->minusentries);
    }
    for (i = 0; i < a->pluscount; i++)
    {
        r = strchr(c, '\t');
        if (r == 0) return(-1);
        *r = 0;

        a->plusentries[i].id = (char *)malloc(strlen(c) + 1);
        strcpy(a->plusentries[i].id, c);
        c += strlen(c) + 1;
        if (*c == 0) return(-1);
        a->plusentries[i].rights = atoi(c);
        c += strlen(c) + 1;
    }
    if (a->minuscount == 0) return(0);
    for (i = 0; i < a->minuscount; i++)
    {
        r = strchr(c, '\t');
        if (r == 0) return(-1);
        *r = 0;
        a->minusentries[i].id = (char *)malloc(strlen(c) + 1);
        strcpy(a->minusentries[i].id, c);
        c += strlen(c) + 1;
        if (*c == 0) return(-1);
        a->minusentries[i].rights = atoi(c);
        c += strlen(c) + 1;
    }
    return(0);

}
void vproc::cfs_translate(char *s, char oldc, char newc)
    /* Changes every occurence of oldc to newc in s */
{
    int i, size;

    size = (int) strlen(s);
    for (i = 0; i < size; i++)
        if (s[i] == oldc) s[i] = newc;
}
void vproc::cfs_fillrights(int x, char *s) {
    *s = 0;
    if (x & PRSFS_READ) strcat(s, "r");
    if (x & PRSFS_LOOKUP) strcat(s, "l");
    if (x & PRSFS_INSERT) strcat(s, "i");
    if (x & PRSFS_DELETE) strcat(s, "d");
    if (x & PRSFS_WRITE) strcat(s, "w");
    if (x & PRSFS_LOCK) strcat(s, "k");
    if (x & PRSFS_ADMINISTER) strcat(s, "a");
}

void vproc::cfs_copyout(char *path, int fd) {
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
    LOG(0, ("\n[For debug]=========================== %d\n", idx));

    switch(idx) {
        case 0: {
            cfs_listcache(fd);
            break;
        }
        case 1: {
            cfs_checkservers(fd);
            break;
        }
        case 2: {
            cfs_checkvolumes(fd);
            break;
        }
        case 3: {
            cfs_clearpriorities(fd);
            break;
        }
        case 4: {
            cfs_flushcache(fd);
            break;
        }
        case 5:
            cfs_flushvolume(fd);
            break;
        case 6:
            cfs_listacl(fd);
            break;
        case 7:
            cfs_lsmount(fd);
            break;
    }

    //int fd = f->GetContainerFD();
    return 0;

}
