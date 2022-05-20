/*
   sIni, v0.2 (WIP)
   * no dependencies
   * simple
   Do this:
	  #define SEMPER_INI_IMPLEMENTATION
   before you include this file in *one* C or C++ file to create the implementation.
   // i.e. it should look like this:
   #include ...
   #include ...
   #include ...
   #define SEMPER_INI_IMPLEMENTATION
   #include "sIni.h"
*/

#ifndef SEMPER_INI_H
#define SEMPER_INI_H

#ifndef S_INI_MAX_NAME_LENGTH
#define S_INI_MAX_NAME_LENGTH 256
#endif

#ifndef S_INI_MAX_LINE_LENGTH
#define S_INI_MAX_LINE_LENGTH 1024
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h> // strncpy()

#ifndef S_INI_ASSERT
#include <assert.h>
#define S_INI_ASSERT(x) assert(x)
#endif

#ifndef S_INI_ALLOC
#define S_INI_ALLOC(x) malloc(x)
#endif

#ifndef S_INI_FREE
#define S_INI_FREE(x) free(x)
#endif

//-----------------------------------------------------------------------------
// [SECTION] Forward declarations and basic types
//-----------------------------------------------------------------------------
struct sIniFile;
struct sIniSection;
struct sIniEntry;
typedef int sIniType; // enum -> sIniType_

//-----------------------------------------------------------------------------
// [SECTION] Semper end-user API functions
//-----------------------------------------------------------------------------
namespace Semper
{
    sIniFile load_ini_file   (const char* file);
    void     save_ini_file   (const char* file, sIniFile* iniFile);
    void     cleanup_ini_file(sIniFile* iniFile);
}

//-----------------------------------------------------------------------------
// [SECTION] Enums
//-----------------------------------------------------------------------------

enum sIniType_
{
	S_INI_TYPE_NONE,
	S_INI_TYPE_STRING,
	S_INI_TYPE_NUMBER,
	S_INI_TYPE_BOOL,
    S_INI_TYPE_STRING_ARRAY,
    S_INI_TYPE_NUMBER_ARRAY,
    S_INI_TYPE_BOOL_ARRAY
};

//-----------------------------------------------------------------------------
// [SECTION] Structs
//-----------------------------------------------------------------------------

struct sIniEntry
{
    char     name[S_INI_MAX_NAME_LENGTH];
    sIniType type;

    union 
    {
        double numberValue;
        bool   boolValue;
        char*  stringValue;
        int    length;
    };

    union
    {
        double* numberArrayValue;
        bool*   boolArrayValue;
        char**  stringArrayValue;
        void*   _data;
    };

 	// cast values
	inline int      as_int()     { S_INI_ASSERT(type == S_INI_TYPE_NUMBER); return (int)numberValue;}
	inline unsigned as_unsigned(){ S_INI_ASSERT(type == S_INI_TYPE_NUMBER); return (unsigned)numberValue;}
	inline float    as_float()   { S_INI_ASSERT(type == S_INI_TYPE_NUMBER); return (float)numberValue;}
	inline double   as_double()  { S_INI_ASSERT(type == S_INI_TYPE_NUMBER); return numberValue;}
	inline char*    as_string()  { S_INI_ASSERT(type == S_INI_TYPE_STRING); return stringValue;}
	inline bool    	as_bool()    { S_INI_ASSERT(type == S_INI_TYPE_BOOL);   return boolValue;} 

