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
/* tracing.cpp: 
 * The tracing routines, used by QUAD tool
 *
 *  Authors: Arash Ostadzadeh
 *           Roel Meeuws
*/
//==============================================================================


#include "tracing.h"

#include "Exception.h"
#include "Q2XMLFile.h"
#include "Channel.h"
#include "RenewalFlags.h"


#define max(a,b) ((a)>(b)?(a):(b))
#define min(a,b) ((a)<(b)?(a):(b))

typedef ADDRINT addr_t; 
FILE* gfp,*ufa;

addr_t MaxLabel=0;

struct trieNode 
{
    struct trieNode * list[16];
 	FNodeList * RenewalFlags;
} 
*trieRoot=NULL,*graphRoot=NULL,*uflist=NULL;

struct AddressSplitter
{
    unsigned int h0:4;
    unsigned int h1:4;
    unsigned int h2:4;
    unsigned int h3:4;
    unsigned int h4:4;
    unsigned int h5:4;
    unsigned int h6:4;
    unsigned int h7:4;
};

// structure definition to keep track of producer->consumer Bindings! (number of bytes, the memory addresses used for exchange ...)
typedef struct 
{
	unsigned long long data_exchange;
	unsigned long long UniqueValues;
	ADDRINT producer;
	ADDRINT consumer;
	set<ADDRINT>* UniqueMemCells;
} 
Binding;

//------------------------------------------------------------------------------------------

void Update_total_statistics(string producer,string consumer,unsigned long int bytes,
			     unsigned long int u_m_a,bool p_f,bool c_f)
{
	if(p_f)
	{
		ML_OUTPUT[producer]->total_OUT_ALL+=bytes;
		ML_OUTPUT[producer]->total_OUT_ALL_UMA+=u_m_a;	
		ML_OUTPUT[producer]->consumers.push_back(consumer);	 // update the list of consumers for the particular producer in the ML
	}
	
	if(c_f)
	{
		ML_OUTPUT[consumer]->total_IN_ALL+=bytes;
		ML_OUTPUT[consumer]->total_IN_ALL_UMA+=u_m_a;
		ML_OUTPUT[consumer]->producers.push_back(producer);	 // update the list of producers for the particular consumer in the ML
	}
	
	if(p_f && c_f)
	{
		ML_OUTPUT[producer]->total_OUT_ML+=bytes;
		ML_OUTPUT[producer]->total_OUT_ML_UMA+=u_m_a;
		ML_OUTPUT[consumer]->total_IN_ML+=bytes;
		ML_OUTPUT[consumer]->total_IN_ML_UMA+=u_m_a;
	}
}

//------------------------------------------------------------------------------------------

