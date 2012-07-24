/*
* QUAD.cpp 
*
* Authour : S. Arash Ostadzadeh (ostadzadeh@gmail.com)
*
*/

// when a monitor file is provided, a list of communicating functions with each kernel is extracted from the profile data and stored
// separately for each function in the monitor list in two modes (In one the kernel is acting as a producer, in the other, as a consumer.
// the names of the output text files are <kernel_(p).txt> and <kernel_(c).txt>

#include "pin.H"
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

stack <string> CallStack; // our own virtual Call Stack to trace function call

set<string> SeenFname;
ADDRINT GlobalfunctionNo=0x1;

UINT64 Total_Ins=0;  // just for counting the total number of executed instructions
UINT32 Total_M_Ins=0; // total number of instructions but divided by a million

BOOL Monitor_ON = FALSE;
BOOL Include_External_Images=FALSE; // a flag showing our interest to trace functions which are not included in the main image file

BOOL Uncommon_Functions_Filter=TRUE;

BOOL No_Stack_Flag = FALSE;   // a flag showing our interest to include or exclude stack memory accesses in analysis. The default value indicates tracing also the stack accesses. Can be modified by 'ignore_stack_access' command line switch
BOOL Verbose_ON = FALSE;  // a flag showing the interest to print something when the tool is running or not!

BOOL BBMODE = FALSE;

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

KNOB<string> KnobBBFile(KNOB_MODE_WRITEONCE, "pintool",
	"bbfile","bbList.txt", "Specify file name for the text file containg BBlocks");

KNOB<BOOL> KnobBBMode(KNOB_MODE_WRITEONCE, "pintool",
	"bbMode","0", "Set bbMode to 1 to use basic block mode <specify basic blocks in bbList.txt >");

KNOB<string> KnobXML(KNOB_MODE_WRITEONCE, "pintool",
	"xmlfile","dek_arch.xml", "Specify file name for output data in XML format");
	
KNOB<string> KnobApplication(KNOB_MODE_WRITEONCE, "pintool", 
	"applic","", "Specify application name for XML file format");

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
    CreateDSGraphFile();
    if(Monitor_ON)
	    CreateTotalStatFile();
	
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

			if (Verbose_ON && Total_Ins>999999)
			{
				Total_M_Ins++;
				cout<<(char)(13)<<"                                                                   ";
				cout<<(char)(13)<<"Instructions executed so far = "<<Total_M_Ins<<" M";
				Total_Ins=0;
			}
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
			if (Verbose_ON && Total_Ins>999999)
			{
				Total_M_Ins++;
				cout<<(char)(13)<<"                                                                   ";
				cout<<(char)(13)<<"Instructions executed so far = "<<Total_M_Ins<<" M";
				Total_Ins=0;
			}
		}//end for
	}// end of not a prefetch
}

/* ===================================================================== */

// increment routine for the total instruction counter
VOID IncreaseTotalInstCounter()
{
	Total_Ins++;
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

	xmlfilename=KnobXML.Value();   // this is the name of the output XML file
	applicationName=KnobApplication.Value();
	
	BBMODE=KnobBBMode.Value(); //weather BB mode or not
	bbFileName=KnobBBFile.Value(); //name of Basic Block File
	
	No_Stack_Flag=KnobIgnoreStackAccess.Value(); // Stack access ok or not?
	monitorfilename=KnobMonitorList.Value(); // this is the name of the monitorlist file to use
	Uncommon_Functions_Filter=KnobIgnoreUncommonFNames.Value(); // interested in uncommon function names or not?
	Include_External_Images=KnobIncludeExternalImages.Value(); // include/exclude external image files?
	Verbose_ON=KnobVerbose_ON.Value();  // print something or not during execution

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
		
		}while(1);	

		monitorin.close();	    
	}    
	// -----------------------------------------------------------------------------------------   

	RTN_AddInstrumentFunction(UpdateCurrentFunctionName,0);
	INS_AddInstrumentFunction(Instruction, 0);
	PIN_AddFiniFunction(Fini, 0); 

	cerr << "Starting the application to be analysed..." << endl;
	PIN_StartProgram(); // Never returns

	return 0;
}

