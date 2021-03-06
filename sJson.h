/*
   sJson, v0.1 (WIP)
   * no dependencies
   * simple
   Do this:
	  #define SEMPER_JSON_IMPLEMENTATION
   before you include this file in *one* C or C++ file to create the implementation.
   // i.e. it should look like this:
   #include ...
   #include ...
   #include ...
   #define SEMPER_JSON_IMPLEMENTATION
   #include "sJson.h"
*/

#ifndef SEMPER_JSON_H
#define SEMPER_JSON_H

#ifndef S_JSON_MAX_NAME_LENGTH
#define S_JSON_MAX_NAME_LENGTH 256
#endif

#ifndef S_JSON_ASSERT
#include <assert.h>
#define S_JSON_ASSERT(x) assert(x)
#endif

#include <stdlib.h>
#include <string.h> // memcpy, strcmp

#ifndef S_JSON_ALLOC
#define S_JSON_ALLOC(x) malloc(x)
#endif

#ifndef S_JSON_FREE
#define S_JSON_FREE(x) free(x)
#endif

//-----------------------------------------------------------------------------
// [SECTION] Forward declarations and basic types
//-----------------------------------------------------------------------------
struct sJsonObject;
typedef int sJsonType; // enum -> sJsonType_

//-----------------------------------------------------------------------------
// [SECTION] Semper end-user API functions
//-----------------------------------------------------------------------------
namespace Semper
{
	sJsonObject* load_json(char* rawData, int size);
	void         free_json(sJsonObject** rootObject);
}

//-----------------------------------------------------------------------------
// [SECTION] Enums
//-----------------------------------------------------------------------------

enum sJsonType_
{
	S_JSON_TYPE_NONE,
	S_JSON_TYPE_STRING,
	S_JSON_TYPE_ARRAY,
	S_JSON_TYPE_NUMBER,
	S_JSON_TYPE_BOOL,
	S_JSON_TYPE_OBJECT,
	S_JSON_TYPE_NULL,
};

//-----------------------------------------------------------------------------
// [SECTION] Structs
//-----------------------------------------------------------------------------

struct sJsonObject
{
	sJsonType    type;
	sJsonObject* children;
	int          childCount;	
	char         name[S_JSON_MAX_NAME_LENGTH];
	char*        value;
	void*        _internal;

	// retrieve members
	inline sJsonObject* getMember      (const char* member){ for (int i = 0; i < childCount; i++) if (strcmp(member, children[i].name) == 0) return &children[i]; return nullptr;}
	inline bool         doesMemberExist(const char* member){ return getMember(member) != nullptr;}

	// cast values
	inline int      asInt()    { S_JSON_ASSERT(type == S_JSON_TYPE_NUMBER); return (int)strtod(value, nullptr);}
	inline unsigned asUInt()   { S_JSON_ASSERT(type == S_JSON_TYPE_NUMBER); return (unsigned)strtod(value, nullptr);}
	inline float    asFloat()  { S_JSON_ASSERT(type == S_JSON_TYPE_NUMBER); return (float)asDouble();}
	inline double   asDouble() { S_JSON_ASSERT(type == S_JSON_TYPE_NUMBER); return strtod(value, nullptr);}
	inline char*    asString() { S_JSON_ASSERT(type == S_JSON_TYPE_STRING); return value;}
	inline bool    	asBool()   { S_JSON_ASSERT(type == S_JSON_TYPE_BOOL);   return value[0] == 't';}
	
	// cast array values
	inline void asIntArray   (int*      out, int size) { S_JSON_ASSERT(type == S_JSON_TYPE_ARRAY); S_JSON_ASSERT(out); if(size > childCount) size = childCount; for(int i = 0; i < size; i++) out[i] = children[i].asInt();}
	inline void asUIntArray  (unsigned* out, int size) { S_JSON_ASSERT(type == S_JSON_TYPE_ARRAY); S_JSON_ASSERT(out); if(size > childCount) size = childCount; for(int i = 0; i < size; i++) out[i] = children[i].asUInt();}
	inline void asFloatArray (float*    out, int size) { S_JSON_ASSERT(type == S_JSON_TYPE_ARRAY); S_JSON_ASSERT(out); if(size > childCount) size = childCount; for(int i = 0; i < size; i++) out[i] = children[i].asFloat();}
	inline void asDoubleArray(double*   out, int size) { S_JSON_ASSERT(type == S_JSON_TYPE_ARRAY); S_JSON_ASSERT(out); if(size > childCount) size = childCount; for(int i = 0; i < size; i++) out[i] = children[i].asDouble();}
	inline void asBoolArray  (bool*     out, int size) { S_JSON_ASSERT(type == S_JSON_TYPE_ARRAY); S_JSON_ASSERT(out); if(size > childCount) size = childCount; for(int i = 0; i < size; i++) out[i] = children[i].asBool();}
	inline void asStringArray(char**    out, int size) { S_JSON_ASSERT(type == S_JSON_TYPE_ARRAY); S_JSON_ASSERT(out); if(size > childCount) size = childCount; for(int i = 0; i < size; i++) out[i] = children[i].asString();}