int CreateTotalStatFile()
{
	ofstream out;
	ofstream out_list;
	unsigned int ii;

	map <string,TTL_ML_Data_Pack*> :: const_iterator pIter;

	out.open("ML_OV_Summary.txt");
	if(!out) 
	{
		cerr<<"\nCan not create the summary report file containing information about the functions specified in the monitor list..."<<endl;
		return 1;
	}

	cerr<< "\nCreating summary report file (ML_OV_Summary.txt) containing information about the functions specified in the monitor list..." << endl;

	out <<setw(30)<<setiosflags(ios::left)<<"Function"<<setw(12)<<"   IN_ML"
		<<setw(12)<<" IN_ML_UnMA"
		<<setw(12)<<"  OUT_ML"
		<<setw(12)<<"OUT_ML_UnMA"
		<<setw(12)<<"  IN_ALL"
		<<setw(12)<<"IN_ALL_UnMA"
		<<setw(12)<<"  OUT_ALL"
		<<setw(12)<<"OUT_ALL_UnMA"
		<<endl;

	out <<setw(30)<<"-----------------------------"<<setw(12)<<"-----------"
		<<setw(12)<<"-----------"
		<<setw(12)<<"-----------"
		<<setw(12)<<"-----------"
		<<setw(12)<<"-----------"
		<<setw(12)<<"-----------"
		<<setw(12)<<"-----------"
		<<setw(12)<<"-----------"
		<<endl;
		
	for ( pIter = ML_OUTPUT.begin( ) ; pIter != ML_OUTPUT.end( ) ; pIter++ )
	{
		out << setw(30) << setiosflags(ios::left) << pIter -> first;
		
		out<<setw(11) << setiosflags(ios::right) << pIter->second->total_IN_ML << " " 
		<<setw(11) << pIter->second->total_IN_ML_UMA << " " 
		<<setw(11) << pIter->second->total_OUT_ML << " " 
		<<setw(11) << pIter->second->total_OUT_ML_UMA << " " 
		<<setw(11) << pIter->second->total_IN_ALL << " " 
		<<setw(11) << pIter->second->total_IN_ALL_UMA << " " 
		<<setw(11) << pIter->second->total_OUT_ALL << " " 
		<<setw(11) << pIter->second->total_OUT_ALL_UMA 
		<< endl ;

		out.unsetf(ios::right);
		
		// store the list of communicating functions for each kernel in ML
		// consumers
		out_list.open((pIter -> first+"_(p).txt").c_str());
		if(!out_list) 
		{
		cerr<<"\nCan not create the report file containing the list of communicating functions for kernels ..."<<endl;
		return 1;
		}
		
		for(ii=0; ii < pIter->second->consumers.size(); ii++)  	  
			out_list << pIter->second->consumers[ii] << endl;
		
		out_list.close();

		// producers
		out_list.open((pIter -> first+"_(c).txt").c_str());
		if(!out_list) 
		{
			cerr<<"\nCan not create the report file containing the list of communicating functions for kernels ..."<<endl;
			return 1;
		}
		
		for(ii=0; ii < pIter->second->producers.size(); ii++)  	  
			out_list << pIter->second->producers[ii] << endl;
		
		out_list.close();

	}//end of for

	out << endl << "--" << endl;
	out << "IN_ML -> Total number of bytes read by this function that a function in the monitor list is responsible for producing the value(s) of the byte(s)" << endl;
	out << "IN_ML_UnMA -> Total number of unique memory addresses used corresponding to 'IN_ML'" << endl;
	out << "OUT_ML -> Total number of bytes read by a function in the monitor list that this function is responsible for producing the value(s) of the byte(s)" << endl;
	out << "OUT_ML_UnMA -> Total number of unique memory addresses used corresponding to 'OUT_ML'" << endl;

	out << "IN_ALL -> Total number of bytes read by this function that a function in the application is responsible for producing the value(s) of the byte(s)" << endl;
	out << "IN_ALL_UnMA -> Total number of unique memory addresses used corresponding to 'IN_ALL'" << endl;
	out << "OUT_ALL -> Total number of bytes read by a function in the application that this function is responsible for producing the value(s) of the byte(s)" << endl;
	out << "OUT_ALL_UnMA -> Total number of unique memory addresses used corresponding to 'OUT_ALL'" << endl;

	out.close();
	return 0;
}	
//------------------------------------------------------------------------------------------
int IsNewFunc(ADDRINT fadd)
{
	int currentLevel=0;
	int i;
	struct trieNode* currentLP;
	struct AddressSplitter* ASP= (struct AddressSplitter *)&fadd;

	unsigned int addressArray[8];

	addressArray[0]=ASP->h0;
	addressArray[1]=ASP->h1;
	addressArray[2]=ASP->h2;
	addressArray[3]=ASP->h3;
	addressArray[4]=ASP->h4;
	addressArray[5]=ASP->h5;
	addressArray[6]=ASP->h6;
	addressArray[7]=ASP->h7;

	currentLP=uflist;                
	while(currentLevel<7)  /* proceed to the last level */
	{
		if(! (currentLP->list[addressArray[currentLevel]]) ) /* create new level on demand */
		{
			if(!(currentLP->list[addressArray[currentLevel]]=(struct trieNode*)malloc(sizeof(struct trieNode))) ) 
			{
				fprintf(stderr,"Memory allocation failed in \'IsNewFunc()\'...");
				return 2; /* memory allocation failed*/
			}
			else
				for (i=0;i<16;i++) 
					(currentLP->list[addressArray[currentLevel]])->list[i]=NULL;
		}

		currentLP=currentLP->list[addressArray[currentLevel]];
		currentLevel++;
	}            
	if( !currentLP->list[addressArray[currentLevel]] )
	{
		currentLP->list[addressArray[currentLevel]] = currentLP; /* a dummy value to show the cell has been filled before!! */
		return 1; /* this function address is new */
	}
	return 0; /* function address exists in the list */
}
//------------------------------------------------------------------------------------------
void set2ranges(set<ADDRINT>* UnMAs, vector<Range> & ranges)
{
	ADDRINT curr, next; 
	Range r;
	set<ADDRINT>::const_iterator pos = UnMAs->begin();
	while(pos != UnMAs->end()) 
	{  
		curr= *pos;
		next= *(++pos);
		//cout<<curr<<'-';
		r.lower = curr;
		while( (next == curr + 1 ) && (pos != UnMAs->end() )  )
		{
			curr=*pos;
			next= *(++pos);
		}
		//cout<<curr<<endl;
		r.upper = curr;
		
		ranges.push_back(r);
	}  
}

