/*
   sStlLoader, v0.0.1 (WIP)
   * no dependencies
   Do this:
	  #define SEMPER_STLLOAD_IMPLEMENTATION
   before you include this file in *one* C or C++ file to create the implementation.
   // i.e. it should look like this:
   #include ...
   #include ...
   #include ...
   #define SEMPER_STLLOAD_IMPLEMENTATION
   #include "sStlLoader.h"
*/

#ifndef SEMPER_STLLOAD_H
#define SEMPER_STLLOAD_H

#ifndef S_STL_ALLOC
#define S_STL_ALLOC(x) malloc(x)
#endif

#ifndef S_STL_FREE
#define S_STL_FREE(x) free(x)
#endif

#ifndef S_STL_ASSERT
#include <assert.h>
#define S_STL_ASSERT(_EXPR) assert(_EXPR)
#endif

//-----------------------------------------------------------------------------
// [SECTION] Forward declarations and basic types
//-----------------------------------------------------------------------------

struct sStlOptions;
struct sStlModel;

//-----------------------------------------------------------------------------
// [SECTION] Semper end-user API functions
//-----------------------------------------------------------------------------

namespace Semper
{
    sStlModel load_stl(const char* file, sStlOptions options);
    void      cleanup_stl(sStlModel& model);
}

//-----------------------------------------------------------------------------
// [SECTION] Structs
//-----------------------------------------------------------------------------

struct sStlModel
{
    char*    name;
    float*   data;
    unsigned count;
    float    xLimits[2];    
    float    yLimits[2];    
    float    zLimits[2];    
};

struct sStlOptions
{
    bool includeNormals;
};

#endif

//-----------------------------------------------------------------------------
// [SECTION] Implementation
//-----------------------------------------------------------------------------
#ifdef SEMPER_STLLOAD_IMPLEMENTATION

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char*
read_file_(const char* file, unsigned& size, const char* mode)
{
    FILE* dataFile = fopen(file, mode);

    if (dataFile == nullptr)
    {
        S_STL_ASSERT(false && "File not found.");
        return nullptr;
    }

    // obtain file size:
    fseek(dataFile, 0, SEEK_END);
    size = ftell(dataFile);
    fseek(dataFile, 0, SEEK_SET);

    // allocate memory to contain the whole file:
    char* data = (char*)S_STL_ALLOC(sizeof(char)*size);

    // copy the file into the buffer:
    size_t result = fread(data, sizeof(char), size, dataFile);
    if (result != size)
    {
        if (feof(dataFile))
            printf("Error reading test.bin: unexpected end of file\n");
        else if (ferror(dataFile)) {
            perror("Error reading test.bin");
        }
        S_STL_ASSERT(false && "File not read.");
    }

    fclose(dataFile);

    return data;
}

#if defined(_MSC_VER) && !defined(__clang__)  && !defined(__INTEL_COMPILER) && !defined(IMGUI_DEBUG_PARANOID)
#define IM_MSVC_RUNTIME_CHECKS_OFF      __pragma(runtime_checks("",off))     __pragma(check_stack(off)) __pragma(strict_gs_check(push,off))
#define IM_MSVC_RUNTIME_CHECKS_RESTORE  __pragma(runtime_checks("",restore)) __pragma(check_stack())    __pragma(strict_gs_check(pop))
#else
#define IM_MSVC_RUNTIME_CHECKS_OFF
#define IM_MSVC_RUNTIME_CHECKS_RESTORE
#endif

IM_MSVC_RUNTIME_CHECKS_OFF
template<typename T>
struct sSTLVector
{
    int size_;
    int capacity_;
    T*  data_;
    inline sSTLVector() {size_=capacity_=0; data_=nullptr;}
    inline void     reserve(int new_capacity) {if (new_capacity <= capacity_) return; T* new_data = (T*)S_STL_ALLOC((size_t)new_capacity * sizeof(T)); if (data_) { memcpy(new_data, data_, (size_t)size_ * sizeof(T)); S_STL_FREE(data_); } data_ = new_data; capacity_ = new_capacity;}
    inline int      _grow_capacity(int sz) const { int new_capacity = capacity_ ? (capacity_ + capacity_ / 2) : 8; return new_capacity > sz ? new_capacity : sz; }
    void            release(){ if(data_) S_STL_FREE(data_); size_=capacity_=0; data_=nullptr;}
    inline void     push_back(const T& v) { if (size_ == capacity_) reserve(_grow_capacity(size_ + 1)); memcpy(&data_[size_], &v, sizeof(v)); size_++; }
    inline T&       operator[](int i) { S_STL_ASSERT(i >= 0 && i < size_); return data_[i]; }
    inline const T& operator[](int i) const { S_STL_ASSERT(i >= 0 && i < size_); return data_[i]; }
};
IM_MSVC_RUNTIME_CHECKS_RESTORE

