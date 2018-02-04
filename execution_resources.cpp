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

  File: executor_resources.cpp : Playground for executor resources

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

   // hwloc-based ExecutorContext
   hwlocxx::experimental::ExecutionContext hwEC(cList[0].resources()[0].resources()[0]);

   auto& eR = hwEC.execution_resource();

   std::cout << "Concurrency: " << eR.concurrency() << std::endl;
   std::cout << "Partition Size: " << eR.partition_size() << std::endl;

   auto mE = hwEC.executor();
   
   if (!hwEC.can_allocate()) {
      std::cout << " Cannot allocate on the given resource " << std::endl;
   }
   auto mA = hwEC.allocator();
   using context_allocator = decltype(hwEC)::AllocatorT;
   // Missing allocator type from Executor
   std::vector<int, context_allocator> v1(mA);
   // How can the methods of the std::vector use the correct executor?
   // it should be possible to retrieve the executor from the AllocatorT
   // This method would use the caller thread
   for(int i = 0; i < 10; i++) {
      v1.push_back(42);
   }

   // This would use a thread based on the same executor resource
   auto fut = mE.twoway_execute([&]() -> unsigned {
      auto retVal = v1.back();
      v1.pop_back();
      return retVal;
   });

   return (42 - fut.get());

};
