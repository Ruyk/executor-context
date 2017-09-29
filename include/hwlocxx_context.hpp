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

#include <future>
#include <chrono>

namespace hwlocxx
{
namespace experimental
{

  class locality_executor {
    public:

    locality_executor(topology topo)
      : topo_{topo} { };

    /*
     * Outputs placement information for the current thread
     * to the givem output stream
     */
    std::ostream& stream_placement_info(std::ostream& out) {
      const auto& currentThreadCPU = topo_.get_last_cpu_location();
      int i = currentThreadCPU.first();
      auto obj = topo_.get_object_by_os_index(i);
      out << " Thread running in : "
        << " Logical " << obj.get_logical_index()
        << " Physical " << i 
        << std::endl;
      return out;
    }

    template <typename Function>
    std::future<unsigned>
    twoway_execute(Function &&func) {

      using return_type = unsigned;
      std::promise<return_type> promise;
      auto fut = promise.get_future();

      std::thread([=, promise{std::move(promise)}]() mutable {
        try {
          // Get thread (implicit: THREAD)
          const auto& currentThreadCPU
                        = topo_.get_last_cpu_location();
          stream_placement_info(std::cout);
          // Get allowed cpus (implicit: PROCESS)
          const auto& allowedCPUs = topo_.get_cpubind();
          // Other CPUs that are not the current one
          auto remainingCPU{allowedCPUs.and_not(currentThreadCPU)};
          topo_.set_cpubind(remainingCPU);
          stream_placement_info(std::cout);
          // Run user-functor
          auto result = func();
          promise.set_value(result);
        } catch (...) {
          promise.set_exception(std::current_exception());
        }
      }).detach();

      return fut;
    }
    private:
    
    topology topo_;
  };

   class ExecutionContext
   {
  public:
      using execution_resource_t = int;  // dummy for now

      ExecutionContext() = default;

      ~ExecutionContext() = default;

      ExecutionContext(ExecutionContext const&) = delete;
      ExecutionContext(ExecutionContext&&) = delete;

      execution_resource_t const& execution_resource() const noexcept;

      locality_executor executor() {
        return locality_executor(get_topo());
      }

      // Waiting functions:
      void wait() = delete;
      template <class Clock, class Duration>
      bool wait_until(
          std::chrono::time_point<Clock, Duration> const&) = delete;
      template <class Rep, class Period>
      bool wait_for(std::chrono::duration<Rep, Period> const&) = delete;

      inline topology get_topo() const {
        return topo_;
      }

  private:
      topology topo_;

   };

} // namespace experimental
} // namespace hwlocxx