	// cast array values
	inline void as_int_array     (int*      out, int size) { S_INI_ASSERT(type == S_INI_TYPE_NUMBER_ARRAY); S_INI_ASSERT(out); if(size > length) size = length; for(int i = 0; i < size; i++) out[i] = (int)numberArrayValue[i];}
	inline void as_unsigned_array(unsigned* out, int size) { S_INI_ASSERT(type == S_INI_TYPE_NUMBER_ARRAY); S_INI_ASSERT(out); if(size > length) size = length; for(int i = 0; i < size; i++) out[i] = (unsigned)numberArrayValue[i];}
	inline void as_float_array   (float*    out, int size) { S_INI_ASSERT(type == S_INI_TYPE_NUMBER_ARRAY); S_INI_ASSERT(out); if(size > length) size = length; for(int i = 0; i < size; i++) out[i] = (float)numberArrayValue[i];}
	inline void as_double_array  (double*   out, int size) { S_INI_ASSERT(type == S_INI_TYPE_NUMBER_ARRAY); S_INI_ASSERT(out); if(size > length) size = length; for(int i = 0; i < size; i++) out[i] = numberArrayValue[i];}
	inline void as_bool_array    (bool*     out, int size) { S_INI_ASSERT(type == S_INI_TYPE_BOOL_ARRAY);   S_INI_ASSERT(out); if(size > length) size = length; for(int i = 0; i < size; i++) out[i] = (bool)boolArrayValue[i];}
	inline void as_string_array  (char**    out, int size) { S_INI_ASSERT(type == S_INI_TYPE_STRING_ARRAY); S_INI_ASSERT(out); if(size > length) size = length; for(int i = 0; i < size; i++) out[i] = stringArrayValue[i];}
   
};

struct sIniSection
{
    char       section[S_INI_MAX_NAME_LENGTH];
    char       subsection[S_INI_MAX_NAME_LENGTH];
    sIniEntry* entries;
    size_t     entrySize;
    size_t     entryCapacity;

    void       initialize(const char* section, size_t entryCapacity=0);
    void       initialize(const char* name, const char* sub, size_t capacity=0);
    sIniEntry* get_entry(const char* name);
    bool       has_entry(const char* name);

    // single values
    sIniSection* add_entry(const char* name, float       value);
    sIniSection* add_entry(const char* name, int         value);
    sIniSection* add_entry(const char* name, unsigned    value);
    sIniSection* add_entry(const char* name, double      value);
    sIniSection* add_entry(const char* name, bool        value);
    sIniSection* add_entry(const char* name, const char* value);

    // arrays
    sIniSection* add_entry(const char* name, int*         value, int size);
    sIniSection* add_entry(const char* name, float*       value, int size);
    sIniSection* add_entry(const char* name, double*      value, int size);
    sIniSection* add_entry(const char* name, bool*        value, int size);
    sIniSection* add_entry(const char* name, const char** value, int size);

	// retrieve and cast values
	inline const char* get_string_entry  (const char* entry, const char* defaultValue=0)     { auto m = get_entry(entry); return m==0       ? defaultValue : m->as_string();}
	inline int         get_int_entry     (const char* entry, int         defaultValue=0)     { auto m = get_entry(entry); return m==nullptr ? defaultValue : m->as_int();}
	inline unsigned    get_unsigned_entry(const char* entry, unsigned    defaultValue=0u)    { auto m = get_entry(entry); return m==nullptr ? defaultValue : m->as_unsigned();}
	inline float       get_float_entry   (const char* entry, float       defaultValue=0.0f)  { auto m = get_entry(entry); return m==nullptr ? defaultValue : m->as_float();}
	inline double      get_double_entry  (const char* entry, double      defaultValue=0.0)   { auto m = get_entry(entry); return m==nullptr ? defaultValue : m->as_double();}
	inline bool        get_bool_entry    (const char* entry, bool        defaultValue=false) { auto m = get_entry(entry); return m==nullptr ? defaultValue : m->as_bool();}

	// retrieve and cast array values
	inline void get_int_array_entry     (const char* member, int*          out, int size){ auto m = get_entry(member); if(m) m->as_int_array(out, size);}
	inline void get_unsigned_array_entry(const char* member, unsigned int* out, int size){ auto m = get_entry(member); if(m) m->as_unsigned_array(out, size);}
	inline void get_float_array_entry   (const char* member, float*        out, int size){ auto m = get_entry(member); if(m) m->as_float_array(out, size);}
	inline void get_double_array_entry  (const char* member, double*       out, int size){ auto m = get_entry(member); if(m) m->as_double_array(out, size);}
	inline void get_bool_array_entry    (const char* member, bool*         out, int size){ auto m = get_entry(member); if(m) m->as_bool_array(out, size);}
	inline void get_string_array_entry  (const char* member, char**        out, int size){ auto m = get_entry(member); if(m) m->as_string_array(out, size);}

	inline sIniEntry* operator[](const char* member) { auto m = get_entry(member); S_INI_ASSERT(m!=nullptr); return m;}
};

