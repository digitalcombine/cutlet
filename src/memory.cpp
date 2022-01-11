/*                                                                 -*- c++ -*-
 * Copyright © 2018 Ron R Wills <ron@digitalcombine.ca>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <libcutlet/memory>

//#define DEBUG_MEM 1
#define MEM_THREAD_SAFE 1

#if DEBUG_MEM
#pragma message ("MEM debugging enabled")
#include <iostream>
#endif

#if MEM_THREAD_SAFE
#include <mutex>

static std::recursive_mutex mtx;
#endif

/*  The gc_cleaner class is used to ensure that memory::gc::collect_all is
 * called at the end of the program. We use the fact that even global objects
 * will get their destructors called at the of the program to clean up the
 * garbage collector's cache.
 */

class gc_cleaner {
public:
  gc_cleaner() {}
  ~gc_cleaner() { memory::gc::collect_all(); }
};

static gc_cleaner cleaner;


/******************************************************************************
 * class memory::recyclable
 */

/************************************
 * memory::recyclable::operator new *
 ************************************/

void *memory::recyclable::operator new(std::size_t size) {
  return gc::alloc(size);
}

/**************************************
 * memory::recyclable::operator new[] *
 **************************************/

void *memory::recyclable::operator new[](std::size_t size) {
  return gc::alloc(size);
}

/***************************************
 * memory::recyclable::operator delete *
 ***************************************/

void memory::recyclable::operator delete(void *ptr) noexcept {
  gc::recycle(ptr);
}

/*****************************************
 * memory::recyclable::operator delete[] *
 *****************************************/

void memory::recyclable::operator delete[](void *ptr) noexcept {
  gc::recycle(ptr);
}

/******************************************************************************
 * class memory::gc
 */

/** Memory tracking structure.
 */
typedef struct memory_s {
  std::size_t size;
  time_t collected_time;
} memory_t;

// Global settings.
std::size_t memory::gc::limit = 5 * 1024 * 1024; // 5M
time_t memory::gc::collection_age = 60; // 1 minute

// Private memory tracking.
std::multimap<std::size_t, void *> memory::gc::free_memory;
std::size_t memory::gc::free_size = 0;

/*********************
 * memory::gc::alloc *
 *********************/

void *memory::gc::alloc(std::size_t size) {
  unsigned char *res = 0;

#if DEBUG_MEM
  std::clog << "memory::gc::alloc(" << size << ")" << std::endl;
#endif

#if MEM_THREAD_SAFE
  mtx.lock();
#endif

  /* To help further speed up allocation, we don't just try to find the
   * exact size of memory requested. Instead we use lower_bound to find a
   * memory block that would fit the request and make sure it's no bigger
   * than the requested size by 50 bytes.
   */
  auto it = free_memory.lower_bound(size);
  if (it != free_memory.end() and (it->first - size) < 50) {

    res = (unsigned char *)(it->second);
    free_memory.erase(it);
#if DEBUG_MEM
    std::clog << "MEM: recycled " << ((memory_t*)res)->size
              << " for " << size << " bytes @ "
              << (void *)res << std::endl;
#endif
    free_size -= ((memory_t*)res)->size;

#if MEM_THREAD_SAFE
    mtx.unlock();
#endif

  } else {

#if MEM_THREAD_SAFE
    mtx.unlock();
#endif

    // An appropriate block couldn't be found so get more from the system.
    res = (unsigned char *)::operator new(size + sizeof(memory_t));
    ((memory_t *)(res))->size = size;
#if DEBUG_MEM
    std::clog << "MEM: allocating " << size << " bytes"
              << " @ " << (void *)res << std::endl;
#endif
  }

#if DEBUG_MEM
  for (auto &block: free_memory) {
    std::clog << "  aging " << ((memory_t *)block.second)->size << " bytes"
              << " @ " << (void *)(block.second) << std::endl;
  }
#endif

  return (res + sizeof(memory_t));
}

/***********************
 * memory::gc::recycle *
 ***********************/

void memory::gc::recycle(void *ptr) noexcept {

#if DEBUG_MEM
  std::clog << "memory::gc::recycle(" << ptr << ")" << std::endl;
#endif

  /* Add the memory block to the free list according to it's size and update
   * it's timestamp.
   */
  memory_t *mptr = (memory_t *)((unsigned char *)ptr - sizeof(memory_t));

#if MEM_THREAD_SAFE
  mtx.lock();
#endif

  if (mptr->size + free_size < limit) {
#if DEBUG_MEM
    std::clog << "MEM: composting " << mptr->size << " bytes @ "
              << (void *)mptr << std::endl;
#endif

    free_memory.insert(std::make_pair(mptr->size, mptr));
    free_size += mptr->size;
    time(&(mptr->collected_time));

#if MEM_THREAD_SAFE
    mtx.unlock();
#endif

  } else {
#if MEM_THREAD_SAFE
    mtx.unlock();
#endif

#if DEBUG_MEM
    std::clog << "MEM: deleting " << mptr->size << " bytes" << std::endl;
#endif
    ::operator delete(mptr);
  }

#if DEBUG_MEM
  for (auto &block: free_memory) {
    std::clog << "  aging " << ((memory_t *)block.second)->size << " bytes"
              << " @ " << (void *)(block.second) << std::endl;
  }
#endif
}

/***********************
 * memory::gc::collect *
 ***********************/

void memory::gc::collect(void) {
  /* Find blocks of memory that are older than the collection_age and
   * actually give them back to the system.
   */
  memory_t *mptr = NULL;

#if MEM_THREAD_SAFE
  mtx.lock();
#endif

  auto it = free_memory.begin();
  while (it != free_memory.end()) {
    mptr = (memory_t *)it->second;
    if (mptr->collected_time - time(NULL) >= collection_age) {
#if DEBUG_MEM
      std::clog << "MEM: collecting old @ " << (void *)it->second << " "
                << mptr->size << " bytes" << std::endl;
#endif
      ::operator delete(it->second);
      it = free_memory.erase(it);
    } else {
      ++it;
    }
  }

#if MEM_THREAD_SAFE
  mtx.unlock();
#endif
}

/***************************
 * memory::gc::collect_all *
 ***************************/

void memory::gc::collect_all(void) {
#if DEBUG_MEM
  std::clog << "memory::gc::collect_all()" << std::endl;
  std::clog << "MEM: gc collecting " << free_memory.size()
            << " remaining allocations" << std::endl;
#endif

#if MEM_THREAD_SAFE
  mtx.lock();
#endif

  for (auto &block: free_memory) {
#if DEBUG_MEM
    std::clog << "MEM: collocting @ " << (void *)block->second
              << " " << ((memory_t *)block->second)->size
              << " bytes" << std::endl;
#endif
    ::operator delete(block.second);
  }
  free_memory.clear();

#if MEM_THREAD_SAFE
  mtx.unlock();
#endif
}
