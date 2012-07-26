/*
 * File : RenewalFlags.cpp
 *
 * Author : Imran Ashraf
 *
 * This file contains the member functions of RenewalFlags class. These functions 
 * associate flags with each memory location to track the unique data values
 * communicated between a producer and consumer in a communication.
 * 
 */

#include<iostream>
#include"RenewalFlags.h"

/*
This method will set all the flags of this location to FRESH
*/
void FNodeList::SetFlags(void)
{
	for(int i=0; i<Size;i++)
		Array[i].setFlag(FRESH);
}

/*
This method makes the flag status to old for the supplied consumer of this location. 
case 1: If the consumer is already available and the status is OLD, nothing will be done.
case 2: If the consumer is already available and the status is FRESH, it will be set to OLD and true will be returned.
case 3: If this consumer is not already available (not known) then it will be added to the array with OLD status.
case 4: When adding new consumers, if there is no capacity, then it is created and this new consumer is added with OLD Status.
*/
bool FNodeList::ClearFlag(ADDRINT cons)
{
	int i;
	
	for(i=0; i<Size; i++)
	{
		if(Array[i].getConsumer() == cons )
		{
			if(Array[i].getFlag() == OLD ) //case 1
				return false;
			else
				Array[i].setFlag(OLD); //case 2
			return true;
		}
	}
		
	if (Size < Capacity ) //case 3
	{
		Array[i].setConsumer(cons);
		Array[i].setFlag(OLD);
		Size++;
	}
	else //case 4
	{
		Capacity = Capacity + 5;
		FNode * tempArray = new FNode[Capacity];
		for(i=0; i<Size; i++)
			tempArray[i] = Array[i];
		
		Array = tempArray;
		Array[i].setConsumer(cons);
		Array[i].setFlag(OLD);
		Size++;
		
		delete [] tempArray;
	}
	return true;
}

/*
Simple printing of flags for testing.
*/
void FNodeList::PrintFlags(void)
{
	if(Size == 0)
	{
		cerr<<"No Flags "<<endl;
		return;
	}
	for(int i=0; i<Size;i++)
	{
		cout<<Array[i].getConsumer()<<" <=> ";
		if(Array[i].getFlag() == FRESH) 
			cerr<<"FRESH    ";
		else
			cerr<<"OLD    ";
	}
	cout<<endl;
}
