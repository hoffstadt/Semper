/*
   sMemory, v0.1 (WIP)
   * no dependencies
   * simple
   Do this:
	  #define SEMPER_MEMORY_IMPLEMENTATION
   before you include this file in *one* C or C++ file to create the implementation.
   // i.e. it should look like this:
   #include ...
   #include ...
   #include ...
   #define SEMPER_MEMORY_IMPLEMENTATION
   #include "sMemory.h"
*/

#ifndef SEMPER_MEMORY_H
#define SEMPER_MEMORY_H

#ifndef S_MEMORY_ASSERT
#include <assert.h>
#define S_MEMORY_ASSERT(x) assert(x)
#endif

#include <stdlib.h>
#include <string.h> // memcpy, strcmp

#ifndef S_MEMORY_ALLOC
#define S_MEMORY_ALLOC(x) Semper::allocate_memory(x)
#endif

#ifndef S_MEMORY_FREE
#define S_MEMORY_FREE(x) Semper::free_memory(x)
#endif

#include <stddef.h>  // size_t

//-----------------------------------------------------------------------------
// [SECTION] Forward declarations and basic types
//-----------------------------------------------------------------------------

// forward declarations
struct sMemoryBlock;
struct sLinearAllocator;
struct sStackAllocator;
struct sStackAllocatorHeader;
struct sPoolAllocator;
struct sPoolAllocatorNode;

// enums/flags
typedef int sAllocatorType; // -> enum sAllocatorType_

//-----------------------------------------------------------------------------
// [SECTION] Semper end-user API functions
//-----------------------------------------------------------------------------

namespace Semper
{
    // memory allocation
    int   get_active_allocations();
    void* allocate_memory(size_t size);
    void  free_memory    (void* ptr);
}

//-----------------------------------------------------------------------------
// [SECTION] Enums
//-----------------------------------------------------------------------------

enum sAllocatorType_
{
    S_EXTERNAL_ALLOCATOR, // external allocator responsible for freeing
    S_DEFAULT_ALLOCATOR,  // malloc
    S_LINEAR_ALLOCATOR,   // Semper Linear Allocator
    S_STACK_ALLOCATOR,    // Semper Stack Allocator
    S_POOL_ALLOCATOR      // Semper Pool Allocator
};

//-----------------------------------------------------------------------------
// [SECTION] Linear Allocator
//-----------------------------------------------------------------------------

struct sLinearAllocator
{
    sAllocatorType type;
    sAllocatorType parentType;
    void*          parentAllocator; // for freeing
    void*          memory;
    size_t         size;       // size (bytes)
    size_t         offset;     // current ptr offset (bytes)

    void intialize(size_t size);                               // creates allocator & allocates memory buffer
    void intialize(size_t size, void* parentAllocatorAddress); // creates allocator & allocates memory buffer
    void intialize(void* memory, size_t size);                 // creates allocator to manage memory

    void         free_memory();                                           // free allocators memory
    void*        request_memory          (size_t size);                   // returns nullptr on failure
    void*        request_aligned_memory  (size_t size, size_t alignment); // returns nullptr on failure
    inline void  reset_allocator (){ offset = 0u;}
};

//-----------------------------------------------------------------------------
// [SECTION] Stack Allocator
//-----------------------------------------------------------------------------

struct sStackAllocatorHeader
{
    size_t size; // size of block (bytes)
    size_t ID;   // block ID (error checking)
};

struct sStackAllocator
{
    sAllocatorType type;
    sAllocatorType parentType;
    void*          parentAllocator; // for freeing
    void*          memory;
    size_t         size;            // size (bytes)
    size_t         offset;          // current ptr offset (bytes)
    size_t         currentID;       // current block ID (error checking)

    // stack allocator
    void        initialize(size_t size);                               // creates allocator & allocates memory buffer
    void        initialize(size_t size, void* parentAllocatorAddress); // creates allocator and allocates memory buffer
    void        initialize(void* memory, size_t size);                 // creates allocator to manage memory
    void        free_memory();                                         // frees allocators memory
    void*       request_memory(size_t size);                           // returns nullptr on failure
    void*       request_aligned_memory(size_t size, size_t alignment); // returns nullptr on failure
    bool        return_memory(void* ptr);                              // sets the offset back (must be in reverse order)
    inline void reset_allocator (){ offset = 0u; currentID = 0u;}      // returns offset to 0

};

