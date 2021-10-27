# Current fork changes:
- Left only CMake build for cross-platform compatibility (Windows is supported for now)
- Updated JUCE
- Changed build instructions section

![image](https://user-images.githubusercontent.com/6141784/60401921-32af9e00-9b80-11e9-8e3a-6c5717f868d6.png)

Latest stable release:

- macOS [2.3.3](https://github.com/Birch-san/juicysfplugin/releases/tag/2.3.3.macOS)
- Windows x64 [2.3.3](https://github.com/Birch-san/juicysfplugin/releases/tag/2.3.3)
- Windows x86 [2.3.3](https://github.com/Birch-san/juicysfplugin/releases/tag/2.3.3)
- Linux was [supported as of 1.0.8](https://github.com/Birch-san/juicysfplugin/pull/3). It has regressed since, but shouldn't be super-hard to get working again. No binary was ever distributed; It has only ever been build-from-source.

Demo track: [mp3](https://github.com/Birch-san/juicysfplugin/releases/download/1.0.5/Demo_track.mp3), [FLAC](https://github.com/Birch-san/juicysfplugin/releases/download/1.0.5/Demo_track.flac), [FLAC +Soundgoodizer](https://github.com/Birch-san/juicysfplugin/releases/download/1.0.5/Demo_track_soundgoodizer.flac)

# What

juicysfplugin is a cross-platform audio plugin for playing MIDI music through a soundfont synthesizer.

It's well-suited for making videogame music. If you have a soundfont of your favourite game, you can write your own melodies with its instruments.

[JUCE](https://github.com/WeAreROLI/JUCE) is the framework for making audio plugins.  
[fluidsynth](http://www.fluidsynth.org/) is the soundfont synthesizer.

Supports SF2 and SF3 soundfont formats.

Supports the following MIDI events:

- noteon, noteoff
- 192 program change (this changes soundfont preset)
- pitch wheel
- aftertouch (key pressure)
- channel pressure
- default modulators as described in [SoundFont specification 2.4](http://freepats.zenvoid.org/sf2/sfspec24.pdf)
  - includes (among others): CC 1 modulation wheel is mapped to vibrato LFO pitch depth
- CC 71 Timbre/Harmonic Intensity (filter resonance)
- CC 72 Release time
- CC 73 Attack time
- CC 74 Brightness (cutoff frequency, FILTERFC)
- CC 75 Decay Time
- CC 79 undefined (being used as Sustain time)
- may support SysEx (untested)

**Mode 1: standalone application**

juicysfplugin.app (or .exe on Windows) is a standalone application, for playing around.

You can plugin your hardware MIDI keyboard, or play with the software MIDI keyboard. Or route MIDI messages into it from a virtual MIDI controller (e.g. the [macOS IAC Bus](http://re-compose.desk.com/customer/portal/articles/1382244-setting-up-the-iac-bus-on-a-mac)).  

**Mode 2: audio plugin**

juicysfplugin audio plugins are provided: VST, VST3, AU, AUv3.  
This means you can host it inside your DAW (e.g. GarageBand, FL Studio Mac, Sibelius…) to record the sounds you make.

# Why

I couldn't find a _free_, _easy-to-use_ macOS audio plugin for making music with soundfonts.

# Install (macOS)

Latest release: https://github.com/Birch-san/juicysfplugin/releases

**Download Release.tar.xz, open it to unzip.**

Release contains:

```
juicysfplugin.app  # standalone application, for playing around
juicysfplugin.component  # AU plugin
juicysfplugin.vst  # VST plugin
juicysfplugin.vst3  # VST3 plugin
```

To install plugins, move them to the following folder:

```
juicysfplugin.component -> ~/Library/Audio/Plug-Ins/Components/juicysfplugin.component
juicysfplugin.vst -> ~/Library/Audio/Plug-Ins/VST/juicysfplugin.vst
juicysfplugin.vst3 -> ~/Library/Audio/Plug-Ins/VST3/juicysfplugin.vst3
```

## Launch (macOS)

Now, you may do one of the following:

- Open the standalone juicysfplugin.app
- Load the AU/VST/VST3 into your favourite DAW (e.g. GarageBand, FL Studio)

# Usage

You must drag-and-drop a soundfont into the file picker.

![image](https://user-images.githubusercontent.com/6141784/37062424-500bc91e-218e-11e8-9916-eae6d82ca566.png)

Here's some soundfonts to get you started:

- [Fatboy](https://fatboy.site/) (no specific license stated, but described as "free")
- MuseScore's [recommended soundfonts](https://musescore.org/en/handbook/soundfonts-and-sfz-files#list) (includes MIT, GPL, other licenses)
- FlameStudios' GPL-licensed [guitar soundfonts](http://www.flamestudios.org/free/Soundfonts)

I'll refrain from recommending certain General MIDI or videogame soundfonts, since the licensing and provenance are often unclear.

## Keybindings

Gain keyboard focus by clicking the plugin's keyboard.

- Up-down arrow keys to switch preset.
- Left-right to switch bank.

ASCII -> MIDI keybinding is the same as FL Studio's:

<img width="256px" alt="image" src="http://s3.amazonaws.com/fl_resource/flkeychart.png">

## Using the standalone .app (macOS) / .exe (Windows)

Generally the .app will Just Work™, but if your audio setup is more bespoke, then you'll need to configure.

**Options > Audio/MIDI settings**

<img width="256px" alt="image" src="https://user-images.githubusercontent.com/6141784/37062230-bdb128b6-218d-11e8-985a-e9b2b5fd0bb2.png">

**Set Output to Built-In Output**

Or any output device that you can hear audio through.

<img width="502" alt="image" src="https://user-images.githubusercontent.com/6141784/62873427-4cafd500-bd17-11e9-80af-03fbf9742802.png">

# Building from source (CMake)
## Windows

CMake, Visual Studio with C++ should be installed.

Run `git submodule update --init --recursive` to checkout JUCE sources. 

Run build script from `scripts\win\`.

DLLs from `bin` dir should be placed along with compiled binary (or in system folders)

For `asio` or `vst2` support sdk headers should be placed to `sdk` dir. Read Steinberg license carefully before doing it.


## Visual Studio Code settings

Concerning the use of Visual Studio Code extension, [C/C++ for Visual Studio Code](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools), as an IDE (i.e. instead of XCode)…

Following advice is for macOS, using brew-installed LLVM 8 Clang.

All includePath entries are shallow (no recursive globbing) for now, since I don't have any deeply-nested headers.  
We don't need to dip into /usr/local, since all library/framework headers are already in this repository.

Ensure that there exists at the root of the repository, a folder named `.vscode`.

`.vscode/c_cpp_properties.json`

```json
{
    "configurations": [
        {
            "name": "Mac",
            "includePath": [
                "${workspaceFolder}/include",
                "${workspaceFolder}/Source",
                "${workspaceFolder}/JuceLibraryCode",
                "${workspaceFolder}/modules"
            ],
            "defines": [],
            "cStandard": "c11",
            "cppStandard": "c++14",
            "intelliSenseMode": "clang-x64",
            "compilerPath": "/usr/local/Cellar/llvm/8.0.0_1/bin/clang++"
        }
    ],
    "version": 4
}
```

I've kept this minimal, but documented some other include paths worthy of consideration (e.g. if more parts of the toolchain were to be used, brew were to be used, or the standard XCode clang were to be used).

`~/Library/Application Support/Code/User/settings.json`

```json
{
    "C_Cpp.updateChannel": "Insiders",
    "C_Cpp.default.intelliSenseMode": "clang-x64",
    "C_Cpp.default.macFrameworkPath": [
        // "/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/System/Library/Frameworks/CoreServices.framework/Frameworks",
        "/System/Library/Frameworks",
        // "/Library/Frameworks"
    ],
    "C_Cpp.default.includePath": [
        // "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/include/c++/v1",
        // "/usr/local/include",
        // "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/lib/clang/10.0.1/include",
        // "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/include",
        // "${workspaceFolder}/include"
        // "/usr/include",
        "/usr/local/Cellar/llvm/8.0.0_1/Toolchains/LLVM8.0.0.xctoolchain/usr/include/c++/v1",
        // "/usr/local/Cellar/llvm/8.0.0_1/Toolchains/LLVM8.0.0.xctoolchain/usr/lib/clang/8.0.0/include",
        // "/usr/local/Cellar/llvm/8.0.0_1/Toolchains/LLVM8.0.0.xctoolchain/usr/include",
    ],
    "C_Cpp.default.cStandard": "c11",
    "C_Cpp.default.cppStandard": "c++14",
    "C_Cpp.default.compilerPath": "/usr/local/Cellar/llvm/8.0.0_1/bin/clang++",
    "C_Cpp.clang_format_fallbackStyle": "LLVM",
    "C_Cpp.clang_format_style": "LLVM",
    "C_Cpp.default.browse.limitSymbolsToIncludedHeaders": true,
    "C_Cpp.default.enableConfigurationSquiggles": true,
    "C_Cpp.errorSquiggles": "Enabled",
    "C_Cpp.enhancedColorization": "Enabled"
}
```

# Test matrix

Known working with:

- macOS Mojave 10.14.5
- Windows 10 x64 1903

# Making releases

Follow the steps in [Building from source](#building-from-source) to output a product to the build folder.

Builds are automatically portable.

```bash
cd juicysfplugin/Builds/MacOSX
# first check that you have a Release or Debug product in the `build` directory

# follows symlinks, archives Release and Debug folders as tar.xz
./archive-for-distribution.sh
```

Note: Release **and** Debug flavors _both_ output targets [VST, VST3, AU] to the same location:  `~/Library/Audio/Plug-Ins/$TARGET/juicysfplugin.$EXT`.  
Whichever flavor you built _most recently_, wins.

# Licenses

Overall, juicysfplugin is GPLv3.

See [licenses for all libraries and frameworks](https://github.com/Birch-san/juicysfplugin/tree/master/licenses_of_dependencies).
