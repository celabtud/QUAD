# QUAD
QUAD (Quantitative Usage Analysis of Data) consists of several tools that provide a comprehensive overview of the memory access behavior of an application. QUAD provides some useful dynamic information regarding data usage between any pair of co-operating functions/basic blocks in an application. The data transfer is calculated in the sense of Producer-Consumer binding. QUAD reports precisely which function is reading data produced by which function. 

# What is reported
Following quantities are reported:

1.	amount of data transferred (in bytes)
2.	number of unique memory addresses used in the transfer
3.	amount of unique data involved (in bytes) in transfer
4.	ranges of unique memory addresses used in the transfer


# Pin Tool Kit
The tool has been developed based on [Pin](http://www.pintool.org). Pin is a tool kit for dynamic instrumentation of programs. It supports Linux binary executables for Intel (R) Xscale (R), IA-32, Intel64 (64 bit x86), and Itanium (R) processors; Windows executables for IA-32 and Intel64; and MacOS executables for IA-32. Pin does not instrument an executable statically by rewriting it, but rather adds the code dynamically while the executable is running.

# How to Setup
Here is the list of steps to use Quad on a linux machine IA32 and Intel64 (x86 32-bit and 64-bit). On windows machine the procedure is same, but basic (copy/paste/extract) commands may differ.

1.	Download Pin tool kit, for instance Pin 2.12 revision 55942 can be downloaded from [here](http://software.intel.com/sites/landingpage/pintool/downloads/pin-2.12-55942-gcc.4.4.7-linux.tar.gz)

2.	Copy the downloaded `pin-2.12-55942-gcc.4.4.7-linux.tar.gz` package to directory where you want to install pin. For this discussion lets assume it is your home directory `/home`. A path variable can be defined as:
		
	export MYHOME=/home
		
and then copy can be performed as:
	
	cp pin-2.12-55942-gcc.4.4.7-linux.tar.gz $(MYHOME)/.

3.	Change directory to $(MYHOME)

	cd $(MYHOME)

4.	Extract Pin tool kit.

	tar -xzf pin-2.12-55942-gcc.4.4.7-linux.tar.gz pin-2.12

5.	Define a path variable to pin directory and also add it to path as:

	export PINHOME=$(MYHOME)/pin-2.12
	export PATH=$(PATH):$(PINHOME)

6.	Download Quad from github (https://github.com/celabtud/QUAD) or from (http://celabtud.github.com/QUAD/) to your home directory $(HOME), or any other directory where you want to install Quad. If you downloaded as tar.gz package, you will need to extract it. Rename the Quad directory to `QUAD`.

7.	Change directory to QUAD

	cd QUAD

8.	Define a path variable for QUAD directory as:

	export QUADHOME=$(MYHOME)/QUAD

9.	Make the Quad tool by:

	make

This will create QUAD.so in the `obj-ia32` (or `intel-64` depending upon your architecture) directory. This concludes the Quad setup and you are ready to use it.

###Note:
The path variables defined in the above process are only for the current terminal session. So in order to make them useful for later use, you can do the following:

Either add these path variables to your `.bashrc` file in your `home` directory.

Or add these paths to a source script. For this add the following lines (lets name file as sourceme) as:
		
	export MYHOME=/home
	export PINHOME=$(MYHOME)/pin-2.12
	export QUADHOME=$(MYHOME)/QUAD
	export PATH=$(PATH):$(PINHOME)

and save it. In order to define all these variables for current session only, you can source it as:

	source sourceme

### Note:
you can also test if these path variables are defined by printing them, for example
		
	echo $(QUADHOME)

### Note:
In order to stay update with the recent releases of Quad, it is recommended clone from github. So step 6 above will become:

	git clone https://github.com/celabtud/QUAD.git


### Note:
If you want to participate in the develpment of Quad, have a look at [Contribution Section](https://github.com/celabtud/QUAD/blob/master/CONTRIBUTING.md#developer-guidelines) below.


## Important Note for Windows users*
Pin on Windows uses 'dbghelp.dll' by Microsoft to provide symbolic information. This DLL is not distributed with the kit. In order to get support for symbolic information in Pin, you have to download the "Debugging Tools For Windows" package, version 6.8.4.0 from http://www.microsoft.com/whdc/devtools/debugging/default.mspx. Use "Debugging Tools For Windows 32-bit Version" for IA-32 architectures and "Debugging Tools For Windows - Native X64" for Intel(R) 64 architectures. Distribution of the debugging tools is provided in .msi format which must be installed to extract a single file. Copy dbghelp.dll from the package into "ia32\bin" or "intel64\bin" directory of the Pin kit. This directory should already contain pin.exe and pinvm.dll files.


# How to Use
Now that QUAD is setup, it can be invoked as follows.

	pin -t $(QUADHOME)/obj-ia32/QUAD.so <QUAD-command-line-options> -- <application-name> <application-command-line-options>

##Example Usage (Quad Hello World):
Lets assume we have an application, hello.c which we want to profile. Compile it (for instance with the following command):

	gcc -o hello hello.c
	
to generate the executable `hello`.

Now Quad can be invoked as follows:
	pin -t $(QUADHOME)/obj-ia32/QUAD.so -ignore_stack_access -- ./hello

or on an intel 64-bit architecture as:
	
	pin -t $(QUADHOME)/intel-64/QUAD.so -ignore_stack_access -- ./hello


# QUAD Command line Options
Beside the common built-in options for Pin-developed tools, there are some specific command line options available to customize QUAD. Here is the list:

### -dotShowBytes <0/1>
If 0 to disable the printing of 'Bytes' on the edges. If 1 enable the printing of 'Bytes' on the edges. Default value : 1

### -dotShowUnDVs <0/1>
If 0 to disable the printing of 'UnDVs' on the edges. If 1 enable the printing of 'UnDVs' on the edges. Default value : 1

### -dotShowRanges <0/1>
If n = 0 to disable the printing of 'ranges' on the edges. If n = 1 enable the printing of 'ranges' on the edges. Default value : 1

### -dotShowRangesLimit <n>
Set dotDotShowRangesLimit to the maximum number of ranges shown on an edge. Default value : 3

### -bbFuncCount <0/1>
Set to 1 to gather call number statistics and dump them to XML file. Default value : 0

### -filter_uncommon_functions <0/1>
Filter out uncommon function names which are unlikely to be defined by user (beginning with question mark, underscore(s), etc.) This is useful if you do not want to see strange function names (usually library functions) appearing in the report file(s). The default value for this flag is set to 'true', so use '-filter_uncommon_functions 0' if you want to see all the function names in the main image file of the application, or there are some functions that are filtered out mistakenly by QUAD!

### -include_external_images <0/1>
Trace functions that are contained in external image file(s). This option enables tracing the functions that are contained in external image file(s). By default, only the functions in the main image file are traced and reported. This option together with '-filter_uncommon_functions' provides more flexibility to include/exclude required/unwanted functions in the report files. This option also has considerable impact on the reported quantitative bindings data and the corresponding producers/consumers.

### -ignore_stack_access <0/1>
Ignore memory accesses within application's stack region. By default, QUAD tracks ALL memory accesses to produce binding information. This means, a function extensively using local variable(s) on the stack results in reporting self-bindings in the form of 'x->x' data transfers, which sometimes makes the reports polluted or we get some biased statistics due to a function's formal input parameter that is referenced many times in the stack region after call. To avoid this, there is a possibility to specify '-ignore_stack_access' in the command line, which tends to provide a clear and straightforward information to the user.

### -use_monitor_list <file_name>
Create output report files only for certain function(s) in the application and filter out the rest (the functions are listed in a text file whose name follows). This option is helpful if there is a need to have the output report files only for specific function(s) and not all. The function names to monitor should be specified in a normal text file, whose path/name should be provided as the following argument.

### -xmlfile  <file_name>
Specify file name for output XML file. The output report file is in XML format and named 'q2profiling.xml' by default. This can be changed by using this option.

### -verbose <0/1>
No silence! print the number of instruction currently being executed on the console. By default, QUAD is silent.

### -elf <0/1>
Used to read the names of the global symbols (using libelf library). Works only on Linux OS.

### -dotShowRanges <0/1>
Enable showing in the dot file, on the edges the ranges of memory accessed. In case libelf library is enabled, and if the ranges are pointing in global variables, the name of the global variables will be shown.

### -dotShowRangesLimit <0/1>
Limits the number of ranges printed on the edges by the option 'dotShowRanges'.

## Helpful resources
The [wiki](https://github.com/celabtud/QUAD/wiki) is your one-stop resource for Tutorials and How-to's, really check it out! Also, feel free to improve these wiki pages.


# Basic Term/Definitions
Here are some basic terms/definitions:

### BINDING
When a function (itself or someone this function is responsible for calling and we are simply not interested to monitor the callee; e.g. system library functions, etc.) writes to a memory location, and later the same memory location is read by another function, we say a binding is established.

### PRODUCER
The name of the function who is responsible for the most recent write to a memory address.

### CONSUMER
The name of the function who is responsible for reading from that memory location.

### DATA_TRANSFER
The total amount of data in terms of number of bytes

### UnMA
This value shows the number of unique memory addresses used for this transfer, it could be regarded as the actual size of memory buffer needed for the data transfer.

### UnDV
This value shows the number of unique data values involved in transfer


# Output Visualization
1-	QUAD will output a file named 'QDUGraph.dot' in the current directory. This file can be converted to the pdf format using the following command:

	dot -Tpdf QDUGraph.dot -O

2-	After the application execution, all the producer/consumer bindings information is stored into an XML file named 'q2profiling.xml' in the current directory by default (the previous <QUAD> elements in the XML file that already exists are overwritten).

3-	Summary report file ('ML_OV_Summary.txt') is also created containing information about the functions specified in a monitor list, only in case the user has specified the relevant option in the command line. This text file basically provides statistics on total bytes/UMA for selected functions. There are eight different values that are explained in the following.

* IN_ML -> Total number of bytes read by this function that a function in the monitor list is responsible for producing the value(s) of the byte(s)

* IN_ML_UMA -> Total number of unique memory addresses used corresponding to 'IN_ML'

* OUT_ML -> Total number of bytes read by a function in the monitor list that this function is responsible for producing the value(s) of the byte(s)

* OUT_ML_UMA -> Total number of unique memory addresses used corresponding to 'OUT_ML'
IN_ALL -> Total number of bytes read by this function that a function in the application is responsible for producing the value(s) of the byte(s)

* IN_ALL_UMA -> Total number of unique memory addresses used corresponding to 'IN_ALL'
OUT_ALL -> Total number of bytes read by a function in the application that this function is responsible for producing the value(s) of the byte(s)

* OUT_ALL_UMA -> Total number of unique memory addresses used corresponding to 'OUT_ALL'


# Develop with us
See Developer Guidelines in [CONTRIBUTING.md](https://github.com/celabtud/QUAD/blob/master/CONTRIBUTING.md). Have a look at our [issue tracker](https://github.com/celabtud/QUAD/issues)

# Get in touch
Join the [mailinglist](http://groups.google.com/group/CEQuad)

# Bug Report/Feature Request
Use Quad [mailinglist](http://groups.google.com/group/CEQuad)
