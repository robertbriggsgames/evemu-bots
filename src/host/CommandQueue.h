#ifndef EVEMU_BOTS_COMMAND_QUEUE_H
#define EVEMU_BOTS_COMMAND_QUEUE_H

#include <functional>
#include <mutex>
#include <vector>

namespace EvEmuBots {

class CommandQueue {
public:
    void Enqueue(std::function<void()> fn);
    void DrainOnMainThread();

private:
    std::mutex m_mutex;
    std::vector<std::function<void()>> m_pending;
};

} // namespace EvEmuBots

#endif
