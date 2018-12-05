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

std::size_t memory::GC::limit = 5 * 1024 * 1024; // 5M
time_t memory::GC::collection_age = 60 * 10; // 10 minutes
std::multimap<std::size_t, void *> memory::GC::free_memory;
std::size_t memory::GC::free_size = 0;

/*********************
 * memory::GC::alloc *
 *********************/

void *memory::GC::alloc(std::size_t size) {
  unsigned char *res = 0;
  std::multimap<std::size_t, void *>::iterator it = free_memory.find(size);

  if (it != free_memory.end()) {
    res = (unsigned char *)(it->second);
    free_memory.erase(it);
    free_size -= ((memory_t*)res)->size;
  } else {
    res = (unsigned char *)::operator new(size + sizeof(memory_t));
    ((memory_t *)(res))->size = size;
  }

  return (res + sizeof(memory_t));
}

/***********************
 * memory::GC::recycle *
 ***********************/

void memory::GC::recycle(void *ptr) noexcept {
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
 * memory::GC::collect *
 ***********************/

void memory::GC::collect() {
  memory_t *mptr = NULL;
  for (std::multimap<std::size_t, void *>::iterator it = free_memory.begin();
       it != free_memory.end(); ++it) {
    mptr = (memory_t *)it->second;
    if (mptr->collected_time - time(NULL) >= collection_age) {
      ::operator delete(mptr);
    }
  }
}

/***************************
 * memory::GC::collect_all *
 ***************************/

void memory::GC::collect_all() {
  for (std::multimap<std::size_t, void *>::iterator it = free_memory.begin();
       it != free_memory.end(); ++it) {
    ::operator delete(it->second);
  }
}

/************************************
 * memory::Recyclable::operator new *
 ************************************/

void *memory::Recyclable::operator new(std::size_t size) {
  return GC::alloc(size);
}

/***************************************
 * memory::Recyclable::operator delete *
 ***************************************/

void memory::Recyclable::operator delete(void *ptr) noexcept {
  GC::recycle(ptr);
}
