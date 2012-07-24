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
		unsigned long UnMA;
		unsigned long Bytes;
		unsigned long Values;
		
public:
	Channel(){;}
	Channel(string p, string c, unsigned long unma, unsigned long bytes, unsigned long vals);
	void setProducer(string prod){producer = prod;}
	string getProducer(){return producer;}
	void setConsumer(string cons){consumer = cons;}
	string getConsumer(){return consumer;}
	void setUnMA(unsigned long int unma) {UnMA = unma;}
	unsigned long int getUnMA() {return UnMA;}
	void setBytes(unsigned long int bytes) { Bytes = bytes;}
	unsigned long int getBytes(){return Bytes;}
	void setValues(unsigned long int values){Values = values;}
	unsigned long int getValues() {return Values;}
	void setChannel(string p, string c, unsigned long unma, unsigned long bytes, unsigned long vals);	
	void printChannel();
	~Channel(){;}
};

#endif 
