#ifndef EVEMU_BOTS_DUMMY_TCP_H
#define EVEMU_BOTS_DUMMY_TCP_H

#include "network/EVETCPConnection.h"

namespace EvEmuBots {

// Loopback TCP stand-in so a Client can stay STATE_CONNECTED without a socket.
// Does not inherit Client (Client.h forbids that). Packets are drained and dropped.
class DummyEVETCPConnection : public EVETCPConnection {
public:
    DummyEVETCPConnection();
    ~DummyEVETCPConnection() override;

protected:
    bool Process() override;
    bool RecvData(char* errbuf = nullptr) override;
    bool SendData(char* errbuf = nullptr) override;
    bool ProcessReceivedData(char* errbuf = nullptr) override;

private:
    void DrainSendQueue();
};

} // namespace EvEmuBots

#endif
