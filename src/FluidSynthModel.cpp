//
// Created by Alex Birch on 10/09/2017.
//

#include <iostream>
#include <iterator>
#include <fluidsynth.h>
#include "FluidSynthModel.h"
#include "MidiConstants.h"
#include "Util.h"

using namespace std;

const map<fluid_midi_control_change, String> FluidSynthModel::controllerToParam{
    {SOUND_CTRL2, "filterResonance"}, // MIDI CC 71 Timbre/Harmonic Intensity (filter resonance)
    {SOUND_CTRL3, "release"}, // MIDI CC 72 Release time
    {SOUND_CTRL4, "attack"}, // MIDI CC 73 Attack time
    {SOUND_CTRL5, "filterCutOff"}, // MIDI CC 74 Brightness (cutoff frequency, FILTERFC)
    {SOUND_CTRL6, "decay"}, // MIDI CC 75 Decay Time
    {SOUND_CTRL10, "sustain"}}; // MIDI CC 79 undefined

const map<String, fluid_midi_control_change> FluidSynthModel::paramToController{[]{
    map<String, fluid_midi_control_change> map;
    transform(
        controllerToParam.begin(),
        controllerToParam.end(),
        inserter(map, map.begin()),
        [](const pair<fluid_midi_control_change, String>& pair) {
            return make_pair(pair.second, pair.first);
        });
    return map;
}()};

fluid_long_long_t fileOffset;
fluid_long_long_t fileSize;

#ifdef _WIN32
#define FLUID_PRIi64 "I64d"
#else
#define FLUID_PRIi64 "lld"
#endif

//ScopedPointer<FileLogger> logger = FileLogger::createDateStampedLogger(
//        "JuicySFTest",
//        "Server_",
//        ".txt",
//        "*** SERVER ***");

void *mem_open(const char *path)
{
    void *buffer;

    if(path[0] != '&')
    {
        return nullptr;
    }
    sscanf(path, "&%p %" FLUID_PRIi64, &buffer, &fileSize);
    fileOffset = 0;
    return buffer;
}

int mem_read(void *buf, fluid_long_long_t count, void * data)
{
    memcpy(buf, ((uint8_t*)data) + fileOffset, (size_t)count);
    fileOffset += count;
    return FLUID_OK;
}

int mem_seek(void * data, fluid_long_long_t offset, int whence)
{
    if (whence == SEEK_SET) {
        fileOffset = offset;
    } else if (whence == SEEK_CUR) {
        fileOffset += offset;
    }  else if (whence == SEEK_END) {
        fileOffset = fileSize + offset;
    }
    return FLUID_OK;
}

int mem_close(void *handle)
{
    fileOffset = 0;
    fileSize = 0;
    return FLUID_OK;
}

fluid_long_long_t mem_tell(void *handle)
{
    return fileOffset;
}

FluidSynthModel::FluidSynthModel(
    AudioProcessorValueTreeState& valueTreeState
    )
: valueTreeState{valueTreeState}
, settings{nullptr, nullptr}
, synth{nullptr, nullptr}
, currentSampleRate{44100}
, sfont_id{-1}
{
    valueTreeState.addParameterListener("bank", this);
    valueTreeState.addParameterListener("preset", this);
    for (const auto &[param, controller]: paramToController) {
        valueTreeState.addParameterListener(param, this);
    }
    valueTreeState.state.addListener(this);
}

FluidSynthModel::~FluidSynthModel() {
    for (const auto &[param, controller]: paramToController) {
        valueTreeState.removeParameterListener(param, this);
    }
    valueTreeState.removeParameterListener("bank", this);
    valueTreeState.removeParameterListener("preset", this);
    valueTreeState.state.removeListener(this);
}

