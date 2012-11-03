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

--------------
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
/* QUADcore.cpp: 
 * This file contains the main routines for the QUAD core tool which detects the 
 * actual data dependencies between the functions in a program.
 *
 *  Authors: Arash Ostadzadeh
 *           Roel Meeuws
*/
//==============================================================================


// when a monitor file is provided, a list of communicating functions with each kernel is extracted from the profile data and stored
// separately for each function in the monitor list in two modes (In one the kernel is acting as a producer, in the other, as a consumer.
// the names of the output text files are <kernel_(p).txt> and <kernel_(c).txt>

#include "pin.H"
#include <fcntl.h>
#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstring>
#include <string>
#include <vector>
#include <stack>
#include <set>
#include <map>

#include "Channel.h"
#include "Exception.h"
#include "Q2XMLFile.h"
#include "BBlock.h"

#ifdef QUAD_LIBELF
#include "gelf.h"
#endif

//----------------------------------------------------------------------
#if defined( WIN32 ) && defined( TUNE )
	#include <crtdbg.h>
	_CrtMemState startMemState;
	_CrtMemState endMemState;
#endif

#ifdef WIN32
#define DELIMITER_CHAR '\\'
#else 
#define DELIMITER_CHAR '/'
#endif

//---------- for current path
#ifdef WIN32
	#include <direct.h>
	#define GetCurrentDir _getcwd
#else
	#include <unistd.h>
	#define GetCurrentDir getcwd
#endif

/* ===================================================================== */
/* Global Variables */
/* ===================================================================== */

Q2XMLFile *q2xml; // also used in Tracing.cpp
BBList bblist;		//list of BBlocks

char main_image_name[100];

map <string,ADDRINT> NametoADD;
map <ADDRINT,string> ADDtoName;

class GlobalSymbol {
	public:
		GlobalSymbol():start(0),size(0){};
		GlobalSymbol(ADDRINT st, ADDRINT sz):start(st),size(sz){};
		ADDRINT start;
		ADDRINT size;
};

map <string, GlobalSymbol*> globalSymbols;


stack <string> CallStack; // our own virtual Call Stack to trace function call

set<string> SeenFname;
ADDRINT GlobalfunctionNo=0x1;

UINT64 Total_Ins=0;  // just for counting the total number of executed instructions
UINT32 Total_M_Ins=0; // total number of instructions but divided by a million
UINT64 Progress_Ins=0;
UINT32 Progress_M_Ins=0;
UINT32 Percentage=0;

BOOL Count_Only = FALSE;

BOOL Monitor_ON = FALSE;
BOOL Include_External_Images=FALSE; // a flag showing our interest to trace functions which are not included in the main image file

BOOL Uncommon_Functions_Filter=TRUE;

BOOL No_Stack_Flag = FALSE;   // a flag showing our interest to include or exclude stack memory accesses in analysis. The default value indicates tracing also the stack accesses. Can be modified by 'ignore_stack_access' command line switch
BOOL Verbose_ON = FALSE;  // a flag showing the interest to print something when the tool is running or not!

BOOL BBMODE = FALSE;

// A mapping between the name used and the functions names. This is needed
// as names can be also basic blocks/code fragments
map <string, string> NameToFunction;

// The number of calls for each function
map <string, int> FunctionToCount;

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
   
map <string,TTL_ML_Data_Pack *> ML_OUTPUT ;  // used to maintain info regarding monitor list statistics

char fileName[FILENAME_MAX];
char cCurrentPath[FILENAME_MAX];
/* ===================================================================== */
/* Commandline Switches */
/* ===================================================================== */

KNOB<BOOL> KnobCountOnly(KNOB_MODE_WRITEONCE, "pintool",
	"count_only", "0", "Set count_only to 1 to only count instructions");

KNOB<UINT32> KnobProgress_M_Ins(KNOB_MODE_WRITEONCE, "pintool",
	"m_ins", "0", "the number of instructions that will be executed divided by one million");

