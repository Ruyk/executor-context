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

  File: locality.cpp  Example of using hwlocxx wrapper

*/
#include <algorithm>
#include <iostream>
#include <numeric>

#include <hwlocxx>

int main()
{
   int depth;
   int i, n;
   int topodepth;

   hwlocxx::topology topo;

   /* Optionally, get some additional topology information
      in case we need the topology depth later. */
   topodepth = topo.get_depth();

   /*
    * Example:
    *   Print the topology
    */
   for (depth = 0; depth < topodepth; depth++)
   {
      std::cout << "*** Objects at level " << depth << std::endl;
      for (i = 0; i < topo.get_width_at_depth(depth); i++)
      {
         std::cout << "Index " << i << ":" << topo.get_obj(depth, i)
                   << std::endl;
      }
   }

   /* Example:
    *    Use the hwlocxx allocator to allocate storage for a vector
    */
   n = topo.get_width_by_type(HWLOC_OBJ_NUMANODE);
   auto obj = topo.get_object_by_type(HWLOC_OBJ_NUMANODE, n - 1);
   hwlocxx::allocator<int> a{topo, obj};

   size_t nElems = 10u;
   std::vector<int, hwlocxx::allocator<int>> v1{nElems, a};
   std::iota(std::begin(v1), std::end(v1), 1);
   auto sum = std::accumulate(std::begin(v1), std::end(v1), 0, std::plus<>());
   auto sumResult = (nElems * (nElems + 1) / 2);
   return static_cast<int>(sumResult - sum);
}
