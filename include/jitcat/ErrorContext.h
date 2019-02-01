/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once


#include <string>

namespace jitcat
{
	class CatRuntimeContext;


	class ErrorContext
	{
	public:
		ErrorContext(CatRuntimeContext* context, const std::string& contextDescription);
		~ErrorContext();

		const std::string& getContextDescription() const;

	private:
		CatRuntimeContext* context;
		std::string contextDescription;
	};

} //End namespace jitcat