void FluidSynthModel::initialise() {
    // deactivate all audio drivers in fluidsynth to avoid FL Studio deadlock when initialising CoreAudio
    // after all: we only use fluidsynth to render blocks of audio. it doesn't output to audio driver.
    const char *DRV[] {NULL};
    fluid_audio_driver_register(DRV);
    
    settings = { new_fluid_settings(), delete_fluid_settings };


//    fluid_settings_setint(this->settings.get(), "synth.reverb.active", 1);
//    fluid_settings_setstr(this->settings.get(), "synth.chorus.active", "no");
    fluid_settings_setstr(this->settings.get(), "synth.midi-bank-select", "gs");
    fluid_settings_setint(this->settings.get(), "synth.midi-channels", 48);

    // https://sourceforge.net/p/fluidsynth/wiki/FluidSettings/
#if JUCE_DEBUG
    fluid_settings_setint(settings.get(), "synth.verbose", 1);
#endif

    synth = { new_fluid_synth(settings.get()), delete_fluid_synth };

    fluid_synth_set_reverb_on(synth.get(), true);
//    fluid_synth_set_reverb(synth.get(), 0.7, 0.4, 0.5, 0.5);
    fluid_synth_set_reverb(synth.get(), 0.5, 0.8, 0.5, 0.3);

    fluid_sfloader_t *my_sfloader = new_fluid_defsfloader(settings.get());
    fluid_sfloader_set_callbacks(my_sfloader,
                                 mem_open,
                                 mem_read,
                                 mem_seek,
                                 mem_tell,
                                 mem_close);
    fluid_synth_add_sfloader(synth.get(), my_sfloader);

    fluid_synth_set_sample_rate(synth.get(), currentSampleRate);


    // I can't hear a damned thing
    fluid_synth_set_gain(synth.get(), 2.0);

//    fluid_synth_reverb_on(synth.get(), -1, true);
//    fluid_synth_set_reverb(synth.get(), 1.0, 0.3, 50, 1.0);
    
    // note: fluid_chan.c#fluid_channel_init_ctrl()
    // > Just like panning, a value of 64 indicates no change for sound ctrls
    // --
    // so, advice is to leave everything at 64
    // and yet, I'm finding that default modulators start at MIN,
    // i.e. we are forced to start at 0 and climb from there
    // --
    // let's zero out every audio param that we manage
//    for (const auto &[controller, param]: controllerToParam) {
//        setControllerValue(static_cast<int>(controller), 0);
//    }
    
    // http://www.synthfont.com/SoundFont_NRPNs.PDF
//    float env_amount{20000.0f};
//
//    unique_ptr<fluid_mod_t, decltype(&delete_fluid_mod)> mod{new_fluid_mod(), delete_fluid_mod};
//    fluid_mod_set_source1(mod.get(),
//                          static_cast<int>(SOUND_CTRL2), // MIDI CC 71 Timbre/Harmonic Intensity (filter resonance)
//                          FLUID_MOD_CC
//                          | FLUID_MOD_UNIPOLAR
//                          | FLUID_MOD_CONCAVE
//                          | FLUID_MOD_POSITIVE);
//    fluid_mod_set_source2(mod.get(), 0, 0);
//    fluid_mod_set_dest(mod.get(), GEN_FILTERQ);
//    fluid_mod_set_amount(mod.get(), FLUID_PEAK_ATTENUATION);
//    fluid_synth_add_default_mod(synth.get(), mod.get(), FLUID_SYNTH_ADD);
//
//    mod = {new_fluid_mod(), delete_fluid_mod};
//    fluid_mod_set_source1(mod.get(),
//                          static_cast<int>(SOUND_CTRL3), // MIDI CC 72 Release time
//                          FLUID_MOD_CC
//                          | FLUID_MOD_UNIPOLAR
//                          | FLUID_MOD_LINEAR
//                          | FLUID_MOD_POSITIVE);
//    fluid_mod_set_source2(mod.get(), 0, 0);
//    fluid_mod_set_dest(mod.get(), GEN_VOLENVRELEASE);
//    fluid_mod_set_amount(mod.get(), env_amount);
//    fluid_synth_add_default_mod(synth.get(), mod.get(), FLUID_SYNTH_ADD);
//
//    mod = {new_fluid_mod(), delete_fluid_mod};
//    fluid_mod_set_source1(mod.get(),
//                          static_cast<int>(SOUND_CTRL4), // MIDI CC 73 Attack time
//                          FLUID_MOD_CC
//                          | FLUID_MOD_UNIPOLAR
//                          | FLUID_MOD_LINEAR
//                          | FLUID_MOD_POSITIVE);
//    fluid_mod_set_source2(mod.get(), 0, 0);
//    fluid_mod_set_dest(mod.get(), GEN_VOLENVATTACK);
//    fluid_mod_set_amount(mod.get(), env_amount);
//    fluid_synth_add_default_mod(synth.get(), mod.get(), FLUID_SYNTH_ADD);
//
//    // soundfont spec says that if cutoff is >20kHz and resonance Q is 0, then no filtering occurs
//    mod = {new_fluid_mod(), delete_fluid_mod};
//    fluid_mod_set_source1(mod.get(),
//                          static_cast<int>(SOUND_CTRL5), // MIDI CC 74 Brightness (cutoff frequency, FILTERFC)
//                          FLUID_MOD_CC
//                          | FLUID_MOD_LINEAR
//                          | FLUID_MOD_UNIPOLAR
//                          | FLUID_MOD_POSITIVE);
//        fluid_mod_set_source2(mod.get(), 0, 0);
//    fluid_mod_set_dest(mod.get(), GEN_FILTERFC);
//    fluid_mod_set_amount(mod.get(), -2400.0f);
//    fluid_synth_add_default_mod(synth.get(), mod.get(), FLUID_SYNTH_ADD);
//
//    mod = {new_fluid_mod(), delete_fluid_mod};
//    fluid_mod_set_source1(mod.get(),
//                          static_cast<int>(SOUND_CTRL6), // MIDI CC 75 Decay Time
//                          FLUID_MOD_CC
//                          | FLUID_MOD_UNIPOLAR
//                          | FLUID_MOD_LINEAR
//                          | FLUID_MOD_POSITIVE);
//    fluid_mod_set_source2(mod.get(), 0, 0);
//    fluid_mod_set_dest(mod.get(), GEN_VOLENVDECAY);
//    fluid_mod_set_amount(mod.get(), env_amount);
//    fluid_synth_add_default_mod(synth.get(), mod.get(), FLUID_SYNTH_ADD);
//
//    mod = {new_fluid_mod(), delete_fluid_mod};
//    fluid_mod_set_source1(mod.get(),
//                          static_cast<int>(SOUND_CTRL10), // MIDI CC 79 undefined
//                          FLUID_MOD_CC
//                          | FLUID_MOD_UNIPOLAR
//                          | FLUID_MOD_CONCAVE
//                          | FLUID_MOD_POSITIVE);
//    fluid_mod_set_source2(mod.get(), 0, 0);
//    fluid_mod_set_dest(mod.get(), GEN_VOLENVSUSTAIN);
//    // fluice_voice.c#fluid_voice_update_param()
//    // clamps the range to between 0 and 1000, so we'll copy that
//    fluid_mod_set_amount(mod.get(), 1000.0f);
//    fluid_synth_add_default_mod(synth.get(), mod.get(), FLUID_SYNTH_ADD);
}