KNOB<UINT64> KnobProgress_Ins(KNOB_MODE_WRITEONCE, "pintool",
	"ins", "0", "the number of instructions that will be executed modulo one million");

KNOB<string> KnobBBFile(KNOB_MODE_WRITEONCE, "pintool",
	"bbfile","bbList.txt", "Specify file name for the text file containg BBlocks");

KNOB<BOOL> KnobBBMode(KNOB_MODE_WRITEONCE, "pintool",
	"bbMode","0", "Set bbMode to 1 to use basic block mode <specify basic blocks in bbList.txt >");

KNOB<BOOL> KnobBBFuncCount(KNOB_MODE_WRITEONCE, "pintool",
	"bbFuncCount","0", "Set bbFuncCount to 1 to gather call number statistics and dump them to XML file.");

KNOB<BOOL> KnobDotShowBytes(KNOB_MODE_WRITEONCE, "pintool",
	"dotShowBytes","1", "Set dotDotShowBytes to 0 to disable the printing of 'Bytes' on the edges.");

KNOB<BOOL> KnobDotShowUnDVs(KNOB_MODE_WRITEONCE, "pintool",
	"dotShowUnDVs","1", "Set dotDotShowUnDVs to 0 to disable the printing of 'UnDVs' on the edges.");

KNOB<BOOL> KnobDotShowRanges(KNOB_MODE_WRITEONCE, "pintool",
	"dotShowRanges","1", "Set dotDotShowRanges to 1 to enable the printing of 'ranges' on the edges.");

KNOB<int> KnobDotShowRangesLimit(KNOB_MODE_WRITEONCE, "pintool",
	"dotShowRangesLimit","3", "Set dotDotShowRangesLimit to the maximum number of ranges you want to show.");

KNOB<string> KnobXML(KNOB_MODE_WRITEONCE, "pintool",
	"xmlfile","dek_arch.xml", "Specify file name for output data in XML format");

KNOB<string> KnobApplication(KNOB_MODE_WRITEONCE, "pintool", 
	"applic","", "Specify application name for XML file format");

// TODO: This should be determined automatically (vladms)
KNOB<string> KnobElf(KNOB_MODE_WRITEONCE, "pintool", 
	"elf","", "Specify the name of the elf file (this works only if libelf support is compiled in QUAD).");

KNOB<string> KnobMonitorList(KNOB_MODE_WRITEONCE, "pintool", 
	"use_monitor_list","", "Create output report files only for certain function(s) in the application and filter out the rest (the functions are listed in a text file whose name follows)");
	
KNOB<BOOL> KnobIgnoreStackAccess(KNOB_MODE_WRITEONCE, "pintool",
	"ignore_stack_access","0", "Ignore memory accesses within application's stack region");

KNOB<BOOL> KnobIgnoreUncommonFNames(KNOB_MODE_WRITEONCE, "pintool",
	"filter_uncommon_functions","1", "Filter out uncommon function names which are unlikely to be defined by user (beginning with question mark, underscore(s), etc.)");

KNOB<BOOL> KnobIncludeExternalImages(KNOB_MODE_WRITEONCE, "pintool",
	"include_external_images","0", "Trace functions that are contained in external image file(s)");

KNOB<BOOL> KnobVerbose_ON(KNOB_MODE_WRITEONCE, "pintool",
	"verbose","0", "Print information on the console during application execution");
    
/* ===================================================================== */

#include "tracing.cpp"

