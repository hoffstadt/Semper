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
struct sGeneralLLAllocatorHeader;
struct sGeneralLLAllocatorNode;
struct sGeneralLLAllocator;

// enums/flags
typedef int sAllocatorType;            // -> enum sAllocatorType_
typedef int sAllocatorPlacementPolicy; // -> enum sAllocatorPlacementPolicy_

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
    S_ALLOCATOR_TYPE_NONE,  // none
    S_EXTERNAL_ALLOCATOR,   // external allocator responsible for freeing
    S_DEFAULT_ALLOCATOR,    // malloc
    S_LINEAR_ALLOCATOR,     // Semper Linear Allocator
    S_STACK_ALLOCATOR,      // Semper Stack Allocator
    S_POOL_ALLOCATOR,       // Semper Pool Allocator
    S_GENERAL_LL_ALLOCATOR, // Semper General Purpose Allocator (using linked list)
    S_GENERAL_RB_ALLOCATOR  // Semper General Purpose Allocator (using red-black binary tree)
};

//-----------------------------------------------------------------------------
// [SECTION] Linear Allocator
//-----------------------------------------------------------------------------

struct sLinearAllocator
{
    sAllocatorType type;
    sAllocatorType parentType;
    void*          parentAllocator; // for freeing
    unsigned char* buffer;
    size_t         bufferSize;    // size (bytes)
    size_t         currentOffset; // current ptr offset (bytes)

    void initialize(size_t size);                  // creates allocator & allocates memory buffer
    void initialize(size_t size, void* allocator); // creates allocator & allocates memory buffer
    void initialize(void* memory, size_t size);    // creates allocator to manage memory

    void         free_memory();                                         // free allocators memory
    void*        request_memory        (size_t size);                   // returns nullptr on failure
    void*        request_aligned_memory(size_t size, size_t alignment); // returns nullptr on failure
    inline void  reset_allocator (){ currentOffset = 0u;}
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
    unsigned char* buffer;
    size_t         bufferSize;    // size (bytes)
    size_t         currentOffset; // current ptr offset (bytes)
    size_t         currentID;     // current block ID (error checking)

    // stack allocator
    void        initialize(size_t size);                    // creates allocator & allocates memory buffer
    void        initialize(size_t size, void* allocator);   // creates allocator and allocates memory buffer
    void        initialize(void* memory, size_t size);      // creates allocator to manage memory
    void        free_memory();                              // frees allocators memory
    void*       request_memory(size_t size);                // returns nullptr on failure
    void*       request_aligned_memory(size_t size, size_t alignment);   // returns nullptr on failure
    bool        return_memory(void* ptr);                                // sets the offset back (must be in reverse order)
    inline void reset_allocator (){ currentOffset = 0u; currentID = 0u;} // returns offset to 0

};

//-----------------------------------------------------------------------------
// [SECTION] Pool Allocator
//-----------------------------------------------------------------------------

struct sPoolAllocatorNode
{
    sPoolAllocatorNode* nextNode;
};

struct sPoolAllocator
{
    sAllocatorType      type;
    sAllocatorType      parentType;
    void*               parentAllocator; // for freeing
    unsigned char*      buffer;
    size_t              bufferSize;      // size (bytes)
    size_t              chunkSize;       // item size + padding for alignment (bytes)
    size_t              count;           // number of items owned by pool
    size_t              freeItemCount;   // number of items available
    sPoolAllocatorNode* head;            // start of freelist

    void  initialize(size_t itemCount, size_t itemSize, size_t alignment); // creates allocator & allocates memory buffer
    void  initialize(size_t itemCount, size_t itemSize, size_t alignment, void* allocator); // creates allocator and allocates memory buffer
    void  initialize(size_t itemCount, size_t itemSize, size_t alignment, void* memory, size_t size); // creates allocator to manage memory
    void  free_memory();            // free allocators memory
    void* request_memory();         // returns nullptr on failure
    bool  return_memory(void* ptr); // returns memory to free list
};

//-----------------------------------------------------------------------------
// [SECTION] General Allocator (Freelist using linked list)
//-----------------------------------------------------------------------------

enum sAllocatorPlacementPolicy_
{
    S_ALLOC_PLACEMENT_POLICY_NONE,
    S_ALLOC_PLACEMENT_POLICY_FIND_FIRST,
    S_ALLOC_PLACEMENT_POLICY_FIND_BEST
};