const StringArray FluidSynthModel::programChangeParams{"bank", "preset"};
void FluidSynthModel::parameterChanged(const String& parameterID, float newValue) {
    DBG("FluidSynthModel::parameterChanged()");

//    if (programChangeParams.contains(parameterID)) {
//        int bank, preset;
//        {
//            RangedAudioParameter *param{valueTreeState.getParameter("bank")};
//            jassert(dynamic_cast<AudioParameterInt*>(param) != nullptr);
//            AudioParameterInt* castParam{dynamic_cast<AudioParameterInt*>(param)};
//            bank = castParam->get();
//        }
//        {
//            RangedAudioParameter *param{valueTreeState.getParameter("preset")};
//            jassert(dynamic_cast<AudioParameterInt*>(param) != nullptr);
//            AudioParameterInt* castParam{dynamic_cast<AudioParameterInt*>(param)};
//            preset = castParam->get();
//        }
//        int bankOffset{fluid_synth_get_bank_offset(synth.get(), sfont_id)};
//        fluid_synth_program_select(
//            synth.get(),
//            channel,
//            sfont_id,
//            static_cast<unsigned int>(bankOffset + bank),
//            static_cast<unsigned int>(preset));
//    } else if (
//        // https://stackoverflow.com/a/55482091/5257399
//        auto it{paramToController.find(parameterID)};
//        it != end(paramToController)) {
//        RangedAudioParameter *param{valueTreeState.getParameter(parameterID)};
//        jassert(dynamic_cast<AudioParameterInt*>(param) != nullptr);
//        AudioParameterInt* castParam{dynamic_cast<AudioParameterInt*>(param)};
//        int value{castParam->get()};
//        int controllerNumber{static_cast<int>(it->second)};
//
//        fluid_synth_cc(
//            synth.get(),
//            channel,
//            controllerNumber,
//            value);
//    }
}

