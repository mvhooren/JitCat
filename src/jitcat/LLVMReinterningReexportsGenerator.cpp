/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2020
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/LLVMReinterningReexportsGenerator.h"

using namespace jitcat;
using namespace jitcat::LLVM;


LLVMReinterningReexportsGenerator::LLVMReinterningReexportsGenerator(llvm::orc::JITDylib& sourceLib) :
	sourceLib(sourceLib)
{
}


llvm::Error LLVMReinterningReexportsGenerator::tryToGenerate(llvm::orc::LookupKind kind, llvm::orc::JITDylib& targetLib, llvm::orc::JITDylibLookupFlags libLookupFlags, const llvm::orc::SymbolLookupSet& lookupSet)
{
	assert(&targetLib != &sourceLib && "Cannot re-export from the same dylib");
	
	bool mismatchingExecutionSessions = &sourceLib.getExecutionSession() != &targetLib.getExecutionSession();

	const llvm::orc::SymbolLookupSet* activeLookupSet = &lookupSet;

	llvm::orc::SymbolLookupSet reInternSet;
	if (mismatchingExecutionSessions)
	{
		//Create a copy of the lookupSet, but re-intern each symbol name to the source library's execution session.
		llvm::orc::ExecutionSession& session = sourceLib.getExecutionSession();
		for (auto& iter : lookupSet)
		{
			std::string name = *iter.first;
			reInternSet.add(session.intern(name), iter.second);
		}
		activeLookupSet = & reInternSet;
	}

	// Use lookupFlags to find the subset of symbols that match our lookup.
	auto flags = sourceLib.lookupFlags(kind, libLookupFlags, *activeLookupSet);
	if (!flags)
	{
		return flags.takeError();
	}

	// Create an alias map.
	llvm::orc::SymbolAliasMap aliasMap;
	if (mismatchingExecutionSessions)
	{
		//If the execution sessions do not match, we need to re-intern the resolved symbols to the target library's execution session.
		for (auto& flag : *flags)
		{
			std::string name = *flag.first;
			aliasMap[targetLib.getExecutionSession().intern(name)] = llvm::orc::SymbolAliasMapEntry(flag.first, flag.second);
		}
	}
	else
	{
		for (auto& flag : *flags)
		{
			aliasMap[flag.first] = llvm::orc::SymbolAliasMapEntry(flag.first, flag.second);
		}
	}

	if (aliasMap.empty())
	{
		return llvm::Error::success();
	}

	// Define the re-exports.
	return targetLib.define(llvm::orc::reexports(sourceLib, aliasMap, llvm::orc::JITDylibLookupFlags::MatchExportedSymbolsOnly));

}