static sSTLVector<float>
parse_for_floats_(char* data, unsigned cursorPos, size_t size)
{
    sSTLVector<float> floats;
    size_t currentPos = cursorPos;
    bool inNumber = false;
    char currentChar = data[currentPos];
    sSTLVector<char> value;
    while(currentChar != 0)
    {
        if(currentChar == ' ')
        {
            if(inNumber)
            {
                value.push_back(0);
                floats.push_back(atof(value.data_));
                value.size_ = 0;
            }
            inNumber = false;
        }
        else if(currentChar == '\t')
        {
            if(inNumber)
            {
                value.push_back(0);
                floats.push_back(atof(value.data_));
                value.size_ = 0;
            }
            inNumber = false;
        }
        else if(currentChar == '\n')
        {
            if(inNumber)
            {
                value.push_back(0);
                floats.push_back(atof(value.data_));
                value.size_ = 0;
            }
            inNumber = false;
        }
        else if(inNumber)
        {
            value.push_back(currentChar);
        }
        else if(currentChar == '-')
        {
            value.push_back(currentChar);
            inNumber = true;
        }
        else if(currentChar > 47 && currentChar < 58) // numbers
        {
            value.push_back(currentChar);
            inNumber = true;
        }
        currentPos++;
        if(currentPos == size)
            currentChar = 0;
        else
            currentChar = data[currentPos];
    }
    value.release();
    return floats;
}

static void
pack_data(sSTLVector<float>& values, sStlModel& model, const sStlOptions& options)
{
    model.xLimits[0] = model.xLimits[1] = values[4];
    model.yLimits[0] = model.yLimits[1] = values[5];
    model.zLimits[0] = model.zLimits[1] = values[6];
    int faceCount = values.size_/12;
    if(options.includeNormals)
    {
        model.data = (float*)S_STL_ALLOC(sizeof(float)*faceCount*18);
        model.count = faceCount*18;
        unsigned currentItem = 0u;
        for(int i = 0; i < values.size_-12; i+=12)
        {
            if(values[i+3] < model.xLimits[0]) model.xLimits[0] = values[i+3];
            if(values[i+3] > model.xLimits[1]) model.xLimits[1] = values[i+3];
            if(values[i+4] < model.yLimits[0]) model.yLimits[0] = values[i+4];
            if(values[i+4] > model.yLimits[1]) model.yLimits[1] = values[i+4];
            if(values[i+5] < model.zLimits[0]) model.zLimits[0] = values[i+5];
            if(values[i+5] > model.zLimits[1]) model.zLimits[1] = values[i+5];
            if(values[i+6] < model.xLimits[0]) model.xLimits[0] = values[i+6];
            if(values[i+6] > model.xLimits[1]) model.xLimits[1] = values[i+6];
            if(values[i+7] < model.yLimits[0]) model.yLimits[0] = values[i+7];
            if(values[i+7] > model.yLimits[1]) model.yLimits[1] = values[i+7];
            if(values[i+8] < model.zLimits[0]) model.zLimits[0] = values[i+8];
            if(values[i+8] > model.zLimits[1]) model.zLimits[1] = values[i+8];
            if(values[i+9] < model.xLimits[0]) model.xLimits[0] = values[i+9];
            if(values[i+9] > model.xLimits[1]) model.xLimits[1] = values[i+9];
            if(values[i+10] < model.yLimits[0]) model.yLimits[0] = values[i+10];
            if(values[i+10] > model.yLimits[1]) model.yLimits[1] = values[i+10];
            if(values[i+11] < model.zLimits[0]) model.zLimits[0] = values[i+11];
            if(values[i+11] > model.zLimits[1]) model.zLimits[1] = values[i+11];

            model.data[currentItem]    = values[i+3];
            model.data[currentItem+1]  = values[i+4];
            model.data[currentItem+2]  = values[i+5];
            model.data[currentItem+3]  = values[i+0];
            model.data[currentItem+4]  = values[i+1];
            model.data[currentItem+5]  = values[i+2];
            model.data[currentItem+6]  = values[i+6];
            model.data[currentItem+7]  = values[i+7];
            model.data[currentItem+8]  = values[i+8];
            model.data[currentItem+9]  = values[i+0];
            model.data[currentItem+10] = values[i+1];
            model.data[currentItem+11] = values[i+2];
            model.data[currentItem+12] = values[i+9];
            model.data[currentItem+13] = values[i+10];
            model.data[currentItem+14] = values[i+11];
            model.data[currentItem+15] = values[i+0];
            model.data[currentItem+16] = values[i+1];
            model.data[currentItem+17] = values[i+2];
            currentItem+=18;
        }
    }
    else
    {
        model.data = (float*)S_STL_ALLOC(sizeof(float)*faceCount*9);
        model.count = faceCount*9;
        unsigned currentItem = 0u;
        for(int i = 0; i < values.size_-12; i+=12)
        {
            if(values[i+3] < model.xLimits[0]) model.xLimits[0] = values[i+3];
            if(values[i+3] > model.xLimits[1]) model.xLimits[1] = values[i+3];
            if(values[i+4] < model.yLimits[0]) model.yLimits[0] = values[i+4];
            if(values[i+4] > model.yLimits[1]) model.yLimits[1] = values[i+4];
            if(values[i+5] < model.zLimits[0]) model.zLimits[0] = values[i+5];
            if(values[i+5] > model.zLimits[1]) model.zLimits[1] = values[i+5];
            if(values[i+6] < model.xLimits[0]) model.xLimits[0] = values[i+6];
            if(values[i+6] > model.xLimits[1]) model.xLimits[1] = values[i+6];
            if(values[i+7] < model.yLimits[0]) model.yLimits[0] = values[i+7];
            if(values[i+7] > model.yLimits[1]) model.yLimits[1] = values[i+7];
            if(values[i+8] < model.zLimits[0]) model.zLimits[0] = values[i+8];
            if(values[i+8] > model.zLimits[1]) model.zLimits[1] = values[i+8];
            if(values[i+9] < model.xLimits[0]) model.xLimits[0] = values[i+9];
            if(values[i+9] > model.xLimits[1]) model.xLimits[1] = values[i+9];
            if(values[i+10] < model.yLimits[0]) model.yLimits[0] = values[i+10];
            if(values[i+10] > model.yLimits[1]) model.yLimits[1] = values[i+10];
            if(values[i+11] < model.zLimits[0]) model.zLimits[0] = values[i+11];
            if(values[i+11] > model.zLimits[1]) model.zLimits[1] = values[i+11];

            model.data[currentItem]    = values[i+3];
            model.data[currentItem+1]  = values[i+4];
            model.data[currentItem+2]  = values[i+5];
            model.data[currentItem+3]  = values[i+6];
            model.data[currentItem+4]  = values[i+7];
            model.data[currentItem+5]  = values[i+8];
            model.data[currentItem+6]  = values[i+9];
            model.data[currentItem+7]  = values[i+10];
            model.data[currentItem+8]  = values[i+11];
            currentItem+=9;
        }
    }
}

