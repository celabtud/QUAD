/*
 * Channel.h
 *
 * Author : Imran Ashraf
 *
 * This file contains the Channel class. This class models the communication
 * channel between a producer and consumer.
 * 
 */

#ifndef CHANNEL_H_
#define CHANNEL_H_

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "Exception.h"

#include <pin.H>

typedef struct
{
	ADDRINT lower;
	ADDRINT upper;
}Range;

using namespace std;

typedef unsigned long long ULL;
class Channel
{
private:
		string producer;
		string consumer;
		ULL UnMA;
		ULL Bytes;
		ULL Values;
		vector<Range> Ranges;
		
public:
	Channel(){;}
	
	Channel(string p, string c, vector<Range>& ranges, ULL unma, ULL bytes, ULL vals);
	
	void setProducer(string prod){producer = prod;}
	string getProducer(){return producer;}
	void setConsumer(string cons){consumer = cons;}
	string getConsumer(){return consumer;}
	void setUnMA(ULL unma) {UnMA = unma;}
	ULL getUnMA() {return UnMA;}
	void setBytes(ULL bytes) { Bytes = bytes;}
	ULL getBytes(){return Bytes;}
	void setValues(ULL values){Values = values;}
	unsigned long int getValues() {return Values;}
	
	void setRanges(vector<Range>& ranges)
	{
		Range r;
		vector<Range>::const_iterator It;  
		for (It = ranges.begin(); It != ranges.end(); ++It) 
		{
			r.lower = It->lower;
			r.upper = It->upper;
			Ranges.push_back(r);
		}
	}
	
	void getRanges(vector<Range>& ranges)
	{
		Range r;
		vector<Range>::const_iterator It;  
		for (It = Ranges.begin(); It != Ranges.end(); ++It) 
		{
			r.lower = It->lower;
			r.upper = It->upper;
			ranges.push_back(r);
		}
	}
	
	void setChannel(string p, string c, vector<Range> & ranges, ULL unma, ULL bytes, ULL vals);	
	void printChannel();
	~Channel(){;}
};

#endif 
