project(JitCat)

include_directories(
    ${JitCatIncludePath}
)

file(GLOB JitCatHeaderFiles 
    ${JitCatIncludePath}/*.h
)

if (JitCatHeaderFiles)
	set_source_files_properties(${JitCatHeaderFiles} PROPERTIES HEADER_FILE_ONLY ON)
endif()



set(JitCatSourceFiles  
	XMLHelper.cpp
	ASTNodeParser.cpp
	AutoCompletion.cpp
	CatArgumentList.cpp
	CatArrayIndex.cpp
	CatError.cpp
	CatFunctionCall.cpp
	CatGenericType.cpp
	CatGrammar.cpp
	CatIdentifier.cpp
	CatInfixOperator.cpp
	CatInfixOperatorType.cpp
	CatLinkNode.cpp
	CatLiteral.cpp
	CatLog.cpp
	CatMemberAccess.cpp
	CatMemberFunctionCall.cpp
	CatPrefixOperator.cpp
	CatRuntimeContext.cpp
	CatScopeRoot.cpp
	CatTokenizer.cpp
	CatType.cpp
	CatValue.cpp
	CommentToken.cpp
	ConstantToken.cpp
	ContainerType.cpp
	CustomTypeInfo.cpp
	CustomTypeInstance.cpp
	DFAState.cpp
	Document.cpp
	ErrorContext.cpp
	ErrorToken.cpp
	ExpressionAny.cpp
	ExpressionErrorManager.cpp
	ExpressionHelperFunctions.cpp
	ExpressionTools.cpp
	Grammar.cpp
	IdentifierToken.cpp
	InfixOperatorOptimizer.cpp
	Item.cpp
	JitCat.cpp
	Lexeme.cpp
	LLVMCatIntrinsics.cpp
	LLVMTypes.cpp
	MemberInfo.cpp
	MemberReference.cpp
	MemberReferencePtr.cpp
	OneCharToken.cpp
	OptimizationHelper.cpp
	ParseHelper.cpp
	ParseToken.cpp
	Production.cpp
	ProductionEpsilonToken.cpp
	ProductionNonTerminalToken.cpp
	ProductionRule.cpp
	ProductionTerminalToken.cpp
	ProductionToken.cpp
	ProductionTokenSet.cpp
	ProductionTokenSetMember.cpp
	Reflectable.cpp
	ReflectableHandle.cpp
	SLRParser.cpp
	SLRParseResult.cpp
	SpecificMemberType.cpp
	StackItemProduction.cpp
	Timer.cpp
	Tokenizer.cpp
	Tools.cpp
	TwoCharToken.cpp
	TypeInfo.cpp
	TypeRegistry.cpp
	TypeTraits.cpp
	WhitespaceToken.cpp
)


if(LLVM_ENABLED)
	list(APPEND JitCatSourceFiles 
		LLVMCodeGenerator.cpp
		LLVMCodeGeneratorHelper.cpp
		LLVMCompileTimeContext.cpp
		LLVMJit.cpp)	
endif()


source_group(Cat FILES 
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
	CatValue.cpp
	${JitCatIncludePath}/CatValue.h
	${JitCatIncludePath}/RootTypeSource.h
)

source_group(Cat\\ASTNodes FILES 
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

source_group(Cat\\ASTNodes\\ASTOptimization FILES
	${JitCatIncludePath}/CatExpression.h
	InfixOperatorOptimizer.cpp
	${JitCatIncludePath}/InfixOperatorOptimizer.h
	OptimizationHelper.cpp
	${JitCatIncludePath}/OptimizationHelper.h
)

source_group(Cat\\CodeGeneration\\LLVM FILES
	LLVMCatIntrinsics.cpp
	${JitCatIncludePath}/LLVMCatIntrinsics.h
	LLVMCodeGenerator.cpp
	${JitCatIncludePath}/LLVMCodeGenerator.h
	LLVMCodeGeneratorHelper.cpp
	${JitCatIncludePath}/LLVMCodeGeneratorHelper.h
	${JitCatIncludePath}/LLVMCompileOptions.h
	LLVMCompileTimeContext.cpp
	${JitCatIncludePath}/LLVMCompileTimeContext.h
	${JitCatIncludePath}/LLVMForwardDeclares.h
	LLVMJit.cpp
	${JitCatIncludePath}/LLVMJit.h
	${JitCatIncludePath}/LLVMJitHeaderImplementation.h
	LLVMTypes.cpp
	${JitCatIncludePath}/LLVMTypes.h
)

source_group(Cat\\Expression FILES
	${JitCatIncludePath}/Expression.h
	ExpressionAny.cpp
	${JitCatIncludePath}/ExpressionAny.h
	${JitCatIncludePath}/ExpressionBase.h
	${JitCatIncludePath}/ExpressionHeaderImplementation.h
	ExpressionHelperFunctions.cpp
	${JitCatIncludePath}/ExpressionHelperFunctions.h
	ExpressionTools.cpp
	${JitCatIncludePath}/ExpressionTools.h
	TypeTraits.cpp
	${JitCatIncludePath}/TypeTraits.h
	${JitCatIncludePath}/TypeTraitsHeaderImplementation.h
)

source_group(Cat\\Expression\\AutoCompletion FILES
	AutoCompletion.cpp
	${JitCatIncludePath}/AutoCompletion.h
)

source_group(Cat\\Expression\\Errors FILES
	ErrorContext.cpp
	${JitCatIncludePath}/ErrorContext.h
	ExpressionErrorManager.cpp
	${JitCatIncludePath}/ExpressionErrorManager.h
)

source_group(Grammar FILES 
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

source_group(Parser FILES 
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

source_group(Reflection FILES 
	Reflectable.cpp
	${JitCatIncludePath}/Reflectable.h
	ReflectableHandle.cpp
	${JitCatIncludePath}/ReflectableHandle.h
	TypeRegistry.cpp
	${JitCatIncludePath}/TypeRegistry.h
)

source_group(Reflection\\Enums FILES 
	ContainerType.cpp
	${JitCatIncludePath}/ContainerType.h
	${JitCatIncludePath}/MemberTypeFlags.h
	SpecificMemberType.cpp
	${JitCatIncludePath}/SpecificMemberType.h
)

source_group(Reflection\\MemberReference FILES
	${JitCatIncludePath}/BasicTypeMemberReference.h
	${JitCatIncludePath}/BasicTypeMemberReferenceHeaderImplementation.h
	${JitCatIncludePath}/ContainerMemberReference.h
	${JitCatIncludePath}/ContainerMemberReferenceHeaderImplementation.h
	MemberReference.cpp
	${JitCatIncludePath}/MemberReference.h
	MemberReferencePtr.cpp
	${JitCatIncludePath}/MemberReferencePtr.h
	${JitCatIncludePath}/ObjectMemberReference.h
	${JitCatIncludePath}/ObjectMemberReferenceHeaderImplementation.h
)

source_group(Reflection\\Serialisation FILES
	XMLHelper.cpp
	${JitCatIncludePath}/XMLHelper.h
)

source_group(Reflection\\TypeInfo FILES	
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

source_group(Tokenizer FILES 
	Document.cpp
	${JitCatIncludePath}/Document.h
	Lexeme.cpp
	${JitCatIncludePath}/Lexeme.h
	ParseToken.cpp
	${JitCatIncludePath}/ParseToken.h
	Tokenizer.cpp
	${JitCatIncludePath}/Tokenizer.h
)

source_group(Tokenizer\\Tokens FILES 
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

source_group(Tools FILES 
	CatLog.cpp
	${JitCatIncludePath}/CatLog.h
	Timer.cpp
	${JitCatIncludePath}/Timer.h
	Tools.cpp
	${JitCatIncludePath}/Tools.h
	${JitCatIncludePath}/ToolsHeaderImplementation.h
)

source_group("" FILES 
	${JitCatIncludePath}/Configuration.h
	JitCat.cpp
	${JitCatIncludePath}/JitCat.h
	llvm.natvis
)

add_library(JitCat STATIC ${JitCatHeaderFiles} ${JitCatSourceFiles})

#Set C++ 17 standard
set_property(TARGET JitCat PROPERTY CXX_STANDARD 17)

if(UNIX)
set_target_properties(JitCat
    PROPERTIES
    LINK_FLAGS_RELEASE "-Wl,-s,-x")
	
endif()