struct sIniFile
{
    sIniSection* sections;
    size_t       sectionCount;
    size_t       _capacity;
    
    void         initialize(size_t sectionCapacity=0);
    sIniSection* get_section(const char* name, const char* sub=nullptr);
    bool         has_section(const char* name, const char* sub=nullptr);
    sIniSection* add_section(const char* name, const char* sub, size_t capacity=0);
    sIniSection* add_section(const char* name, size_t capacity=0);

};

#endif // SEMPER_INI_H

#ifdef SEMPER_INI_IMPLEMENTATION

static void _grow_ini_file(sIniFile* file)
{
    if(file->_capacity == 0)
    {
        file->_capacity = 16;
        file->sections = (sIniSection*)S_INI_ALLOC(sizeof(sIniSection)*file->_capacity);
        return;
    }
    sIniSection* oldMemory = file->sections;
    file->sections = (sIniSection*)S_INI_ALLOC(sizeof(sIniSection)*file->_capacity*2);
    memcpy(file->sections, oldMemory, sizeof(sIniSection)*file->_capacity);
    S_INI_FREE(oldMemory);
    file->_capacity*=2;
}

sIniSection*
sIniFile::get_section(const char* name, const char* sub)
{
    if(sub)
    {
        for(int i = 0; i < sectionCount; i++)
        { 
            if(strncmp(sections[i].section, name, S_INI_MAX_NAME_LENGTH) == 0 && strncmp(sections[i].subsection, sub, S_INI_MAX_NAME_LENGTH) == 0) 
                return &sections[i];
        }
    }
    else
    {
        for(int i = 0; i < sectionCount; i++)
        { 
            if(strncmp(sections[i].section, name, S_INI_MAX_NAME_LENGTH) == 0 && sections[i].subsection[0] == 0) 
                return &sections[i];
        }
    } 
    return nullptr;
}

bool        
sIniFile::has_section(const char* name, const char* sub)
{
    return get_section(name, sub) == nullptr ? false : true;
}

void
sIniFile::initialize(size_t sectionCapacity)
{
    sectionCount = 0u;
    _capacity = sectionCapacity;
    if(sectionCapacity == 0) _capacity = 16;
    sections = (sIniSection*)S_INI_ALLOC(sizeof(sIniSection)*_capacity);       
}

sIniSection*
sIniFile::add_section(const char* name, const char* sub, size_t capacity)
{
    if(sectionCount >= _capacity) _grow_ini_file(this);
    sections[sectionCount].initialize(name, sub, capacity);
    sectionCount++;
    return &sections[sectionCount-1];
}

sIniSection*
sIniFile::add_section(const char* name, size_t capacity)
{
    return add_section(name, nullptr, capacity);
}

void
sIniSection::initialize(const char* name, const char* sub, size_t capacity)
{  
    if(capacity == 0) capacity = 16;
    section[0] = '?'; subsection[0] = entrySize = 0;
    if(name)   strncpy(section, name, S_INI_MAX_NAME_LENGTH);
    if(sub)    strncpy(subsection, sub, S_INI_MAX_NAME_LENGTH);
    entryCapacity = capacity; entries = (sIniEntry*)S_INI_ALLOC(sizeof(sIniEntry)*entryCapacity);
}

void
sIniSection::initialize(const char* section, size_t entryCapacity) 
{ 
    initialize(section, nullptr, entryCapacity);
}

static void
_cleanup_ini_section(sIniSection* section)
{
    for(int i = 0; i < section->entrySize; i++)
    {
        sIniEntry& entry = section->entries[i];
        switch(entry.type)
        {
            
            case S_INI_TYPE_NUMBER:  entry._data=nullptr; entry.numberValue=0.0; break;
            case S_INI_TYPE_BOOL:    entry._data=nullptr; entry.boolValue=false; break;
            case S_INI_TYPE_NUMBER_ARRAY:
            case S_INI_TYPE_BOOL_ARRAY: entry.length=0; S_INI_FREE(entry._data); entry._data = nullptr; break;
            case S_INI_TYPE_STRING: S_INI_FREE(entry.stringValue); entry._data=nullptr; entry.stringValue=nullptr; break;
            case S_INI_TYPE_STRING_ARRAY:
            {
                for(int j = 0; j < entry.length; j++)
                {
                    S_INI_FREE(entry.stringArrayValue[j]);
                    entry.stringArrayValue[j] = nullptr;
                }
                S_INI_FREE(entry.stringArrayValue);
                entry.stringArrayValue = nullptr;
                entry.length=0;
                break;
            }
            default: S_INI_ASSERT(false);
        }
        entry.type = S_INI_TYPE_NONE;
    }

    S_INI_FREE(section->entries);
    section->entries = nullptr;
    section->entrySize=0;
    section->entryCapacity=0;
    for(int i = 0; i < S_INI_MAX_NAME_LENGTH; i++)
    {
        section->section[i]=0;
        section->subsection[i]=0;
    }
}

