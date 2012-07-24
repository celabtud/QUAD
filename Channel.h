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
#include "Exception.h"

using namespace std;

class Channel
{
private:
		string producer;
		string consumer;
		unsigned long long UnMA;
		unsigned long long Bytes;
		unsigned long long Values;
		
public:
	Channel(){;}
	Channel(string p, string c, unsigned long long unma, unsigned long long bytes, unsigned long long vals);
	void setProducer(string prod){producer = prod;}
	string getProducer(){return producer;}
	void setConsumer(string cons){consumer = cons;}
	string getConsumer(){return consumer;}
	void setUnMA(unsigned long long unma) {UnMA = unma;}
	unsigned long long getUnMA() {return UnMA;}
	void setBytes(unsigned long long bytes) { Bytes = bytes;}
	unsigned long long getBytes(){return Bytes;}
	void setValues(unsigned long long values){Values = values;}
	unsigned long int getValues() {return Values;}
	void setChannel(string p, string c, unsigned long long unma, unsigned long long bytes, unsigned long long vals);	
	void printChannel();
	~Channel(){;}
};

#endif 
