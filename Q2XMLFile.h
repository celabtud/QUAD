/*
* Q2XMLFile.h
*
* Author : Imran Ashraf
* 
* This file contains the Q2XMLFile class. This class models the xml file
* related functionality responsible for putting the information in the q2 xml file.
* 
*/

#ifndef Q2XMLFILE_H_
#define Q2XMLFILE_H_

#include "Channel.h"
#include "ticpp.h"
class Q2XMLFile
{
	private:
		string m_filename;
		string m_namespace;
		string m_applicname;
		ticpp::Document m_file;
		ticpp::Iterator< ticpp::Element > m_appfinger;
		ticpp::Element * m_qdufinger;
		
	public:
		Q2XMLFile(const string&, const string&, const string&);
		~Q2XMLFile();
		void save();
		void reset();
		Channel * getChannel(string prod, string cons) const;
		void printAllChValues() const;
		void insertChannel(Channel * ch);
};

#endif /* Q2XMLFILE_H_ */