static void _grow_ini_section(sIniSection* section)
{
    if(section->entryCapacity == 0)
    {
        section->entryCapacity = 16;
        section->entries = (sIniEntry*)S_INI_ALLOC(sizeof(sIniEntry)*section->entryCapacity);
        return;
    }
    sIniEntry* oldMemory = section->entries;
    section->entries = (sIniEntry*)S_INI_ALLOC(sizeof(sIniEntry)*section->entryCapacity*2);
    memcpy(section->entries, oldMemory, sizeof(sIniEntry)*section->entryCapacity);
    S_INI_FREE(oldMemory);
    section->entryCapacity*=2;
}

sIniSection*      
sIniSection::add_entry(const char* name, float value)    
{ 
    return add_entry(name, (double)value);
}

sIniSection*      
sIniSection::add_entry(const char* name, int value)      
{ 
    return add_entry(name, (double)value);
}

sIniSection*      
sIniSection::add_entry(const char* name, unsigned value) 
{ 
    return add_entry(name, (double)value);
}

sIniEntry*
sIniSection::get_entry(const char* name)                 
{ 
    for(int i = 0; i < entrySize; i++)
    { 
        if(strncmp(entries[i].name, name, S_INI_MAX_NAME_LENGTH) == 0) 
        return &entries[i];
    } 
    return nullptr;
}

bool      
sIniSection::has_entry(const char* name)                 
{ 
    return get_entry(name) == nullptr ? false : true;
}

sIniSection*
sIniSection::add_entry(const char* name, double value)
{
    if(entrySize >= entryCapacity) _grow_ini_section(this);
    strncpy(entries[entrySize].name, name, S_INI_MAX_NAME_LENGTH);
    entries[entrySize].numberValue = value;
    entries[entrySize].numberArrayValue = nullptr;
    entries[entrySize].type = S_INI_TYPE_NUMBER;
    entrySize++;
    return this;
}

sIniSection* 
sIniSection::add_entry(const char* name, bool value)
{
    if(entrySize >= entryCapacity) _grow_ini_section(this);
    strncpy(entries[entrySize].name, name, S_INI_MAX_NAME_LENGTH);
    entries[entrySize].boolValue = value;
    entries[entrySize].boolArrayValue = nullptr;
    entries[entrySize].type = S_INI_TYPE_BOOL;
    entrySize++;
    return this;
}

sIniSection* 
sIniSection::add_entry(const char* name, const char* value)
{
    if(entrySize >= entryCapacity) _grow_ini_section(this);
    strncpy(entries[entrySize].name, name, S_INI_MAX_NAME_LENGTH);
    entries[entrySize].stringValue = (char*)S_INI_ALLOC(sizeof(char)*(strlen(value)+1));
    strcpy(entries[entrySize].stringValue, value);
    entries[entrySize].stringArrayValue = nullptr;
    entries[entrySize].type = S_INI_TYPE_STRING;
    entrySize++;
    return this;
}

sIniSection* 
sIniSection::add_entry(const char* name, int* value, int size)
{
    if(entrySize >= entryCapacity) _grow_ini_section(this);
    strncpy(entries[entrySize].name, name, S_INI_MAX_NAME_LENGTH);
    entries[entrySize].numberArrayValue = (double*)S_INI_ALLOC(size*sizeof(double));
    for(int i = 0; i < size; i++) entries[entrySize].numberArrayValue[i] = (double)value[i];
    entries[entrySize].length = size;
    entries[entrySize].type = S_INI_TYPE_NUMBER_ARRAY;
    entrySize++;
    return this;
}

