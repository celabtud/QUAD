/*
* Q2XMLFile.cpp
*
* Author : Imran Ashraf
*
* This file contains the member functions of Q2XMLFile class. These functions
* are mainly responsible for putting the information in the q2 xml file.
* 
*/
#include "Q2XMLFile.h"
#include "Channel.h"
#include "ticpp.h"

Q2XMLFile::Q2XMLFile(const string& filename, const string& ns, const string &appname)
	:m_filename(filename) , m_namespace(ns),m_appfinger(ns + "application"),m_applicname(appname)
{
	try
	{
		m_file.LoadFile(filename);
	}
	catch (ticpp::Exception& ex)
	{
		ticpp::Declaration * decl = new ticpp::Declaration( "1.0", "UTF-8", "" );
		m_file.LinkEndChild( decl );
		
		ticpp::Element * element = new ticpp::Element(ns + "profiles" );
		element->SetAttribute("xmlns:q2","http://www.example.org/q2profiling");
		element->SetAttribute("xmlns:xsi","http://www.w3.org/2001/XMLSchema-instance");
		element->SetAttribute("xsi:schemaLocation","http://www.example.org/q2profiling q2profiling.xsd");
		m_file.LinkEndChild( element );
	}
	
	reset();
}

Q2XMLFile::~Q2XMLFile()
{
	// TODO Auto-generated destructor stub
}


void Q2XMLFile::save()
{
	m_file.SaveFile(m_filename);
}

void Q2XMLFile::reset()
{
	// find first set of metrics
	// = first app -> functions -> first functionp
	
	ticpp::Element *profTag, *appTag, *qduTag;
	
	try
	{
		profTag = m_file.FirstChildElement(m_namespace + "profiles");
	}
	catch (ticpp::Exception& ex)
	{
		profTag=new ticpp::Element(m_namespace + "profiles");
		profTag->SetAttribute("xmlns:q2","http://www.example.org/q2profiling");
		profTag->SetAttribute("xmlns:xsi","http://www.w3.org/2001/XMLSchema-instance");
		profTag->SetAttribute("xsi:schemaLocation","http://www.example.org/q2profiling q2profiling.xsd");
		m_file.LinkEndChild(profTag);  
	}

	// get application with name <m_applicname>
	ticpp::Iterator< ticpp::Element > appItr(m_namespace + "application");
	for(appItr = appItr.begin(profTag); appItr != appItr.end(); appItr++)
	{
		if(appItr->GetAttribute("name").compare(m_applicname) == 0)
		{
			break;
		}
	}
	
	if(appItr == appItr.end())
	{
		// add new tag
		appTag = new ticpp::Element(m_namespace + "application");
		appTag->SetAttribute("name",m_applicname);
		profTag->LinkEndChild(appTag);
	} 
	else 
		appTag = appItr.Get();
	
	m_appfinger = appTag;
	
	//get QDU tag
	try
	{
		qduTag = appTag->FirstChildElement(m_namespace + "QDUGraph");
	}
	catch (ticpp::Exception& ex)
	{
		qduTag = new ticpp::Element(m_namespace + "QDUGraph");
		appTag->LinkEndChild(qduTag);  
	}
	
	m_qdufinger = qduTag;
}


//return the channel with given prod and cons 
Channel * Q2XMLFile::getChannel(string prod, string cons) const
{
	unsigned long long unma, bytes, values;
	
	ticpp::Iterator< ticpp::Element > channelItr(m_namespace + "channel");
	for(channelItr = channelItr.begin(m_qdufinger); channelItr != channelItr.end(); channelItr++)
	{
		if	(
			channelItr->GetAttribute("producer").compare(prod) == 0 && 
			channelItr->GetAttribute("consumer").compare(cons) == 0 
			)
		{
			break;
		}
	}
	
	if(channelItr == channelItr.end() )
	{
		cout<<"\n Channel not found !"<<endl;
		return NULL;
	}
	else
	{
		channelItr->FirstChildElement(m_namespace + "hUnMA")->GetText(&unma);
		channelItr->FirstChildElement(m_namespace + "hBytes")->GetText(&bytes);
		channelItr->FirstChildElement(m_namespace + "hValues")->GetText(&values);

		return new Channel(prod,cons,unma,bytes,values);
	}
}


void Q2XMLFile::printAllChValues() const
{
	unsigned long long unma, bytes, values;
	
	ticpp::Iterator< ticpp::Element > channelItr(m_namespace + "channel");
	for(channelItr = channelItr.begin(m_qdufinger); channelItr != channelItr.end(); channelItr++)
	{
		string prod = channelItr->GetAttribute("producer");
		string cons = channelItr->GetAttribute("consumer");
		channelItr->FirstChildElement(m_namespace + "hUnMA")->GetText(&unma);
		channelItr->FirstChildElement(m_namespace + "hBytes")->GetText(&bytes);
		channelItr->FirstChildElement(m_namespace + "hValues")->GetText(&values);
		
		Channel *ch = new Channel(prod,cons,unma,bytes,values);
		ch->printChannel();
	}
}

void Q2XMLFile::insertChannel(Channel * ch)
{
	ticpp::Element *chTag;
	ticpp::Element *unmaTag, *bytesTag, * valuesTag;
	string m_applicname("canny");

	// get channel
	ticpp::Iterator< ticpp::Element > channelItr(m_namespace + "channel");
	for(channelItr = channelItr.begin(m_qdufinger); channelItr != channelItr.end(); channelItr++)
		if	(	channelItr->GetAttribute("producer").compare(ch->getProducer()) == 0 && 
				channelItr->GetAttribute("consumer").compare(ch->getConsumer()) == 0
			)
			break;
	
	if(channelItr == channelItr.end())
	{
		// add new tag
		chTag = new ticpp::Element(m_namespace + "channel");
		chTag->SetAttribute("producer",ch->getProducer());
		chTag->SetAttribute("consumer",ch->getConsumer());
		m_qdufinger->LinkEndChild(chTag);            
	} 
	else 
		chTag = channelItr.Get();

	try
	{
		unmaTag = chTag->FirstChildElement(m_namespace + "hUnMA");
		unmaTag->SetText(ch->getUnMA());
	}
	catch( ticpp::Exception& ex )
	{
		unmaTag = new ticpp::Element(m_namespace + "hUnMA");
		unmaTag->SetText( ch->getUnMA() );
		chTag->LinkEndChild(unmaTag);
	}
	

	try
	{
		bytesTag = chTag->FirstChildElement(m_namespace + "hBytes");
		bytesTag->SetText(ch->getBytes());
	}
	catch( ticpp::Exception& ex )
	{
		bytesTag = new ticpp::Element(m_namespace + "hBytes");
		bytesTag->SetText( ch->getBytes() );
		chTag->LinkEndChild(bytesTag);
	}

	try
	{
		valuesTag = chTag->FirstChildElement(m_namespace + "hValues");
		valuesTag->SetText(ch->getValues());
	}
	catch( ticpp::Exception& ex )
	{
		valuesTag = new ticpp::Element(m_namespace + "hValues");
		valuesTag->SetText( ch->getValues() );
		chTag->LinkEndChild(valuesTag);
	}
}


