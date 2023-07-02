
#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class InstrClient: public InterprocessConnection {
public:
    InstrClient();
    ~InstrClient() override;

    void connectionMade() override;
    void connectionLost() override;

    void messageReceived(const juce::MemoryBlock &message) override;

    bool IsConnected() { return fIsConnected; };

private:
    bool fIsConnected;
};