//-----------------------------------------------------------------------------
// [SECTION] Pool Allocator
//-----------------------------------------------------------------------------

struct sPoolAllocatorNode
{
    sPoolAllocatorNode* nextNode;
    void*               chunkAddress;
};

struct sPoolAllocator
{
    sAllocatorType      type;
    sAllocatorType      parentType;
    void*               parentAllocator; // for freeing
    void*               memory;
    size_t              size;            // size (bytes)
    size_t              itemSize;        // item size (bytes)
    size_t              chunkSize;       // item size + padding for alignment (bytes)
    size_t              headerSize;      // linked list header size (bytes)
    size_t              itemCount;       // number of items owned by pool
    sPoolAllocatorNode* firstNode;       // start of freelist
    size_t              freeItemCount;   // number of items available

    void  initialize(size_t itemCount, size_t itemSize);                               // creates allocator & allocates memory buffer
    void  initialize(size_t itemCount, size_t itemSize, void* parentAllocatorAddress); // creates allocator & allocates memory buffer
    void  initialize(void* memory, size_t itemCount, size_t itemSize);                 // creates allocator to manage memory
    void  free_memory();            // free allocators memory
    void* request_memory();         // returns nullptr on failure
    bool  return_memory(void* ptr); // returns memory to free list
};

#endif

#ifdef SEMPER_MEMORY_IMPLEMENTATION

#include <stdint.h> // uintptr_t
#include <stdlib.h> // malloc, free
typedef unsigned char byte;

static int g_semperMetricsActiveAllocations = 0;

int
Semper::get_active_allocations()
{
    return g_semperMetricsActiveAllocations;
}

void*
Semper::allocate_memory(size_t size)
{
    g_semperMetricsActiveAllocations++;
    return malloc(size);
}

void
Semper::free_memory(void* ptr)
{
    g_semperMetricsActiveAllocations--;
    free(ptr);
}

//-----------------------------------------------------------------------------
// [SECTION] Linear Allocator
//-----------------------------------------------------------------------------

void
sLinearAllocator::intialize(size_t size)
{
    S_MEMORY_ASSERT(size > 0u);
    type = S_LINEAR_ALLOCATOR;
    parentType = S_DEFAULT_ALLOCATOR;
    parentAllocator = nullptr;
    this->size = size;
    offset = 0u;
    memory = S_MEMORY_ALLOC(size);
}

void
sLinearAllocator::intialize(size_t size, void* parentAllocatorAddress)
{
    S_MEMORY_ASSERT(size > 0u);
    S_MEMORY_ASSERT(parentAllocatorAddress != nullptr);
    type = S_LINEAR_ALLOCATOR;
    parentAllocator = parentAllocatorAddress;
    this->size = size;
    offset = 0u;
    memory = nullptr;

    if (parentAllocatorAddress == nullptr)
        return;

    auto allocatorType = (sAllocatorType*)parentAllocatorAddress;
    switch (*allocatorType)
    {
    case S_LINEAR_ALLOCATOR:
    {
        auto parentAllocator = (sLinearAllocator*)parentAllocatorAddress;
        memory = parentAllocator->request_aligned_memory(size, size);
        parentType = S_LINEAR_ALLOCATOR;
        break;
    }
    case S_STACK_ALLOCATOR:
    {
        auto parentAllocator = (sStackAllocator*)parentAllocatorAddress;
        memory = parentAllocator->request_aligned_memory(size, size);
        parentType = S_STACK_ALLOCATOR;
        break;
    }
    case S_POOL_ALLOCATOR:
    {
        auto parentAllocator = (sPoolAllocator*)parentAllocatorAddress;
        memory = parentAllocator->request_memory();
        parentType = S_POOL_ALLOCATOR;
        break;
    }
    default:
        S_MEMORY_ASSERT(false && "Parent allocator type not supported");
        parentType = S_DEFAULT_ALLOCATOR;
        break;
    }
}

