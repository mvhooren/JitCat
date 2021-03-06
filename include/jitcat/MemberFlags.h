/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include <type_traits>


namespace jitcat::Reflection
{

	enum class MemberFlags: unsigned int
	{
		none = 0,
		isConst = 1,
		isStaticConst = 2,
		isWritable = 4
	};
	typedef MemberFlags MF;


	inline MemberFlags operator | (MemberFlags lhs, MemberFlags rhs)
	{
		using UnderlyingT = std::underlying_type_t <MemberFlags>;
		return static_cast<MemberFlags>(static_cast<UnderlyingT>(lhs) | static_cast<UnderlyingT>(rhs));
	}

	inline std::underlying_type_t<MemberFlags> operator& (std::underlying_type_t<MemberFlags> lhs, MemberFlags rhs)
	{
		using UnderlyingT = std::underlying_type_t <MemberFlags>;
		return lhs & static_cast<UnderlyingT>(rhs);
	}

	inline std::underlying_type_t<MemberFlags> operator& (MemberFlags lhs, MemberFlags rhs)
	{
		using UnderlyingT = std::underlying_type_t <MemberFlags>;
		return static_cast<UnderlyingT>(lhs) & static_cast<UnderlyingT>(rhs);
	}


	inline MemberFlags& operator |= (MemberFlags& lhs, MemberFlags rhs)
	{
		lhs = lhs | rhs;
		return lhs;
	}

} //End namespace jitcat::Reflection