struct sGeneralLLAllocatorHeader
{
    size_t blockSize;
    size_t padding;
};

struct sGeneralLLAllocatorNode
{
    sGeneralLLAllocatorNode* nextNode;
    size_t                   blockSize;
};

struct sGeneralLLAllocator
{
    sAllocatorType            type;
    sAllocatorType            parentType;
    void*                     parentAllocator; // for freeing
    void*                     buffer;
    size_t                    bufferSize;
    size_t                    used;
    sGeneralLLAllocatorNode*  head;
    sAllocatorPlacementPolicy placementPolicy;

    void  initialize(size_t size, sAllocatorPlacementPolicy policy);
    void  initialize(size_t size, sAllocatorPlacementPolicy policy, void* allocator); // creates allocator and allocates memory buffer
    void  initialize(void* memory, sAllocatorPlacementPolicy policy, size_t size);    // creates allocator to manage memory
    void  free_memory();
    void* request_memory(size_t size);
    void* request_aligned_memory(size_t size, size_t alignment);
    void  return_memory(void* ptr);
};

//-----------------------------------------------------------------------------
// [SECTION] General Allocator (Freelist using red-black binary tree)
//-----------------------------------------------------------------------------

#endif

#ifdef SEMPER_MEMORY_IMPLEMENTATION

#include <stdint.h> // uintptr_t
#include <stdlib.h> // malloc, free
typedef unsigned char byte;

inline bool _is_power_of_two(uintptr_t x) { return (x & (x-1)) == 0;}

static uintptr_t
_align_forward_uintptr(uintptr_t ptr, size_t align) 
{
	S_MEMORY_ASSERT(_is_power_of_two(align));
	uintptr_t a = (uintptr_t)align;
	uintptr_t p = ptr;
	uintptr_t modulo = p & (a-1);
	if (modulo != 0) p += a - modulo;
	return p;
}

static size_t 
_align_forward_size(size_t ptr, size_t align) 
{
	S_MEMORY_ASSERT(_is_power_of_two((uintptr_t)align));
	size_t a = align;
	size_t p = ptr;
	size_t modulo = p & (a-1);
	if (modulo != 0) p += a - modulo;
	return p;
}

static size_t
_calc_padding_with_header(uintptr_t ptr, uintptr_t alignment, size_t header_size) 
{
	S_MEMORY_ASSERT(_is_power_of_two(alignment));
	uintptr_t p = ptr;
	uintptr_t a = alignment;
    uintptr_t padding = 0;
	uintptr_t needed_space = (uintptr_t)header_size;
	uintptr_t modulo = p & (a-1); // (p % a) as it assumes alignment is a power of two
	if (modulo != 0) padding = a - modulo;

	if (padding < needed_space) 
    {
		needed_space-=padding;
		if ((needed_space & (a-1)) != 0) padding+=a*(1+(needed_space/a));
        else padding+=a*(needed_space/a);
	}
	return (size_t)padding;
}

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

static void
_set_default_state(sLinearAllocator* allocator)
{
    S_MEMORY_ASSERT(allocator);
    allocator->type = S_LINEAR_ALLOCATOR;
    allocator->parentType = S_ALLOCATOR_TYPE_NONE;
    allocator->parentAllocator = nullptr;
    allocator->buffer = nullptr;
    allocator->bufferSize = 0u;
    allocator->currentOffset = 0u;
}

void
sLinearAllocator::initialize(size_t size)
{
    _set_default_state(this);
    S_MEMORY_ASSERT(size > 0u);
    parentType = S_DEFAULT_ALLOCATOR;
    bufferSize = size;
    buffer = (unsigned char*)S_MEMORY_ALLOC(size);
}

void
sLinearAllocator::initialize(size_t size, void* allocator)
{
    _set_default_state(this);
    S_MEMORY_ASSERT(size > 0u);
    S_MEMORY_ASSERT(allocator != nullptr);

    if (allocator == nullptr)
        return;

    parentAllocator = allocator;
    bufferSize = size;
    parentType = *(sAllocatorType*)allocator;
    switch (parentType)
    {
    case S_LINEAR_ALLOCATOR:
    {
        auto parentAllocator = (sLinearAllocator*)allocator;
        buffer = (unsigned char*)parentAllocator->request_aligned_memory(size, size);
        break;
    }
    case S_STACK_ALLOCATOR:
    {
        auto parentAllocator = (sStackAllocator*)allocator;
        buffer = (unsigned char*)parentAllocator->request_aligned_memory(size, size);
        break;
    }
    case S_POOL_ALLOCATOR:
    {
        auto parentAllocator = (sPoolAllocator*)allocator;
        buffer = (unsigned char*)parentAllocator->request_memory();
        break;
    }
    case S_GENERAL_LL_ALLOCATOR:
    {
        auto parentAllocator = (sGeneralLLAllocator*)allocator;
        buffer = (unsigned char*)parentAllocator->request_aligned_memory(size, size);
        break;
    }
    default:
        S_MEMORY_ASSERT(false && "Parent allocator type not supported");
        break;
    }
}

