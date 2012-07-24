/*
 * BBlock.h 
 *
 * Author : Imran Ashraf
 *
 * This file contains the BBlock class. This class models the Basic Blocks
 * to record the communication at the line level between a producer and consumer.
 * 
 */

#ifndef BBLOCK_H
#define BBLOCK_H

#include <string>
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>

using namespace std;

class BBlock
{
	private:
		std::string bbFileName;
		std::string bbFtnName;
		int bbStartLine;
		int bbEndLine;
		
	public:
		BBlock(){};
		BBlock(std::string fname, std::string ftnname, int st, int end)
		{
			bbFileName  = fname;
			bbFtnName   = ftnname;
			bbStartLine = st;
			bbEndLine   = end;
		}
		void print();
		bool probeBB(string file, string ftn, int line, string & ret);
};

class BBList
{
	private:
		vector<BBlock> bbList;
		
	public:
		BBList(){}
		void insert(BBlock bb);
		void insert(string file, string ftn, int st, int end);
		void print();
		int initFromFile(string fname);
		string probeBB(string file, string ftn, int line);
		int size()	{	return bbList.size(); 	}
};

#endif
