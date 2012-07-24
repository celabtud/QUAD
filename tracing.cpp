// final revision Jan 19th, 2011

#include "tracing.h"

#include "Channel.h"
#include "Exception.h"
#include "Q2XMLFile.h"

#define max(a,b) ((a)>(b)?(a):(b))
#define min(a,b) ((a)<(b)?(a):(b))

typedef ADDRINT addr_t;
FILE* gfp,*ufa;
bool First_Rec_in_XML = true;
//TiXmlNode* Put_QUAD_here=NULL;

addr_t MaxLabel=0;

struct trieNode {
    struct trieNode * list[16];
} *trieRoot=NULL,*graphRoot=NULL,*uflist=NULL;

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
	unsigned long int data_exchange;
	ADDRINT producer;
	ADDRINT consumer;
	set<ADDRINT>* UniqueMemCells;
} Binding;

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

    cerr << "\nCreating summary report file (ML_OV_Summary.txt) containing information about the functions specified in the monitor list..." << endl;

    out<<setw(30)<<setiosflags(ios::left)<<"Function"<<setw(12)<<"   IN_ML"
    <<setw(12)<<" IN_ML_UMA"
    <<setw(12)<<"  OUT_ML"
    <<setw(12)<<"OUT_ML_UMA"
    <<setw(12)<<"  IN_ALL"
    <<setw(12)<<"IN_ALL_UMA"
    <<setw(12)<<"  OUT_ALL"
    <<setw(12)<<"OUT_ALL_UMA"
    <<endl;

    out<<setw(30)<<"-----------------------------"<<setw(12)<<"-----------"
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
     
     out << setw(11) << setiosflags(ios::right) << pIter->second->total_IN_ML << " " <<
	    setw(11) << pIter->second->total_IN_ML_UMA << " " <<
	    setw(11) << pIter->second->total_OUT_ML << " " << 
	    setw(11) << pIter->second->total_OUT_ML_UMA << " " <<
	    setw(11) << pIter->second->total_IN_ALL << " " <<
	    setw(11) << pIter->second->total_IN_ALL_UMA << " " <<
	    setw(11) << pIter->second->total_OUT_ALL << " " <<
	    setw(11) << pIter->second->total_OUT_ALL_UMA << endl ;
	    
     
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
    out << "IN_ML_UMA -> Total number of unique memory addresses used corresponding to 'IN_ML'" << endl;
    out << "OUT_ML -> Total number of bytes read by a function in the monitor list that this function is responsible for producing the value(s) of the byte(s)" << endl;
    out << "OUT_ML_UMA -> Total number of unique memory addresses used corresponding to 'OUT_ML'" << endl;
    
    out << "IN_ALL -> Total number of bytes read by this function that a function in the application is responsible for producing the value(s) of the byte(s)" << endl;
    out << "IN_ALL_UMA -> Total number of unique memory addresses used corresponding to 'IN_ALL'" << endl;
    out << "OUT_ALL -> Total number of bytes read by a function in the application that this function is responsible for producing the value(s) of the byte(s)" << endl;
    out << "OUT_ALL_UMA -> Total number of unique memory addresses used corresponding to 'OUT_ALL'" << endl;
    
    
    out.close();
    return 0;
}	


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


void recTrieTraverse(struct trieNode* current,int level)
{
    int i;
	if (level==15)
	{   
		Binding *temp;
		bool producer_in_ML=false,consumer_in_ML=false;
		for (i=0; i<16; i++)
		{
		   temp=(Binding*)(current->list[i]);
		   if (temp) 
		   {
                	string name2,name3;
                	int color;
                	name2 = ADDtoName[temp->producer];
			name3 = ADDtoName[temp->consumer];
			
			// If monitor list is specified, lets see we like the current functions' names or not!!
			// if we do not like the names skip to the next binding!
			
			if (Monitor_ON)
			{
				producer_in_ML = ( ML_OUTPUT.find(name2) != ML_OUTPUT.end() );
				consumer_in_ML = ( ML_OUTPUT.find(name3) != ML_OUTPUT.end() );
				
				if( ! (producer_in_ML || consumer_in_ML) ) break;
				
			}	
			if(IsNewFunc( temp->producer ) )
                	{
				fprintf(gfp,"\"%08x\" [label=\"%s\"];\n", (unsigned int)temp->producer , name2.c_str());
                	}
			if(IsNewFunc( temp->consumer ) )
                	{
				fprintf(gfp,"\"%08x\" [label=\"%s\"];\n", (unsigned int)temp->consumer , name3.c_str());
               		}
			color = (int) (  1023 *  log((double)(temp->data_exchange)) / log((double)MaxLabel)  ); 
			fprintf(gfp,"\"%08x\" -> \"%08x\"  [label=\"%lu bytes (%lu UMA)\" color=\"#%02x%02x%02x\"]\n",(unsigned int)temp->producer,(unsigned int)temp->consumer,temp->data_exchange,(unsigned long int)temp->UniqueMemCells->size(), max(0,color-768),min(255,512-abs(color-512)), max(0,min(255,512-color)));
			
			//Put_Binding_in_XML_file(name2,name3,temp->data_exchange,temp->UniqueMemCells->size());
			Channel * ch = new Channel(name2,name3,temp->UniqueMemCells->size(),temp->data_exchange,0);
			q2xml->insertChannel(ch);
			
			if (Monitor_ON)  // do we need the total statistics file always or not? ... should be modified if we need this in any case... do not forget to make also the relevant modifications in the monitor list input file processing ... this can also be moved up in the previous condition if we need output file only when monitor list is specified!
			
			Update_total_statistics(
						name2,name3,temp->data_exchange,
					        temp->UniqueMemCells->size(),
					        producer_in_ML,consumer_in_ML
					       );
		   
		   } // end of this item in the last level of the trie has a binding we need to check!   
		} // end of for which goes thru all the items in the last level of the trie...
	 return;
	} // end of if ...(we reached the last level of the trie, time to check binding items!)
  
	for (i=0;i<16;i++)   // we have not reached the last level of trie, we are in the middle somewhere! DO spawn further searches deep in the trie, covering everywhere!
    	  if (current->list[i]) 
	    recTrieTraverse(current->list[i],level+1);

  return;
}
		  

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
   recTrieTraverse(graphRoot,0);

   /* write epilogue */

   cerr << "writing QDU graph epilogue..." << endl; 
   fprintf(gfp,"}\n");
   
   //printf("max: %u bytes\n",MaxLabel);
   
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