void
sLinearAllocator::initialize(void* memory, size_t size)
{
    _set_default_state(this);
    S_MEMORY_ASSERT(size > 0u);
    S_MEMORY_ASSERT(memory != nullptr);
    parentType = S_EXTERNAL_ALLOCATOR;
    bufferSize = size;
    buffer = (unsigned char*)memory;
}

void
sLinearAllocator::free_memory()
{
    if (buffer)
    {
        switch (parentType)
        {
        case S_STACK_ALLOCATOR:
        {
            S_MEMORY_ASSERT(parentAllocator);
            auto parentAllocator = (sStackAllocator*)this->parentAllocator;
            parentAllocator->return_memory(buffer);
            break;
        }
        case S_POOL_ALLOCATOR:
        {
            S_MEMORY_ASSERT(this->parentAllocator);
            auto parentAllocator = (sPoolAllocator*)this->parentAllocator;
            parentAllocator->return_memory(buffer);
            break;
        }
        case S_GENERAL_LL_ALLOCATOR:
        {
            S_MEMORY_ASSERT(parentAllocator);
            auto parentAllocator = (sGeneralLLAllocator*)this->parentAllocator;
            parentAllocator->return_memory(buffer);
            break;
        }
        case S_DEFAULT_ALLOCATOR:
        {
            S_MEMORY_FREE(buffer);
            break;
        }
        }     
    }
    _set_default_state(this);
}

void*
sLinearAllocator::request_memory(size_t size)
{
    size_t offset = currentOffset + size;

    if (offset > bufferSize) // make sure we have enough memory
    {
        S_MEMORY_ASSERT(false && "Linear allocator doesn't have enough free memory.");
        return nullptr;
    }

    // update offset pointer
    auto memory = (byte*)buffer + currentOffset;
    currentOffset = offset; // new offset
    return memory;
}

void*
sLinearAllocator::request_aligned_memory(size_t size, size_t alignment)
{
    S_MEMORY_ASSERT(_is_power_of_two(alignment));
    uintptr_t curr_ptr = (uintptr_t)buffer + (uintptr_t)currentOffset;
    uintptr_t offset = _align_forward_uintptr(curr_ptr, alignment);
    offset -= (uintptr_t)buffer;

    // Check to see if the backing memory has space left
	if (offset+size <= bufferSize) 
    {
		void *ptr = &buffer[offset];
		currentOffset = offset+size;

		// Zero new memory by default
		memset(ptr, 0, size);
		return ptr;
	}
	// Return NULL if the arena is out of memory (or handle differently)
    S_MEMORY_ASSERT(false && "Linear allocator doesn't have enough free memory.");
	return nullptr;
}

//-----------------------------------------------------------------------------
// [SECTION] Stack Allocator
//-----------------------------------------------------------------------------

static void
_set_default_state(sStackAllocator* allocator)
{
    S_MEMORY_ASSERT(allocator);
    allocator->type = S_STACK_ALLOCATOR;
    allocator->parentType = S_ALLOCATOR_TYPE_NONE;
    allocator->parentAllocator = nullptr;
    allocator->buffer = nullptr;
    allocator->bufferSize = 0u;
    allocator->currentOffset = 0u;
    allocator->currentID = 0u;
}

void
sStackAllocator::initialize(size_t size)
{
    _set_default_state(this);
    S_MEMORY_ASSERT(size > 0u);
    type = S_STACK_ALLOCATOR;
    parentType = S_DEFAULT_ALLOCATOR;
    bufferSize = size;
    buffer = (unsigned char*)S_MEMORY_ALLOC(size);
}

