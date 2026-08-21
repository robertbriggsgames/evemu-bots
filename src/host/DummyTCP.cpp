#include "eve-server.h"
#include "host/DummyTCP.h"

#include <arpa/inet.h>

namespace EvEmuBots {

DummyEVETCPConnection::DummyEVETCPConnection()
: EVETCPConnection()
{
    mSock = nullptr;
    mrIP = htonl(INADDR_LOOPBACK);
    mrPort = 0;
    mSockState = STATE_CONNECTED;
    StartLoop();
}

DummyEVETCPConnection::~DummyEVETCPConnection()
{
    Disconnect();
}

void DummyEVETCPConnection::DrainSendQueue()
{
    MutexLock lock(mMSendQueue);
    while (!mSendQueue.empty()) {
        Buffer* buf = mSendQueue.front();
        mSendQueue.pop_front();
        SafeDelete(buf);
    }
}

bool DummyEVETCPConnection::Process()
{
    DrainSendQueue();
    MutexLock lock(mMSock);
    if (mSockState == STATE_DISCONNECTING || mSockState == STATE_DISCONNECTED)
        return false;
    return true;
}

bool DummyEVETCPConnection::RecvData(char* errbuf)
{
    if (errbuf)
        errbuf[0] = 0;
    return true;
}

bool DummyEVETCPConnection::SendData(char* errbuf)
{
    if (errbuf)
        errbuf[0] = 0;
    DrainSendQueue();
    return true;
}

bool DummyEVETCPConnection::ProcessReceivedData(char* errbuf)
{
    if (errbuf)
        errbuf[0] = 0;
    return true;
}

} // namespace EvEmuBots
