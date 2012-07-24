/*
 * tracing.h
 *
 * Authour : S. Arash Ostadzadeh (ostadzadeh@gmail.com)
 * Authour : Roel Meeuws (r.j.meeuws@gmail.com)
 *
 */

#ifndef __TRACING__H__
#define __TRACING__H__

#include <pin.H>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <map>

#ifndef NULL
#define NULL 0L
#endif

int CreateDSGraphFile();
int RecordMemoryAccess(VOID *, long int,long int,long int);

#endif //__TRACING__H__
