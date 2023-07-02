#include "InstrClient.h"


InstrClient::InstrClient()
        :  InterprocessConnection(false, 0xf2b49e2c)
        ,  fIsConnected(false)
{

}

InstrClient::~InstrClient()
{

}

void InstrClient::connectionMade()
{
    DBG("InstrClient::connectionMade()");
    fIsConnected = true;

//    AlertWindow::showMessageBoxAsync (MessageBoxIconType::InfoIcon, "This is an AlertWindow",
//                                      "InstrClient::connectionMade().",
//                                      "OK");
}

void InstrClient::connectionLost()
{
    DBG("InstrClient::connectionLost()");
    fIsConnected = true;
}

void InstrClient::messageReceived(const MemoryBlock& message)
{
//    if (nullptr != fController)
//    {
//        fController->HandleReceivedMessage(message);
//    }
}