	// retrieve and cast values
	inline const char* getStringMember(const char* member, const char* defaultValue=0)     { auto m = getMember(member); return m==0       ? defaultValue : m->asString();}
	inline int         getIntMember   (const char* member, int         defaultValue=0)     { auto m = getMember(member); return m==nullptr ? defaultValue : m->asInt();}
	inline unsigned    getUIntMember  (const char* member, unsigned    defaultValue=0u)    { auto m = getMember(member); return m==nullptr ? defaultValue : m->asUInt();}
	inline float       getFloatMember (const char* member, float       defaultValue=0.0f)  { auto m = getMember(member); return m==nullptr ? defaultValue : m->asFloat();}
	inline double      getDoubleMember(const char* member, double      defaultValue=0.0)   { auto m = getMember(member); return m==nullptr ? defaultValue : m->asDouble();}
	inline bool        getBoolMember  (const char* member, bool        defaultValue=false) { auto m = getMember(member); return m==nullptr ? defaultValue : m->asBool();}

	// retrieve and cast array values
	inline void getIntArrayMember   (const char* member, int*          out, int size){ auto m = getMember(member); if(m) m->asIntArray(out, size);}
	inline void getUIntArrayMember  (const char* member, unsigned int* out, int size){ auto m = getMember(member); if(m) m->asUIntArray(out, size);}
	inline void getFloatArrayMember (const char* member, float*        out, int size){ auto m = getMember(member); if(m) m->asFloatArray(out, size);}
	inline void getDoubleArrayMember(const char* member, double*       out, int size){ auto m = getMember(member); if(m) m->asDoubleArray(out, size);}
	inline void getBoolArrayMember  (const char* member, bool*         out, int size){ auto m = getMember(member); if(m) m->asBoolArray(out, size);}
	inline void getStringArrayMember(const char* member, char**        out, int size){ auto m = getMember(member); if(m) m->asStringArray(out, size);}

	inline sJsonObject& operator[](const char* member) { auto m = getMember(member); S_JSON_ASSERT(m!=nullptr); return m==nullptr ? *this : *m;}
	inline sJsonObject& operator[](int i)              { S_JSON_ASSERT(children != nullptr); S_JSON_ASSERT(i < childCount); return children[i]; };
};

#endif

#ifdef SEMPER_JSON_IMPLEMENTATION

// borrowed from Dear ImGui
template<typename T>
struct sJsonVector_
{
	int size     = 0u;
	int capacity = 0u;
	T*  data     = nullptr;
	inline sJsonVector_() { size = capacity = 0; data = nullptr; }
	inline sJsonVector_<T>& operator=(const sJsonVector_<T>& src) { clear(); resize(src.size); memcpy(data, src.data, (size_t)size * sizeof(T)); return *this; }
	inline bool empty() const { return size == 0; }
	inline int  size_in_bytes() const   { return size * (int)sizeof(T); }
	inline T&   operator[](int i) { S_JSON_ASSERT(i >= 0 && i < size); return data[i]; }
	inline void clear() { if (data) { size = capacity = 0; S_JSON_FREE(data); data = nullptr; } }
	inline T*   begin() { return data; }
    inline T*   end() { return data + size; }
	inline T&   back() { S_JSON_ASSERT(size > 0); return data[size - 1]; }
	inline void swap(sJsonVector_<T>& rhs) { int rhs_size = rhs.size; rhs.size = size; size = rhs_size; int rhs_cap = rhs.capacity; rhs.capacity = capacity; capacity = rhs_cap; T* rhs_data = rhs.data; rhs.data = data; data = rhs_data; }
	inline int  _grow_capacity(int sz) { int new_capacity = capacity ? (capacity + capacity / 2) : 8; return new_capacity > sz ? new_capacity : sz; }
	inline void resize(int new_size) { if (new_size > capacity) reserve(_grow_capacity(new_size)); size = new_size; }
	inline void reserve(int new_capacity) { if (new_capacity <= capacity) return; T* new_data = (T*)S_JSON_ALLOC((size_t)new_capacity * sizeof(T)); if (data) { memcpy(new_data, data, (size_t)size * sizeof(T)); S_JSON_FREE(data); } data = new_data; capacity = new_capacity; }
	inline void push_back(const T& v) { if (size == capacity) reserve(_grow_capacity(size*2)); memcpy(&data[size], &v, sizeof(v)); size++;}
	inline void pop_back() { S_JSON_ASSERT(size > 0); size--; }
};

