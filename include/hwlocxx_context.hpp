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
     friend class ExecutionContext;

      thread_execution_resource_t() = delete;
      ~thread_execution_resource_t() = default;
      thread_execution_resource_t(const thread_execution_resource_t&) = delete;
      thread_execution_resource_t(thread_execution_resource_t&&) = delete;
      thread_execution_resource_t&
      operator=(thread_execution_resource_t&&) = delete;
      thread_execution_resource_t&
      operator=(const thread_execution_resource_t&) = delete;

      const thread_execution_resource_t& partition(size_t i) const
          noexcept = delete;

      const thread_execution_resource_t& member_of() const noexcept = delete;

      size_t concurrency() const {
        return concurrency_;
      }

      size_t partition_size() const {
        return partition_size_;
      }

  private:
      size_t concurrency_;
      size_t partition_size_;

      thread_execution_resource_t(size_t concurrency, size_t partition_size)
          : concurrency_{concurrency}, partition_size_{partition_size}
      {
      }
   };

   class ExecutionContext;
   class locality_executor
   {

  public:
      explicit locality_executor(ExecutionContext& eC) : eC_{eC} {};

      void prepare_thread();

      template <typename Function>
      std::future<unsigned> twoway_execute(Function&& func)
      {

         using return_type = unsigned;
         std::promise<return_type> promise;
         auto fut = promise.get_future();

         std::thread([ =, promise{std::move(promise)} ]() mutable {
            try
            {
               prepare_thread();
               // Run user-functor
               auto result = func();
               promise.set_value(result);
            }
            catch (...)
            {
               promise.set_exception(std::current_exception());
            }
         }).detach();

         return fut;
      }

  private:
      ExecutionContext& eC_;
   };


   class ExecutionContext
   {
     friend class locality_executor;
  public:
     ExecutionContext() : topo_(), partition_(), 
      eR_{concurrency(), partition_size()} {
     }

      ~ExecutionContext() = default;

      ExecutionContext(ExecutionContext const&) = delete;
      ExecutionContext(ExecutionContext&&) = delete;

      using execution_resource_t = thread_execution_resource_t;

      execution_resource_t const& execution_resource() const noexcept
      {
         return eR_;
      }

      locality_executor executor() { return locality_executor(*this); }

      // Waiting functions:
      void wait() = delete;
      template <class Clock, class Duration>
      bool wait_until(std::chrono::time_point<Clock, Duration> const&) = delete;
      template <class Rep, class Period>
      bool wait_for(std::chrono::duration<Rep, Period> const&) = delete;

      // Returns the topology of the system
      inline topology get_topology() const { return topo_; }

  protected:
      void place_thread()
      {
         // Execute on a different thread that is on the same processor
         // (will only work for HT)
         // This returns an os index
         const auto& currentThreadCPU =
             get_topology().get_last_cpu_location(cpubind::thread);
         stream_placement_info(std::cout);
         const auto& allowedCPUs = get_topology().get_cpubind(cpubind::process);
         // Other CPUs that are not the current one
         auto remainingCPU{allowedCPUs.and_not(currentThreadCPU)};
         // Other CPUs in the same core
         auto obj =
             get_topology().get_object_by_os_index(currentThreadCPU.first());
         auto otherCore = obj.get_closest()[0];
         std::cout << "Obj is " << otherCore << std::endl;
         // get_topology().get_object_by_type(HWLOC_OBJ_CORE, 0);
         get_topology().set_cpubind(remainingCPU, cpubind::thread);
         stream_placement_info(std::cout);
      }


  private:
      topology topo_;
      bitmap partition_;
      execution_resource_t eR_; 

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

      size_t concurrency() const noexcept
      {
         // Count of threads
         return get_topology().get_width_at_depth(0);
      }

      size_t partition_size() const noexcept
      {
         // total number of CPUs
         return get_topology().get_width_at_depth(0);
      }
   };

} // namespace experimental
} // namespace hwlocxx
