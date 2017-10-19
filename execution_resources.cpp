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

// Include the Hwloc C++ wrapper
#include <hwlocxx>


int main()
{
   // System level
   auto cList = hwlocxx::experimental::this_system::resources();

   // Print topology
   for (auto& c : cList) {
      std::cout << c.name() << std::endl;
      for (auto& c1 : c.resources()) {
         std::cout << " " << c1.name() << std::endl;
         for (auto& c2 : c1.resources()) {
            std::cout << "  " << c2.name() << std::endl;
            for (auto& c3 : c2.resources()) {
               std::cout << "   " << c3.name() << std::endl;
               for (auto& c4 : c3.resources()) {
                  std::cout << "    " << c4.name() << std::endl;
                  for (auto& c5 : c4.resources()) {
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
