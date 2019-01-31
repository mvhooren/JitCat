/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

namespace llvm
{
	class Function;
	namespace legacy
	{
		class FunctionPassManager;
	};
	namespace Intrinsic {
		enum ID : unsigned;
	};
	class LLVMContext;
	class FunctionType;
	class Module;
	class Value;
	class CallInst;
	class ConstantFolder;
	class IRBuilderDefaultInserter;
	template<typename T, typename Inserter>
	class IRBuilder;
	class Type;
	class CallInst;
	namespace orc
	{
		class JITDylib;
	};
};