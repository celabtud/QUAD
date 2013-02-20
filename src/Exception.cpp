/*
 * Exception.cpp
 *
 *  Created on: 25 nov. 2011
 *      Author: rmeeuws
 */

#include "Exception.h"

Exception::Exception(string reason)
	:m_reason(reason),m_file(""),m_line(-1)
{
}

Exception::Exception(string reason, string file, int line)
	:m_reason(reason),m_file(file),m_line(line)
{
}

Exception::~Exception()
{
// TODO Auto-generated destructor stub
}


