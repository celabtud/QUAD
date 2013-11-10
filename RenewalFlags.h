/*
 * File : RenewalFlags.h
 *
 * Author : Imran Ashraf
 *
 * This file contains the RenewalFlags class. This class contains dynamic structure to
 * associate flags with each memory location to track the unique data values
 * communicated between a producer and consumer in a communication.
 *
 */

#ifndef _RENWALFLAGS_H_
#define _RENWALFLAGS_H_

#include "pin.H"

//a location can be fresh or old
enum MemFlagStatus {OLD, FRESH};

using namespace std;

/*
This class is for the node of the flag list. Each node has a record of a Consumer and its status
as MemFlagStatus (OLD,FRESH) for this consumer.
*/
class FNode
{
private:
    MemFlagStatus Flag;
    ADDRINT Consumer;

public:
    FNode()
    {
        ;
    }
    FNode(ADDRINT cons, MemFlagStatus flag)
    {
        Consumer= cons;
        Flag  = flag;
    }
    void setFlag(MemFlagStatus f)
    {
        Flag = f;
    }
    MemFlagStatus getFlag()
    {
        return Flag;
    }
    void setConsumer(ADDRINT cons)
    {
        Consumer = cons;
    }
    ADDRINT getConsumer()
    {
        return Consumer;
    }
};

/*
FNodeList is the list of the nodes (FNode).
Array is a dynamic array of nodes contaning consumer and status information.
Size is the current number of consumers for this location.
Capacity is the maximum number of consumers for this location. When Size reaches
Capacity, it is increased to accomodate more nodes.
*/
class FNodeList
{
private:
    FNode * Array;
    int Size;
    int Capacity;

public:

    FNodeList()
    {
        Capacity = 10;  //This is the initial no of assumed consumers of a location
        Size = 0;
        Array = new FNode[Capacity];
    }

    void SetFlags(void); //ReNew all the flags for this location
    bool ClearFlag(ADDRINT cons); //Make the flags old for a certain consumer of this location
    void PrintFlags(void); //print all flags (debugging)
};
#endif