void recTrieTraverse(struct trieNode* current,int level)
{
    int i;
	vector<Range> ranges;
	
	if (level==15)
	{   
		Binding *temp;
		bool producer_in_ML=false,consumer_in_ML=false;
		for (i=0; i<16; i++)
		{
		   temp=(Binding*)(current->list[i]);
		   if (temp) 
		   {
				string prodName,consName;
				int color;
				prodName = ADDtoName[temp->producer];
				consName = ADDtoName[temp->consumer];
			
				// If monitor list is specified, lets see we like the current functions' names or not!!
				// if we do not like the names skip to the next binding!
				if (Monitor_ON)
				{
					producer_in_ML = ( ML_OUTPUT.find(prodName) != ML_OUTPUT.end() );
					consumer_in_ML = ( ML_OUTPUT.find(consName) != ML_OUTPUT.end() );
					if( ! (producer_in_ML || consumer_in_ML) ) 
						break;
				}	
				
				if(IsNewFunc( temp->producer ) ) 
				{
					fprintf(gfp,"\"%08x\" [label=\"%s", (unsigned int)temp->producer, prodName.c_str());
					if(KnobBBFuncCount.Value()==TRUE) { 
						fprintf(gfp," count:%d", FunctionToCount[NameToFunction[prodName]]);
					}
					fprintf(gfp,"\"];\n");
				}

				if(IsNewFunc( temp->consumer ) ) 
				{
					fprintf(gfp,"\"%08x\" [label=\"%s", (unsigned int)temp->consumer, consName.c_str());
					if(KnobBBFuncCount.Value()==TRUE) { 
						fprintf(gfp," count:%d", FunctionToCount[NameToFunction[consName]]);
					}
					fprintf(gfp,"\"];\n");
				}

				color = (int) (  1023 *  log((double)(temp->data_exchange)) / log((double)MaxLabel)  ); 
				//fprintf(gfp,"\"%08x\" -> \"%08x\"  [label=\"%llu Bytes (%lu UnMAs %llu UnDVs)\" color=\"#%02x%02x%02x\"]\n",(unsigned int)temp->producer,(unsigned int)temp->consumer,temp->data_exchange,(unsigned long int)temp->UniqueMemCells->size(),temp->UniqueValues, max(0,color-768),min(255,512-abs(color-512)), max(0,min(255,512-color)));
				
				unsigned long int unma = temp->UniqueMemCells->size();
				float unmaPerCall = 0;
				if(KnobBBFuncCount.Value()==TRUE && 
				  FunctionToCount[NameToFunction[consName]]>0) 
				{
					unmaPerCall = ((float)unma/FunctionToCount[NameToFunction[consName]]);
				};
				
				fprintf(gfp,"\"%08x\" -> \"%08x\"  [label=",(unsigned int)temp->producer,(unsigned int)temp->consumer);
				if(KnobDotShowBytes.Value()==TRUE) 
				{
					fprintf(gfp,"\"%llu Bytes\\n",temp->data_exchange);
				}
				fprintf(gfp,"%lu UnMAs \\n",unma);
				if(KnobBBFuncCount.Value()==TRUE && 
				  FunctionToCount[NameToFunction[consName]]>0) 
				{
					fprintf(gfp,"%8.3f UnMAs/call\\n",unmaPerCall);
				}

				if(KnobDotShowUnDVs.Value()==TRUE) 
				{
					fprintf(gfp,"%llu UnDVs\\n",temp->UniqueValues);
				}
				
				
				//Put_Binding_in_XML_file(prodName,consName,temp->data_exchange,temp->UniqueMemCells->size());
// 				q2xml->insertChannel(new Channel(prodName,consName,temp->UniqueMemCells->size(),temp->data_exchange,temp->UniqueValues));
				set2ranges(temp->UniqueMemCells, ranges);
				q2xml->insertChannel(new Channel(prodName,consName,ranges,temp->UniqueMemCells->size(),temp->data_exchange,temp->UniqueValues));

				if(KnobDotShowRanges.Value()==TRUE) 
				{
					vector<Range>::iterator it = ranges.begin();
					int crt=0;
					while(it!=ranges.end()) 
					{
						fprintf(gfp,"(%8x-%8x)",(*it).lower,(*it).upper);
#ifdef QUAD_LIBELF
						map<string,GlobalSymbol*>::iterator its = globalSymbols.begin();
						while(its!=globalSymbols.end()) 
						{
							if(its->second->start<=it->lower &&
							  its->second->start+its->second->size>=it->upper) 
							{
								fprintf(gfp," from %s (%2.1f%%)",its->first.c_str(),
								  its->second->size!=0?((it->upper-it->lower+1)/(float)its->second->size)*100:100);
								break;
							}
							its++;
						}
#endif
						fprintf(gfp,"\\n");
						it++;
						crt++;
						if(KnobDotShowRangesLimit.Value()<crt+1) 
						{
							break;
						}
					}
					
					if(it!=ranges.end()) 
					{
						fprintf(gfp," and other...\\n");
					}
				}

				fprintf(gfp,"\" color=\"#%02x%02x%02x\"]\n", max(0,color-768),min(255,512-abs(color-512)), max(0,min(255,512-color)));

				// do we need the total statistics file always or not? ... should be modified if we need this in any case... 
				// do not forget to make also the relevant modifications in the monitor list input file processing ... 
				// this can also be moved up in the previous condition if we need output file only when monitor list is specified!			
				if (Monitor_ON)  
					Update_total_statistics(
						prodName,
						consName,
						temp->data_exchange,
						temp->UniqueMemCells->size(),
						producer_in_ML,
						consumer_in_ML);
		   } // end of this item in the last level of the trie has a binding we need to check!   
		} // end of for which goes thru all the items in the last level of the trie...
	 return;
	} // end of if ...(we reached the last level of the trie, time to check binding items!)
  
	for (i=0;i<16;i++)   // we have not reached the last level of trie, we are in the middle somewhere! DO spawn further searches deep in the trie, covering everywhere!
		if (current->list[i]) 
			recTrieTraverse(current->list[i],level+1);

  return;
}
//------------------------------------------------------------------------------------------
int CreateDSGraphFile()
{
   int i;

   if (!(gfp=fopen("QDUGraph.dot","wt")) ) return 1; /*can't create the output file */
   
   if(!(uflist=(struct trieNode*)malloc(sizeof(struct trieNode)) ) ) return 2; /* memory allocation failed*/
   else
        for (i=0;i<16;i++) 
			uflist->list[i]=NULL;

   cerr << "\nwriting QDU graph preamble..." << endl;

   /* write prologue */
   fprintf(gfp,"digraph {\ngraph [];\nnode [fontcolor=black, style=filled, fontsize=20];\nedge [fontsize=14, arrowhead=vee, arrowsize=0.5];\n");

   cerr << "writing QDU graph..." << endl; 
   if(graphRoot)
		recTrieTraverse(graphRoot,0);

   /* write epilogue */
   cerr << "writing QDU graph epilogue..." << endl; 
   fprintf(gfp,"}\n");
   
   cerr << "writing <QUAD> in the XML file ...\n";
   
	try
	{
		q2xml->save();
	}
	catch (ticpp::Exception& ex)
	{
		cerr << "Error occurred while saving XML file ... \n";
		cerr << ex.what();
	}
   
   delete q2xml;
   fclose(gfp);	
   return 0;
}

