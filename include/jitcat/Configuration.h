#pragma once


class Configuration
{
public:
	static constexpr bool sretBeforeThis = 
#ifdef WIN32
		false;
#else
		true;
#endif

	static constexpr bool useThisCall = 
#ifdef WIN32
		true;
#else
		false;
#endif

	static constexpr int basicMemberFunctionPointerSize = sizeof(uintptr_t);

	static constexpr bool dumpFunctionIR = 
#ifdef _DEBUG
		false;
#else
		false;
#endif

	static constexpr bool enableSymbolSearchWorkaround =
#ifdef WIN32
		true;
#else
		false;
#endif
};
