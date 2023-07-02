#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class InstrServerConnection;

#define kPortNumber 0xBEEF

class InstrServer : public InterprocessConnectionServer
{
public:
    InstrServer(AudioProcessorValueTreeState& valueTreeState);
    ~InstrServer() override;

    InterprocessConnection* createConnectionObject() override;
private:
    AudioProcessorValueTreeState& valueTreeState;
    OwnedArray<InstrServerConnection> fConnections;
};


class InstrServerConnection: public InterprocessConnection {
public:
    InstrServerConnection(AudioProcessorValueTreeState& valueTreeState);
    ~InstrServerConnection() override;

    enum ConnectionState
    {
        kConnecting = 0,
        kConnected,
        kDisconnected
    };

    void connectionMade() override;
    void connectionLost() override;

    void messageReceived(const juce::MemoryBlock &message) override;

private:
    AudioProcessorValueTreeState& valueTreeState;
    CriticalSection fLock;
    ConnectionState fConnected;
};

//class LoadMessage
//{
//public:
//    LoadMessage();
//    void AppendString(const String& s);
//    void AppendData(const void* data, size_t numBytes);
//
//    const MemoryBlock& GetMemoryBlock() const {
//        return fData;
//    }
//
//
//
//private:
//    MemoryBlock fData;
//};