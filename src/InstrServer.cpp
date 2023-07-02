#include "InstrServer.h"
#include "InstrServerMessage.h"

InstrServer::InstrServer() {

}

InstrServer::~InstrServer() {

}

InterprocessConnection *InstrServer::createConnectionObject() {
    DBG("InstrServer::createConnectionObject()")
    InstrServerConnection* connection = new InstrServerConnection();
    fConnections.add(connection);
    return connection;
}

InstrServerConnection::InstrServerConnection() {
    DBG("InstrServerConnection::InstrServerConnection()")
}

InstrServerConnection::~InstrServerConnection() {

}

void InstrServerConnection::connectionMade() {
    DBG("InstrServerConnection::connectionMade()");
    fConnected = InstrServerConnection::kConnected;
//    AlertWindow::showMessageBoxAsync (MessageBoxIconType::InfoIcon, "This is an AlertWindow",
//                                      "InstrServerConnection::connectionMade().",
//                                      "OK");
}

void InstrServerConnection::connectionLost() {
    DBG("InstrServerConnection::connectionLost()")

}

void InstrServerConnection::messageReceived(const juce::MemoryBlock &message) {
    DBG("InstrServerConnection::messageReceived()")
    const char* test = static_cast<const char*>(message.getData());
    AlertWindow::showMessageBoxAsync (MessageBoxIconType::InfoIcon, "Received Message going to parse",
                                      String("YEP YEP"),
                                      "OK");


    InstrServerMessage serverMessage = InstrServerMessage(message);
    InstrServerMessageCode code = serverMessage.GetCode();




    switch (code) {
        case InstrServerMessageCode::kSendSF2:
            AlertWindow::showMessageBoxAsync (MessageBoxIconType::InfoIcon, "Got SendSF2 message",
                                              "Got Code " + String(code),
                                              "OK");
            break;
        default:
            break;
    }
}
//
//LoadMessage::LoadMessage() {
//}
//
//void LoadMessage::AppendString(const String& s)
//{
//    const char* p = s.toRawUTF8();
//    // getBytesRequiredFor() does *not* include the trailing NULL.
//    size_t len = 1 + CharPointer_UTF8::getBytesRequiredFor(s.getCharPointer());
//    this->AppendData(static_cast<void*>(const_cast<char*>(p)), len);
//
//}
//
//void LoadMessage::AppendData(const void* data, size_t numBytes)
//{
//    fData.append(data, numBytes);
//}