void
sLinearAllocator::intialize(void* memory, size_t size)
{
    S_MEMORY_ASSERT(size > 0u);
    S_MEMORY_ASSERT(memory != nullptr);
    sLinearAllocator allocator;
    type = S_LINEAR_ALLOCATOR;
    parentType = S_DEFAULT_ALLOCATOR;
    parentAllocator = nullptr;
    this->size = size;
    offset = 0u;
    this->memory = memory;
}

void
sLinearAllocator::free_memory()
{
    if (memory)
    {

        switch (parentType)
        {
        case S_STACK_ALLOCATOR:
        {
            S_MEMORY_ASSERT(parentAllocator);
            auto parentAllocator = (sStackAllocator*)this->parentAllocator;
            parentAllocator->return_memory(memory);
            break;
        }
        case S_POOL_ALLOCATOR:
        {
            S_MEMORY_ASSERT(this->parentAllocator);
            auto parentAllocator = (sPoolAllocator*)this->parentAllocator;
            parentAllocator->return_memory(memory);
            break;
        }
        case S_DEFAULT_ALLOCATOR:
        {
            S_MEMORY_FREE(memory);
            break;
        }
        default:
            break;
        }     
    }
    memory = nullptr;
    size = 0u;
    parentAllocator = nullptr;
    offset = 0u;
    parentType = S_DEFAULT_ALLOCATOR;
}

void*
sLinearAllocator::request_memory(size_t size)
{
    size_t offset = this->offset + size;

    if (offset > this->size) // make sure we have enough memory
    {
        S_MEMORY_ASSERT(false && "Linear allocator doesn't have enough free memory.");
        return nullptr;
    }

    // update offset pointer
    auto memory = (byte*)this->memory + this->offset;
    this->offset = offset; // new offset
    return memory;
}

void*
sLinearAllocator::request_aligned_memory(size_t size, size_t alignment)
{
    auto paddingBytes = ((uintptr_t)this->memory + this->offset) % alignment;
    size_t offset = this->offset + paddingBytes + size;

    if (offset > this->size) // make sure we have enough memory
    {
        S_MEMORY_ASSERT(false && "Linear allocator doesn't have enough free memory.");
        return nullptr;
    }

    auto memory = (byte*)this->memory + this->offset + paddingBytes;
    this->offset = offset; // new offset
    return memory;
}

//-----------------------------------------------------------------------------
// [SECTION] Stack Allocator
//-----------------------------------------------------------------------------

void
sStackAllocator::initialize(size_t size)
{
    S_MEMORY_ASSERT(size > 0u);
    type = S_STACK_ALLOCATOR;
    parentType = S_DEFAULT_ALLOCATOR;
    parentAllocator = nullptr;
    this->size = size;
    offset = 0u;
    memory = S_MEMORY_ALLOC(size);
    currentID = 0u;
}

void 
sStackAllocator::initialize(size_t size, void* parentAllocatorAddress)
{
    S_MEMORY_ASSERT(size > 0u);
    S_MEMORY_ASSERT(parentAllocatorAddress != nullptr);
    type = S_STACK_ALLOCATOR;
    parentAllocator = parentAllocatorAddress;
    this->size = size;
    offset = 0u;
    memory = nullptr;
    currentID = 0u;

    if (parentAllocatorAddress == nullptr)
        return;

    auto allocatorType = (sAllocatorType*)parentAllocatorAddress;
    switch (*allocatorType)
    {
    case S_LINEAR_ALLOCATOR:
    {
        auto parentAllocator = (sLinearAllocator*)parentAllocatorAddress;
        memory = parentAllocator->request_aligned_memory(size, size);
        break;
    }
    case S_STACK_ALLOCATOR:
    {
        auto parentAllocator = (sStackAllocator*)parentAllocatorAddress;
        memory = parentAllocator->request_aligned_memory(size, size);
        break;
    }
    case S_POOL_ALLOCATOR:
    {
        auto parentAllocator = (sPoolAllocator*)parentAllocatorAddress;
        memory = parentAllocator->request_memory();
        break;
    }
    default:
        S_MEMORY_ASSERT(false && "Parent allocator type not supported");
        break;
    }
}