void FluidSynthModel::valueTreePropertyChanged(ValueTree& treeWhosePropertyHasChanged,
                                               const Identifier& property) {
    if (treeWhosePropertyHasChanged.getType() == StringRef("soundFont")) {
        if (property == StringRef("path")) {
            String soundFontPath = treeWhosePropertyHasChanged.getProperty("path", "");
            if (soundFontPath.isNotEmpty()) {
                unloadAndLoadFont(soundFontPath);
            }
        } else if (property == StringRef("memfile")) {
            var memfile = treeWhosePropertyHasChanged.getProperty("memfile", var(nullptr, 0));

            if (memfile.isBinaryData() && memfile.getBinaryData()->getSize() != 0) {
                const size_t dataSize = memfile.getBinaryData()->getSize();
                unloadAndLoadFontFromMemory(memfile.getBinaryData()->getData(), dataSize);
            }
        }
    }
}

//void FluidSynthModel::setControllerValue(int controller, int value) {
//    fluid_synth_cc(
//        synth.get(),
//        channel,
//        controller,
//        value);
//}

void FluidSynthModel::unloadAndLoadFont(const String &absPath) {
    // in the base case, there is no font loaded
    if (fluid_synth_sfcount(synth.get()) > 0) {
        // if -1 is returned, that indicates failure
        // not really sure how to handle "fail to unload"
        fluid_synth_sfunload(synth.get(), sfont_id, 1);
        sfont_id = -1;
    }
    loadFont(absPath);
}

void FluidSynthModel::unloadAndLoadFontFromMemory(void *sf, size_t fileSize) {
    // in the base case, there is no font loaded
    if (fluid_synth_sfcount(synth.get()) > 0) {
        // if -1 is returned, that indicates failure
        // not really sure how to handle "fail to unload"
        fluid_synth_sfunload(synth.get(), sfont_id, 1);
        sfont_id = -1;
    }
    loadFontFromMemory(sf, fileSize);
}

void FluidSynthModel::loadFont(const String &absPath) {
    if (!absPath.isEmpty()) {
        sfont_id = fluid_synth_sfload(synth.get(), absPath.toStdString().c_str(), 1);
        // if -1 is returned, that indicates failure
    }
    // refresh regardless of success, if only to clear the table
    refreshBanks();
}

void FluidSynthModel::loadFontFromMemory(void *sf, fluid_long_long_t fileSize) {
    char abused_filename[128];
    snprintf(abused_filename, 128, "&%p %" FLUID_PRIi64, sf, fileSize);

    const char* testString = (char*)sf;

    sfont_id = fluid_synth_sfload(synth.get(), abused_filename, 1);
        // if -1 is returned, that indicates failure
    // refresh regardless of success, if only to clear the table
    refreshBanks();
}

