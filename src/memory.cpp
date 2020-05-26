/*                                                                 -*- c++ -*-
 * Copyright © 2018 Ron R Wills <ron@digitalcombine.ca>
 *
 * This file is part of Cutlet.
 *
 * Cutlet is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cutlet is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cutlet.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <cutlet/memory.h>

//#define DEBUG_MEM 1

#if DEBUG_MEM
#pragma message ("MEM debugging enabled")
#include <iostream>
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

void *
memory::recyclable::operator new(std::size_t size) {
  return gc::alloc(size);
}

/**************************************
 * memory::recyclable::operator new[] *
 **************************************/

void *
memory::recyclable::operator new[](std::size_t size) {
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

  } else {
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
  if (mptr->size + free_size < limit) {
#if DEBUG_MEM
    std::clog << "MEM: composting " << mptr->size << " bytes @ "
              << (void *)mptr << std::endl;
#endif
    free_memory.insert(std::make_pair(mptr->size, mptr));
    free_size += mptr->size;
    time(&(mptr->collected_time));
  } else {
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
  while (free_memory.size()) {
    auto block = free_memory.begin();
    //for (auto &block: free_memory) {
#if DEBUG_MEM
    std::clog << "MEM: collocting @ " << (void *)block->second
              << " " << ((memory_t *)block->second)->size
              << " bytes" << std::endl;
#endif
    ::operator delete(block->second);
    free_memory.erase(block);
  }
  free_memory.clear();
}
