project(JitCat)

set(JitCatHeaderPath ${JitCatIncludePath}/jitcat)

include_directories(${JitCatIncludePath})

file(GLOB JitCatHeaderFiles ${JitCatHeaderPath}/*.h)

if (JitCatHeaderFiles)
	set_source_files_properties(${JitCatHeaderFiles} PROPERTIES HEADER_FILE_ONLY ON)
endif()

#Create a list variable for every virtual folder of source files.
#These lists are then used in source_group and add_library

set(Source_Root
	${JitCatHeaderPath}/Configuration.h
	JitCat.cpp
	${JitCatHeaderPath}/JitCat.h
)

set(Source_Cat
	CatError.cpp
	${JitCatHeaderPath}/CatError.h
	CatGenericType.cpp
	${JitCatHeaderPath}/CatGenericType.h
	CatGrammar.cpp
	${JitCatHeaderPath}/CatGrammar.h
	${JitCatHeaderPath}/CatGrammarType.h
	CatRuntimeContext.cpp
	${JitCatHeaderPath}/CatRuntimeContext.h
	${JitCatHeaderPath}/CatScopeID.h
	${JitCatHeaderPath}/CatTokenIds.h
	CatTokenizer.cpp
	${JitCatHeaderPath}/CatTokenizer.h
)

set(Source_Cat_ASTNodes
	CatArgumentList.cpp 
	${JitCatHeaderPath}/CatArgumentList.h 
	CatArrayIndex.cpp 
	${JitCatHeaderPath}/CatArrayIndex.h 
	${JitCatHeaderPath}/CatAssignableExpression.h
	${JitCatHeaderPath}/CatASTNode.h 
	${JitCatHeaderPath}/CatASTNodes.h 
	${JitCatHeaderPath}/CatASTNodesDeclares.h 
	${JitCatHeaderPath}/CatASTNodeType.h 
	${JitCatHeaderPath}/CatAssignmentOperator.h 
	CatAssignmentOperator.cpp 
	${JitCatHeaderPath}/CatBuiltInFunctionType.h
	CatClassDefinition.cpp
	${JitCatHeaderPath}/CatClassDefinition.h
	${JitCatHeaderPath}/CatDefinition.h
	${JitCatHeaderPath}/CatExpression.h
	CatFunctionCall.cpp
	${JitCatHeaderPath}/CatFunctionCall.h
	CatFunctionDefinition.cpp
	${JitCatHeaderPath}/CatFunctionDefinition.h
	CatFunctionParameterDefinitions.cpp
	${JitCatHeaderPath}/CatFunctionParameterDefinitions.h
	CatIdentifier.cpp
	${JitCatHeaderPath}/CatIdentifier.h
	CatIfStatement.cpp
	${JitCatHeaderPath}/CatIfStatement.h
	CatInfixOperator.cpp
	${JitCatHeaderPath}/CatInfixOperator.h
	${JitCatHeaderPath}/CatInfixOperatorHeaderImplementation.h
	CatInfixOperatorType.cpp
	${JitCatHeaderPath}/CatInfixOperatorType.h
	CatLinkNode.cpp
	${JitCatHeaderPath}/CatLinkNode.h
	CatLiteral.cpp
	${JitCatHeaderPath}/CatLiteral.h
	CatMemberAccess.cpp
	${JitCatHeaderPath}/CatMemberAccess.h
	CatMemberFunctionCall.cpp
	${JitCatHeaderPath}/CatMemberFunctionCall.h
	CatPrefixOperator.cpp
	${JitCatHeaderPath}/CatPrefixOperator.h
	CatReturnStatement.cpp
	${JitCatHeaderPath}/CatReturnStatement.h
	${JitCatHeaderPath}/CatScope.h
	CatScopeBlock.cpp
	${JitCatHeaderPath}/CatScopeBlock.h
	CatScopeRoot.cpp
	${JitCatHeaderPath}/CatScopeRoot.h
	CatTypeNode.cpp
	CatSourceFile.cpp
	${JitCatHeaderPath}/CatSourceFile.h
	${JitCatHeaderPath}/CatStatement.h
	${JitCatHeaderPath}/CatTypeNode.h
	${JitCatHeaderPath}/CatTypedExpression.h
	CatVariableDeclaration.cpp
	${JitCatHeaderPath}/CatVariableDeclaration.h
	CatVariableDefinition.cpp
	${JitCatHeaderPath}/CatVariableDefinition.h
)

set(Source_Cat_ASTNodes_ASTHelper
	InfixOperatorOptimizer.cpp
	${JitCatHeaderPath}/InfixOperatorOptimizer.h
	ASTHelper.cpp
	${JitCatHeaderPath}/ASTHelper.h
)

set(Source_Cat_CodeGeneration_LLVM
	LLVMCatIntrinsics.cpp
	${JitCatHeaderPath}/LLVMCatIntrinsics.h
	${JitCatHeaderPath}/LLVMCodeGenerator.h
	${JitCatHeaderPath}/LLVMCodeGeneratorHelper.h
	${JitCatHeaderPath}/LLVMCompileOptions.h
	${JitCatHeaderPath}/LLVMCompileTimeContext.h
	${JitCatHeaderPath}/LLVMForwardDeclares.h
	${JitCatHeaderPath}/LLVMJit.h
	${JitCatHeaderPath}/LLVMJitHeaderImplementation.h
	LLVMTypes.cpp
	${JitCatHeaderPath}/LLVMTypes.h
)

if(LLVM_ENABLED)
	list(APPEND Source_Cat_CodeGeneration_LLVM 
		LLVMCodeGenerator.cpp
		LLVMCodeGeneratorHelper.cpp
		LLVMCompileTimeContext.cpp
		LLVMJit.cpp)	
endif()

set(Source_Cat_Expression
	${JitCatHeaderPath}/Expression.h
	ExpressionAny.cpp
	${JitCatHeaderPath}/ExpressionAny.h
	ExpressionAssignAny.cpp
	${JitCatHeaderPath}/ExpressionAssignAny.h
	${JitCatHeaderPath}/ExpressionAssignment.h
	${JitCatHeaderPath}/ExpressionAssignmentHeaderImplementation.h
	ExpressionBase.cpp
	${JitCatHeaderPath}/ExpressionBase.h	
	${JitCatHeaderPath}/ExpressionHeaderImplementation.h
	ExpressionHelperFunctions.cpp
	${JitCatHeaderPath}/ExpressionHelperFunctions.h
)

set(Source_Cat_SourceFile
	SourceFile.cpp
	${JitCatHeaderPath}/SourceFile.h
)

set(Source_Cat_Expression_Autocompletion
	AutoCompletion.cpp
	${JitCatHeaderPath}/AutoCompletion.h
)

set(Source_Cat_Expression_Errors
	ErrorContext.cpp
	${JitCatHeaderPath}/ErrorContext.h
	ExpressionErrorManager.cpp
	${JitCatHeaderPath}/ExpressionErrorManager.h
)

set(Source_Grammar
	GrammarBase.cpp
	${JitCatHeaderPath}/GrammarBase.h
	Production.cpp
	${JitCatHeaderPath}/Production.h
	ProductionEpsilonToken.cpp
	${JitCatHeaderPath}/ProductionEpsilonToken.h
	ProductionNonTerminalToken.cpp
	${JitCatHeaderPath}/ProductionNonTerminalToken.h
	ProductionRule.cpp
	${JitCatHeaderPath}/ProductionRule.h
	ProductionTerminalToken.cpp
	${JitCatHeaderPath}/ProductionTerminalToken.h
	ProductionToken.cpp
	${JitCatHeaderPath}/ProductionToken.h
	ProductionTokenSet.cpp
	${JitCatHeaderPath}/ProductionTokenSet.h
	ProductionTokenSetMember.cpp
	${JitCatHeaderPath}/ProductionTokenSetMember.h
	${JitCatHeaderPath}/ProductionTokenType.h
	${JitCatHeaderPath}/TokenFlag.h
)

set(Source_Parser
	${JitCatHeaderPath}/ASTNode.h
	ASTNodeParser.cpp
	${JitCatHeaderPath}/ASTNodeParser.h
	DFAState.cpp
	${JitCatHeaderPath}/DFAState.h
	Item.cpp
	${JitCatHeaderPath}/Item.h
	${JitCatHeaderPath}/RuntimeContext.h
	SLRParser.cpp
	${JitCatHeaderPath}/SLRParser.h
	SLRParseResult.cpp
	${JitCatHeaderPath}/SLRParseResult.h
	${JitCatHeaderPath}/StackItem.h
	StackItemProduction.cpp
	${JitCatHeaderPath}/StackItemProduction.h
	${JitCatHeaderPath}/StackItemToken.h
)

set(Source_Reflection
	Reflectable.cpp
	${JitCatHeaderPath}/Reflectable.h
	ReflectableHandle.cpp
	${JitCatHeaderPath}/ReflectableHandle.h
	TypeRegistry.cpp
	${JitCatHeaderPath}/TypeRegistry.h
)

set(Source_Reflection_Enums
	${JitCatHeaderPath}/AssignableType.h
	ContainerType.cpp
	${JitCatHeaderPath}/ContainerType.h
	${JitCatHeaderPath}/MemberFlags.h
)


set(Source_Reflection_Serialisation
	XMLHelper.cpp
	${JitCatHeaderPath}/XMLHelper.h
)

set(Source_Reflection_TypeInfo
	${JitCatHeaderPath}/ContainerManipulator.h
	CustomTypeInfo.cpp
	${JitCatHeaderPath}/CustomTypeInfo.h
	CustomTypeInstance.cpp
	${JitCatHeaderPath}/CustomTypeInstance.h
	${JitCatHeaderPath}/CustomTypeMemberFunctionInfo.h
	${JitCatHeaderPath}/CustomTypeMemberInfo.h
	${JitCatHeaderPath}/CustomTypeMemberInfoHeaderImplementation.h
	${JitCatHeaderPath}/MemberFunctionInfo.h
	${JitCatHeaderPath}/MemberInfo.h
	${JitCatHeaderPath}/MemberInfoHeaderImplementation.h
	${JitCatHeaderPath}/MemberTypeInfoCreator.h
	${JitCatHeaderPath}/TypeCaster.h
	TypeInfo.cpp
	${JitCatHeaderPath}/TypeInfo.h
	${JitCatHeaderPath}/TypeInfoHeaderImplementation.h
	${JitCatHeaderPath}/TypeTraits.h
	${JitCatHeaderPath}/TypeTraitsHeaderImplementation.h
	${JitCatHeaderPath}/VariableEnumerator.h
)

set(Source_Tokenizer
	Document.cpp
	${JitCatHeaderPath}/Document.h
	${JitCatHeaderPath}/DocumentSelection.h
	${JitCatHeaderPath}/Lexeme.h
	ParseToken.cpp
	${JitCatHeaderPath}/ParseToken.h
	TokenizerBase.cpp
	${JitCatHeaderPath}/TokenizerBase.h
)

set(Source_Tokenizer_Tokens
	CommentToken.cpp
	${JitCatHeaderPath}/CommentToken.h
	${JitCatHeaderPath}/CommentTokenSubTypes.h
	ConstantToken.cpp
	${JitCatHeaderPath}/ConstantToken.h
	${JitCatHeaderPath}/ConstantTokenSubTypes.h
	ErrorToken.cpp
	${JitCatHeaderPath}/ErrorToken.h
	IdentifierToken.cpp
	${JitCatHeaderPath}/IdentifierToken.h
	${JitCatHeaderPath}/IdentifierTokenSubTypes.h
	OneCharToken.cpp
	${JitCatHeaderPath}/OneCharToken.h
	${JitCatHeaderPath}/OneCharTokenSubTypes.h
	ParseHelper.cpp
	${JitCatHeaderPath}/ParseHelper.h
	TwoCharToken.cpp
	${JitCatHeaderPath}/TwoCharToken.h
	${JitCatHeaderPath}/TwoCharTokenSubTypes.h
	WhitespaceToken.cpp
	${JitCatHeaderPath}/WhitespaceToken.h
)

set(Source_Tools
	CatLog.cpp
	${JitCatHeaderPath}/CatLog.h
	Timer.cpp
	${JitCatHeaderPath}/Timer.h
	Tools.cpp
	${JitCatHeaderPath}/Tools.h
	${JitCatHeaderPath}/ToolsHeaderImplementation.h
)


#Project file organisation
source_group(Cat FILES ${Source_Cat})
source_group(Cat\\ASTNodes FILES ${Source_Cat_ASTNodes})
source_group(Cat\\ASTNodes\\ASTHelper FILES ${Source_Cat_ASTNodes_ASTHelper})
source_group(Cat\\CodeGeneration\\LLVM FILES ${Source_Cat_CodeGeneration_LLVM})
source_group(Cat\\Expression FILES ${Source_Cat_Expression})
source_group(Cat\\Expression\\AutoCompletion FILES ${Source_Cat_Expression_Autocompletion})
source_group(Cat\\Expression\\Errors FILES ${Source_Cat_Expression_Errors})
source_group(Cat\\SourceFile FILES ${Source_Cat_SourceFile})
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
	${Source_Cat_ASTNodes_ASTHelper}
	${Source_Cat_CodeGeneration_LLVM}
	${Source_Cat_Expression}
	${Source_Cat_Expression_Autocompletion}
	${Source_Cat_Expression_Errors}
	${Source_Cat_SourceFile}
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

