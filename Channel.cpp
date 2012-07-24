/*
 * File : Channel.cpp
 *
 * Author : Imran Ashraf
 *
 * This file contains the member functions of Channel class. The operate on 
 * the communication channel between a producer and consumer.
 * 
 */

#include <fstream>
#include <string>
#include "Exception.h"
#include "ticpp.h"
#include "Channel.h"

using namespace std;

Channel::Channel(string p, string c, unsigned long unma, unsigned long bytes, unsigned long vals)
{
	producer = p;
	consumer = c;
	UnMA = unma;
	Bytes = bytes;
	Values = vals;
}

void Channel::setChannel(string p, string c, unsigned long unma, unsigned long bytes, unsigned long vals)
{
	setProducer(p);
	setConsumer(c);
	setUnMA(unma);
	setBytes(bytes);
	setValues(vals);
}

void Channel::printChannel()
{
	if(this != NULL)
	{
		cout<<getProducer()<<endl;
		cout<<getConsumer()<<endl;
		cout<<getUnMA()<<endl;
		cout<<getBytes()<<endl;
		cout<<getValues()<<endl;
	}
	else
	{
		cout<<"Cannot Print Channel (NULL) ..."<<endl;
	}
}
