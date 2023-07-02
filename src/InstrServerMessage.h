
#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

enum InstrServerMessageCode
{
    kMessageError = 0,
    kResponseOk,
    kResponseError,
    kSendSF2,
};

class InstrServerMessage {
public:
    InstrServerMessage(InstrServerMessageCode code, void * data, size_t numBytes);
    explicit InstrServerMessage(const MemoryBlock& message);

    bool FromFile(String filePath);
    InstrServerMessageCode GetCode();
    void* GetData();
    size_t GetDataLength();

    const MemoryBlock& GetMemoryBlock() const {
        return fData;
    }

    InstrServerMessageCode code;
    uint32 dataLength;
    MemoryBlock fData;
};