void
sStackAllocator::initialize(void* memory, size_t size)
{
    S_MEMORY_ASSERT(size > 0u);
    S_MEMORY_ASSERT(memory != nullptr);
    sStackAllocator allocator;
    type = S_STACK_ALLOCATOR;
    parentType = S_EXTERNAL_ALLOCATOR;
    parentAllocator = nullptr;
    this->size = size;
    offset = 0u;
    this->memory = memory;
    currentID = 0u;
}

void
sStackAllocator::free_memory()
{
    if (memory)
    {
        switch (parentType)
        {
        case S_STACK_ALLOCATOR:
        {
            S_MEMORY_ASSERT(parentAllocator);
            auto parentAllocator = (sStackAllocator*)this->parentAllocator;
            parentAllocator->return_memory(memory);
            break;
        }
        case S_POOL_ALLOCATOR:
        {
            S_MEMORY_ASSERT(parentAllocator);
            auto parentAllocator = (sPoolAllocator*)this->parentAllocator;
            parentAllocator->return_memory(memory);
            break;
        }
        case S_DEFAULT_ALLOCATOR:
        {
            S_MEMORY_FREE(memory);
            break;
        }
        default:
            break;
        }
    }
    memory = nullptr;
    size = 0u;
    currentID = 0u;
    parentAllocator = nullptr;
    offset = 0u;
    parentType = S_DEFAULT_ALLOCATOR;
}

void*
sStackAllocator::request_memory(size_t size)
{
    size_t offset = this->offset + size + sizeof(sStackAllocatorHeader);

    if (offset > this->size) // make sure we have enough memory
    {
        S_MEMORY_ASSERT(false && "Stack allocator is full.");
        return nullptr;
    }

    // update header
    auto header_raw_memory = (byte*)this->memory + this->offset;
    auto header = (sStackAllocatorHeader*)header_raw_memory;
    header->size = size+sizeof(sStackAllocatorHeader);
    header->ID = ++currentID;

    // update offset pointer
    auto memory = (byte*)this->memory + this->offset + sizeof(sStackAllocatorHeader);
    this->offset = offset; // new offset
    return memory;
}

void*
sStackAllocator::request_aligned_memory(size_t size, size_t alignment)
{

    if (alignment < 2u)
        return request_memory(size);


    auto currentMemoryLocation = (byte*)memory + offset;
    const size_t paddingBytes = (size_t)(currentMemoryLocation + sizeof(sStackAllocatorHeader)) % alignment;
    const size_t sizeOfBlock = paddingBytes + size + sizeof(sStackAllocatorHeader);
    const size_t offset = this->offset + sizeOfBlock;

    if (offset > this->size)
    {
        S_MEMORY_ASSERT(false && "Stack allocator doesn't have enough room.");
        return nullptr;
    }
    this->offset = offset; // new offset

    auto headerMemoryLocation = currentMemoryLocation + paddingBytes;
    auto header = (sStackAllocatorHeader*)headerMemoryLocation;
    header->size = sizeOfBlock;
    header->ID = ++currentID;
    return currentMemoryLocation + paddingBytes + sizeof(sStackAllocatorHeader);
}

bool
sStackAllocator::return_memory(void* ptr)
{
    // check if order is correct
    auto header_raw_memory = (byte*)ptr - sizeof(sStackAllocatorHeader);
    auto header = (sStackAllocatorHeader*)header_raw_memory;

    if (header->ID == currentID)
    {
        currentID--;
        offset = offset - header->size;
        return true;
    }
    S_MEMORY_ASSERT(false && "Stack allocator requires memory to be returned in order.");
    return false;
}

//-----------------------------------------------------------------------------
// [SECTION] Pool Allocator
//-----------------------------------------------------------------------------

