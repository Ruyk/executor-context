/**
  Copyright 2017 Ruyman Reyes 

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
  

   file: hwlocxx.hpp A trivial hwlocxx wrapper

*/

#include <cerrno>
#include <gsl/gsl>
#include <memory>
#include <string>

#include <hwloc.h>

/**
 * Alias to the internal hwloc bitmap structure
 */
using __bitMaskStruct = struct hwloc_bitmap_s;

namespace hwlocxx
{

/**
 * Contains a pointer to a hwloc bitmap structure
 */
class bitmap {
  public:

    bitmap()
      : bMap_{nullptr} {
      hwloc_bitmap_t ptr = hwloc_bitmap_alloc();
      bMap_ = std::shared_ptr<__bitMaskStruct> {ptr, 
                bitmapDeleter()};
    }

    bitmap(hwloc_bitmap_t ptr)
      : bMap_{ptr, bitmapDeleter()} { }

    ~bitmap() = default;

    bitmap(const bitmap&) = default;
    bitmap(bitmap&&) = default;
    bitmap& operator=(const bitmap& bMap) = default;
    bitmap& operator=(bitmap&& bMap) = default;

    /* Modifying and not */
    bitmap and_not(bitmap rhs) {
      hwloc_bitmap_andnot(bMap_.get(), bMap_.get(), rhs.get());
      return *this;
    }

    bitmap and_not(bitmap rhs) const {
      bitmap ret;
      hwloc_bitmap_andnot(ret.get(), bMap_.get(), rhs.get());
      return ret;
    }

    int first() const {
      return hwloc_bitmap_first(get());
    }

    hwloc_bitmap_t get() const {
      return bMap_.get();
    }

    friend std::ostream& operator<<(std::ostream& stream, const bitmap& rhs)
    {
      char str[128];
      hwloc_bitmap_snprintf(str, 128, rhs.get());
      stream << str << std::endl;
      return stream;
    }


  private:
    std::shared_ptr<__bitMaskStruct> bMap_;

    /** 
     * Custom deleters that guarantees bitmap is freed using the
     * correct hwloc functionality
     */
    struct bitmapDeleter {
    void operator()(hwloc_bitmap_t ptr) { 
          hwloc_bitmap_free(ptr);
        }
    };
};

/**
 * Topology of the system.
 */
class topology
{
   public:
   /* An object in the hwloc topology.
    * The underlying hwloc_obj_t is nothing but a pointer to an
    * element in the topology.
    * */
   class object
   {
  public:
      struct index {
        using os = int;
      };

      object(gsl::not_null<const topology*> topo, index::os i) 
        : obj_{hwloc_get_pu_obj_by_os_index(topo->get(), i)},
          topo_{topo} { }
      
      object(gsl::not_null<const topology*> topo, int depth, int i)
          : obj_{hwloc_get_obj_by_depth(topo->get(), depth, i)}, topo_{topo}
      {
      }

      object(gsl::not_null<const topology*> topo, hwloc_obj_type_t type, int i)
          : obj_{hwloc_get_obj_by_type(topo->get(), type, i)}, topo_{topo}
      {
      }

      object(const object& rhs) = default;
      object(object&& rhs) = default;
      object& operator=(const object& rhs) = default;
      object& operator=(object&& rhs) = default;
      ~object() = default;

      hwloc_obj_t get() { return obj_; }
      hwloc_obj_t get() const { return obj_; }

      int get_logical_index() const {
        return obj_.get()->logical_index;
      }

      friend std::ostream& operator<<(std::ostream& stream, const object& rhs)
      {
         char str[128];
         hwloc_obj_type_snprintf(static_cast<char*>(str), sizeof(str),
                                 rhs.get(), 0);
         stream << gsl::zstring<>{str};
         return stream;
      }

  private:
      gsl::not_null<hwloc_obj*> obj_;
      gsl::not_null<const topology*> topo_;
   };

   topology() : topology_{nullptr}
   {
      hwloc_topology_t ptr = nullptr;
      hwloc_topology_init(&ptr);
      topology_ = std::shared_ptr<hwloc_topology> {
         ptr, [=](hwloc_topology_t ptr) { hwloc_topology_destroy(ptr); }
      };
      hwloc_topology_load(topology_.get());
   }

   /* @todo Constructors that takes filters for elements of the topology */

   ~topology() = default;

   topology& operator=(const topology&) = default;
   topology& operator=(topology&&) = default;
   topology(topology&&) = default;
   topology(const topology& rhs) : topology_{rhs.topology_} {}

   int get_depth() { return hwloc_topology_get_depth(get()); }

   hwloc_topology_t get() const { return topology_.get(); }

   object get_obj(int depth, int elem) { return {this, depth, elem}; }

   int get_width_at_depth(int depth) const
   {
      return hwloc_get_nbobjs_by_depth(get(), depth);
   }

   int get_width_by_type(hwloc_obj_type_t type) const
   {
      return hwloc_get_nbobjs_by_type(get(), type);
   }

   object get_object_by_type(hwloc_obj_type_t type, int id) const
   {
      return {this, type, id};
   }

   object get_object_by_os_index(object::index::os i) const {
     return {this, i};
   }

   /* Returns the last cpu where the thread executed
    * @todo Enable passing CPUBIND as parameter
    */
   bitmap get_last_cpu_location() const {
     bitmap loc;
     hwloc_get_last_cpu_location(get(), loc.get(), HWLOC_CPUBIND_THREAD);
     return {loc};
   }

   /* Returns the current CPU bind set for the process
    * @todo Enable passing CPUBIND as parameter
    */
   bitmap get_cpubind() const {
     bitmap cpuBind;
     hwloc_get_cpubind(get(), cpuBind.get(), HWLOC_CPUBIND_PROCESS);
     return {cpuBind};
   }

   /* Sets a new CPU bind set for the THREAD
    * @todo Enable passing CPUBIND as parameter
    */
   void set_cpubind(bitmap new_set) {
     hwloc_set_cpubind(get(), new_set.get(), HWLOC_CPUBIND_THREAD);
   }

   protected:
   std::shared_ptr<hwloc_topology> topology_;
};
}; // namespace hwlocxx