sIniSection*
sIniSection::add_entry(const char* name, float* value, int size)
{
    if(entrySize >= entryCapacity) _grow_ini_section(this);
    strncpy(entries[entrySize].name, name, S_INI_MAX_NAME_LENGTH);
    entries[entrySize].numberArrayValue = (double*)S_INI_ALLOC(size*sizeof(double));
    for(int i = 0; i < size; i++) entries[entrySize].numberArrayValue[i] = (double)value[i];
    entries[entrySize].length = size;
    entries[entrySize].type = S_INI_TYPE_NUMBER_ARRAY;
    entrySize++;
    return this;
}

sIniSection*
sIniSection::add_entry(const char* name, double* value, int size)
{
    if(entrySize >= entryCapacity) _grow_ini_section(this);
    strncpy(entries[entrySize].name, name, S_INI_MAX_NAME_LENGTH);
    entries[entrySize].numberArrayValue = (double*)S_INI_ALLOC(size*sizeof(double));
    memcpy(entries[entrySize].numberArrayValue, value, size*sizeof(double));
    entries[entrySize].length = size;
    entries[entrySize].type = S_INI_TYPE_NUMBER_ARRAY;
    entrySize++;
    return this;
}

sIniSection*
sIniSection::add_entry(const char* name, bool* value, int size)
{
    if(entrySize >= entryCapacity) _grow_ini_section(this);
    strncpy(entries[entrySize].name, name, S_INI_MAX_NAME_LENGTH);
    entries[entrySize].boolArrayValue = (bool*)S_INI_ALLOC(size*sizeof(bool));
    memcpy(entries[entrySize].boolArrayValue, value, size*sizeof(bool));
    entries[entrySize].length = size;
    entries[entrySize].type = S_INI_TYPE_BOOL_ARRAY;
    entrySize++;
    return this;
}

sIniSection*
sIniSection::add_entry(const char* name, const char** value, int size)
{
    if(entrySize >= entryCapacity) _grow_ini_section(this);
    strncpy(entries[entrySize].name, name, S_INI_MAX_NAME_LENGTH);
    entries[entrySize].stringArrayValue = (char**)S_INI_ALLOC(size*sizeof(char*));
    for(int i = 0; i < size; i++)
    {
        entries[entrySize].stringArrayValue[i] = (char*)S_INI_ALLOC((strlen(value[i])+1)*sizeof(char*));
        
        strcpy(entries[entrySize].stringArrayValue[i], value[i]);
        auto blah = entries[entrySize].stringArrayValue[i];
        int a = 5;
    }
    entries[entrySize].length = size;
    entries[entrySize].type = S_INI_TYPE_STRING_ARRAY;
    entrySize++;
    return this;
}