void
sPoolAllocator::initialize(size_t itemCount, size_t itemSize)
{
    S_MEMORY_ASSERT(itemSize > 0u);
    S_MEMORY_ASSERT(itemCount > 0u);

    type = S_POOL_ALLOCATOR;
    parentType = S_DEFAULT_ALLOCATOR;
    parentAllocator = nullptr;
    this->itemCount = itemCount;
    this->itemSize = itemSize;
    freeItemCount = itemCount;
    headerSize = itemCount* sizeof(sPoolAllocatorNode); // memory requirement for linked list
    const size_t paddingBytes = headerSize % itemSize; // padding for alignment
    chunkSize = (itemSize+paddingBytes) * itemCount;
    size = headerSize + chunkSize * itemCount;
    memory = S_MEMORY_ALLOC(size);
    firstNode = (sPoolAllocatorNode*)memory;

    // construct linked list
    auto currentChunkAddress = (byte*)memory + headerSize;
    auto currentNodeAddress = (byte*)memory;
    auto nextNodeAddress = (byte*)memory + sizeof(sPoolAllocatorNode);
    for (size_t i = 0; i < itemCount-1; i++)
    {
        auto currentNode = (sPoolAllocatorNode*)currentNodeAddress;
        currentNode->nextNode = (sPoolAllocatorNode*)nextNodeAddress;
        currentNode->chunkAddress = currentChunkAddress;
        currentNodeAddress += sizeof(sPoolAllocatorNode);
        nextNodeAddress += sizeof(sPoolAllocatorNode);
        currentChunkAddress += chunkSize;
    }

    // terminal node
    auto currentNode = (sPoolAllocatorNode*)currentNodeAddress;
    currentNode->nextNode = nullptr;
    currentNode->chunkAddress = currentChunkAddress;
}

void
sPoolAllocator::initialize(size_t itemCount, size_t itemSize, void* parentAllocatorAddress)
{
    S_MEMORY_ASSERT(itemSize > 0u);
    S_MEMORY_ASSERT(itemCount > 0u);
    S_MEMORY_ASSERT(parentAllocatorAddress != nullptr);

    type = S_POOL_ALLOCATOR;
    parentAllocator = nullptr;
    this->itemCount = itemCount;
    this->itemSize = itemSize;
    freeItemCount = itemCount;
    headerSize = itemCount * sizeof(sPoolAllocatorNode); // memory requirement for linked list
    const size_t paddingBytes = headerSize % itemSize; // padding for alignment
    chunkSize = (itemSize + paddingBytes) * itemCount;
    size = headerSize + chunkSize * itemCount;

    auto allocatorType = (sAllocatorType*)parentAllocatorAddress;
    switch (*allocatorType)
    {
    case S_LINEAR_ALLOCATOR:
    {
        auto parentAllocator = (sLinearAllocator*)parentAllocatorAddress;
        memory = parentAllocator->request_aligned_memory(size, size);
        parentType = S_LINEAR_ALLOCATOR;
        break;
    }
    case S_STACK_ALLOCATOR:
    {
        auto parentAllocator = (sStackAllocator*)parentAllocatorAddress;
        memory = parentAllocator->request_aligned_memory(size, size);
        parentType = S_STACK_ALLOCATOR;
        break;
    }
    case S_POOL_ALLOCATOR:
    {
        auto parentAllocator = (sPoolAllocator*)parentAllocatorAddress;
        memory = parentAllocator->request_memory();
        parentType = S_POOL_ALLOCATOR;
        break;
    }
    default:
        S_MEMORY_ASSERT(false && "Parent allocator type not supported");
        return;
    }

    firstNode = (sPoolAllocatorNode*)memory;

    // construct linked list
    auto currentChunkAddress = (byte*)memory + headerSize;
    auto currentNodeAddress = (byte*)memory;
    auto nextNodeAddress = (byte*)memory + sizeof(sPoolAllocatorNode);
    for (size_t i = 0; i < itemCount - 1; i++)
    {
        auto currentNode = (sPoolAllocatorNode*)currentNodeAddress;
        currentNode->nextNode = (sPoolAllocatorNode*)nextNodeAddress;
        currentNode->chunkAddress = currentChunkAddress;
        currentNodeAddress += sizeof(sPoolAllocatorNode);
        nextNodeAddress += sizeof(sPoolAllocatorNode);
        currentChunkAddress += chunkSize;
    }

    // terminal node
    auto currentNode = (sPoolAllocatorNode*)currentNodeAddress;
    currentNode->nextNode = nullptr;
    currentNode->chunkAddress = currentChunkAddress;
}

