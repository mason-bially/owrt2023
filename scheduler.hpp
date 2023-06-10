#pragma once

#include <functional>

namespace scheduler
{
    class SchedulerSingle
    {
        public:
            using ScheduleCall = std::function<void ()>;
            using ScheduleBuffer = std::vector<ScheduleCall>;

        private:
            ScheduleBuffer _current;
            ScheduleBuffer _next;
        
        public:
            inline void schedule(ScheduleCall&& call)
            {
                _next.emplace_back(call);
            }

            inline void wait()
            {
                for (auto& c : _current)
                {
                    c();
                }
                _current.clear();

                std::swap(_current, _next);
            }
    };
}