int RecordCommunicationInDSGraph(ADDRINT producer, ADDRINT consumer, ADDRINT addy)
{
    int currentLevel=0;
    Binding* tempptr;
    int i;
    struct trieNode* currentLP;
    struct AddressSplitter* ASP= (struct AddressSplitter *)&producer;
    
    unsigned int addressArray[16];
    
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
                       for (i=0;i<16;i++) 
                              (currentLP->list[addressArray[currentLevel]])->list[i]=NULL;
        }
        
        currentLP=currentLP->list[addressArray[currentLevel]];
        currentLevel++;
    }            
    
    
    if( currentLP->list[addressArray[currentLevel]] == NULL ) /* create new bucket to store number of accesses between the two functions*/
    {
        if(!(  currentLP->list[addressArray[currentLevel]] = (trieNode *)malloc(sizeof(Binding)) ) ) return 1; /* memory allocation failed*/
        else 
        {
            tempptr=(Binding*) ( currentLP->list[addressArray[currentLevel]] );

			tempptr->data_exchange=0;  /* set number of times to zero */
			tempptr->producer=producer;
			tempptr->consumer=consumer;
			tempptr->UniqueMemCells=new set<ADDRINT>;
			if (!tempptr->UniqueMemCells) return 1; /* memory allocation failed*/
    	}	
    }
	
	tempptr=(Binding*) ( currentLP->list[addressArray[currentLevel]] );
	tempptr->data_exchange=tempptr->data_exchange+1;
	if (tempptr->data_exchange > MaxLabel) MaxLabel=tempptr->data_exchange; // only needed for graph visualization coloring!
	tempptr->UniqueMemCells->insert(addy);

	//****************************** what to do if insertion is not successful, memory problems!!!!!!!!!!!!
	
    return 0; /* successful recording */
}


int RecordMemoryAccess(ADDRINT addy, ADDRINT func,bool writeFlag)
{
    int currentLevel=0;
    int i,retv;
    struct trieNode* currentLP;
    struct AddressSplitter* ASP= (struct AddressSplitter *)&addy;
    
    unsigned int addressArray[8];
    
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
            if(!(trieRoot=(struct trieNode*)malloc(sizeof(struct trieNode)) ) ) return 1; /* memory allocation failed*/
            else
                       for (i=0;i<16;i++) 
                              trieRoot->list[i]=NULL;
    }
            
    currentLP=trieRoot;                
    while(currentLevel<7)  /* proceed to the last level */
    {
        if(! (currentLP->list[addressArray[currentLevel]]) ) /* create new level on demand */
        {
                if(!(currentLP->list[addressArray[currentLevel]]=(struct trieNode*)malloc(sizeof(struct trieNode))) ) return 1; /* memory allocation failed*/
                else
                       for (i=0;i<16;i++) 
                              (currentLP->list[addressArray[currentLevel]])->list[i]=NULL;
        }
        
        currentLP=currentLP->list[addressArray[currentLevel]];
        currentLevel++;
    }            
    
    if(!currentLP->list[addressArray[currentLevel]]) /* create new bucket to store last function's access to this memory location */
    {
        if(!(currentLP->list[addressArray[currentLevel]]=(struct trieNode*)malloc(sizeof(ADDRINT)) ) ) return 1; /* memory allocation failed*/
        else *((ADDRINT*) (currentLP->list[addressArray[currentLevel]]) )=0; /* no write access has been recorded yet!!! */
    }           
    if (writeFlag)
            *((ADDRINT*) (currentLP->list[addressArray[currentLevel]])  )=func;  /* only record the last function's write to a memory location?!! */
        
    else 
    	{

			retv=RecordCommunicationInDSGraph(*((ADDRINT*) (currentLP->list[addressArray[currentLevel]]) ),func,addy); /* producer -> consumer , and the address used for making this binding! */
    		if (retv) return 1; /* memory exhausted */
    	}

    return 0; /* successful trace */
}
