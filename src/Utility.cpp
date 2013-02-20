/*
 * File : Utility.cpp
 *
 * Author : Imran Ashraf
 *
 * This file contains some utility functions.
 * 
 */

#include<sstream>
#include<cstring>

#include"Utility.h"

unsigned long int str2no(std::string Text)
{
	unsigned long int Result;          //number which will contain the result
	std::istringstream convert(Text); // stringstream used for the conversion constructed with the contents of 'Text' 
	// ie: the stream will start containing the characters of 'Text'
	
	if ( !(convert >> Result) ) //give the value to 'Result' using the characters in the stream
		Result = 0;             //if that fails set 'Result' to 0
		
		return Result;
}

std::string no2str(unsigned long no)
{
	std::stringstream ss;//create a stringstream
	ss << no;//add number to the stream
	return ss.str();//return a string with the contents of the stream
}