static void
load_stl_ascii_model_(char* fileData, unsigned fileSize, sStlModel& model, const sStlOptions& options)
{
    sSTLVector<float> data;
    bool firstItem = true;

    char currentChar = fileData[6];
    unsigned cursorPos = 6u;
    sSTLVector<char> name;
    while(currentChar != '\n')
    {
        name.push_back(currentChar);
        cursorPos++;
        currentChar = fileData[cursorPos];
    }
    model.name = name.data_;

    sSTLVector<float> values = parse_for_floats_(fileData, cursorPos, fileSize);
    pack_data(values, model, options);
    values.release();
}

static void
load_stl_binary_model_(char* fileData, unsigned fileSize, sStlModel& model, const sStlOptions& options)
{
    unsigned int facetCount = *(unsigned int*)&fileData[80];

    sSTLVector<float> values;
    values.reserve(facetCount*12);
    auto floatCount = facetCount*12;
    unsigned currentByte = 84;
    for(int i = 0; i<facetCount; i++)
    {
        // normal vector
        values.push_back(*(float*)&fileData[currentByte]);
        values.push_back(*(float*)&fileData[currentByte+4]);
        values.push_back(*(float*)&fileData[currentByte+8]);

        // vertex 1
        values.push_back(*(float*)&fileData[currentByte+12]);
        values.push_back(*(float*)&fileData[currentByte+16]);
        values.push_back(*(float*)&fileData[currentByte+20]);
        
        // vertex 2
        values.push_back(*(float*)&fileData[currentByte+24]);
        values.push_back(*(float*)&fileData[currentByte+28]);
        values.push_back(*(float*)&fileData[currentByte+32]);
        
        // vertex 3
        values.push_back(*(float*)&fileData[currentByte+36]);
        values.push_back(*(float*)&fileData[currentByte+40]);
        values.push_back(*(float*)&fileData[currentByte+44]);

        currentByte+=50;
    }
    pack_data(values, model, options);
    values.release();
}

sStlModel
Semper::load_stl(const char* file, sStlOptions options)
{
    sStlModel model;
    unsigned fileSize = 0u;
    char* fileData = read_file_(file, fileSize, "rb");
    if(strncmp(fileData, "solid", 5) == 0)
        load_stl_ascii_model_(fileData, fileSize, model, options);
    else
        load_stl_binary_model_(fileData, fileSize, model, options);

    return model;
}

void
Semper::cleanup_stl(sStlModel& model)
{
    model.count = 0;
    S_STL_FREE(model.data);
    S_STL_FREE(model.name);
    model.data = nullptr;
    model.name = nullptr;
}

#endif