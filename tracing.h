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
int RecordMemoryAccess(long int,long int,long int);

#endif //__TRACING__H__
