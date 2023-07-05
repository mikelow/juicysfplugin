#include "InstrServer.h"
#include "InstrServerMessage.h"
#include "InstrClient.h"

InstrServer::InstrServer(AudioProcessorValueTreeState& valueTreeState)
: valueTreeState(valueTreeState) {

}

InstrServer::~InstrServer() {

}

InterprocessConnection *InstrServer::createConnectionObject() {
    DBG("InstrServer::createConnectionObject()");
    auto connection = new InstrServerConnection(valueTreeState);
    connections.add(connection);
    return connection;
}

void InstrServer::openPipe() {
    DBG("InstrServer::openPipe()");
    InstrServerConnection* connection = new InstrServerConnection(valueTreeState);
    bool openedOk = false;
    openedOk = connection->createPipe(kPipeName, kPipeTimeout);
    if (openedOk) {
        connections.add(connection);
    } else {
        AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon,
                                          "Instr Server",
                                          "Failed to open the pipe.");
    }
}

InstrServerConnection::InstrServerConnection(AudioProcessorValueTreeState& valueTreeState)
: InterprocessConnection(true), valueTreeState(valueTreeState), connectionState(Disconnected), messageState(WaitingForMessage)  {
    DBG("InstrServerConnection::InstrServerConnection()");
}

InstrServerConnection::~InstrServerConnection() {
    disconnect();
}

void InstrServerConnection::connectionMade() {
    DBG("InstrServerConnection::connectionMade()");
    fConnected = InstrServerConnection::Connected;
}

void InstrServerConnection::connectionLost() {
    DBG("InstrServerConnection::connectionLost()");

}

void InstrServerConnection::messageReceived(const juce::MemoryBlock& message) {
    DBG("InstrServerConnection::messageReceived()");

    InstrServerMessage serverMessage = InstrServerMessage(message);
    InstrServerMessageCode code = serverMessage.GetCode();

    switch (code) {
        case InstrServerMessageCode::SF2Start: {
            DBG("SF2Start");

            // Reset the memory block
            fileContent.reset(new juce::MemoryBlock());
            messageState = ReceivingSF2;

            break;
        }
        case InstrServerMessageCode::SF2Content: {
            DBG("SF2Content");

            if (messageState != ReceivingSF2) {
                AlertWindow::showMessageBoxAsync (MessageBoxIconType::InfoIcon, "Named Pipe messaging error",
                                  "Received SF2Content but not preceded by SF2Start","OK");
                break;
            }
            juce::MemoryBlock contentBlock(serverMessage.GetData(), serverMessage.GetDataLength());
            fileContent->append(contentBlock.getData(), contentBlock.getSize());
            break;
        }
        case InstrServerMessageCode::SF2End: {
            DBG("SF2End");
            Value value{valueTreeState.state.getChildWithName("soundFont").getPropertyAsValue("memfile", nullptr)};
            value.setValue(var(fileContent.get()->getData(), fileContent.get()->getSize()));

            messageState = WaitingForMessage;
            break;
        }
        default:
            break;
    }
}
