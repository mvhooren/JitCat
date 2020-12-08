/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/CatLog.h"

#include <iostream>


using namespace jitcat::Tools;


void CatLog::log(const char* message)
{
	for (CatLogListener* listener : listeners)
	{
		listener->catLog(message);
	}
}


void CatLog::addListener(CatLogListener* listener)
{
	for (CatLogListener* listenerIter : listeners)
	{
		if (listenerIter == listener)
		{
			return;
		}
	}
	listeners.push_back(listener);
}


void CatLog::removeListener(CatLogListener* listener)
{
	for (int i = 0; i < (int)listeners.size(); i++)
	{
		if (listeners[(unsigned int)i] == listener)
		{
			listeners.erase(listeners.begin() + i);
			i--;
		}
	}
}


std::vector<CatLogListener*> CatLog::listeners = std::vector<CatLogListener*>();


void CatLogStdOut::catLog(const char* message)
{
	std::cout << message;
}


CatLogFile::CatLogFile(const std::string& filename):
	logFile(filename)
{
}


CatLogFile::~CatLogFile()
{
}


void CatLogFile::catLog(const char* message)
{
	logFile << message;
	logFile.flush();
}