typedef int sJsonToken_Type;

enum sJsonToken_Type_
{
	S_JSON_TOKEN_NONE,
	S_JSON_TOKEN_LEFT_BRACE,
	S_JSON_TOKEN_RIGHT_BRACE,
	S_JSON_TOKEN_LEFT_BRACKET,
	S_JSON_TOKEN_RIGHT_BRACKET,
	S_JSON_TOKEN_COMMA,
	S_JSON_TOKEN_COLON,
	S_JSON_TOKEN_STRING,
	S_JSON_TOKEN_PRIMITIVE,
	S_JSON_TOKEN_MEMBER, // string but on the left of ":"
};

struct sJsonToken_
{
	sJsonToken_Type type = S_JSON_TOKEN_NONE;
	sJsonVector_<char> value;
};

struct sJsonStack_
{
	inline void push(int id){ if (data.empty()) data.resize(2048); if (currentIndex == data.size) data.resize(data.size * 2); data[currentIndex++] = id;}
	inline void pop()   { data[currentIndex] = -1; currentIndex--;}
	inline int  top()   { return data[currentIndex - 1];}
	inline bool empty() { return currentIndex == 0;}

	int currentIndex = 0;
	sJsonVector_<int> data;
};

static void
_parse_for_tokens(char* rawData, sJsonVector_<sJsonToken_>& tokens)
{
	int currentPos = 0u;
	char currentChar = rawData[currentPos];
	char basicTokens[] = { '{', '}', '[', ']', ':', ',' };

	bool inString = false;
	sJsonVector_<char> buffer;

	while (currentChar != 0)
	{
		bool tokenFound = false;
		if (!inString)
		{
			for (int i = 0; i < 6; i++)
			{
				if (currentChar == basicTokens[i])
				{
					if (!buffer.empty())
					{
						sJsonToken_ primitivetoken{};
						primitivetoken.type = S_JSON_TOKEN_PRIMITIVE;
						buffer.push_back('\0');
						for (int i = 0; i < buffer.size; i++)
						{
							primitivetoken.value.push_back(buffer[i]);
						}
						tokens.push_back({ primitivetoken.type });
						primitivetoken.value.swap(tokens.back().value);
						buffer.clear();
					}

					sJsonToken_ token{};
					if      (currentChar == '{') token.type = S_JSON_TOKEN_LEFT_BRACE;
					else if (currentChar == '}') token.type = S_JSON_TOKEN_RIGHT_BRACE;
					else if (currentChar == '[') token.type = S_JSON_TOKEN_LEFT_BRACKET;
					else if (currentChar == ']') token.type = S_JSON_TOKEN_RIGHT_BRACKET;
					else if (currentChar == ',') token.type = S_JSON_TOKEN_COMMA;
					else if (currentChar == ':') token.type = S_JSON_TOKEN_COLON;

					token.value.push_back(currentChar);
					token.value.push_back(0);
					tokens.push_back({ token.type });
					token.value.swap(tokens.back().value);
					tokenFound = true;
					break;
				}
			}
		}

		// strings
		if (!tokenFound)
		{
			if (currentChar == '"')
			{
				if (inString)
				{
					sJsonToken_ token{};
					buffer.push_back('\0');
					for (int i = 0; i < buffer.size; i++)
					{
						token.value.push_back(buffer[i]);
					}


					if (rawData[currentPos + 1] == ':')
						token.type = S_JSON_TOKEN_MEMBER;
					else
						token.type = S_JSON_TOKEN_STRING;
					tokens.push_back({token.type});
					token.value.swap(tokens.back().value);
					tokenFound = true;
					inString = false;
					buffer.clear();
				}
				else
				{
					inString = true;
					buffer.clear();
					tokenFound = true;
				}
			}
			else if (inString)
			{
				tokenFound = true;
				buffer.push_back(currentChar);
			}
		}

		// primitives
		if (!tokenFound)
		{
			buffer.push_back(currentChar);
		}

		currentPos++;
		currentChar = rawData[currentPos];
	}

}

