#include "host/CommandQueue.h"

namespace EvEmuBots {

void CommandQueue::Enqueue(std::function<void()> fn)
{
    if (!fn)
        return;
    std::lock_guard<std::mutex> lock(m_mutex);
    m_pending.push_back(std::move(fn));
}

void CommandQueue::DrainOnMainThread()
{
    std::vector<std::function<void()>> batch;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        batch.swap(m_pending);
    }
    for (auto& fn : batch)
        fn();
}

} // namespace EvEmuBots