sIniFile
Semper::load_ini_file(const char* file)
{
    sIniFile result;
    result.sectionCount = 0u;
    result.sections = nullptr;
    result._capacity = 0u;

	FILE* dataFile = fopen(file, "r");

	if (dataFile == nullptr)
	{
        return result;
	}

    int currentSize = -1;
    char nameBuffer[S_INI_MAX_NAME_LENGTH];
    char lineBuffer[S_INI_MAX_LINE_LENGTH];
    for(int i = 0; i < S_INI_MAX_LINE_LENGTH; i++) lineBuffer[i]=0;

    // find section count

    while(fgets(lineBuffer, S_INI_MAX_LINE_LENGTH, dataFile))
    {
        char currentChar = 0;
        for(int i = 0; i < S_INI_MAX_LINE_LENGTH; i++)
        {
            currentChar = lineBuffer[i];
            if(currentChar == '[')
            {
                result._capacity++;
                break;
            }
            if(currentChar == ' ' || currentChar == '\t')
            {
                continue;
            }
            else if(currentChar == '#' || currentChar == ';' || currentChar == '\n' || currentChar == 0)
            {
                break;
            }       
        }      
    }  

    fseek(dataFile, 0, SEEK_SET);

    result.sections = (sIniSection*)S_INI_ALLOC(sizeof(sIniSection)*result._capacity);

    // preallocate entries
    {
        int currentSectionIndex = -1;
        int currentEntryCount = 0;
        while(fgets(lineBuffer, S_INI_MAX_LINE_LENGTH, dataFile))
        {
            char currentChar = 0;
            for(int i = 0; i < S_INI_MAX_LINE_LENGTH; i++)
            {
                currentChar = lineBuffer[i];
                if(currentChar == ' ' || currentChar == '\t')
                {
                    continue;
                }
                break;
            }

            if(currentChar == 0 || currentChar == '\n')
                continue;

            if(currentChar == '[') 
            {
                if(currentSectionIndex != -1)
                {
                    result.sections[currentSectionIndex].entries = (sIniEntry*)S_INI_ALLOC(sizeof(sIniEntry)*currentEntryCount);
                    result.sections[currentSectionIndex].entryCapacity = currentEntryCount;
                    result.sections[currentSectionIndex].entrySize = 0;
                    result.sections[currentSectionIndex].section[0] = 0;
                    result.sections[currentSectionIndex].subsection[0] = 0;
                }
                currentSectionIndex++;
                currentEntryCount = 0;    
                continue;
            }
            else if(currentChar == '#' || currentChar == ';' || currentChar == '\n' || currentChar == 0) 
            {
                continue;
            }
            currentEntryCount++;
        }
        result.sections[currentSectionIndex].entries = (sIniEntry*)S_INI_ALLOC(sizeof(sIniEntry)*currentEntryCount);
        result.sections[currentSectionIndex].entryCapacity = currentEntryCount;
        result.sections[currentSectionIndex].entrySize = 0;
        result.sections[currentSectionIndex].section[0] = 0;
        result.sections[currentSectionIndex].subsection[0] = 0;
    }

    fseek(dataFile, 0, SEEK_SET);

    while(fgets(lineBuffer, S_INI_MAX_LINE_LENGTH, dataFile))
    {
        int currentNamePos = 0;
        int currentPos = 0;
        int currentSectionNamePos = 0;
        char currentChar = lineBuffer[currentPos];

        // find first actual character
        bool inComment = false;
        for(int i = 0; i < S_INI_MAX_LINE_LENGTH; i++)
        {
            if(currentChar == ' ' || currentChar == '\t')
            {
                currentPos++;
                currentChar = lineBuffer[currentPos];
                continue;
            }
            else if(currentChar == '#' || currentChar == ';' || currentChar == '\n' || currentChar == 0)
            {
                inComment = true;
                break;
            }           
            break;
        }
        if(inComment) continue;
    
        // section header stuff
        if(currentChar == '[')
        {
            currentSize++;
            bool inSection = true;
            bool inSubSection = false;
            currentPos++;
            currentChar = lineBuffer[currentPos];

            for(int i = 0; i < S_INI_MAX_LINE_LENGTH-currentPos; i++)
            {
                if(currentChar == ']' && inSection)
                {
                    result.sections[currentSize].section[currentSectionNamePos] = 0;
                    currentSectionNamePos=0;
                    inSection=false;
                    currentPos++;
                    currentChar = lineBuffer[currentPos];
                    
                    if(currentChar == '[') 
                    {
                        currentPos++;
                        currentChar = lineBuffer[currentPos];
                        inSubSection=true;
                    }
                    continue;
                }
                if(currentChar == ']' && inSubSection)
                {
                    result.sections[currentSize].subsection[currentSectionNamePos] = 0;
                    currentSectionNamePos=0;
                    inSubSection=false;
                    currentPos++;
                    currentChar = lineBuffer[currentPos];
                    continue;
                }

                if(inSection)
                {
                    result.sections[currentSize].section[currentSectionNamePos] = currentChar;
                    currentSectionNamePos++;
                    currentPos++;
                    currentChar = lineBuffer[currentPos];
                    continue;
                }
                if(inSubSection)
                {
                    result.sections[currentSize].subsection[currentSectionNamePos] = currentChar;
                    currentSectionNamePos++;
                    currentPos++;
                    currentChar = lineBuffer[currentPos];
                    continue;
                }
                break;

            }    

            continue;
        }

        else
        {
            while (currentChar != '=')
            {
                nameBuffer[currentNamePos] = currentChar;
                currentNamePos++;
                currentPos++;
                currentChar = lineBuffer[currentPos];        
            }
            currentPos++;
            currentChar = lineBuffer[currentPos];  

            if(nameBuffer[currentNamePos-1] == ' ') nameBuffer[currentNamePos-1] = 0;
            strncpy(result.sections[currentSize].entries[result.sections[currentSize].entrySize].name, nameBuffer, S_INI_MAX_NAME_LENGTH);

            int a = 5;
        }


        // find first character
        for(int i = 0; i < S_INI_MAX_LINE_LENGTH; i++)
        {
            if(currentChar == ' ' || currentChar == '\t')
            {
                currentPos++;
                currentChar = lineBuffer[currentPos];
                continue;
            }  
            break;
        }

        if(currentChar == 't' || currentChar == 'f')
        {

            // check if array
            int valueCount = 1;
            for(int i = 0; i < S_INI_MAX_LINE_LENGTH-currentPos; i++)
            {
                char currentSubChar = lineBuffer[currentPos+i];
                if(currentSubChar == ',')
                {
                    valueCount++;
                }
                if(currentSubChar == '\n' || currentSubChar == 0)
                    break;
            }

            if(valueCount == 1)
            {
                result.sections[currentSize].add_entry(nameBuffer, currentChar == 't');
            }
            else
            {
                result.sections[currentSize].entries[result.sections[currentSize].entrySize].type = S_INI_TYPE_BOOL_ARRAY;
                result.sections[currentSize].entries[result.sections[currentSize].entrySize].length = valueCount;
                result.sections[currentSize].entries[result.sections[currentSize].entrySize].boolArrayValue = (bool*)S_INI_ALLOC(valueCount*sizeof(bool));
                char currentSubChar = lineBuffer[currentPos];
                int currentValuePos = 0;
                while(currentSubChar != 0)
                {
                    if(currentSubChar == 't')
                    {
                       result.sections[currentSize].entries[result.sections[currentSize].entrySize].boolArrayValue[currentValuePos] = true;
                       currentValuePos++;
                    }
                    if(currentSubChar == 'f')
                    {
                       result.sections[currentSize].entries[result.sections[currentSize].entrySize].boolArrayValue[currentValuePos] = false;
                       currentValuePos++;
                    }
                    currentPos++;
                    currentSubChar = lineBuffer[currentPos];
                }

                result.sections[currentSize].entrySize++;
                
            }
        }
        else if(currentChar == '"')
        {
            // check if array
            int valueCount = 1;
            bool leftQuoteFound = false;
            for(int i = 0; i < S_INI_MAX_LINE_LENGTH-currentPos; i++)
            {
                char currentSubChar = lineBuffer[currentPos+i];
                if(currentSubChar == ',')
                {
                    valueCount++;
                }
                if(currentSubChar == '\n' || currentSubChar == 0)
                    break;
                if(currentSubChar == '"' && leftQuoteFound) {leftQuoteFound=false;lineBuffer[currentPos+i]=0;}
                else if(currentSubChar == '"') leftQuoteFound=true;
            }

            if(valueCount == 1)
            {
                result.sections[currentSize].add_entry(nameBuffer, &lineBuffer[currentPos+1]);
            }
            else
            {
                result.sections[currentSize].entries[result.sections[currentSize].entrySize].type = S_INI_TYPE_STRING_ARRAY;
                result.sections[currentSize].entries[result.sections[currentSize].entrySize].length = valueCount;
                result.sections[currentSize].entries[result.sections[currentSize].entrySize].stringArrayValue = (char**)S_INI_ALLOC(valueCount*sizeof(char*));

                currentPos++;
                for(int i = 0; i < valueCount; i++)
                {
                    auto lengthOfCurrentString = strlen(&lineBuffer[currentPos]);
                    result.sections[currentSize].entries[result.sections[currentSize].entrySize].stringArrayValue[i] = (char*)S_INI_ALLOC((lengthOfCurrentString+1)*sizeof(char));
                    strcpy(result.sections[currentSize].entries[result.sections[currentSize].entrySize].stringArrayValue[i], &lineBuffer[currentPos]);
                    currentPos+=lengthOfCurrentString+3;
                    // TODO: FIX
                }

                result.sections[currentSize].entrySize++;
                
            }

        }
        else // number
        {
            // check if array
            int valueCount = 1;
            for(int i = 0; i < S_INI_MAX_LINE_LENGTH-currentPos; i++)
            {
                char currentSubChar = lineBuffer[currentPos+i];
                if(currentSubChar == ',')
                {
                    valueCount++;
                }
                if(currentSubChar == '\n' || currentSubChar == 0)
                    break;
            }

            if(valueCount == 1)
            {
                result.sections[currentSize].add_entry(nameBuffer, strtod(&lineBuffer[currentPos], nullptr));
            }
            else
            {
                result.sections[currentSize].entries[result.sections[currentSize].entrySize].type = S_INI_TYPE_NUMBER_ARRAY;
                result.sections[currentSize].entries[result.sections[currentSize].entrySize].length = valueCount;
                result.sections[currentSize].entries[result.sections[currentSize].entrySize].numberArrayValue = (double*)S_INI_ALLOC(valueCount*sizeof(double));
                char* currentNumberOffset = &lineBuffer[currentPos];
                for(int i = 0; i < valueCount; i++)
                {
                  result.sections[currentSize].entries[result.sections[currentSize].entrySize].numberArrayValue[i]  = strtod(currentNumberOffset, &currentNumberOffset);
                  currentNumberOffset++;
                }
                result.sections[currentSize].entrySize++;
            }
        }

    }
    result.sectionCount = (size_t)currentSize+1;
    fclose(dataFile);
    return result;
}