void FluidSynthModel::refreshBanks() {
    ValueTree banks{"banks"};
    fluid_sfont_t* sfont{
        sfont_id == -1
        ? nullptr
        : fluid_synth_get_sfont_by_id(synth.get(), sfont_id)
    };
    if (sfont) {
        int greatestEncounteredBank{-1};
        ValueTree bank;

        fluid_sfont_iteration_start(sfont);
        for(fluid_preset_t* preset {fluid_sfont_iteration_next(sfont)};
        preset != nullptr;
        preset = fluid_sfont_iteration_next(sfont)) {
            int bankNum{fluid_preset_get_banknum(preset)};
            if (bankNum > greatestEncounteredBank) {
                if (greatestEncounteredBank > -1) {
                    banks.appendChild(bank, nullptr);
                }
                bank = { "bank", {
                    { "num", bankNum }
                } };
                greatestEncounteredBank = bankNum;
            }
            bank.appendChild({ "preset", {
                { "num", fluid_preset_get_num(preset) },
                { "name", String{fluid_preset_get_name(preset)} }
            }, {} }, nullptr);
        }
        if (greatestEncounteredBank > -1) {
            banks.appendChild(bank, nullptr);
        }
    }
    valueTreeState.state.getChildWithName("banks").copyPropertiesAndChildrenFrom(banks, nullptr);
    valueTreeState.state.getChildWithName("banks").sendPropertyChangeMessage("synthetic");
    
#if JUCE_DEBUG
//    unique_ptr<XmlElement> xml{valueTreeState.state.createXml()};
//    Logger::outputDebugString(xml->createDocument("",false,false));
#endif
}

void FluidSynthModel::setSampleRate(float sampleRate) {
    currentSampleRate = sampleRate;
    // https://stackoverflow.com/a/40856043/5257399
    // test if a smart pointer is null
    if (!synth) {
        // don't worry; we'll do this in initialise phase regardless
        return;
    }
    fluid_synth_set_sample_rate(synth.get(), sampleRate);
}

void FluidSynthModel::handleMidiMessage(MidiMessage& m) {
    if (m.isNoteOn()) {
        fluid_synth_noteon(
                synth.get(),
                m.getChannel() - 1 + (channelGroup*16),
                m.getNoteNumber(),
                m.getVelocity());
    } else if (m.isNoteOff()) {
        fluid_synth_noteoff(
                synth.get(),
                m.getChannel() - 1 + (channelGroup*16),
                m.getNoteNumber());
    } else if (m.isController()) {
        fluid_synth_cc(
                synth.get(),
                m.getChannel() - 1 + (channelGroup*16),
                m.getControllerNumber(),
                m.getControllerValue());

//            fluid_midi_control_change controllerNum{static_cast<fluid_midi_control_change>(m.getControllerNumber())};
//            if (auto it{controllerToParam.find(controllerNum)};
//                it != end(controllerToParam)) {
//                String parameterID{it->second};
//                RangedAudioParameter *param{valueTreeState.getParameter(parameterID)};
//                jassert(dynamic_cast<AudioParameterInt*>(param) != nullptr);
//                AudioParameterInt* castParam{dynamic_cast<AudioParameterInt*>(param)};
//                *castParam = m.getControllerValue();
//            }
    } else if (m.isProgramChange()) {
#if JUCE_DEBUG
        String debug{"MIDI program change: "};
        debug << m.getProgramChangeNumber();
        Logger::outputDebugString(debug);
#endif
        int result{fluid_synth_program_change(
                synth.get(),
                m.getChannel() - 1 + (channelGroup*16),
                m.getProgramChangeNumber())};
        if (result == FLUID_OK) {
            RangedAudioParameter *param{valueTreeState.getParameter("preset")};
            jassert(dynamic_cast<AudioParameterInt*>(param) != nullptr);
            AudioParameterInt* castParam{dynamic_cast<AudioParameterInt*>(param)};
            *castParam = m.getProgramChangeNumber();
        }
    } else if (m.isPitchWheel()) {
        fluid_synth_pitch_bend(
                synth.get(),
                m.getChannel() - 1 + (channelGroup*16),
                m.getPitchWheelValue());
    } else if (m.isChannelPressure()) {
        fluid_synth_channel_pressure(
                synth.get(),
                m.getChannel() - 1 + (channelGroup*16),
                m.getChannelPressureValue());
    } else if (m.isAftertouch()) {
        fluid_synth_key_pressure(
                synth.get(),
                m.getChannel() - 1 + (channelGroup*16),
                m.getNoteNumber(),
                m.getAfterTouchValue());
//        } else if (m.isMetaEvent()) {
//            fluid_midi_event_t *midi_event{new_fluid_midi_event()};
//            fluid_midi_event_set_type(midi_event, static_cast<int>(MIDI_SYSTEM_RESET));
//            fluid_synth_handle_midi_event(synth.get(), midi_event);
//            delete_fluid_midi_event(midi_event);
    } else if (m.isSysEx()) {
        auto sysexData = m.getSysExData();
        if (sysexData[0] == 0xBF) {
            channelGroup = static_cast<int>(sysexData[1]);
            auto wrappedMsg = juce::MidiMessage(sysexData+2, m.getSysExDataSize()-2, 0);
            handleMidiMessage(wrappedMsg);
        } else {
            fluid_synth_sysex(
                    synth.get(),
                    reinterpret_cast<const char *>(m.getSysExData()),
                    m.getSysExDataSize(),
                    nullptr, // no response pointer because we have no interest in handling response currently
                    nullptr, // no response_len pointer because we have no interest in handling response currently
                    nullptr, // no handled pointer because we have no interest in handling response currently
                    static_cast<int>(false));
        }
    }
}

