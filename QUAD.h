/*

QUAD

This tool is part of QUAD Toolset
http://sourceforge.net/projects/quadtoolset

Copyright © 2008-2011 Arash Ostadzadeh (ostadzadeh@gmail.com)
http://ce.et.tudelft.nl/~arash/


This file is part of QUADcore.

QUADcore is free software: you can redistribute it and/or modify 
it under the terms of the GNU Lesser General Public License as 
published by the Free Software Foundation, either version 3 of 
the License, or (at your option) any later version.

QUADcore is distributed in the hope that it will be useful, but 
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU Lesser General Public License for more details. You should have 
received a copy of the GNU Lesser General Public License along with QUADcore.
If not, see <http://www.gnu.org/licenses/>.


<LEGAL NOTICE>
--------------
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.  Redistributions
in binary form must reproduce the above copyright notice, this list of
conditions and the following disclaimer in the documentation and/or
other materials provided with the distribution. The names of the contributors 
must be retained to endorse or promote products derived from this software.
 
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL ITS CONTRIBUTORS 
BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER 
IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF 
THE POSSIBILITY OF SUCH DAMAGE.
*/

//==============================================================================
/* QUAD.h: 
 * The prototypes of tracing routines, used by QUAD tool
 *
 *  Authors: Imran Ashraf
*/
//==============================================================================

#ifndef __QUAD__H__
#define __QUAD__H__

#include "pin.H"
#include <pin.H>
#include <fcntl.h>
#include <stdio.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstring>
#include <string>
#include <vector>
#include <stack>
#include <set>
#include <map>
#include <algorithm>
#include <cmath>

#include "Channel.h"
#include "Exception.h"
#include "Q2XMLFile.h"
#include "BBlock.h"

#ifndef NULL
#define NULL 0L
#endif

extern Q2XMLFile *q2xml;

typedef struct 
{
    UINT64 total_IN_ML;  // total bytes consumed by this function, produced by a function in the monitor list
    UINT64 total_OUT_ML; // total bytes produced by this function, consumed by a function in the monitor list
    UINT64 total_IN_ML_UMA; // total UMA used by this function, produced by a function in the monitor list
    UINT64 total_OUT_ML_UMA; // total UMA used by this function, consumed by a function in the monitor list
    UINT64 total_IN_ALL; // total bytes consumed by this function, produced by any function in the application
    UINT64 total_OUT_ALL; // total bytes produced by this function, consumed by any function in the application
    UINT64 total_IN_ALL_UMA; // total UMA used by this function, produced by any function in the application
    UINT64 total_OUT_ALL_UMA; // total UMA used by this function, consumed by any function in the application
    vector<string> consumers;
    vector<string> producers;
}
TTL_ML_Data_Pack ;

extern map <string,TTL_ML_Data_Pack *> ML_OUTPUT ;  // used to maintain info regarding monitor list statistics

extern BOOL Monitor_ON;

extern map <string,ADDRINT> NametoADD;
extern map <ADDRINT,string> ADDtoName;

// A mapping between the name used and the functions names. This is needed
// as names can be also basic blocks/code fragments
extern map <string, string> NameToFunction;

// The number of calls for each function
extern map <string, int> FunctionToCount;

class GlobalSymbol {
    public:
        GlobalSymbol():start(0),size(0){};
        GlobalSymbol(ADDRINT st, ADDRINT sz):start(st),size(sz){};
        ADDRINT start;
        ADDRINT size;
};

extern map <string, GlobalSymbol*> globalSymbols;

extern KNOB<BOOL> KnobBBFuncCount;
extern KNOB<BOOL> KnobDotShowBytes;
extern KNOB<BOOL> KnobDotShowUnDVs;
extern KNOB<BOOL> KnobDotShowRanges;
extern KNOB<BOOL> KnobElf;
extern KNOB<int> KnobDotShowRangesLimit;


#endif //__QUAD__H__