void
Semper::save_ini_file(const char* file, sIniFile* iniFile)
{
    FILE* dataFile = fopen(file, "w");

    if (dataFile == nullptr)
	{
		S_INI_ASSERT(false && "File not found.");
		return;
	}
    for(size_t i = 0; i < iniFile->sectionCount; i++)
    {
        sIniSection& section = iniFile->sections[i];
        if(section.subsection[0] != 0) fprintf(dataFile, "[%s][%s]\n", section.section, section.subsection);
        else                           fprintf(dataFile, "[%s]\n", section.section);

        for(size_t j = 0; j < section.entrySize; j++)
        {
            sIniEntry& entry = section.entries[j];
            fprintf(dataFile, "%s = ", entry.name);
            switch(entry.type)
            {
                case S_INI_TYPE_STRING: fprintf(dataFile, "\"%s\"\n", entry.stringValue); break;
                case S_INI_TYPE_NUMBER: fprintf(dataFile, "%lf\n", entry.numberValue); break;
                case S_INI_TYPE_BOOL:   fprintf(dataFile, "%s\n", entry.boolValue ? "true":"false"); break;
                case S_INI_TYPE_NUMBER_ARRAY:
                {
                    for(int k = 0; k < entry.length-1; k++)
                    {
                        fprintf(dataFile, "%lf,", entry.numberArrayValue[k]);
                    }
                    fprintf(dataFile, "%lf\n", entry.numberArrayValue[entry.length-1]); break;
                    break;
                }
                case S_INI_TYPE_BOOL_ARRAY:
                {
                    for(int k = 0; k < entry.length-1; k++)
                    {
                        fprintf(dataFile, "%s,", entry.boolArrayValue[k] ? "true":"false");
                    }
                    fprintf(dataFile, "%s\n", entry.boolArrayValue[entry.length-1] ? "true":"false"); break;
                    break;
                }
                case S_INI_TYPE_STRING_ARRAY:
                {
                    for(int k = 0; k < entry.length-1; k++)
                    {
                        fprintf(dataFile, "\"%s\",", entry.stringArrayValue[k]);
                    }
                    fprintf(dataFile, "\"%s\"\n", entry.stringArrayValue[entry.length-1]); break;
                    break;
                }

                default: S_INI_ASSERT(false);
            }    
        }
        fprintf(dataFile, "\n");
    }

    fclose(dataFile);
}

void
Semper::cleanup_ini_file(sIniFile* iniFile)
{
    S_INI_ASSERT(iniFile);
    for(int i = 0; i < iniFile->sectionCount; i++)
        _cleanup_ini_section(&iniFile->sections[i]);
    S_INI_FREE(iniFile->sections);
    iniFile->sections = nullptr;
    iniFile->sectionCount = 0u;
    iniFile->_capacity = 0u;
}

#endif
