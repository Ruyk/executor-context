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

#include <chrono>
#include <future>

namespace hwlocxx
{
namespace experimental
{

   struct thread_execution_resource_t
   {
      topology topo_;
      bitmap partition_;

      thread_execution_resource_t() = default;
      ~thread_execution_resource_t() = default;
      thread_execution_resource_t(const thread_execution_resource_t&) = delete;
      thread_execution_resource_t(thread_execution_resource_t&&) = default;
      thread_execution_resource_t&
      operator=(thread_execution_resource_t&&) = default;
      thread_execution_resource_t&
      operator=(const thread_execution_resource_t&) = default;

      topology get_topology() { return topo_; }

      size_t concurrency() const noexcept
      {
         // Count of threads
         return topo_.get_width_at_depth(0);
      }

      size_t partition_size() const noexcept
      {
         // total number of CPUs
         return topo_.get_width_at_depth(0);
      }

      const thread_execution_resource_t& partition(size_t i) const
          noexcept = delete;

      const thread_execution_resource_t& member_of() const noexcept = delete;

      void place_thread()
      {
         const auto& currentThreadCPU =
             get_topology().get_last_cpu_location(cpubind::thread);
         stream_placement_info(std::cout);
         const auto& allowedCPUs = get_topology().get_cpubind(cpubind::process);
         // Other CPUs that are not the current one
         auto remainingCPU{allowedCPUs.and_not(currentThreadCPU)};
         get_topology().set_cpubind(remainingCPU);
         stream_placement_info(std::cout);
      }

  private:
      /*
       * Outputs placement information for the current thread
       * to the givem output stream
       */
      std::ostream& stream_placement_info(std::ostream& out)
      {
         const auto& currentThreadCPU =
             get_topology().get_last_cpu_location(cpubind::thread);
         int i = currentThreadCPU.first();
         auto obj = get_topology().get_object_by_os_index(i);
         out << " Thread running in : "
             << " Logical " << obj.get_logical_index() << " Physical " << i
             << std::endl;
         return out;
      }
   };

   class locality_executor
   {
  public:
      explicit locality_executor(thread_execution_resource_t& eR) : eR_{eR} {};

      template <typename Function>
      std::future<unsigned> twoway_execute(Function&& func)
      {

         using return_type = unsigned;
         std::promise<return_type> promise;
         auto fut = promise.get_future();

         std::thread([=, promise{std::move(promise)}]() mutable {
            try
            {
               eR_.place_thread();
               // Run user-functor
               auto result = func();
               promise.set_value(result);
            }
            catch (...)
            {
               promise.set_exception(std::current_exception());
            }
         })
             .detach();

         return fut;
      }

  private:
      thread_execution_resource_t& eR_;
   };

   class ExecutionContext
   {
  public:
      ExecutionContext() = default;

      ~ExecutionContext() = default;

      ExecutionContext(ExecutionContext const&) = delete;
      ExecutionContext(ExecutionContext&&) = delete;

      using execution_resource_t = thread_execution_resource_t;

      execution_resource_t const& execution_resource() const noexcept
      {
         return eR_;
      }

      locality_executor executor() { return locality_executor(eR_); }

      // Waiting functions:
      void wait() = delete;
      template <class Clock, class Duration>
      bool wait_until(std::chrono::time_point<Clock, Duration> const&) = delete;
      template <class Rep, class Period>
      bool wait_for(std::chrono::duration<Rep, Period> const&) = delete;

      // Returns the topology of the system
      inline topology get_topo() const { return eR_.topo_; }

  private:
      execution_resource_t eR_;
   };

} // namespace experimental
} // namespace hwlocxx
