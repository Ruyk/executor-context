/* Copyright 2017 Ruyman Reyes

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/

#ifndef HWLOCXX_ALLOCATOR_HPP
#define HWLOCXX_ALLOCATOR_HPP

namespace hwlocxx
{

template <class T>
class allocator
{
   public:
   using value_type = T;
   using size_type = size_t;
   using difference_type = ptrdiff_t;
   using pointer = T*;
   using const_pointer = const T*;
   using reference = T&;
   using const_reference = const T&;

   allocator(topology topo, topology::object obj) : topo_{topo}, obj_{obj} {}

   allocator(const allocator&) = default;
   allocator(allocator&&) = default;
   allocator& operator=(const allocator&) = default;
   allocator& operator=(allocator&&) = default;

   /**
    * Construct element in place
    */
   void construct(pointer p, const_reference val) { ::new ((T*) p) T(val); }

   /**
    * Address of an element x
    */
   pointer address(reference x) const { return &x; }

   /**
    * Address of an element x (const)
    */
   const_pointer address(const_reference x) const { return &x; }

   /**
    * Allocate size value_type elements
    */
   value_type* allocate(size_t size)
   {
      value_type* result = nullptr;
      result = static_cast<value_type*>(hwloc_alloc_membind(
          topo_.get(), size * sizeof(value_type), obj_.get()->nodeset,
          HWLOC_MEMBIND_BIND, HWLOC_MEMBIND_BYNODESET));
      return result;
   }

   /**
    * Deallocate size value_type elements
    */
   void deallocate(value_type* ptr, size_t size)
   {
      hwloc_free(topo_.get(), ptr, size * sizeof(value_type));
   }

   private:
   topology topo_;
   topology::object obj_;
};

template <class T1, class T2>
bool operator==(const allocator<T1>&, const allocator<T2>&)
{
   return true;
}

template <class T1, class T2>
bool operator!=(const allocator<T1>& lhs, const allocator<T2>& rhs)
{
   return !(lhs == rhs);
}

} // namespace hwlocxx
#endif // HWLOCXX_ALLOCATOR_HPP
