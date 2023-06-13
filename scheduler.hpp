#pragma once

#include <vector>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <algorithm>
#include <ranges>


namespace scheduler
{
    template <size_t NThreadCount, class TThreadLocal>
    class Scheduler;

    template <class TThreadLocal>
    class Scheduler<1, TThreadLocal>
    {
        public:
            using ScheduleCall = std::function<void (TThreadLocal&)>;
            using ScheduleBuffer = std::vector<ScheduleCall>;

        private:
            TThreadLocal _tl;
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
                    c(_tl);
                }
                _current.clear();

                std::swap(_current, _next);
            }
    };

    template <size_t NThreadCount, class TThreadLocal>
    class Scheduler
    {
        public:
            using ScheduleCall = std::function<void (TThreadLocal&)>;

        private:
            struct ThreadBlock {
                TThreadLocal tl;

                std::mutex workLock;
                std::vector<ScheduleCall> queueCurrent;
                size_t queueCurrentSize;
                std::atomic<size_t> workTaken;
                std::atomic<size_t> workDone;

                std::condition_variable workCondition;

                std::jthread thread;
                std::vector<ScheduleCall> queueNext;

                inline auto resetAndSwap()
                {
                    std::lock_guard const lock { workLock };

                    queueCurrent.clear();
                    workTaken.store(0);
                    workDone.store(0);

                    std::swap(queueCurrent, queueNext);
                    queueCurrentSize = queueCurrent.size();
                }

                inline auto pending() const -> size_t
                {
                    return queueCurrentSize - workDone.load();
                }
                inline auto available() const
                {
                    return queueCurrentSize - workTaken.load();
                }

                inline void addWork(ScheduleCall&& work)
                {
                    queueNext.emplace_back(std::forward<ScheduleCall&&>(work));
                }

                inline auto tryGetLocalWork() -> std::optional<ScheduleCall*>
                {
                    if (available() > 0) {
                        return &queueCurrent[workTaken++];
                    }
                    return {};
                }

                inline void main()
                {
                    std::stop_token stop = thread.get_stop_token();
                    std::unique_lock lock { workLock };
                    while (!stop.stop_requested())
                    {
                        auto work = tryGetLocalWork();
                        if (work) {
                            (**work)(tl);
                            workDone++;
                        } else {
                            workCondition.wait(lock);
                        }
                    }
                }

                inline ThreadBlock()
                    : tl {}
                    , workLock {}
                    , queueCurrent {}, queueCurrentSize {0}
                    , workTaken {0}, workDone {0}
                    , workCondition {}
                    , thread(&ThreadBlock::main, this)
                    , queueNext {}
                {

                }

                inline ~ThreadBlock()
                {
                    workCondition.notify_all();
                }
            };

            std::array<std::unique_ptr<ThreadBlock>, NThreadCount> _blocks;
            size_t _scheduleToBlock = 0;

        private:
            inline auto _reduceThreadBlocks(std::invocable<ThreadBlock const&> auto tbfn) {
                decltype(tbfn(*_blocks[0])) result = 0;
                for (auto const& tb : _blocks)
                    result += tbfn(*tb);
                return result;
            }

        public:
            Scheduler() {
                for (auto& b : _blocks) { b = std::make_unique<ThreadBlock>(); }
            }

            inline void schedule(ScheduleCall&& call)
            {
                _blocks[_scheduleToBlock++]->addWork(std::forward<ScheduleCall&&>(call));

                _scheduleToBlock%=NThreadCount;
            }

            inline void wait(std::function<void(double)>& status_fn)
            {
                size_t total = _reduceThreadBlocks([](auto const& tb){ return tb.queueCurrentSize; });
                if (total > 0) {
                    size_t pending;
                    do {
                        pending = _reduceThreadBlocks([](auto const& tb){ return tb.pending(); });
                        status_fn(double(total-pending) / total);
                    } while(pending != 0);
                }

                for (auto& b : _blocks) {
                    b->resetAndSwap();
                    b->workCondition.notify_all();
                }
            }

            inline void wait()
            {
                while(std::ranges::any_of(_blocks, [](auto const& tb){ return tb->pending() > 0; })) {
                }

                for (auto& b : _blocks) {
                    b->resetAndSwap();
                    b->workCondition.notify_all();
                }
            }
    };
}