static void
_remove_whitespace(char* rawData, char* spacesRemoved, size_t size)
{
	size_t currentPos = 0;
	size_t newCursor = 0;
	bool insideString = false;
	char currentChar = rawData[currentPos];
	while (currentChar != 0)
	{
		if (currentChar == '"' && insideString)
			insideString = false;
		else if (currentChar == '"')
			insideString = true;

		if (currentChar == ' ' || currentChar == '\n'
			|| currentChar == '\r' || currentChar == '\t')
		{
			if (insideString)
			{
				spacesRemoved[newCursor] = rawData[currentPos];
				newCursor++;
			}
			currentPos++;
		}
		else
		{
			spacesRemoved[newCursor] = rawData[currentPos];
			currentPos++;
			newCursor++;
		}

		if (currentPos >= size || newCursor >= size)
		{
			spacesRemoved[newCursor] = 0;
			break;
		}
		currentChar = rawData[currentPos];
	}
}

static void
_update_children_pointers(sJsonObject* object, sJsonVector_<sJsonObject*>* objects)
{
	S_JSON_ASSERT(object->_internal);

	if((*(sJsonVector_<int>*)(object->_internal)).empty())
	{
		S_JSON_FREE(object->_internal);
		object->_internal = nullptr;
		return;
	}

	object->childCount = (*(sJsonVector_<int>*)(object->_internal)).size;
	object->children = (sJsonObject*)S_JSON_ALLOC(sizeof(sJsonObject)*object->childCount);
	for(int i = 0; i < (*(sJsonVector_<int>*)(object->_internal)).size; i++)
	{
		object->children[i] = *(*objects)[(*(sJsonVector_<int>*)(object->_internal))[i]];
		_update_children_pointers(&object->children[i], objects);
	}

	S_JSON_FREE(object->_internal);
	object->_internal = nullptr;
}

