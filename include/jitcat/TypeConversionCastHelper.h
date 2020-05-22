/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/TypeTools.h"

#include <cassert>


namespace jitcat
{
	class TypeConversionCast
	{
	public:
		template<typename OutCVT, typename InT>
		static inline OutCVT convertCast(InT&& in)
		{
			using OutT = typename RemoveConst<OutCVT>::type;
			if constexpr (std::is_same<InT, OutT>::value)
			{
				return std::move(in);
			}
			else if constexpr (std::is_reference<OutT>::value)
			{
				//Out&
				if constexpr (std::is_reference<InT>::value
							  && std::is_pointer<typename std::remove_reference<InT>::type>::value)
				{
					//InT*&
					return **(&in);
				}
				else if constexpr (std::is_pointer<InT>::value)
				{
					///InT*
					return *in;
				}
				else
				{
					//InT
					return in;
				}
			}
			else if constexpr (std::is_pointer<OutT>::value)
			{
				//Out*
				if constexpr (std::is_reference<InT>::value
							  && std::is_pointer<typename std::remove_reference<InT>::type>::value)
				{
					//InT*&
					return in;
				}
				else
				{
					//InT or InT&
					return &in;
				}
				assert(false);
			}
			else
			{
				//Out
				if constexpr (std::is_reference<InT>::value
							  && std::is_pointer<typename std::remove_reference<InT>::type>::value)
				{
					//InT*&
					return **(&in);
				}
				if constexpr (std::is_pointer<InT>::value)
				{
					//InT*
					return *in;
				}
				else
				{
					//Int& 
					return in;
				}
			}
		}
	};
}
