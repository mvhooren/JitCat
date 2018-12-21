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