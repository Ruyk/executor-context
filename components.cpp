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

  File: executor_context.cpp : Playground for executor context

*/
#include <algorithm>
#include <future>
#include <iostream>
#include <numeric>
#include <type_traits>

// Include the Hwloc C++ wrapper
#include <hwlocxx>

#include <iostream>
#include <optional>
#include <sstream>
#include <vector>

namespace hwlocxx
{
namespace experimental
{
   namespace execution
   {
      struct Component;
      namespace detail
      {

         std::vector<Component> make_components_from_level(
             gsl::not_null<const hwlocxx::topology*> topo_, int lvl)
         {
            auto objs = topo_->get_objects(lvl);
            std::vector<Component> retVal;
            retVal.reserve(objs.size());
            for (auto o : objs) {
               retVal.emplace_back(o, lvl);
            }
            return retVal;
         }

         std::vector<Component> make_components_from_level(
             gsl::not_null<const hwlocxx::topology*> topo_, int lvl,
             topology::object subtree)
         {
            auto objs = topo_->get_objects(lvl);
            std::vector<Component> retVal;
            retVal.reserve(objs.size());
            for (auto o : objs) {
               if (o.in_subtree(subtree)) {
                  retVal.emplace_back(o, lvl);
               }
            }
            return retVal;
         }
      }

      struct Component
      {

         std::string name() const
         {
            std::stringstream s;
            s << o_ << std::endl;
            return s.str();
         }

         std::vector<Component> components()
         {
            return detail::make_components_from_level(o_.get_topo(), depth_ + 1,
                                                      o_);
         }

         size_t num_components() const
         {
            return o_.get_topo()->get_width_at_depth(depth_);
         }

         bool can_place_memory() const { return true; };

         bool can_place_agent() const { return true; };

         std::optional<Component> member_of() const
         {
            /* if (depth_ > 0) {
               return Component{}
             } */
            return std::nullopt;
         }

         Component(hwlocxx::topology::object o, size_t depth)
             : o_{o}, depth_{depth}
         {
         }

     private:
         hwlocxx::topology::object o_;
         size_t depth_;
      };

      auto components() -> decltype(std::vector<Component>())
      {
         static hwlocxx::topology topo_;
         gsl::not_null<const hwlocxx::topology*> t{&topo_};
         return detail::make_components_from_level(t, 0);
      }
   }
}
}

int main()
{
   // System level
   auto cList = hwlocxx::experimental::execution::components();

   // Print topology
   for (auto& c : cList) {
      std::cout << c.name() << std::endl;
      for (auto& c1 : c.components()) {
         std::cout << " " << c1.name() << std::endl;
         for (auto& c2 : c1.components()) {
            std::cout << "  " << c2.name() << std::endl;
            for (auto& c3 : c2.components()) {
               std::cout << "   " << c3.name() << std::endl;
               for (auto& c4 : c3.components()) {
                  std::cout << "    " << c4.name() << std::endl;
                  for (auto& c5 : c4.components()) {
                     std::cout << "     " << c5.name() << std::endl;
                  }
               }
            }
         }
      }
   }

#if 0
   // hwloc-based ExecutorContext
   hwlocxx::experimental::ExecutionContext hwEC;

   auto& eR = hwEC.execution_resource();

   std::cout << "Concurrency: " << eR.concurrency() << std::endl;
   std::cout << "Partition Size: " << eR.partition_size() << std::endl;

   auto mE = hwEC.executor();
   /**
    * eR.allocator();
    *
    */

   auto fut = mE.twoway_execute([&]() -> unsigned {
      std::cout << " Hello World " << std::endl;
      return 42u;
   });

   return (42 - fut.get());
#endif // 0
};