void 
sStackAllocator::initialize(size_t size, void* allocator)
{
    _set_default_state(this);
    S_MEMORY_ASSERT(size > 0u);
    S_MEMORY_ASSERT(allocator != nullptr);

    if (allocator == nullptr)
        return;

    parentType = *(sAllocatorType*)allocator;
    parentAllocator = allocator;
    bufferSize = size;
    switch (parentType)
    {
    case S_LINEAR_ALLOCATOR:
    {
        auto parentAllocator = (sLinearAllocator*)allocator;
        buffer = (unsigned char*)parentAllocator->request_aligned_memory(size, size);
        break;
    }
    case S_STACK_ALLOCATOR:
    {
        auto parentAllocator = (sStackAllocator*)allocator;
        buffer = (unsigned char*)parentAllocator->request_aligned_memory(size, size);
        break;
    }
    case S_POOL_ALLOCATOR:
    {
        auto parentAllocator = (sPoolAllocator*)allocator;
        buffer = (unsigned char*)parentAllocator->request_memory();
        break;
    }
        case S_GENERAL_LL_ALLOCATOR:
    {
        auto parentAllocator = (sGeneralLLAllocator*)allocator;
        buffer = (unsigned char*)parentAllocator->request_aligned_memory(size, size);
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
    _set_default_state(this);
    S_MEMORY_ASSERT(size > 0u);
    S_MEMORY_ASSERT(memory != nullptr);
    parentType = S_EXTERNAL_ALLOCATOR;
    bufferSize = size;
    buffer = (unsigned char*)memory;
}

void
sStackAllocator::free_memory()
{
    if (buffer)
    {
        switch (parentType)
        {
        case S_STACK_ALLOCATOR:
        {
            S_MEMORY_ASSERT(parentAllocator);
            auto parentAllocator = (sStackAllocator*)this->parentAllocator;
            parentAllocator->return_memory(buffer);
            break;
        }
        case S_POOL_ALLOCATOR:
        {
            S_MEMORY_ASSERT(parentAllocator);
            auto parentAllocator = (sPoolAllocator*)this->parentAllocator;
            parentAllocator->return_memory(buffer);
            break;
        }
        case S_GENERAL_LL_ALLOCATOR:
        {
            S_MEMORY_ASSERT(parentAllocator);
            auto parentAllocator = (sGeneralLLAllocator*)this->parentAllocator;
            parentAllocator->return_memory(buffer);
            break;
        }
        case S_DEFAULT_ALLOCATOR:
        {
            S_MEMORY_FREE(buffer);
            break;
        }
        default:
            break;
        }
    }
    _set_default_state(this);
}

void*
sStackAllocator::request_memory(size_t size)
{
    size_t offset = currentOffset + size + sizeof(sStackAllocatorHeader);

    if (offset > bufferSize) // make sure we have enough memory
    {
        S_MEMORY_ASSERT(false && "Stack allocator is full.");
        return nullptr;
    }

    // update header
    auto header_raw_memory = (byte*)buffer + currentOffset;
    auto header = (sStackAllocatorHeader*)header_raw_memory;
    header->size = size+sizeof(sStackAllocatorHeader);
    header->ID = ++currentID;

    // update offset pointer
    auto memory = (byte*)buffer + currentOffset + sizeof(sStackAllocatorHeader);
    currentOffset = offset; // new offset
    return memory;
}

void*
sStackAllocator::request_aligned_memory(size_t size, size_t alignment)
{

    S_MEMORY_ASSERT(_is_power_of_two(alignment));
    if (alignment < 2u)
        return request_memory(size);

    auto currentMemoryLocation = (byte*)buffer + currentOffset;
    const size_t paddingBytes = (size_t)(currentMemoryLocation + sizeof(sStackAllocatorHeader)) % alignment;
    const size_t sizeOfBlock = paddingBytes + size + sizeof(sStackAllocatorHeader);
    const size_t offset = currentOffset + sizeOfBlock;

    if (offset > bufferSize)
    {
        S_MEMORY_ASSERT(false && "Stack allocator doesn't have enough room.");
        return nullptr;
    }
    currentOffset = offset; // new offset

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
        currentOffset = currentOffset - header->size;
        return true;
    }
    S_MEMORY_ASSERT(false && "Stack allocator requires memory to be returned in order.");
    return false;
}

//-----------------------------------------------------------------------------
// [SECTION] Pool Allocator
//-----------------------------------------------------------------------------

static void
_set_default_state(sPoolAllocator* allocator)
{
    S_MEMORY_ASSERT(allocator);
    allocator->type = S_POOL_ALLOCATOR;
    allocator->parentType = S_ALLOCATOR_TYPE_NONE;
    allocator->parentAllocator = nullptr;
    allocator->buffer = nullptr;
    allocator->bufferSize = 0u;
    allocator->chunkSize = 0u;
    allocator->count = 0u;
    allocator->freeItemCount = 0u;
    allocator->head = nullptr;
}

void
sPoolAllocator::initialize(size_t itemCount, size_t itemSize, size_t alignment)
{
    _set_default_state(this);
    S_MEMORY_ASSERT(itemSize > 0u);
    S_MEMORY_ASSERT(itemCount > 0u);
    S_MEMORY_ASSERT(alignment > 0u);
    parentType = S_DEFAULT_ALLOCATOR;
    count = itemCount;
    freeItemCount = itemCount;
    chunkSize = _align_forward_size(itemSize, alignment);
    S_MEMORY_ASSERT(chunkSize >= sizeof(sPoolAllocatorNode) && "Chunk size too small.");
    bufferSize = chunkSize * itemCount;
    buffer = (unsigned char*)S_MEMORY_ALLOC(bufferSize);
    head = (sPoolAllocatorNode*)buffer;

    for(int i = 0; i < itemCount-1; i++)
    {
        ((sPoolAllocatorNode*)&buffer[chunkSize*i])->nextNode = (sPoolAllocatorNode*)&buffer[chunkSize*(i+1)];
        ((sPoolAllocatorNode*)&(buffer[chunkSize*(i+1)]))->nextNode = nullptr;
    }
}

void
sPoolAllocator::initialize(size_t itemCount, size_t itemSize, size_t alignment, void* allocator)
{
    _set_default_state(this);
    S_MEMORY_ASSERT(itemSize > 0u);
    S_MEMORY_ASSERT(itemCount > 0u);
    S_MEMORY_ASSERT(alignment > 0u);
    S_MEMORY_ASSERT(allocator != nullptr);

    if (allocator == nullptr)
        return;
    parentType = *(sAllocatorType*)allocator;
    parentAllocator = allocator;
    count = itemCount;
    freeItemCount = itemCount;
    chunkSize = _align_forward_size(itemSize, alignment);
    bufferSize = chunkSize*itemCount;
    switch (parentType)
    {
    case S_LINEAR_ALLOCATOR:
    {   
        auto parentAllocator = (sLinearAllocator*)allocator;
        buffer = (unsigned char*)parentAllocator->request_aligned_memory(bufferSize, bufferSize);
        break;
    }
    case S_STACK_ALLOCATOR:
    {
        auto parentAllocator = (sStackAllocator*)allocator;
        buffer = (unsigned char*)parentAllocator->request_aligned_memory(bufferSize, bufferSize);
        break;
    }
    case S_POOL_ALLOCATOR:
    {
        auto parentAllocator = (sPoolAllocator*)allocator;
        buffer = (unsigned char*)parentAllocator->request_memory();
        break;
    }
    case S_GENERAL_LL_ALLOCATOR:
    {
        auto parentAllocator = (sGeneralLLAllocator*)allocator;
        buffer = (unsigned char*)parentAllocator->request_aligned_memory(bufferSize, bufferSize);
        break;
    }
    default:
        S_MEMORY_ASSERT(false && "Parent allocator type not supported");
        break;
    }

    head = (sPoolAllocatorNode*)buffer;

    for(int i = 0; i < itemCount-1; i++)
    {
        ((sPoolAllocatorNode*)&buffer[chunkSize*i])->nextNode = (sPoolAllocatorNode*)&buffer[chunkSize*(i+1)];
        ((sPoolAllocatorNode*)&(buffer[chunkSize*(i+1)]))->nextNode = nullptr;
    }
}

void
sPoolAllocator::initialize(size_t itemCount, size_t itemSize, size_t alignment, void* memory, size_t size)
{
    _set_default_state(this);
    S_MEMORY_ASSERT(itemSize > 0u);
    S_MEMORY_ASSERT(itemCount > 0u);
    S_MEMORY_ASSERT(alignment > 0u);
    S_MEMORY_ASSERT(size > 0u);
    S_MEMORY_ASSERT(memory);

    if(memory == nullptr)
        return;

    parentType = S_EXTERNAL_ALLOCATOR;
    count = itemCount;
    freeItemCount = itemCount;
    chunkSize = _align_forward_size(itemSize, alignment);

    uintptr_t initial_start = (uintptr_t)memory;
	uintptr_t start = _align_forward_uintptr(initial_start, (uintptr_t)alignment);
    size-=(size_t)(start - initial_start);
    bufferSize = size;
    
    S_MEMORY_ASSERT(chunkSize >= sizeof(sPoolAllocatorNode) && "Chunk size too small.");
    S_MEMORY_ASSERT(size >= chunkSize * itemCount && "Chunk size too small.");

    buffer = (unsigned char*)memory;
    head = (sPoolAllocatorNode*)buffer;

    for(int i = 0; i < itemCount-1; i++)
    {
        ((sPoolAllocatorNode*)&buffer[chunkSize*i])->nextNode = (sPoolAllocatorNode*)&buffer[chunkSize*(i+1)];
        ((sPoolAllocatorNode*)&(buffer[chunkSize*(i+1)]))->nextNode = nullptr;
    }
}

void
sPoolAllocator::free_memory()
{
    if (buffer)
    {
        switch (parentType)
        {
        case S_STACK_ALLOCATOR:
        {
            S_MEMORY_ASSERT(parentAllocator);
            auto parentAllocator = (sStackAllocator*)this->parentAllocator;
            parentAllocator->return_memory(buffer);
            break;
        }
        case S_POOL_ALLOCATOR:
        {
            S_MEMORY_ASSERT(parentAllocator);
            auto parentAllocator = (sPoolAllocator*)this->parentAllocator;
            parentAllocator->return_memory(buffer);
            break;
        }
        case S_DEFAULT_ALLOCATOR:
        {
            S_MEMORY_FREE(buffer);
            break;
        }
        case S_GENERAL_LL_ALLOCATOR:
        {
            S_MEMORY_ASSERT(parentAllocator);
            auto parentAllocator = (sGeneralLLAllocator*)this->parentAllocator;
            parentAllocator->return_memory(buffer);
            break;
        }
        default:
            break;
        }
    }
    _set_default_state(this);
}

void*
sPoolAllocator::request_memory()
{
    S_MEMORY_ASSERT(freeItemCount > 0u);
    freeItemCount--;
    sPoolAllocatorNode* firstFreeNode = head;
    sPoolAllocatorNode* nextNode = firstFreeNode->nextNode;
    head = nextNode;
    return firstFreeNode;
}

bool
sPoolAllocator::return_memory(void* ptr)
{
    freeItemCount++;
    sPoolAllocatorNode* oldFreeNode = head;
    head = (sPoolAllocatorNode*)ptr;
    head->nextNode = oldFreeNode;
    return false;
}

//-----------------------------------------------------------------------------
// [SECTION] General Allocator (Freelist using linked list)
//-----------------------------------------------------------------------------

static void
_set_default_state(sGeneralLLAllocator* allocator)
{
    S_MEMORY_ASSERT(allocator);
    allocator->type = S_GENERAL_LL_ALLOCATOR;
    allocator->parentType = S_ALLOCATOR_TYPE_NONE;
    allocator->parentAllocator = nullptr;
    allocator->buffer = nullptr;
    allocator->bufferSize = 0u;
    allocator->used = 0u;
    allocator->head = nullptr;
    allocator->placementPolicy = S_ALLOC_PLACEMENT_POLICY_NONE;
}

inline void
_remove_node(sGeneralLLAllocatorNode **phead, sGeneralLLAllocatorNode *prev_node, sGeneralLLAllocatorNode *del_node) 
{
    if (prev_node == nullptr) *phead = del_node->nextNode; 
    else prev_node->nextNode = del_node->nextNode; 
}

inline void
_insert_node(sGeneralLLAllocatorNode **phead, sGeneralLLAllocatorNode *prev_node, sGeneralLLAllocatorNode *new_node) 
{
    if (prev_node == nullptr) 
    {
        if (*phead != nullptr) new_node->nextNode = *phead; 
        else *phead = new_node;
    } 
    else 
    {
        if (prev_node->nextNode == nullptr) 
        {
            prev_node->nextNode = new_node;
            new_node->nextNode  = nullptr;
        }
         else
          {
            new_node->nextNode  = prev_node->nextNode;
            prev_node->nextNode = new_node;
        }
    }
}

void
sGeneralLLAllocator::initialize(size_t size, sAllocatorPlacementPolicy policy)
{
    _set_default_state(this);
    buffer = S_MEMORY_ALLOC(size);
    placementPolicy = policy;
    bufferSize = size;
    parentType = S_DEFAULT_ALLOCATOR;
    head = (sGeneralLLAllocatorNode*)buffer;
    head->blockSize = size;
    head->nextNode = nullptr;
}

void
sGeneralLLAllocator::initialize(size_t size, sAllocatorPlacementPolicy policy, void* allocator)
{
    _set_default_state(this);
    S_MEMORY_ASSERT(allocator != nullptr);
    S_MEMORY_ASSERT(size > 0);
    
    if (allocator == nullptr)
        return;
 
    parentType = *(sAllocatorType*)allocator;
    switch (parentType)
    {
    case S_LINEAR_ALLOCATOR:
    {   
        auto parentAllocator = (sLinearAllocator*)allocator;
        buffer = (unsigned char*)parentAllocator->request_aligned_memory(size, size);
        break;
    }
    case S_STACK_ALLOCATOR:
    {
        auto parentAllocator = (sStackAllocator*)allocator;
        buffer = (unsigned char*)parentAllocator->request_aligned_memory(size, size);
        break;
    }
    case S_POOL_ALLOCATOR:
    {
        auto parentAllocator = (sPoolAllocator*)allocator;
        buffer = (unsigned char*)parentAllocator->request_memory();
        break;
    }
    case S_GENERAL_LL_ALLOCATOR:
    {
        auto parentAllocator = (sGeneralLLAllocator*)allocator;
        buffer = (unsigned char*)parentAllocator->request_aligned_memory(size, size);
        break;
    }
    default:
        S_MEMORY_ASSERT(false && "Parent allocator type not supported");
        break;
    }
    
    placementPolicy = policy;
    parentAllocator = allocator;
    bufferSize = size;
    head = (sGeneralLLAllocatorNode*)buffer;
    head->blockSize = size;
    head->nextNode = nullptr;
    if(buffer == nullptr)
    {
        S_MEMORY_ASSERT(false && "Buffer could not be allocated.");
       _set_default_state(this); 
    }
}

void
sGeneralLLAllocator::initialize(void* memory, sAllocatorPlacementPolicy policy, size_t size)
{
    _set_default_state(this);
    S_MEMORY_ASSERT(memory);
    S_MEMORY_ASSERT(size > 0);
    if(memory == nullptr)
        return;
    buffer = memory;
    placementPolicy = policy;
    bufferSize = size;
    parentType = S_EXTERNAL_ALLOCATOR;
    head = (sGeneralLLAllocatorNode*)buffer;
    head->blockSize = size;
    head->nextNode = nullptr;
}

void sGeneralLLAllocator::free_memory()
{
    if (buffer)
    {
        switch (parentType)
        {
        case S_STACK_ALLOCATOR:
        {
            S_MEMORY_ASSERT(parentAllocator);
            auto parentAllocator = (sStackAllocator*)this->parentAllocator;
            parentAllocator->return_memory(buffer);
            break;
        }
        case S_POOL_ALLOCATOR:
        {
            S_MEMORY_ASSERT(parentAllocator);
            auto parentAllocator = (sPoolAllocator*)this->parentAllocator;
            parentAllocator->return_memory(buffer);
            break;
        }
        case S_GENERAL_LL_ALLOCATOR:
        {
            S_MEMORY_ASSERT(parentAllocator);
            auto parentAllocator = (sGeneralLLAllocator*)this->parentAllocator;
            parentAllocator->return_memory(buffer);
            break;
        }
        case S_DEFAULT_ALLOCATOR:
        {
            S_MEMORY_FREE(buffer);
            break;
        }
        default:
            break;
        }
    }
    _set_default_state(this);
}

void*
sGeneralLLAllocator::request_memory(size_t size)
{
    return request_aligned_memory(size, size);
}

static sGeneralLLAllocatorNode*
_request_aligned_memory_find_first(sGeneralLLAllocator* allocator, size_t size, size_t alignment, size_t *padding_, sGeneralLLAllocatorNode **prev_node_)
{
    sGeneralLLAllocatorNode* node = allocator->head;
    sGeneralLLAllocatorNode* prevNode = nullptr;
    size_t padding = 0;
    while(node)
    {
        padding = _calc_padding_with_header((uintptr_t)node, alignment, sizeof(sGeneralLLAllocatorHeader));
        size_t requiredSpace = size + padding;
        if(node->blockSize >= requiredSpace)
            break;
        prevNode = node;
        node = node->nextNode;
    }
    if (padding_) *padding_ = padding;
    if (prev_node_) *prev_node_ = prevNode;
    return node;
}

static sGeneralLLAllocatorNode*
_request_aligned_memory_find_best(sGeneralLLAllocator* allocator, size_t size, size_t alignment, size_t *padding_, sGeneralLLAllocatorNode **prev_node_)
{
    size_t smallestDiff = ~(size_t)0;
    sGeneralLLAllocatorNode* node = allocator->head;
    sGeneralLLAllocatorNode* prevNode = nullptr;
    sGeneralLLAllocatorNode* bestNode = nullptr;
    size_t padding = 0;
    while(node)
    {
        padding = _calc_padding_with_header((uintptr_t)node, (uintptr_t)alignment, sizeof(sGeneralLLAllocatorHeader));
        size_t requiredSpace = size + padding;
        if(node->blockSize >= requiredSpace && (node->blockSize - requiredSpace < smallestDiff))
        {
            bestNode = node;
            smallestDiff = node->blockSize - requiredSpace;
            if (padding_) *padding_ = padding;
            if (prev_node_) *prev_node_ = prevNode;
        }
        prevNode = node;
        node = node->nextNode;
    }

    return bestNode;
}

void*
sGeneralLLAllocator::request_aligned_memory(size_t size, size_t alignment)
{
    S_MEMORY_ASSERT(size > 0u);

    alignment = alignment < 8 ? 8 : alignment;
    size = size < sizeof(sGeneralLLAllocatorNode) ? sizeof(sGeneralLLAllocatorNode) : size;

    size_t padding = 0;  
    sGeneralLLAllocatorNode *node = nullptr;
    sGeneralLLAllocatorNode *prev_node = nullptr;
    switch(placementPolicy)
    {
        case S_ALLOC_PLACEMENT_POLICY_FIND_BEST:  node = _request_aligned_memory_find_best(this, size, alignment, &padding, &prev_node); break;
        case S_ALLOC_PLACEMENT_POLICY_FIND_FIRST: node = _request_aligned_memory_find_first(this, size, alignment, &padding, &prev_node); break;
    }

    if (node == nullptr) 
    {
        S_MEMORY_ASSERT(false && "Free list has no free memory");
        return nullptr;
    }
    
    size_t alignment_padding = padding - sizeof(sGeneralLLAllocatorHeader);
    size_t required_space = size + padding;
    used += required_space;
    size_t remaining = node->blockSize - required_space;
    
    if (remaining > 0) 
    {
        auto new_node = (sGeneralLLAllocatorNode*)((char *)node + required_space);
        new_node->blockSize = remaining;
        _insert_node(&head, node, new_node);
    }
    _remove_node(&head, prev_node, node);
    
    auto header_ptr = (sGeneralLLAllocatorHeader*)((char*)node + alignment_padding);
    header_ptr->blockSize = required_space;
    header_ptr->padding = alignment_padding;
       
    return ((char *)header_ptr + sizeof(sGeneralLLAllocatorHeader));
}

void
sGeneralLLAllocator::return_memory(void* ptr)
{
    S_MEMORY_ASSERT(ptr);
    if (ptr == nullptr) return;
    
    auto header = (sGeneralLLAllocatorHeader*)((char*)ptr - sizeof(sGeneralLLAllocatorHeader));
    auto free_node = (sGeneralLLAllocatorNode*)header;
    free_node->blockSize = header->blockSize + header->padding;
    free_node->nextNode = nullptr;
    
    sGeneralLLAllocatorNode *node = head;
    sGeneralLLAllocatorNode* prev_node = nullptr;
    while (node != nullptr) 
    {
        if (ptr < node) 
        {
            _insert_node(&head, prev_node, free_node);
            break;
        }
        prev_node = node;
        node = node->nextNode;
    }
    
    used -= free_node->blockSize;

    if(prev_node == nullptr) // end of list
        return;

    
    // coalescence
    if (free_node->nextNode != nullptr && (void *)((char *)free_node + free_node->blockSize) == free_node->nextNode) 
    {
        free_node->blockSize += free_node->nextNode->blockSize;
        _remove_node(&head, free_node, free_node->nextNode);
    }
    
    if (prev_node->nextNode != nullptr && (void *)((char *)prev_node + prev_node->blockSize) == free_node) 
    {
        prev_node->blockSize += free_node->nextNode->blockSize;
        _remove_node(&head, prev_node, free_node);
    }
}

#endif // SEMPER_MEMORY_IMPLEMENTATION