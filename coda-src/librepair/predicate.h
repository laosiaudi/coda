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

typedef int (*PtrFuncInt)(int, resreplica *, resdir_entry **, int, char *realm);
extern PtrFuncInt Predicates[];

extern int nPredicates;
/* the conflict types order must match the array Predicates order */
#define	STRONGLY_EQUAL	0
#define	WEAKLY_EQUAL	1
#define	ALL_PRESENT	2
#define SUBSET_RENAME	3
#define	SUBSET_CREATE	4
#define	SUBSET_REMOVE	5
#define MAYBESUBSET_REMOVE 6
#define	UNKNOWN_CONFLICT    -1

