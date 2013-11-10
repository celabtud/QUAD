/*
 * Exception.h
 *
 *  Created on: 25 nov. 2011
 *      Author: rmeeuws
 */

#ifndef EXCEPTION_H_
#define EXCEPTION_H_

#include <string>

using namespace std;

class Exception
{
private:
    string m_reason;
    string m_file;
    int m_line;
public:
    Exception(string reason);
    Exception(string reason, string file, int line);

    virtual
    ~Exception();

    const string &Reason()
    {
        return m_reason;
    }
    const string &File()
    {
        return m_file;
    }
    const int &Line()
    {
        return m_line;
    }

};

#endif /* EXCEPTION_H_ */
