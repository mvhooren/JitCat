/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include <string>

namespace jitcat
{
	
	class CatError
	{
	public:
		CatError();
		CatError(const CatError& other);
		CatError(const std::string& message);
		CatError(const char* message);

		CatError& operator=(const CatError& other);
		bool operator==(const CatError& other) const;

		const std::string& getMessage() const;

	private:
		std::string message;

	public:
		static CatError defaultError;
	};

} //End namespace jitcat