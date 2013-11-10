/*
 * File : BBlock.cpp
 *
 * Author : Imran Ashraf
 *
 * This file contains the member functions of BBlock class. These operate on 
 * basic blcoks provided in the text file as input and record the communication
 * between a producer and consumer at the basic block level.
 * 
 */

#include"BBlock.h"

using namespace std;

void BBlock::print()
{
	cout<<"\t Basic Block Info"<<endl;
	cout<<" bbFileName = "<<bbFileName<<endl;
	cout<<" bbFtnName = "<<bbFtnName<<endl;
	cout<<" bbStartLine = "<<bbStartLine<<endl;
	cout<<" bbEndLine = "<<bbEndLine<<endl;
}

bool BBlock::probeBB(string file, string ftn, int line, string & ret)
{
	if(ftn.compare(bbFtnName) == 0)
	{
		if( (line >= bbStartLine) && (line <= bbEndLine) )
		{
			ostringstream ss;
			ss << ftn;
			ss << "[L";
			ss << (bbStartLine);
			ss << " -> ";
			ss << "L";
			ss << (bbEndLine);
			ss << "]";
			
			ret = ss.str();
			return true;
		}
	}
	return false;
}

void BBList::insert(BBlock bb)
{
	bbList.push_back(bb);
}

void BBList::insert(string file, string ftn, int st, int end)
{
	BBlock bb(file,ftn,st,end);
	bbList.push_back(bb);
}

void BBList::print()
{
	vector<BBlock>::iterator i;
	for (i = bbList.begin(); i != bbList.end(); ++i )
	{
		i->print();
	}
}

int BBList::initFromFile(string fname)
{
	string line;
	string file, ftn;
	int st, end;
	ifstream myfile;
    int retVal=0;
	myfile.open(fname.c_str());
	
	if (myfile.is_open())
	{
		while ( myfile.good() )
		{
			myfile>>file>>ftn>>st>>end;
			if( !(file.empty())  && !(ftn.empty()) ) 
				insert(file,ftn,st,end);
			
			file="";
			ftn="";
		}
		retVal=1;
	}
	else 
    {
        cout << "Unable to open file"; 
        retVal=0;
    }
	
	myfile.close();
    return retVal;
}

string BBList::probeBB(string file, string ftn, int line)
{
	string temp;
	vector<BBlock>::iterator i;
	for (i = bbList.begin(); i != bbList.end(); ++i )
	{
		if( i->probeBB(file, ftn, line , temp) == true)
			return temp;
	}
	return ftn;
}
