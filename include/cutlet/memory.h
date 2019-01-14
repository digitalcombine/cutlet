/*                                                                  -*- c++ -*-
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
 *
 * This does require C++ 11 to compile.
 */

#include <ctime>
#include <map>
#include <new>

#ifndef _CUTLET_MEMORY_H
#define _CUTLET_MEMORY_H

namespace memory {

  /** Makes dynamically allocated objects managed by the garbage collector.
   */
  class recyclable {
  public:

    /** Allocate memory for a new dynamic object from the garbage collector's
     * cache if possible, otherwise get more memory from the system.
     *
     * @param size The size of the object to allocate.
     * @return A pointer to the newly allocated object.
     * @see operator delete()
     */
    void *operator new(std::size_t size);

    /** Allocate memory for a new dynamic object array from the garbage
     * collector's cache if possible, otherwise get more memory from the
     * system.
     *
     * @param size The size of the object to allocate.
     * @return A pointer to the newly allocated object.
     * @see operator delete[]()
     */
    void *operator new[](std::size_t size);

    /** Deallocate a dynamically allocated object and place the memory
     * in the garbage collector's cache if the cache size hasn't exceded
     * memory::gc::limit.
     *
     * @param ptr The pointer to the dynamically allocated object.
     * @see operator new()
     */
    void operator delete(void *ptr) noexcept;

    /** Deallocate a dynamically allocated object array and place the memory
     * in the garbage collector's cache if the cache size hasn't exceded
     * memory::gc::limit.
     *
     * @param ptr The pointer to the dynamically allocated object.
     * @see operator new[]()
     */
    void operator delete[](void *ptr) noexcept;
  };

  /** Garbage collector for recyclable objects.
   * @see recyclable
   */
  class gc {
  public:
    /** This is the hard limit of the amount of memory we'll actually cache
     * for recycling. Anything about this will just get deleted.
     */
    static std::size_t limit;

    /** This is how long we'll keep recycled memory cached for before we return
     * it back to the system.
     */
    static time_t collection_age;

    /** Iterate through the cache and deallocate any memory that is older than
    * collection_age.
     */
    static void collect();

    /** Deallocate all the memory in the cache. This will always be called at
     * the end of the program and is not necessary to call implicitly.
     *
     * The only problem with this is if any global objects are recyclable and
     * possibly managed by reference pointers for example. There is no guaranty
     * that they will be cleaned up automatically before the garbage collector
     * cache is. The program should make sure all of these are deallocated to
     * prevent memory leaks, although at this point it's not critical.
     *
     * It is safe to call this method many times and even continue using the
     * garbage collect afterwards.
     */
    static void collect_all();

    friend class recyclable;

  private:
    static std::multimap<std::size_t, void *> free_memory;
    static std::size_t free_size;

    static void *alloc(std::size_t size);

    static void recycle(void *ptr) noexcept;
  };

  /** Reference counting smart pointer.
   */
  template <typename Ty>
  class reference {
  private:

    /** Reference Counting Class Wrapper.
     *  Used by reference to implement reference counting around a
     * dynamically created object of type Ty.
     */
    class __reference : public recyclable {
    public:

      /** Creates a wrapper around an existing native object.
       * obj    Pointer to the native object to add reference count to.
       * irefs  The initial reference count.
       */
      __reference(Ty *obj, unsigned int irefs = 0)
        : refs(irefs), wrefs(0), nobj(obj) {
      }

      /** Destroy the native object. */
      virtual ~__reference() throw() {
        if (nobj) delete nobj;
      }

      bool is_null() { return (nobj == nullptr); }

      void free() {
        /* We have to set nobj to null before deleting it to prevent any kind
         * of race conditions with weak pointers.
         */
        Ty *obj = nobj;
        nobj = nullptr;
        delete obj;
      }

      /** Increment the weak reference counter by 1. */
      void inc_weak_ref() { wrefs++; }

      /** Decrease the weak reference counter by 1. */
      void dec_weak_ref() { if (wrefs > 0) wrefs--; }

      /** Get the current weak reference count. */
      unsigned int __wref_cnt() const { return wrefs; }

      /** Increment the reference counter by 1. */
      void inc_ref() { refs++; }

      /** Decrease the reference counter by 1. */
      void dec_ref() { refs--; }

      /** Get the current reference count. */
      unsigned int __ref_cnt() const { return refs; }

      /** Cast this as a reference to the object being wrapped. */
      operator Ty &() { return (Ty &)*nobj; }

      /** Cast this as a pointer to the object being wrapped. */
      operator Ty*() { return (Ty *)nobj; }

    private:
      unsigned int refs;  // The number of references.
      unsigned int wrefs; // The number of weak references.
      Ty *nobj;           // The actual object.
    };

  public:
    /** Create a null reference pointer.
     */
    reference() : weak_ref(false), obj(nullptr) {
    }

    /** Create a new reference to a newly allocated object.
     */
    reference(Ty *obj) : weak_ref(false), obj(nullptr) {
      if (obj != nullptr) this->obj = new __reference(obj, 1);
    }

    /** Default copy constructor.
     */
    reference(const reference<Ty> &obj)
      : weak_ref(obj.weak_ref), obj(obj.obj) {
      inc_reference();
    }

