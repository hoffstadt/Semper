/*
   sStlLoader, v0.0.2 (WIP)
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

#define S_STL_MAX_LINE_LENGTH 1024

#ifndef S_STL_ASSERT
#include <assert.h>
#define S_STL_ASSERT(_EXPR) assert(_EXPR)
#endif

//-----------------------------------------------------------------------------
// [SECTION] Forward declarations and basic types
//-----------------------------------------------------------------------------

struct sStlOptions;
struct sStlModelInfo;

//-----------------------------------------------------------------------------
// [SECTION] Semper end-user API functions
//-----------------------------------------------------------------------------

namespace Semper
{
    void load_stl(const char* file, sStlOptions options, float* data, size_t& count, sStlModelInfo* info);
}

//-----------------------------------------------------------------------------
// [SECTION] Structs
//-----------------------------------------------------------------------------

struct sStlOptions
{
    bool  includeNormals;
    bool  includeColor;
    float color[4];
};

struct sStlModelInfo
{
    char*    name;
    float    xLimits[2];    
    float    yLimits[2];    
    float    zLimits[2];  
};

#endif

//-----------------------------------------------------------------------------
// [SECTION] Implementation
//-----------------------------------------------------------------------------
#ifdef SEMPER_STLLOAD_IMPLEMENTATION

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char 
get_first_char_(char* linebuffer, size_t& currentPos)
{
    for(; currentPos < S_STL_MAX_LINE_LENGTH; currentPos++)
    {
        char currentChar = linebuffer[currentPos];
        if(currentChar == ' ') continue;
        if(currentChar == '\t') continue;
        if(currentChar == 0) break;
        return currentChar;
    }
    return 0;
}

void
Semper::load_stl(const char* file, sStlOptions options, float* data, size_t& count, sStlModelInfo* info)
{
    bool asci = false;
    char valueBuffer[64];
    char lineBuffer[S_STL_MAX_LINE_LENGTH];

    sStlModelInfo _internalInfo;

    if(info == nullptr)
        info = &_internalInfo;

    info->xLimits[0] = FLT_MAX;
    info->yLimits[0] = FLT_MAX;
    info->zLimits[0] = FLT_MAX;
    info->xLimits[1] = -FLT_MAX;
    info->yLimits[2] = -FLT_MAX;
    info->zLimits[3] = -FLT_MAX;

    for(int i = 0; i < 64; i++) valueBuffer[i]=0;
    for(int i = 0; i < S_STL_MAX_LINE_LENGTH; i++) lineBuffer[i]=0;

 	FILE* dataFile = fopen(file, "rb");

	if (dataFile == nullptr)
	{
		S_STL_ASSERT(false && "File not found.");
        count = 0u;
        return;
	}

    fgets(lineBuffer, S_STL_MAX_LINE_LENGTH, dataFile);
    if(strncmp(lineBuffer, "solid", 5) == 0) asci = true;

    count = 0u;
    size_t facetCount = 0u;

    if(asci)
    {
        while(fgets(lineBuffer, S_STL_MAX_LINE_LENGTH, dataFile))
        {
            size_t currentPos = 0u;
            if(get_first_char_(lineBuffer, currentPos) == 'v')
            {
                count+=3;
                if(options.includeNormals) count+=3;
                if(options.includeColor) count+=4;
                continue;
            }     
        }
    }
    else
    {
        count = *(unsigned int*)&lineBuffer[80]; 
        facetCount = count;
        count*=9;
        if(options.includeNormals) count*=2;
        if(options.includeColor) count+=(4*facetCount);
    }

    if(data == nullptr)
    {
        fclose(dataFile);
        return;
    }

    fseek(dataFile, 0, SEEK_SET); // move back to beginning of file

    if(asci)
    {
        size_t currentValue = 0u;
        float currentNormal[3];
        while(fgets(lineBuffer, S_STL_MAX_LINE_LENGTH, dataFile))
        {
            size_t currentPos = 0u;
            char firstChar = get_first_char_(lineBuffer, currentPos);
            if(firstChar == 'f')
            {
                currentPos+=12;
                char* currentString = &lineBuffer[currentPos];
                currentNormal[0] = (float)strtod(currentString, &currentString);
                currentNormal[1] = (float)strtod(currentString, &currentString);
                currentNormal[2] = (float)strtod(currentString, &currentString);
                continue;
            }
            if(firstChar == 'v')
            {
                currentPos+=6;
                char* currentString = &lineBuffer[currentPos];
                data[currentValue] = (float)strtod(currentString, &currentString);
                data[currentValue+1] = (float)strtod(currentString, &currentString);
                data[currentValue+2] = (float)strtod(currentString, &currentString);
                if(data[currentValue]   < info->xLimits[0]) info->xLimits[0] = data[currentValue];
                if(data[currentValue+1] < info->yLimits[0]) info->yLimits[0] = data[currentValue+1];
                if(data[currentValue+2] < info->zLimits[0]) info->zLimits[0] = data[currentValue+2];
                if(data[currentValue]   > info->xLimits[1]) info->xLimits[1] = data[currentValue];
                if(data[currentValue+1] > info->yLimits[1]) info->yLimits[1] = data[currentValue+1];
                if(data[currentValue+2] > info->zLimits[1]) info->zLimits[1] = data[currentValue+2];
                currentValue+=3;
                if(options.includeNormals)
                {
                    data[currentValue] = currentNormal[0];
                    data[currentValue+1] = currentNormal[1];
                    data[currentValue+2] = currentNormal[2];
                    currentValue+=3;
                }
                if(options.includeColor)
                {
                    data[currentValue] = options.color[0];
                    data[currentValue+1] = options.color[1];
                    data[currentValue+2] = options.color[2];
                    data[currentValue+3] = options.color[3];
                    currentValue+=4;
                }
                continue;
            }
        }        
    }
    else // binary
    {
        for(int i = 0; i < 84; i++) fgetc(dataFile);
        size_t currentValue = 0u;
        unsigned char facetBuffer[50];
        for(int facet = 0; facet < facetCount; facet++)
        {
            for(int i = 0; i < 50; i++)
            {
                facetBuffer[i] = fgetc(dataFile);
                assert(facetBuffer[i] != EOF);
            }
            
            // normal vector
            float currentNormal[3];
            currentNormal[0] = *(float*)&facetBuffer[0];
            currentNormal[1] = *(float*)&facetBuffer[4];
            currentNormal[2] = *(float*)&facetBuffer[8];

            // vertex 1
            data[currentValue] = *(float*)&facetBuffer[12];
            data[currentValue+1] = *(float*)&facetBuffer[16];
            data[currentValue+2] = *(float*)&facetBuffer[20];
            if(data[currentValue]   < info->xLimits[0]) info->xLimits[0] = data[currentValue];
            if(data[currentValue+1] < info->yLimits[0]) info->yLimits[0] = data[currentValue+1];
            if(data[currentValue+2] < info->zLimits[0]) info->zLimits[0] = data[currentValue+2];
            if(data[currentValue]   > info->xLimits[1]) info->xLimits[1] = data[currentValue];
            if(data[currentValue+1] > info->yLimits[1]) info->yLimits[1] = data[currentValue+1];
            if(data[currentValue+2] > info->zLimits[1]) info->zLimits[1] = data[currentValue+2];
            currentValue+=3;
            if(options.includeNormals)
            {
                data[currentValue] = currentNormal[0];
                data[currentValue+1] = currentNormal[1];
                data[currentValue+2] = currentNormal[2];
                currentValue+=3;
            }
            if(options.includeColor)
            {
                data[currentValue] = options.color[0];
                data[currentValue+1] = options.color[1];
                data[currentValue+2] = options.color[2];
                data[currentValue+3] = options.color[3];
                currentValue+=4;
            }
            
            // vertex 2
            data[currentValue] = *(float*)&facetBuffer[24];
            data[currentValue+1] = *(float*)&facetBuffer[28];
            data[currentValue+2] = *(float*)&facetBuffer[32];
            if(data[currentValue]   < info->xLimits[0]) info->xLimits[0] = data[currentValue];
            if(data[currentValue+1] < info->yLimits[0]) info->yLimits[0] = data[currentValue+1];
            if(data[currentValue+2] < info->zLimits[0]) info->zLimits[0] = data[currentValue+2];
            if(data[currentValue]   > info->xLimits[1]) info->xLimits[1] = data[currentValue];
            if(data[currentValue+1] > info->yLimits[1]) info->yLimits[1] = data[currentValue+1];
            if(data[currentValue+2] > info->zLimits[1]) info->zLimits[1] = data[currentValue+2];
            currentValue+=3;
            if(options.includeNormals)
            {
                data[currentValue] = currentNormal[0];
                data[currentValue+1] = currentNormal[1];
                data[currentValue+2] = currentNormal[2];
                currentValue+=3;
            }
            if(options.includeColor)
            {
                data[currentValue] = options.color[0];
                data[currentValue+1] = options.color[1];
                data[currentValue+2] = options.color[2];
                data[currentValue+3] = options.color[3];
                currentValue+=4;
            }
            
            // vertex 3
            data[currentValue] = *(float*)&facetBuffer[36];
            data[currentValue+1] = *(float*)&facetBuffer[40];
            data[currentValue+2] = *(float*)&facetBuffer[44];
            if(data[currentValue]   < info->xLimits[0]) info->xLimits[0] = data[currentValue];
            if(data[currentValue+1] < info->yLimits[0]) info->yLimits[0] = data[currentValue+1];
            if(data[currentValue+2] < info->zLimits[0]) info->zLimits[0] = data[currentValue+2];
            if(data[currentValue]   > info->xLimits[1]) info->xLimits[1] = data[currentValue];
            if(data[currentValue+1] > info->yLimits[1]) info->yLimits[1] = data[currentValue+1];
            if(data[currentValue+2] > info->zLimits[1]) info->zLimits[1] = data[currentValue+2];
            currentValue+=3;
            if(options.includeNormals)
            {
                data[currentValue] = currentNormal[0];
                data[currentValue+1] = currentNormal[1];
                data[currentValue+2] = currentNormal[2];
                currentValue+=3;
            }
            if(options.includeColor)
            {
                data[currentValue] = options.color[0];
                data[currentValue+1] = options.color[1];
                data[currentValue+2] = options.color[2];
                data[currentValue+3] = options.color[3];
                currentValue+=4;
            }
        }
    }
    fclose(dataFile);
}

#endif