/* ===================================================================== */
VOID EnterFC(char *name,bool flag) 
{
	// revise the following in case you want to exclude some unwanted functions under Windows and/or Linux
	if (!flag) return;   // not found in the main image, so skip the current function name update

	#ifdef WIN32
	if (Uncommon_Functions_Filter)
	{
		if	(
			//commented the following as the functions in libraries were needed (e.g. in KLT)
			name[0]=='_' ||
			name[0]=='?' ||
			!strcmp(name,"GetPdbDll") || 
			!strcmp(name,"DebuggerRuntime") || 
			!strcmp(name,"atexit") || 
			!strcmp(name,"failwithmessage") ||
			!strcmp(name,"pre_c_init") ||
			!strcmp(name,"pre_cpp_init") ||
			!strcmp(name,"mainCRTStartup") ||
			!strcmp(name,"NtCurrentTeb") ||
			!strcmp(name,"check_managed_app") ||
			!strcmp(name,"DebuggerKnownHandle") ||
			!strcmp(name,"DebuggerProbe") ||
			!strcmp(name,"failwithmessage") ||
			!strcmp(name,"unnamedImageEntryPoint")
			) 
			return;
	}
	#else
	if (Uncommon_Functions_Filter)
	{
		if  (
			//commented the following as the functions in libraries were needed (e.g. in KLT)
			name[0]=='_' || 
			name[0]=='?' || 
			name[0]=='.' ||
			!strcmp(name,"call_gmon_start") || 
			!strcmp(name,"frame_dummy") 
			) 
			
			return;
	}
	#endif

	// update the current function name
	string RName(name);
	CallStack.push(RName);
	
	if(FunctionToCount.find(RName)==FunctionToCount.end()) {
		FunctionToCount[RName]=1;
	} else {
		FunctionToCount[RName]++;
	}
}

//============================================================================

VOID EnterFC_EXTERNAL_OK(char *name) 
{
	// revise the following in case you want to exclude some unwanted functions under Windows and/or Linux
	#ifdef WIN32
	if (Uncommon_Functions_Filter)
	{
		if	(		
			name[0]=='_' ||
			name[0]=='?' ||
			!strcmp(name,"GetPdbDll") || 
			!strcmp(name,"DebuggerRuntime") || 
			!strcmp(name,"atexit") || 
			!strcmp(name,"failwithmessage") ||
			!strcmp(name,"pre_c_init") ||
			!strcmp(name,"pre_cpp_init") ||
			!strcmp(name,"mainCRTStartup") ||
			!strcmp(name,"NtCurrentTeb") ||
			!strcmp(name,"check_managed_app") ||
			!strcmp(name,"DebuggerKnownHandle") ||
			!strcmp(name,"DebuggerProbe") ||
			!strcmp(name,"failwithmessage") ||
			!strcmp(name,"unnamedImageEntryPoint")
			) 
			return;
	}
	#else
	if (Uncommon_Functions_Filter)
	{
 		if(
			name[0]=='_' || 
			name[0]=='?' ||
			name[0]=='.' ||
			!strcmp(name,"call_gmon_start") || 
			!strcmp(name,"frame_dummy")
			) 
			return;
	}
	#endif

	// update the current function name	 
	string RName(name);
	CallStack.push(RName);
}

//============================================================================

INT32 Usage()
{
    cerr<<endl
		<<"QUAD (Quantitative Usage Analysis of Data) "
		<<endl
		<<"This tool provides quantitative data usage statistics by rigorously tracing and analysing"
		<<"all memory accesses within an application.\n"
		<<KNOB_BASE::StringKnobSummary()
		<<endl;

    return -1;
}

/* ===================================================================== */

VOID  Return(VOID *ip)
{
	string fn_name = RTN_FindNameByAddress((ADDRINT)ip);

	if(!(CallStack.empty()) && (CallStack.top()==fn_name))
	{  
		CallStack.pop();
	}
			
}

/* ===================================================================== */
VOID UpdateCurrentFunctionName(RTN rtn,VOID *v)
{
	bool flag;
	char *s=new char[120];
	string RName;
		
	RName=RTN_Name(rtn);
	strcpy(s,RName.c_str());
	RTN_Open(rtn);
            
	// Insert a call at the entry point of a routine to push the current routine to Call Stack
	if (!Include_External_Images)  // I need to know whether or not the function is in the main image
	{
		flag=(!((IMG_Name(SEC_Img(RTN_Sec(rtn))).find(main_image_name)) == string::npos));
		RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)EnterFC, IARG_PTR, s, IARG_BOOL, flag, IARG_END);    
	}
	else
	{
		RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)EnterFC_EXTERNAL_OK, IARG_PTR, s, IARG_END);    
	}
	
	// Insert a call at the exit point of a routine to pop the current routine from Call Stack if we have the routine on the top
	// RTN_InsertCall(rtn, IPOINT_AFTER, (AFUNPTR)exitFc, IARG_PTR, RName.c_str(), IARG_END);
	RTN_Close(rtn);
}