    /** Decreases the reference count and clean up the object if necessary.
     */
    ~reference() noexcept {
      if (obj) {
        if (weak_ref) {
          obj->dec_weak_ref();
          if (obj->__ref_cnt() == 0 and obj->__wref_cnt() == 0) {
            delete obj; obj = nullptr;
          }
        } else {
          obj->dec_ref();
          if (obj->__ref_cnt() == 0) {
            if (obj->__wref_cnt()) {
              obj->free();
            } else {
              delete obj; obj = nullptr;
            }
          }
        }
      }
    }

    /** Get the number of references to a object.
     * @return The number of references
     */
    unsigned int references() {
      return (obj != nullptr ? obj->__ref_cnt() : 0);
    }

    unsigned int weak_references() {
      return (obj != nullptr ? obj->__wref_cnt() : 0);
    }

    /** Test if we reference an object or just a NULL pointer.
     * @return If true if the reference in pointing to null.
     */
    bool is_null() const {
      return ((obj == nullptr) or obj->is_null());
    }

    /** Creates a new weak reference.
     * @see normal()
     */
    reference weak() {
      reference result;
      result.obj = obj;
      result.weak_ref = true;
      result.inc_reference();
      return result;
    }

    /** Creates a new normal reference.
     * @see weak()
     */
    reference normal() {
      reference result;
      result.obj = obj;
      result.weak_ref = false;
      result.inc_reference();
      return result;
    }

    /** Tests if the reference is a weak reference.
     * @return True if the reference is a weak reference otherwise false.
     */
    bool is_weak() {
      return weak_ref;
    }

    /** Decrement any current reference and create a new reference for the
     * the new unreference object. @b Note that this could be dangerous
     * if there is anothor reference already referencing @c obj. If
     * another reference is referencing @c obj then this will cause obj
     * to be destroyed twice.
     *
     * Example
     * @code
     * class MyClass;
     *
     * typedef reference<MyClass> SPMyClass;
     * class MyClass {
     * public:
     *   virtual ~MyClass(void) throw() {}
     *   SPMyClass get_reference() {
     *     return this;
     *   }
     * };
     *
     * void sample(void) {
     *   SPMyClass obj1;
     *   SPMyClass obj2 = obj1.get_reference();
     *   // Now both obj1 and obj2 will have a reference count of one and
     *   // both pointing to the same object. Now here we will end up
     *   // a segfault or worse...
     * }
     * @endcode
     * Basically you'll want to be very careful when passing around the
     * @c this pointer when using References. This is the greatest trigger
     * of this feature (@e or @e bug ;).
     *
     * @param obj A newly created object we want to reference.
     * @return A C++ reference of this.
     */
    reference<Ty> &operator=(Ty *obj) {
      dec_reference();
      if (obj) {
        this->obj = new __reference(obj, 1);
      } else
        this->obj = nullptr;
      return *this;
    }

    /** Decrement any current reference and reference the passed object.
     */
    reference<Ty> &operator=(const reference<Ty> &obj) {
      if (this != &obj) {
        dec_reference();
        this->obj = obj.obj;
        this->weak_ref = obj.weak_ref;
        inc_reference();
      }
      return *this;
    }

    /** Check if the reference is referencing the same object as @c other.
     */
    bool operator ==(const reference<Ty> &other) const {
      return (obj == other.obj);
    }

    /** Check if the reference is referencing the same object as @c other.
     */
    bool operator ==(const Ty *other) const {
      return ((Ty *)(obj) == other);
    }

    /** Check if the reference is referencing a different object as @c other.
     */
    bool operator !=(const reference<Ty> &other) const {
      return (this->obj != other.obj);
    }

    /** Manually increase the reference count.
     * @see operator --()
     */
    reference &operator ++() { inc_reference(); return *this; }

    /** Manually decrease the reference count.
     * @see operator ++()
     */
    reference &operator --() { dec_reference(); return *this; }

    /** Dereference the reference.
     */
    Ty &operator *(void) { return (Ty &)(*obj); }

    /** Constant dereference the reference.
     */
    const Ty &operator *(void) const { return (Ty &)(*obj); }
    Ty *operator ->() { return (Ty *)(*obj); }
    const Ty *operator ->(void) const { return (Ty *)(*obj); }
    operator Ty &(void) { return (Ty &)(*obj); }
    operator const Ty &(void) const { return (Ty &)(*obj); }

  private:

    void dec_reference() throw() {
      if (obj) {
        if (weak_ref) {
          obj->dec_weak_ref();
          if (obj->__ref_cnt() == 0 and obj->__wref_cnt() == 0) {
            delete obj; obj = nullptr;
          }
        } else {
          obj->dec_ref();
          if (obj->__ref_cnt() == 0) {
            if (obj->__wref_cnt())
              obj->free();
            else {
              delete obj; obj = nullptr;
            }
          }
        }
      }
    }

    void inc_reference() {
      if (obj) {
        if (weak_ref)
          obj->inc_weak_ref();
        else
          obj->inc_ref();
      }
    }

    bool weak_ref;
    __reference *obj;

  };
}
#endif /* _CUTLET_MEMORY_H */
