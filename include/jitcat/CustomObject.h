#pragma once

namespace jitcat::Reflection
{

	//This is the type that is used to pass objects created by a CustomTypeInfo in std::any
	class CustomObject 
	{
	public:
		CustomObject() {}
		~CustomObject() {}
	};

}