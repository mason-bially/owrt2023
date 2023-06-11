#pragma once

#include <deque>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>

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
            using ScheduleBuffer = std::vector<ScheduleCall>;

        private:
            struct ThreadBlock {
                std::mutex workLock;
                std::vector<ScheduleCall*> queue;
                std::atomic<size_t> queueSize;
                std::atomic<size_t> workTaken;
                std::atomic<size_t> workDone;

                std::condition_variable workCondition;

                std::jthread thread;
                TThreadLocal tl;

                inline auto reset()
                {
                    std::lock_guard const lock { workLock };
                    queue.clear();
                    queueSize.store(0);
                    workTaken.store(0);
                    workDone.store(0);
                }

                inline auto pending()
                {
                    return queueSize - workDone.load();
                }
                inline auto available()
                {
                    return queueSize - workTaken.load();
                }

                inline void addWork(ScheduleCall* work)
                {
                    //std::lock_guard const lock { workLock };
                    queue.emplace_back(work);
                    queueSize++;
                }

                inline auto tryGetLocalWork() -> std::optional<ScheduleCall*>
                {
                    //std::lock_guard const lock { workLock };
                    if (available() > 0) {
                        return queue[workTaken++];
                    }
                    return {};
                }

                inline void main()
                {
                    std::stop_token stop = thread.get_stop_token();
                    while (!stop.stop_requested())
                    {
                        auto work = tryGetLocalWork();
                        if (work) {
                            (**work)(tl);
                            workDone++;
                        } else {
                            std::unique_lock lock { workLock };
                            workCondition.wait(lock);
                        }
                    }
                }

                inline ThreadBlock()
                    : workLock {}
                    , queue {}, queueSize {0}
                    , workTaken {0}, workDone {0}
                    , workCondition {}
                    , thread(&ThreadBlock::main, this), tl {}
                {

                }

                inline ~ThreadBlock()
                {
                    workCondition.notify_all();
                }
            };

            std::array<std::unique_ptr<ThreadBlock>, NThreadCount> _blocks;

            ScheduleBuffer _current;
            ScheduleBuffer _next;
        
        public:
            Scheduler() {
                for (auto& b : _blocks) { b = std::make_unique<ThreadBlock>(); }
            }

            inline void schedule(ScheduleCall&& call)
            {
                _next.emplace_back(call);
            }

            inline void wait()
            {
                while(std::any_of(_blocks.begin(), _blocks.end(), [](auto& tb){ return tb->pending() > 0; }))
                {
                    for (auto& b : _blocks) { b->workCondition.notify_all(); } // probably shouldn't need this, but it seems to hang sometimes, why?
                }
                for (auto& b : _blocks) { b->reset(); }
                _current.clear();

                std::swap(_current, _next);

                auto batch = _current.size() / NThreadCount;
                for (auto j = 0u; j < NThreadCount; ++j) {
                    for (auto i = 0u; i < batch; ++i)
                        _blocks[j]->addWork(&_current[j*batch + i]);
                    _blocks[j]->workCondition.notify_all();
                }
                for (auto i = batch*NThreadCount; i < _current.size(); ++i)
                    _blocks[0]->addWork(&_current[i]);
                _blocks[0]->workCondition.notify_all();
            }
    };
}