void FluidSynthModel::processBlock(AudioBuffer<float>& buffer, MidiBuffer& midiMessages) {
    MidiBuffer processedMidi;
    int time;
    MidiMessage m;

    for (MidiBuffer::Iterator i{midiMessages}; i.getNextEvent(m, time);) {
        DEBUG_PRINT(m.getDescription());

        handleMidiMessage(m);
    }

    // fluid_synth_get_cc(fluidSynth, 0, 73, &pval);
    // Logger::outputDebugString ( juce::String::formatted("hey: %d\n", pval) );
    auto bufferWritePointers = buffer.getArrayOfWritePointers();

    fluid_synth_process(
        synth.get(),
        buffer.getNumSamples(),
        2,
        bufferWritePointers,
        buffer.getNumChannels(),
        bufferWritePointers);
}

int FluidSynthModel::getNumPrograms()
{
    return 128;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int FluidSynthModel::getCurrentProgram()
{
    RangedAudioParameter *param{valueTreeState.getParameter("preset")};
    jassert(dynamic_cast<AudioParameterInt*>(param) != nullptr);
    AudioParameterInt* castParam{dynamic_cast<AudioParameterInt*>(param)};
    return castParam->get();
}

void FluidSynthModel::setCurrentProgram(int index)
{
    RangedAudioParameter *param{valueTreeState.getParameter("preset")};
    jassert(dynamic_cast<AudioParameterInt*>(param) != nullptr);
    AudioParameterInt* castParam{dynamic_cast<AudioParameterInt*>(param)};
    // setCurrentProgram() gets invoked from non-message thread.
    // AudioParameterInt#operator= will activate any listeners of audio parameter "preset".
    // This includes TableComponent, who will update its UI.
    // we need to lock the message thread whilst it does that UI update.
    const MessageManagerLock mmLock;
    *castParam = index;
}

const String FluidSynthModel::getProgramName(int index)
{
     fluid_sfont_t* sfont{
         sfont_id == -1
         ? nullptr
         : fluid_synth_get_sfont_by_id(synth.get(), sfont_id)
     };
     if (!sfont) {
         String presetName{"Preset "};
         return presetName << index;
     }
     RangedAudioParameter *param{valueTreeState.getParameter("bank")};
     jassert(dynamic_cast<AudioParameterInt*>(param) != nullptr);
     AudioParameterInt* castParam{dynamic_cast<AudioParameterInt*>(param)};
     int bank{castParam->get()};
    
     fluid_preset_t *preset{fluid_sfont_get_preset(
         sfont,
         bank,
         index)};
     if (!preset) {
         String presetName{"Preset "};
         return presetName << index;
     }
     return {fluid_preset_get_name(preset)};
}

void FluidSynthModel::changeProgramName(int index, const String& newName)
{
    // no-op; we don't support modifying the soundfont, so let's not support modification of preset names.
}