void
sPoolAllocator::initialize(void* memory, size_t itemCount, size_t itemSize)
{
    S_MEMORY_ASSERT(itemSize > 0u);
    S_MEMORY_ASSERT(itemCount > 0u);
    S_MEMORY_ASSERT(memory != nullptr);

    type = S_POOL_ALLOCATOR;
    parentType = S_EXTERNAL_ALLOCATOR;
    parentAllocator = nullptr;
    this->itemCount = itemCount;
    this->itemSize = itemSize;
    freeItemCount = itemCount;
    headerSize = itemCount * sizeof(sPoolAllocatorNode); // memory requirement for linked list
    const size_t paddingBytes = headerSize % itemSize; // padding for alignment
    chunkSize = (itemSize + paddingBytes) * itemCount;
    size = headerSize + chunkSize * itemCount;
    this->memory = memory;
    parentType = S_LINEAR_ALLOCATOR;
    firstNode = (sPoolAllocatorNode*)memory;

    // construct linked list
    auto currentChunkAddress = (byte*)memory + headerSize;
    auto currentNodeAddress = (byte*)memory;
    auto nextNodeAddress = (byte*)memory + sizeof(sPoolAllocatorNode);
    for (size_t i = 0; i < itemCount - 1; i++)
    {
        auto currentNode = (sPoolAllocatorNode*)currentNodeAddress;
        currentNode->nextNode = (sPoolAllocatorNode*)nextNodeAddress;
        currentNode->chunkAddress = currentChunkAddress;
        currentNodeAddress += sizeof(sPoolAllocatorNode);
        nextNodeAddress += sizeof(sPoolAllocatorNode);
        currentChunkAddress += chunkSize;
    }

    // terminal node
    auto currentNode = (sPoolAllocatorNode*)currentNodeAddress;
    currentNode->nextNode = nullptr;
    currentNode->chunkAddress = currentChunkAddress;
}

void
sPoolAllocator::free_memory()
{
    if (memory)
    {
        switch (parentType)
        {
        case S_STACK_ALLOCATOR:
        {
            S_MEMORY_ASSERT(parentAllocator);
            auto parentAllocator = (sStackAllocator*)this->parentAllocator;
            parentAllocator->return_memory(memory);
            break;
        }
        case S_POOL_ALLOCATOR:
        {
            S_MEMORY_ASSERT(parentAllocator);
            auto parentAllocator = (sPoolAllocator*)this->parentAllocator;
            parentAllocator->return_memory(memory);
            break;
        }
        case S_DEFAULT_ALLOCATOR:
        {
            S_MEMORY_FREE(memory);
            break;
        }
        default:
            break;
        }
    }
    this->parentAllocator = nullptr;
    parentType = S_DEFAULT_ALLOCATOR;
    memory = nullptr;
    size = 0u;          
    itemSize = 0u;      
    chunkSize = 0u;
    headerSize = 0u;
    itemCount = 0u;
    firstNode = 0u;     
    freeItemCount = 0u; 
}

void*
sPoolAllocator::request_memory()
{
    S_MEMORY_ASSERT(freeItemCount > 0u);
    freeItemCount--;
    sPoolAllocatorNode* firstFreeNode = this->firstNode;
    sPoolAllocatorNode* nextNode = firstFreeNode->nextNode;
    firstNode = nextNode;
    return firstFreeNode->chunkAddress;
}

bool
sPoolAllocator::return_memory(void* ptr)
{
    freeItemCount++;

    sPoolAllocatorNode* oldFreeNode = firstNode;
    firstNode = (sPoolAllocatorNode*)ptr;
    firstNode->nextNode = oldFreeNode;
    S_MEMORY_ASSERT(false && "Node could not be returned.");
    return false;
}

#endif // SEMPER_MEMORY_IMPLEMENTATION