/* ===================================================================== */

VOID Fini(INT32 code, VOID *v)
{
    cerr << "\nFinished executing the instrumented application..." << endl;

    if (Count_Only)
    {
    	cerr << "Counted Instructions: " << Total_M_Ins << " M + " << Total_Ins << endl;
    }
    else
    {
	    CreateDSGraphFile();
	    if(Monitor_ON)
		    CreateTotalStatFile();
    }
	
    cerr << "done!" << endl;
}

/* ===================================================================== */

static VOID RecordMem(VOID * ip, CHAR r, VOID * addr, INT32 size, BOOL isPrefetch)
{
	if(!isPrefetch) // if this is not a prefetch memory access instruction  
	{
		string ftnName=CallStack.top();
		if(!SeenFname.count(ftnName))  // this is the first time I see this function name in charge of access
		{
			SeenFname.insert(ftnName);  // mark this function name as seen
			GlobalfunctionNo++;      // create a dummy Function Number for this function
			NametoADD[ftnName]=GlobalfunctionNo;   // create the string -> Number binding
			ADDtoName[GlobalfunctionNo]=ftnName;   // create the Number -> String binding
		} 
		
		for(int i=0;i<size;i++)
		{
			RecordMemoryAccess((ADDRINT)addr,NametoADD[ftnName],r=='W');
			addr=((char *)addr)+1;  // cast not needed anyway!
		}//end for
	}// end of not a prefetch
}

/* ===================================================================== */

static VOID RecordMemSP(VOID * ip, VOID * ESP, CHAR r, VOID * addr, INT32 size, BOOL isPrefetch)
{
	if(!isPrefetch) // if this is not a prefetch memory access instruction  
	{
		string ftnName=CallStack.top(); //top of the stack is the currently open function
		
		if(BBMODE)
		{
			string filename;    // This will hold the source file name.
			INT32 line = 0;     // This will hold the line number within the file.
			
			PIN_LockClient();
			// In this example, we don't print the column number so there is no reason to obtain it.
			// Simply pass a NULL pointer instead. Also, acquiring the client lock is required here
			// in analysis functions.
			PIN_GetSourceLocation( (ADDRINT)ip, NULL, &line, &filename);
			PIN_UnlockClient();
			ftnName = bblist.probeBB(filename, ftnName, line);
			
			if(NameToFunction.find(ftnName)==NameToFunction.end()) {
				NameToFunction[ftnName]=CallStack.top();
			}
		}
		
		if (addr >= ESP) return;  // if we are reading from the stack range, ignore this access

		if(!SeenFname.count(ftnName))  // this is the first time I see this function name in charge of access
		{
			SeenFname.insert(ftnName);  // mark this function name as seen
			GlobalfunctionNo++;      // create a dummy Function Number for this function
			NametoADD[ftnName]=GlobalfunctionNo;   // create the string -> Number binding
			ADDtoName[GlobalfunctionNo]=ftnName;   // create the Number -> String binding
		} 

		for(int i=0;i<size;i++)
		{
			RecordMemoryAccess((ADDRINT)addr,NametoADD[ftnName],r=='W');
			addr=((char *)addr)+1;  // cast not needed anyway!
		}//end for
	}// end of not a prefetch
}

/* ===================================================================== */

