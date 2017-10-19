/** Copyright 2017 Ruyman Reyes

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

#include <hwlocxx>

namespace hwlocxx
{
namespace experimental
{
   void locality_executor::prepare_thread() { eC_.place_thread(); }

    namespace this_system
    {
        auto resources() -> decltype(std::vector<thread_execution_resource_t>()) {
            static hwlocxx::topology topo_;
            auto objects = topo_.get_objects(0);
            std::vector<thread_execution_resource_t> ret;
            ret.reserve(objects.size());
            for (auto o : objects) {
                ret.emplace_back(o);
            }
            return ret;
        }
    }
}
}
