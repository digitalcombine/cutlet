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

typedef struct memory_s {
  std::size_t size;
  time_t collected_time;
} memory_t;

std::size_t memory::gc::limit = 5 * 1024 * 1024; // 5M
time_t memory::gc::collection_age = 60; // 1 minute
std::multimap<std::size_t, void *> memory::gc::free_memory;
std::size_t memory::gc::free_size = 0;

/*********************
 * memory::gc::alloc *
 *********************/

#include <iostream>

void *memory::gc::alloc(std::size_t size) {
  unsigned char *res = 0;

  /* To help further speed up allocation, we don't just try to find the
   * exact size of memory requested. Instead we use lower_bound to find a
   * memory block that would fit the request and make sure it's no bigger
   * than the requested size by 50 bytes.
   */
  auto it = free_memory.lower_bound(size);
  if (it != free_memory.end() and (it->first - size) < 50) {
    res = (unsigned char *)(it->second);
    free_memory.erase(it);
    free_size -= ((memory_t*)res)->size;
  } else {
    // An appropriate block couldn't be found so get more from the system.
    res = (unsigned char *)::operator new(size + sizeof(memory_t));
    ((memory_t *)(res))->size = size;
  }

  return (res + sizeof(memory_t));
}

/***********************
 * memory::gc::recycle *
 ***********************/

void memory::gc::recycle(void *ptr) throw() {
  /* Add the memory block to the free list according to it's size and update
   * it's timestamp.
   */
  memory_t *mptr = (memory_t *)((unsigned char *)ptr - sizeof(memory_t));
  if (mptr->size + free_size < limit) {
    free_memory.insert(std::make_pair(mptr->size, mptr));
    free_size += mptr->size;
    time(&(mptr->collected_time));
  } else {
    ::operator delete(mptr);
  }
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
  for (auto &block: free_memory)
    ::operator delete(block.second);
  free_memory.clear();
}

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
