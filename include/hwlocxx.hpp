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
#include <vector>

#include <hwloc.h>

/**
 * Alias to the internal hwloc bitmap structure
 */
using __bitMaskStruct = struct hwloc_bitmap_s;

namespace hwlocxx
{

enum class cpubind
{
   process = HWLOC_CPUBIND_PROCESS,
   thread = HWLOC_CPUBIND_THREAD
};

/**
 * Contains a pointer to a hwloc bitmap structure
 */
class bitmap
{
   public:
   bitmap() : bMap_{nullptr}
   {
      hwloc_bitmap_t ptr = hwloc_bitmap_alloc();
      bMap_ = std::shared_ptr<__bitMaskStruct>{ptr, bitmapDeleter()};
   }

   explicit bitmap(hwloc_bitmap_t ptr) : bMap_{ptr, bitmapDeleter()} {}

   ~bitmap() = default;

   bitmap(const bitmap&) = default;
   bitmap(bitmap&&) = default;
   bitmap& operator=(const bitmap& bMap) = default;
   bitmap& operator=(bitmap&& bMap) = default;

   /* Modifying and not */
   bitmap and_not(const bitmap& rhs)
   {
      hwloc_bitmap_andnot(bMap_.get(), bMap_.get(), rhs.get());
      return *this;
   }

   bitmap and_not(const bitmap& rhs) const
   {
      bitmap ret;
      hwloc_bitmap_andnot(ret.get(), bMap_.get(), rhs.get());
      return ret;
   }

   int first() const { return hwloc_bitmap_first(get()); }

   hwloc_bitmap_t get() const { return bMap_.get(); }

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
   struct bitmapDeleter
   {
      void operator()(hwloc_bitmap_t ptr) { hwloc_bitmap_free(ptr); }
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
      struct index
      {
         using os = int;
      };

      object(gsl::not_null<const topology*> topo, index::os i)
          : obj_{hwloc_get_pu_obj_by_os_index(topo->get(), i)}, topo_{topo}
      {
      }

      object(gsl::not_null<const topology*> topo, int depth, int i)
          : obj_{hwloc_get_obj_by_depth(topo->get(), depth, i)}, topo_{topo}
      {
      }

      object(gsl::not_null<const topology*> topo, hwloc_obj_type_t type, int i)
          : obj_{hwloc_get_obj_by_type(topo->get(), type, i)}, topo_{topo}
      {
      }

      object(gsl::not_null<const topology*> topo, hwloc_obj_t obj)
          : obj_{obj}, topo_{topo}
      {
      }

      object(const object& rhs) = default;
      object(object&& rhs) = default;
      object& operator=(const object& rhs) = default;
      object& operator=(object&& rhs) = default;
      ~object() = default;

      hwloc_obj_t get() { return obj_; }
      hwloc_obj_t get() const { return obj_; }

      gsl::not_null<const topology*> get_topo() const { return topo_; }

      bool in_subtree(object subtree) const
      {
         return (hwloc_obj_is_in_subtree(topo_->get(), obj_.get(),
                                         subtree.get()) > 0);
      }

      /**
       * Obtain the descendants of the current object in the topology
       * @return vector of objects whose the current object is parent in the
       * topology
       */
      std::vector<object> get_descendants() const
      {
         std::vector<object> retVal; // = topo_->get_objects(obj_.get()->depth);
         for (int current = 0; current < obj_.get()->arity; current++) {
            retVal.emplace_back(topo_.get(), obj_.get()->children[current]);
         }
         return retVal;
      }

      int get_logical_index() const { return obj_.get()->logical_index; }

      std::vector<object> get_closest() const
      {
         const size_t maxObjects = 10u;
         std::vector<hwloc_obj_t> hwObjs(maxObjects);
         unsigned numElems = hwloc_get_closest_objs(topo_->get(), obj_.get(),
                                                    hwObjs.data(), maxObjects);
         std::vector<object> closest;
         closest.reserve(numElems);
         for (unsigned i = 0; i < numElems; i++) {
            closest.emplace_back(topo_, hwObjs[i]);
         }
         return closest;
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
      hwloc_topology_t system = nullptr;
      hwloc_topology_init(&system);
      topology_ = std::shared_ptr<hwloc_topology>{
          system, [=](hwloc_topology_t ptr) { hwloc_topology_destroy(ptr); }};
      hwloc_topology_load(topology_.get());
   }

   /* @todo Constructors that takes filters for elements of the topology */

   ~topology() = default;

   topology& operator=(const topology&) = default;
   topology& operator=(topology&&) = default;
   topology(topology&&) = default;
   topology(const topology& rhs) = default;

   int get_depth() { return hwloc_topology_get_depth(get()); }

   hwloc_topology_t get() const { return topology_.get(); }

   object get_obj(int depth, int elem) const { return {this, depth, elem}; }

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

   object get_object_by_os_index(object::index::os i) const
   {
      return {this, i};
   }

   std::vector<object> get_objects(int lvl) const
   {
      const int maxObjects = get_width_at_depth(lvl);
      std::vector<object> retObjs;
      retObjs.reserve(maxObjects);

      for (int i = 0; i < maxObjects; i++) {
         retObjs.emplace_back(this, lvl, i);
      }

      return retObjs;
   }

   /* Returns the last cpu where the thread executed
    */
   bitmap get_last_cpu_location(cpubind b) const
   {
      bitmap loc;
      auto err =
          hwloc_get_last_cpu_location(get(), loc.get(), static_cast<int>(b));
      if (err > 0) {
         std::terminate();
      }
      return {loc};
   }

   /* Returns the current CPU bind set for the process
    */
   bitmap get_cpubind(cpubind b) const
   {
      bitmap cpuBind;
      auto err = hwloc_get_cpubind(get(), cpuBind.get(), static_cast<int>(b));
      if (err > 0) {
         std::terminate();
      }
      return {cpuBind};
   }

   /* Sets a new CPU bind set for the THREAD
    */
   void set_cpubind(const bitmap& new_set, cpubind b)
   {
      hwloc_set_cpubind(get(), new_set.get(), static_cast<int>(b));
   }

   protected:
   std::shared_ptr<hwloc_topology> topology_;
};
}; // namespace hwlocxx
