# JitCat
A C++17 library for parsing and executing expressions. Allows easy exposure of variables and functions from C++ through built-in reflection functionality.

[Get it on github](https://github.com/mvhooren/JitCat).

## Features
* Supports floating point, integer, boolean, string and object pointer typed expressions as well as void typed and any typed expressions. More types to come.
* Optional native code compilation of expressions using the [LLVM](http://www.llvm.org) core libraries.
* Expose variables for use in expressions through reflection of C++ classes.
* Create and expose custom data structures to expressions at runtime.
* Reflection supports easy reflection of C++ classes and structs including reflection of functions. Limited to the following member types (with more types to come):
	* float, bool, int, std::string
	* Reflected objects
	* Pointers to reflected objects
	* std::unique_ptr to reflected objects
	* std::vector of pointers to reflected objects  
	* std::map of pointers to reflected objects with lowercase std::string as key (Support for arbitrary key types and other std containers is coming)  
	* member functions that return one of the supported types or void and that have parameters that are among the supported types (or no parameters).  
* Built-in functions for use in expressions for common operations  
* Basic optimizations of expressions such as const collapse.  
* Graceful error handling with human-readable error messages, though this area can use improvement.  
* Builds and is tested to work on Linux with clang (6.0) and gcc (8.0) and on Microsoft Windows with VS2017.  
* Built-in functionality for code completion of expressions.  
* Can export reflected types to a XML file for use with the JitCatValidator shared library to validate and code-complete expressions outside of the main C++ application.  
* Unit tests testing all aspects of the expression language and reflection.  

## Usage example
```c++
#include <jitcat/Expression.h>
#include <jitcat/ExpressionAny.h>
using namespace jitcat;

//A simple floating point calculation
Expression<float> anExpression("2.0 * abs(21.0)");
anExpression.compile(nullptr);
float value = anExpression.getValue(nullptr);

//String addition
Expression<std::string> anotherExpression("\"Hello\" + \" World\"");
anotherExpression.compile(nullptr);
std::string message = anotherExpression.getValue(nullptr);

//An expression returning an object. It uses a context that contains variables that can be referenced inside the expression.
Expression<MyObject*> objectTypeExpression("anObject.member.list[42].getMyObject()");
objectTypeExpression.compile(myContext);
MyObject* objectResult = objectTypeExpression.getValue(myContext);

//An expression that accepts any return type, in this case it is a std::vector.
//Passing myContext to the constructor will call compile automatically.
ExpressionAny anyTypedExpression(myContext, "anObject.member.list");
std::any anyValue = anyTypedExpression.getValue(myContext);
if (anyTypedExpression.getType().isFloatType())
{
	float floatValue = std::any_cast<float>(anyValue);
	//Do something
}
```

## Documentation
[Building JitCat](BUILDING_JITCAT.md)  
[Building LLVM for use with JitCat](BUILDING_LLVM.md)  
See the the [examples](examples) directory for a basic example.  

## Roadmap
JitCat is under active development. These are some of the major features that can be expected in the future:  
* Documentation and more example projects.  
* Support for more types.  
* Extending the language beyond simple expressions.  

## License
Uses the permissive MIT license, see the included LICENSE file.

## Author
Machiel van Hooren 

## Acknowledgements
Thanks to Ronimo Games, my ex-employer, for supporting and actively using this library. Many improvements have been made during office hours!