// increment routine for the total instruction counter
VOID IncreaseTotalInstCounter()
{
	Total_Ins++;
	if (Total_Ins>999999)
	{
		Total_M_Ins++;
		Total_Ins=0;
		if (Verbose_ON) {
			cout<<(char)(13)<<"                                                                   ";
			cout<<(char)(13)<<"Instructions executed so far = "<<Total_M_Ins<<" M";
		}
	}
	if (!Count_Only && (Progress_Ins > 0 || Progress_M_Ins > 0)) {
		double PTot = Progress_Ins / 100 + Progress_M_Ins * 10000 /*one million divided by one hundred*/;
		UINT32 pr = Total_M_Ins * (1000000 / PTot) + Total_Ins / PTot;
		if (Percentage != pr) {
			Percentage = pr;
			cerr << "Progress: |"
				<< setfill('=') << setw(Percentage / 5) << ""
				<< setfill(' ') << setw(20 - Percentage / 5) << ""
				<< "| "
				<< setiosflags(ios_base::right) << setw(3) << Percentage << "%"
				<< "\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b" << flush;
		}
	}
}

/* ===================================================================== */

// Is called for every instruction and instruments reads and writes and the Ret instruction
VOID Instruction(INS ins, VOID *v)
{
	
	INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)IncreaseTotalInstCounter, IARG_END);

	if (INS_IsRet(ins))  	
	{
		// we are monitoring the 'ret' instructions since we need to know when we are leaving functions 
		//in order to update our own virtual 'Call Stack'. The mechanism to inject instrumentation code 
		//to update the Call Stack (pop) upon leave is not implemented directly contrary to the dive 
		//in mechanism. Could be a point for further improvement?! ...
		INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)Return, IARG_INST_PTR, IARG_END);
	}

	if (!Count_Only)
	{
		if (!No_Stack_Flag)
		{
			if (INS_IsMemoryRead(ins) || INS_IsStackRead(ins) )
			{
				INS_InsertPredicatedCall(
					ins, IPOINT_BEFORE, (AFUNPTR)RecordMem,
					IARG_INST_PTR,
					IARG_UINT32, 'R',
					IARG_MEMORYREAD_EA,
					IARG_MEMORYREAD_SIZE,
					IARG_UINT32, INS_IsPrefetch(ins),
					IARG_END);
			}

			if (INS_HasMemoryRead2(ins))
			{
				INS_InsertPredicatedCall(
					ins, IPOINT_BEFORE, (AFUNPTR)RecordMem,
					IARG_INST_PTR,
					IARG_UINT32, 'R',
					IARG_MEMORYREAD2_EA,
					IARG_MEMORYREAD_SIZE,
					IARG_UINT32, INS_IsPrefetch(ins),
					IARG_END);
			}

			if (INS_IsMemoryWrite(ins) || INS_IsStackWrite(ins) ) 
			{
				INS_InsertPredicatedCall(
					ins, IPOINT_BEFORE, (AFUNPTR)RecordMem,
					IARG_INST_PTR,
					IARG_UINT32, 'W',
					IARG_MEMORYWRITE_EA,
					IARG_MEMORYWRITE_SIZE,
					IARG_UINT32, INS_IsPrefetch(ins),
					IARG_END);
			}
		} // end of Stack is ok!
		else  // ignore stack access
		{
			if (INS_IsMemoryRead(ins) )
			{
			INS_InsertPredicatedCall(
					ins, IPOINT_BEFORE, (AFUNPTR)RecordMemSP,
					IARG_INST_PTR,
					IARG_REG_VALUE, REG_STACK_PTR,
					IARG_UINT32, 'R',
					IARG_MEMORYREAD_EA,
					IARG_MEMORYREAD_SIZE,
					IARG_UINT32, INS_IsPrefetch(ins),
					IARG_END);
			}

			if (INS_HasMemoryRead2(ins))
			{
				INS_InsertPredicatedCall(
					ins, IPOINT_BEFORE, (AFUNPTR)RecordMemSP,
					IARG_INST_PTR,
					IARG_REG_VALUE, REG_STACK_PTR,
					IARG_UINT32, 'R',
					IARG_MEMORYREAD2_EA,
					IARG_MEMORYREAD_SIZE,
					IARG_UINT32, INS_IsPrefetch(ins),
					IARG_END);
			}

			if (INS_IsMemoryWrite(ins)) 
			{
				INS_InsertPredicatedCall(
					ins, IPOINT_BEFORE, (AFUNPTR)RecordMemSP,
					IARG_INST_PTR,
					IARG_REG_VALUE, REG_STACK_PTR,
					IARG_UINT32, 'W',
					IARG_MEMORYWRITE_EA,
					IARG_MEMORYWRITE_SIZE,
					IARG_UINT32, INS_IsPrefetch(ins),
					IARG_END);
			}
		} // end of ignore stack 
	}
}