//------------------------------------------------------------------------------------------
int RecordCommunicationInDSGraph(ADDRINT producer, ADDRINT consumer, ADDRINT locAddr, struct trieNode * currentLPold)
{
	int currentLevel=0;
	Binding* tempptr;
	int i;
	struct trieNode* currentLP;
	unsigned int addressArray[16];

	struct AddressSplitter* ASP;
	
	ASP= (struct AddressSplitter *)&producer;
	addressArray[0]=ASP->h0;
	addressArray[1]=ASP->h1;
	addressArray[2]=ASP->h2;
	addressArray[3]=ASP->h3;
	addressArray[4]=ASP->h4;
	addressArray[5]=ASP->h5;
	addressArray[6]=ASP->h6;
	addressArray[7]=ASP->h7;

	ASP=(struct AddressSplitter *)&consumer;
	addressArray[8]=ASP->h0;
	addressArray[9]=ASP->h1;
	addressArray[10]=ASP->h2;
	addressArray[11]=ASP->h3;
	addressArray[12]=ASP->h4;
	addressArray[13]=ASP->h5;
	addressArray[14]=ASP->h6;
	addressArray[15]=ASP->h7;

	if(!graphRoot)  /* create the first level in graph trie */
	{
		if(!(graphRoot=(struct trieNode*)malloc(sizeof(struct trieNode)) ) ) 
			return 1; /* memory allocation failed*/
		else
			for (i=0;i<16;i++) 
				graphRoot->list[i]=NULL;
	}                         
			
	currentLP=graphRoot;                
	while(currentLevel<15)  /* proceed to the last level */
	{
		if(! (currentLP->list[addressArray[currentLevel]]) ) /* create new level on demand */
		{
				if(!(currentLP->list[addressArray[currentLevel]]=(struct trieNode*)malloc(sizeof(struct trieNode))) ) return 1; /* memory allocation failed*/
				else
				{
					for (i=0;i<16;i++) 
						(currentLP->list[addressArray[currentLevel]])->list[i]=NULL;
				}
		}
		currentLP=currentLP->list[addressArray[currentLevel]];
		currentLevel++;
	}            

	/* create new bucket to store number of accesses between the two functions*/
	if( currentLP->list[addressArray[currentLevel]] == NULL ) 
	{
		if(!(  currentLP->list[addressArray[currentLevel]] = (trieNode *)malloc(sizeof(Binding)) ) ) 
			return 1; /* memory allocation failed*/
		else 
		{
			tempptr=(Binding*) ( currentLP->list[addressArray[currentLevel]] );
			tempptr->data_exchange=0;  /* set number of times to zero */
			tempptr->UniqueValues=0;
			tempptr->producer=producer;
			tempptr->consumer=consumer;
			tempptr->UniqueMemCells=new set<ADDRINT>;
			if (!tempptr->UniqueMemCells) 
				return 1; /* memory allocation failed*/
		}	
	}

	tempptr=(Binding*) ( currentLP->list[addressArray[currentLevel]] );
	tempptr->data_exchange=tempptr->data_exchange+1;
	
	//make the status of this location as OLD by ClearFlag() for this consumer. 
	//A true will be returned if this value is fresh and now it will be set to old
	//A false will be returned if this value is already old (read) and is being re-read
 	if(currentLPold->RenewalFlags->ClearFlag(consumer))
 		tempptr->UniqueValues = tempptr->UniqueValues + 1;

	// only needed for graph visualization coloring!
	if (tempptr->data_exchange > MaxLabel) 
		MaxLabel=tempptr->data_exchange; 
	
	tempptr->UniqueMemCells->insert(locAddr);

	//********* what to do if insertion is not successful, memory problems !!!!!!!!!!!!
	return 0; /* successful recording */
}
//------------------------------------------------------------------------------------------
int RecordMemoryAccess(ADDRINT locAddr, ADDRINT func,bool writeFlag)
{
	int currentLevel=0;
	int i,retv;
	struct trieNode* currentLP; //current level pointer
	
	unsigned int addressArray[8];	
	struct AddressSplitter* ASP= (struct AddressSplitter *)&locAddr;
	addressArray[0]=ASP->h0;
	addressArray[1]=ASP->h1;
	addressArray[2]=ASP->h2;
	addressArray[3]=ASP->h3;
	addressArray[4]=ASP->h4;
	addressArray[5]=ASP->h5;
	addressArray[6]=ASP->h6;
	addressArray[7]=ASP->h7;

	if(!trieRoot)  /* create the first level in trie */
	{
		if(!(trieRoot=(struct trieNode*)malloc(sizeof(struct trieNode)) ) ) 
			return 1; /* memory allocation failed*/
		else
			for (i=0;i<16;i++) 
				trieRoot->list[i]=NULL;
	}
	currentLP=trieRoot;
	
	while(currentLevel<7)  /* proceed to the last level */
	{
		if(! (currentLP->list[addressArray[currentLevel]]) ) /* create new level on demand */
		{
			if(!(currentLP->list[addressArray[currentLevel]]=(struct trieNode*)malloc(sizeof(struct trieNode))) ) 
				return 1; /* memory allocation failed*/
			else
				for (i=0;i<16;i++) 
					(currentLP->list[addressArray[currentLevel]])->list[i]=NULL;
		}
		
		currentLP=currentLP->list[addressArray[currentLevel]];
		currentLevel++;
	}            

	if(!currentLP->list[addressArray[currentLevel]]) /* create new bucket to store last function's access to this memory location */
	{
		if(!(currentLP->list[addressArray[currentLevel]]=(struct trieNode*)malloc(sizeof(struct trieNode)) ) ) //ADDRINT
			return 1; /* memory allocation failed*/
		else 
		{
			*((ADDRINT*) (currentLP->list[addressArray[currentLevel]]) )=0; /* no write access has been recorded yet!!! */
 			currentLP->RenewalFlags = new FNodeList(); //RenewalFlags for Unique value computations
		}
	}           
	if (writeFlag)
	{
		*((ADDRINT*) (currentLP->list[addressArray[currentLevel]])  )=func;  /* only record the last function's write to a memory location?!! */
		
		//As this lovation is just written so ReNew the flags for this lovation for all the existing consumers of this location
 		currentLP->RenewalFlags->SetFlags();
	}
	else 
	{
		/* producer , consumer , address used for making this binding! , location in the tree */
		retv=RecordCommunicationInDSGraph(*((ADDRINT*) (currentLP->list[addressArray[currentLevel]]) ), func, locAddr, currentLP); 
		//DS = Data Structure Graph
		if (retv) return 1; /* memory exhausted */
	}
	return 0; /* successful trace */
}
