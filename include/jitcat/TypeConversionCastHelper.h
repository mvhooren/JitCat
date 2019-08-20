/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

namespace jitcat
{
	class TypeConversionCast
	{
	public:
		template<typename OutCVT, typename InT>
		static inline OutCVT convertCast(InT&& in)
		{
			constexpr bool outIsConst = std::is_const<OutCVT>::value;
			typedef typename std::remove_cv<OutCVT>::type OutT;
			if constexpr (std::is_same<InT, OutT>::value)
			{
				return in;
			}
			else if constexpr (std::is_reference<OutT>::value)
			{
				//Out&
				if constexpr (std::is_pointer<InT>::value)
				{
					return *in;
				}
				else
				{
					return in;
				}
			}
			else if constexpr (std::is_pointer<OutT>::value)
			{
				//Out*
				return &in;
			}
			else
			{
				//Out
				if constexpr (std::is_pointer<InT>::value)
				{
					return *in;
				}
				else
				{
					return in;
				}
			}
		}
	};
}