/* ===================================================================== */

const char * StripPath(const char * path)
{
	const char * file = strrchr(path,DELIMITER_CHAR);
	if (file)
		return file+1;
	else
		return path;
}
/* ===================================================================== */

int main(int argc, char *argv[])
{
	char fileNameOnly[FILENAME_MAX]="test.c";
	if (!GetCurrentDir(fileName, sizeof(fileName) ) )
		return -1;
	
	strcat(fileName, "/");
	strcat(fileName, fileNameOnly);
	
	cerr << endl << "Initializing QUAD framework..." << endl;
	string xmlfilename,monitorfilename,bbFileName;
	string applicationName;
	char temp[100];

	// assume Out_of_the_main_function_scope as the first routine
	CallStack.push("Out_of_the_main_function_scope");
	SeenFname.insert("Out_of_the_main_function_scope");
	NametoADD["Out_of_the_main_function_scope"]=GlobalfunctionNo; 
	ADDtoName[GlobalfunctionNo]="Out_of_the_main_function_scope";

	// reserve the function ID #0 for the case of reading from a memory with no producer!
	NametoADD["UNKNOWN_PRODUCER(CONSTANT_DATA)"]=0x0; 
	ADDtoName[0x0]="UNKNOWN_PRODUCER(CONSTANT_DATA)";

	PIN_InitSymbols();

	if( PIN_Init(argc,argv) )
		return Usage();

	Count_Only=KnobCountOnly.Value();  // whether to count instructions only.
	Progress_M_Ins = KnobProgress_M_Ins.Value();
	Progress_Ins = KnobProgress_Ins.Value();
#ifndef QUAD_LIBELF
	if(KnobElf.Value().size()>0) {
		printf("ERROR: Trying to use Elf file option when libelf support not compiled in QUAD\n");
	}
#endif

	xmlfilename=KnobXML.Value();   // this is the name of the output XML file
	applicationName=KnobApplication.Value();
	
	BBMODE=KnobBBMode.Value(); //weather BB mode or not
	bbFileName=KnobBBFile.Value(); //name of Basic Block File
	
	No_Stack_Flag=KnobIgnoreStackAccess.Value(); // Stack access ok or not?
	monitorfilename=KnobMonitorList.Value(); // this is the name of the monitorlist file to use
	Uncommon_Functions_Filter=KnobIgnoreUncommonFNames.Value(); // interested in uncommon function names or not?
	Include_External_Images=KnobIncludeExternalImages.Value(); // include/exclude external image files?
	Verbose_ON=KnobVerbose_ON.Value();  // print something or not during execution

	if (!Count_Only)
	{
		if(BBMODE)
		{
			//initialize the basic blocks from file specified
			bblist.initFromFile(bbFileName);
		}
	
		// parse the command line arguments for the main image name and the status of the monitorlist flag
		for (int i=1;i<argc-1;i++)
		{
			if (!strcmp(argv[i],"-use_monitor_list") ) 
				Monitor_ON = TRUE;
		
			if (!strcmp(argv[i],"--")) 
			{
				strcpy(temp,argv[i+1]);
				break;
			}   
		}
		strcpy(main_image_name,StripPath(temp));

		// ------------------ XML file pre-processing ---------------------------------------   
		string ns("q2:");
		string fileName("q2profiling.xml");
		q2xml = new Q2XMLFile(fileName,ns,applicationName);
		// ------------------ Monitorlist file processing ---------------------------------------   
		if (Monitor_ON)  // user is interested in filtering out 
		{
			ifstream monitorin;
		
			monitorin.open(monitorfilename.c_str());
		
			if (!monitorin)
			{
				cerr<<"\nCan not open the monitor list file ("<<monitorfilename.c_str()<<")... Aborting!\n";
				return 4;
			}
		
			TTL_ML_Data_Pack * DPP;
			string item;

			do
			{
				monitorin>>item;	// get the next function name in the monitor list
				if (monitorin.eof()) break;	// oops we are finished!
				DPP=new TTL_ML_Data_Pack;
				if (!DPP) 
				{
					cerr<<"\nMemory allocation failure in monitor list construction... Aborting!\n";
					return 5;
				}
			
				DPP->total_IN_ML=0;
				DPP->total_OUT_ML=0;
				DPP->total_IN_ML_UMA=0;
				DPP->total_OUT_ML_UMA=0;
				DPP->total_IN_ALL=0;
				DPP->total_OUT_ALL=0;
				DPP->total_IN_ALL_UMA=0;
				DPP->total_OUT_ALL_UMA=0;
			
				ML_OUTPUT[item]=DPP;
		
			} while(1);	

			monitorin.close();	    
		}    
		// -----------------------------------------------------------------------------------------   


		RTN_AddInstrumentFunction(UpdateCurrentFunctionName,0);
	}
	
	INS_AddInstrumentFunction(Instruction, 0);
	PIN_AddFiniFunction(Fini, 0); 

	cerr << "Starting the application to be analysed..." << endl;

#ifdef QUAD_LIBELF
	if(KnobElf.Value().size()>0) {

		int elf_fd = open(KnobElf.Value().c_str(),O_RDONLY);
		Elf* elf;
		if (elf_version(EV_CURRENT) == EV_NONE) {
			printf("ERROR: ELF library initialization failed: %s",elf_errmsg(-1));
		}
	
		elf = elf_begin(elf_fd, ELF_C_READ ,NULL);
		// We read the symbol array

		if(elf==NULL) {
			printf("ERROR: ELF loading failed: %s",elf_errmsg(-1));
		} else {
			Elf_Scn *scn;
			int symbol_count, i;
			Elf_Data *edata =NULL;
			GElf_Shdr shdr;
			GElf_Sym sym;
			scn = NULL; 
			
			if (Verbose_ON) {
				printf("  QUAD global variable detection enabled\n");
			}
			while ((scn = elf_nextscn(elf, scn)) != NULL) { 
				if (gelf_getshdr(scn, &shdr) != &shdr)
					printf( "getshdr() failed: %s.", elf_errmsg(-1));
				if(shdr.sh_type == SHT_SYMTAB) {
					edata = elf_getdata(scn, edata);
					symbol_count = shdr.sh_size / shdr.sh_entsize;
					
					// loop through to grab all symbols
					for(i = 0; i < symbol_count; i++) {
						// libelf grabs the symbol data using gelf_getsym()
						gelf_getsym(edata, i, &sym);
						
						if(ELF32_ST_BIND(sym.st_info)==STB_GLOBAL &&
						  ELF32_ST_TYPE(sym.st_info)==STT_OBJECT && sym.st_size>0) {
							if (Verbose_ON) {
								printf("    Symbol name %s (%08x %08d)\n",
								  elf_strptr(elf, shdr.sh_link, sym.st_name),
								  (int)sym.st_value, (int)sym.st_size);
							}
							globalSymbols[string(elf_strptr(elf, shdr.sh_link, sym.st_name))] =
							  new GlobalSymbol((int)sym.st_value, (int)sym.st_size);
						}
					}
				}
			}
		}
		close(elf_fd);
	}
#endif // QUAD_LIBELF

	PIN_StartProgram(); // Never returns

	return 0;
}