sJsonObject*
Semper::load_json(char* rawData, int size)
{
	sJsonStack_ parentIDStack;
	sJsonVector_<sJsonObject*> objectArray;

	char* spacesRemoved = (char*)S_JSON_ALLOC(sizeof(char)*size);
	_remove_whitespace(rawData, spacesRemoved, size);

	sJsonVector_<sJsonToken_>* tokens =  (sJsonVector_<sJsonToken_>*)S_JSON_ALLOC(sizeof(sJsonVector_<sJsonToken_>));
	new (tokens) sJsonVector_<sJsonToken_>();
	_parse_for_tokens(spacesRemoved, *tokens);

	sJsonObject *rootObject = (sJsonObject *)S_JSON_ALLOC(sizeof(sJsonObject));
	rootObject->type = S_JSON_TYPE_OBJECT;
	rootObject->_internal = (sJsonVector_<int>*)S_JSON_ALLOC(sizeof(sJsonVector_<int>));
	new (rootObject->_internal) sJsonVector_<int>();
	
	objectArray.push_back(rootObject);
	parentIDStack.push(0);

	int i = 0;
	bool waitingOnValue = true;
	while (true)
	{

		if (i >= tokens->size)
			break;

		sJsonObject* parent = objectArray[parentIDStack.top()];

		switch ((*tokens)[i].type)
		{

		case S_JSON_TOKEN_LEFT_BRACE:
		{

			if(waitingOnValue)
			{
				waitingOnValue = false; // object was created in S_JSON_TOKEN_MEMBER case below.
			}
			else
			{
				sJsonObject *newObject = (sJsonObject*)S_JSON_ALLOC(sizeof(sJsonObject));
				newObject->type = S_JSON_TYPE_OBJECT;
				newObject->_internal = (sJsonVector_<int>*)S_JSON_ALLOC(sizeof(sJsonVector_<int>));
				new (newObject->_internal) sJsonVector_<int>();
				objectArray.push_back(newObject);
				parentIDStack.push(objectArray.size-1);
				(*(sJsonVector_<int>*)(parent->_internal)).push_back(objectArray.size-1);
			}
			i++;
			break;
		}

		case S_JSON_TOKEN_LEFT_BRACKET:
		{

			if(waitingOnValue)
			{
				waitingOnValue = false; // object was created in S_JSON_TOKEN_MEMBER case below.
			}
			else
			{
				sJsonObject *newObject = (sJsonObject*)S_JSON_ALLOC(sizeof(sJsonObject));
				newObject->type = S_JSON_TYPE_ARRAY;
				newObject->_internal = (sJsonVector_<int>*)S_JSON_ALLOC(sizeof(sJsonVector_<int>));
				new (newObject->_internal) sJsonVector_<int>();
				objectArray.push_back(newObject);
				parentIDStack.push(objectArray.size-1);
				(*(sJsonVector_<int>*)(parent->_internal)).push_back(objectArray.size-1);	
			}
			i++;
			break;
		}

		case S_JSON_TOKEN_RIGHT_BRACE:
		{
			parentIDStack.pop();
			i++;
			break;
		}

		case S_JSON_TOKEN_RIGHT_BRACKET:
		{
			parentIDStack.pop();
			i++;
			break;
		}

		case S_JSON_TOKEN_MEMBER:
		{

			sJsonObject* newObject = (sJsonObject*)S_JSON_ALLOC(sizeof(sJsonObject));
			objectArray.push_back(newObject);
			newObject->_internal = (sJsonVector_<int>*)S_JSON_ALLOC(sizeof(sJsonVector_<int>));
			new (newObject->_internal) sJsonVector_<int>();
			parentIDStack.push(objectArray.size-1);
			(*(sJsonVector_<int>*)(parent->_internal)).push_back(objectArray.size-1);
			memcpy(newObject->name, (*tokens)[i].value.data, (*tokens)[i].value.size_in_bytes());

			// look ahead to 2 tokens to look at type (skipping over ':' )
			sJsonToken_ valueToken = (*tokens)[i + 2];
			sJsonToken_Type valueType = valueToken.type;
			if      (valueType == S_JSON_TOKEN_LEFT_BRACKET) newObject->type = S_JSON_TYPE_ARRAY;
			else if (valueType == S_JSON_TOKEN_LEFT_BRACE)   newObject->type = S_JSON_TYPE_OBJECT;
			else if (valueType == S_JSON_TOKEN_STRING)       newObject->type = S_JSON_TYPE_STRING;
			else if (valueType == S_JSON_TOKEN_PRIMITIVE)
			{
				if(valueToken.value[0] == 't')      newObject->type = S_JSON_TYPE_BOOL;
				else if(valueToken.value[0] == 'f') newObject->type = S_JSON_TYPE_BOOL;
				else if(valueToken.value[0] == 'n') newObject->type = S_JSON_TYPE_NULL;
				else                                newObject->type = S_JSON_TYPE_NUMBER;
			}
			i++;
			waitingOnValue = true;
			break;
		}

		case S_JSON_TOKEN_STRING:
		{
			if(waitingOnValue)
			{
				parent->value = (*tokens)[i].value.data;
				waitingOnValue=false;
				parentIDStack.pop();
			}
			else // in array
			{
				sJsonObject *newObject = (sJsonObject*)S_JSON_ALLOC(sizeof(sJsonObject));
				newObject->type = S_JSON_TYPE_STRING;
				newObject->_internal = (sJsonVector_<int>*)S_JSON_ALLOC(sizeof(sJsonVector_<int>));
				new (newObject->_internal) sJsonVector_<int>();
				objectArray.push_back(newObject);
				newObject->value = (*tokens)[i].value.data;
				(*(sJsonVector_<int>*)(parent->_internal)).push_back(objectArray.size-1);	
			}
			i++;
			break;
		}

		case S_JSON_TOKEN_PRIMITIVE:
		{
			if(waitingOnValue)
			{
				parent->value = (*tokens)[i].value.data;
				waitingOnValue=false;
				parentIDStack.pop();
			}
			else // in array
			{
				sJsonObject *newObject = (sJsonObject*)S_JSON_ALLOC(sizeof(sJsonObject));
				if((*tokens)[i].value.data[0] == 't')      newObject->type = S_JSON_TYPE_BOOL;
				else if((*tokens)[i].value.data[0] == 'f') newObject->type = S_JSON_TYPE_BOOL;
				else if((*tokens)[i].value.data[0] == 'n') newObject->type = S_JSON_TYPE_NULL;
				else                                       newObject->type = S_JSON_TYPE_NUMBER;
				newObject->_internal = (sJsonVector_<int>*)S_JSON_ALLOC(sizeof(sJsonVector_<int>));
				new (newObject->_internal) sJsonVector_<int>();
				objectArray.push_back(newObject);
				newObject->value = (*tokens)[i].value.data;
				(*(sJsonVector_<int>*)(parent->_internal)).push_back(objectArray.size-1);	
			}
			i++;
			break;
		}

		default:
			i++;
			break;
		}

	}
	parentIDStack.pop();
	_update_children_pointers(rootObject, &objectArray);
	objectArray.clear();
	parentIDStack.data.clear();
	return rootObject;
}

static void
_free_json(sJsonObject* object)
{
	for(int i = 0; i < object->childCount; i++)
	{
		_free_json(&object->children[i]);
	}
	if(object->childCount > 0)
		S_JSON_FREE(object->children);
}

void
Semper::free_json(sJsonObject** rootObjectPtr)
{
	sJsonObject* rootObject = *rootObjectPtr;
	_free_json(rootObject);
	S_JSON_FREE(rootObject);
	rootObjectPtr = nullptr;
}

#endif
