project(JitCat)

include_directories(${JitCatIncludePath})

file(GLOB JitCatHeaderFiles ${JitCatIncludePath}/*.h)

if (JitCatHeaderFiles)
	set_source_files_properties(${JitCatHeaderFiles} PROPERTIES HEADER_FILE_ONLY ON)
endif()

#Create a list variable for every virtual folder of source files.
#These lists are then used in source_group and add_library

set(Source_Root
	${JitCatIncludePath}/Configuration.h
	JitCat.cpp
	${JitCatIncludePath}/JitCat.h
)

set(Source_Cat
	CatError.cpp
	${JitCatIncludePath}/CatError.h
	CatGenericType.cpp
	${JitCatIncludePath}/CatGenericType.h
	CatGrammar.cpp
	${JitCatIncludePath}/CatGrammar.h
	CatInfixOperatorType.cpp
	${JitCatIncludePath}/CatInfixOperatorType.h
	CatRuntimeContext.cpp
	${JitCatIncludePath}/CatRuntimeContext.h
	${JitCatIncludePath}/CatTokenIds.h
	CatTokenizer.cpp
	${JitCatIncludePath}/CatTokenizer.h
	CatType.cpp
	${JitCatIncludePath}/CatType.h
	${JitCatIncludePath}/RootTypeSource.h
)

set(Source_Cat_ASTNodes
	CatArgumentList.cpp 
	${JitCatIncludePath}/CatArgumentList.h 
	CatArrayIndex.cpp 
	${JitCatIncludePath}/CatArrayIndex.h 
	${JitCatIncludePath}/CatASTNode.h 
	${JitCatIncludePath}/CatASTNodes.h 
	${JitCatIncludePath}/CatASTNodesDeclares.h 
	${JitCatIncludePath}/CatASTNodeType.h 
	${JitCatIncludePath}/CatBuiltInFunctionType.h
	CatFunctionCall.cpp
	${JitCatIncludePath}/CatFunctionCall.h
	CatIdentifier.cpp
	${JitCatIncludePath}/CatIdentifier.h
	CatInfixOperator.cpp
	${JitCatIncludePath}/CatInfixOperator.h
	${JitCatIncludePath}/CatInfixOperatorHeaderImplementation.h
	CatLinkNode.cpp
	${JitCatIncludePath}/CatLinkNode.h
	CatLiteral.cpp
	${JitCatIncludePath}/CatLiteral.h
	CatMemberAccess.cpp
	${JitCatIncludePath}/CatMemberAccess.h
	CatMemberFunctionCall.cpp
	${JitCatIncludePath}/CatMemberFunctionCall.h
	CatPrefixOperator.cpp
	${JitCatIncludePath}/CatPrefixOperator.h
	CatScopeRoot.cpp
	${JitCatIncludePath}/CatScopeRoot.h
	${JitCatIncludePath}/CatTypedExpression.h
)

set(Source_Cat_ASTNodes_ASTOptimization
	${JitCatIncludePath}/CatExpression.h
	InfixOperatorOptimizer.cpp
	${JitCatIncludePath}/InfixOperatorOptimizer.h
	OptimizationHelper.cpp
	${JitCatIncludePath}/OptimizationHelper.h
)

set(Source_Cat_CodeGeneration_LLVM
	LLVMCatIntrinsics.cpp
	${JitCatIncludePath}/LLVMCatIntrinsics.h
	${JitCatIncludePath}/LLVMCodeGenerator.h
	${JitCatIncludePath}/LLVMCodeGeneratorHelper.h
	${JitCatIncludePath}/LLVMCompileOptions.h
	${JitCatIncludePath}/LLVMCompileTimeContext.h
	${JitCatIncludePath}/LLVMForwardDeclares.h
	${JitCatIncludePath}/LLVMJit.h
	${JitCatIncludePath}/LLVMJitHeaderImplementation.h
	LLVMTypes.cpp
	${JitCatIncludePath}/LLVMTypes.h
)

if(LLVM_ENABLED)
	list(APPEND Source_Cat_CodeGeneration_LLVM 
		LLVMCodeGenerator.cpp
		LLVMCodeGeneratorHelper.cpp
		LLVMCompileTimeContext.cpp
		LLVMJit.cpp)	
endif()

set(Source_Cat_Expression
	${JitCatIncludePath}/Expression.h
	ExpressionAny.cpp
	${JitCatIncludePath}/ExpressionAny.h
	ExpressionBase.cpp
	${JitCatIncludePath}/ExpressionBase.h	
	${JitCatIncludePath}/ExpressionHeaderImplementation.h
	ExpressionHelperFunctions.cpp
	${JitCatIncludePath}/ExpressionHelperFunctions.h
	${JitCatIncludePath}/TypeTraits.h
	${JitCatIncludePath}/TypeTraitsHeaderImplementation.h
)

set(Source_Cat_Expression_Autocompletion
	AutoCompletion.cpp
	${JitCatIncludePath}/AutoCompletion.h
)

set(Source_Cat_Expression_Errors
	ErrorContext.cpp
	${JitCatIncludePath}/ErrorContext.h
	ExpressionErrorManager.cpp
	${JitCatIncludePath}/ExpressionErrorManager.h
)

set(Source_Grammar
	Grammar.cpp
	${JitCatIncludePath}/Grammar.h
	Production.cpp
	${JitCatIncludePath}/Production.h
	ProductionEpsilonToken.cpp
	${JitCatIncludePath}/ProductionEpsilonToken.h
	ProductionNonTerminalToken.cpp
	${JitCatIncludePath}/ProductionNonTerminalToken.h
	ProductionRule.cpp
	${JitCatIncludePath}/ProductionRule.h
	ProductionTerminalToken.cpp
	${JitCatIncludePath}/ProductionTerminalToken.h
	ProductionToken.cpp
	${JitCatIncludePath}/ProductionToken.h
	ProductionTokenSet.cpp
	${JitCatIncludePath}/ProductionTokenSet.h
	ProductionTokenSetMember.cpp
	${JitCatIncludePath}/ProductionTokenSetMember.h
	${JitCatIncludePath}/ProductionTokenType.h
	${JitCatIncludePath}/TokenFlag.h
)

set(Source_Parser
	${JitCatIncludePath}/ASTNode.h
	ASTNodeParser.cpp
	${JitCatIncludePath}/ASTNodeParser.h
	DFAState.cpp
	${JitCatIncludePath}/DFAState.h
	Item.cpp
	${JitCatIncludePath}/Item.h
	${JitCatIncludePath}/RuntimeContext.h
	SLRParser.cpp
	${JitCatIncludePath}/SLRParser.h
	SLRParseResult.cpp
	${JitCatIncludePath}/SLRParseResult.h
	${JitCatIncludePath}/StackItem.h
	StackItemProduction.cpp
	${JitCatIncludePath}/StackItemProduction.h
	${JitCatIncludePath}/StackItemToken.h
)

set(Source_Reflection
	Reflectable.cpp
	${JitCatIncludePath}/Reflectable.h
	ReflectableHandle.cpp
	${JitCatIncludePath}/ReflectableHandle.h
	${JitCatIncludePath}/TypeCaster.h
	TypeRegistry.cpp
	${JitCatIncludePath}/TypeRegistry.h
)

set(Source_Reflection_Enums
	ContainerType.cpp
	${JitCatIncludePath}/ContainerType.h
	${JitCatIncludePath}/MemberTypeFlags.h
	SpecificMemberType.cpp
	${JitCatIncludePath}/SpecificMemberType.h
)


set(Source_Reflection_Serialisation
	XMLHelper.cpp
	${JitCatIncludePath}/XMLHelper.h
)

set(Source_Reflection_TypeInfo
	CustomTypeInfo.cpp
	${JitCatIncludePath}/CustomTypeInfo.h
	CustomTypeInstance.cpp
	${JitCatIncludePath}/CustomTypeInstance.h
	${JitCatIncludePath}/CustomTypeMemberInfo.h
	${JitCatIncludePath}/CustomTypeMemberInfoHeaderImplementation.h
	${JitCatIncludePath}/MemberFunctionInfo.h
	MemberInfo.cpp
	${JitCatIncludePath}/MemberInfo.h
	${JitCatIncludePath}/MemberInfoHeaderImplementation.h
	${JitCatIncludePath}/MemberTypeInfoCreator.h
	TypeInfo.cpp
	${JitCatIncludePath}/TypeInfo.h
	${JitCatIncludePath}/TypeInfoHeaderImplementation.h
	${JitCatIncludePath}/VariableEnumerator.h
)

set(Source_Tokenizer
	Document.cpp
	${JitCatIncludePath}/Document.h
	Lexeme.cpp
	${JitCatIncludePath}/Lexeme.h
	ParseToken.cpp
	${JitCatIncludePath}/ParseToken.h
	Tokenizer.cpp
	${JitCatIncludePath}/Tokenizer.h
)

set(Source_Tokenizer_Tokens
	CommentToken.cpp
	${JitCatIncludePath}/CommentToken.h
	${JitCatIncludePath}/CommentTokenSubTypes.h
	ConstantToken.cpp
	${JitCatIncludePath}/ConstantToken.h
	${JitCatIncludePath}/ConstantTokenSubTypes.h
	ErrorToken.cpp
	${JitCatIncludePath}/ErrorToken.h
	IdentifierToken.cpp
	${JitCatIncludePath}/IdentifierToken.h
	${JitCatIncludePath}/IdentifierTokenSubTypes.h
	OneCharToken.cpp
	${JitCatIncludePath}/OneCharToken.h
	${JitCatIncludePath}/OneCharTokenSubTypes.h
	ParseHelper.cpp
	${JitCatIncludePath}/ParseHelper.h
	TwoCharToken.cpp
	${JitCatIncludePath}/TwoCharToken.h
	${JitCatIncludePath}/TwoCharTokenSubTypes.h
	WhitespaceToken.cpp
	${JitCatIncludePath}/WhitespaceToken.h
)

set(Source_Tools
	CatLog.cpp
	${JitCatIncludePath}/CatLog.h
	Timer.cpp
	${JitCatIncludePath}/Timer.h
	Tools.cpp
	${JitCatIncludePath}/Tools.h
	${JitCatIncludePath}/ToolsHeaderImplementation.h
)


#Project file organisation
source_group(Cat FILES ${Source_Cat})
source_group(Cat\\ASTNodes FILES ${Source_Cat_ASTNodes})
source_group(Cat\\ASTNodes\\ASTOptimization FILES ${Source_Cat_ASTNodes_ASTOptimization})
source_group(Cat\\CodeGeneration\\LLVM FILES ${Source_Cat_CodeGeneration_LLVM})
source_group(Cat\\Expression FILES ${Source_Cat_Expression})
source_group(Cat\\Expression\\AutoCompletion FILES ${Source_Cat_Expression_Autocompletion})
source_group(Cat\\Expression\\Errors FILES ${Source_Cat_Expression_Errors})
source_group(Grammar FILES ${Source_Grammar})
source_group(Parser FILES ${Source_Parser})
source_group(Reflection FILES ${Source_Reflection})
source_group(Reflection\\Enums FILES ${Source_Reflection_Enums})
source_group(Reflection\\Serialisation FILES ${Source_Reflection_Serialisation})
source_group(Reflection\\TypeInfo FILES	${Source_Reflection_TypeInfo})
source_group(Tokenizer FILES ${Source_Tokenizer})
source_group(Tokenizer\\Tokens FILES ${Source_Tokenizer_Tokens})
source_group(Tools FILES ${Source_Tools})
source_group("" FILES ${Source_Root} llvm.natvis)

if(NOT MSVC)
	add_compile_options(-fPIC)
endif()

add_library(JitCat STATIC 
	${Source_Root}
	${Source_Cat}
	${Source_Cat_ASTNodes}
	${Source_Cat_ASTNodes_ASTOptimization}
	${Source_Cat_CodeGeneration_LLVM}
	${Source_Cat_Expression}
	${Source_Cat_Expression_Autocompletion}
	${Source_Cat_Expression_Errors}
	${Source_Grammar}
	${Source_Parser}
	${Source_Reflection}
	${Source_Reflection_Enums}
	${Source_Reflection_MemberReference}
	${Source_Reflection_Serialisation}
	${Source_Reflection_TypeInfo}
	${Source_Tokenizer}
	${Source_Tokenizer_Tokens}
	${Source_Tools}
)

