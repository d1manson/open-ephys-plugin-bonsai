# Bonsai Plugin

![image](https://github.com/user-attachments/assets/25d66467-8fc4-40d6-a082-40c90fecc8d6)

This runs an OSC UDP Server (using an open ephys DataThread plugin). The Server expects messages that optionally start with a timestamp value, and then contain 1-8 float values
(you must specify the exact number of values in the interface and specify if there is a timetamp first or not). Each of the float values is given its own continuous channel.
You can view the raw values in openephys using the LFP viewer, though it's not going to be that interesting in that form. The timestamp is not plugged into the proper timestamp
machinery of openephys because that doesn't actually seem to be visible to the record engine downstream. Instead we put the timestamp into its own channel (the first channel,
before the other channels).

There is a slight problem with having a 32bit timestamp though:  32 bit floats can only just about represent 3 decimal places above 20,000 (5hs in seconds); at that point there are
about 5 representable values in the third decimal place for each value in the second decimal place. But hopefully that's enough here. The timestamp provided by Bonsai should be
a 64 bit double, but it will then be converted to a single (32bit) float here, with the first received timestamp treated as the zero point (which ensures that the number are small
so long as acquisition is restarted at least every 4hrs or so).

If you do provide a timestamp, the plugin is then able to "smooth" the samples to match the exact sample rate you stated. This means that if a given sample's timestamp is too early
it may get dropped entirely, and if it's very late NaNs will be used to fill the gap. The widget has a way of visualising this: dropped samples are shown in red; gaps filled are
shown in white; green is good samples, specifically a full green bar is a timestamp that is accurate with partial green bars indicating a slightly early or late sample.  To be clear
this is not referencing when the data was recieved, it's just looking at the value of the timestamps that were recieved relative to the first such timestamp and the stated sample rate.

The intention is that the data is coming from a camera via Bonsai, with the float values corresponding to some kind of x and y values - indeed we provide a python script here,
`dummy-bonsai.py` which can be used to simmulate an animal running around indefinitely with two leds on its head.

WARNING: this is not yet going to allow for any kind of precision in terms of timing latency relative to other data sources, it only provides internal consistency.


# Data Thread Plugin Template

This repository contains a template for building **Data Thread** plugins for the [Open Ephys GUI](https://github.com/open-ephys/plugin-GUI). Data Thread plugins typically communicate with an external piece of hardware with an acquisition clock that is not synchronized with the GUI's signal processing callbacks. Examples of commonly used Data Thread plugins include [Neuropixels PXI](https://github.com/open-ephys-plugins/neuropixels-pxi) and the [Rhythm Plugins](https://github.com/open-ephys-plugins/rhythm-plugins).

Information on the Open Ephys Plugin API can be found on [the GUI's documentation site](https://open-ephys.github.io/gui-docs/Developer-Guide/Open-Ephys-Plugin-API.html).

## Creating a new Data Thread Plugin

1. Click "Use this template" to instantiate a new repository under your GitHub account. 
2. Clone the new repository into a directory at the same level as the `plugin-GUI` repository. This is typically named `OEPlugins`, but it can have any name you'd like.
3. Modify the [OpenEphysLib.cpp file](https://open-ephys.github.io/gui-docs/Developer-Guide/Creating-a-new-plugin.html) to include your plugin's name and version number.
4. Create the plugin [build files](https://open-ephys.github.io/gui-docs/Developer-Guide/Compiling-plugins.html) using CMake.
5. Use Visual Studio (Windows), Xcode (macOS), or `make` (Linux) to compile the plugin.
6. Edit the code to add custom functionality, and add additional source files as needed.

## Repository structure

This repository contains 3 top-level directories:

- `Build` - Plugin build files will be auto-generated here. These files will be ignored in all `git` commits.
- `Source` - All plugin source files (`.h` and `.cpp`) should live here. There can be as many source code sub-directories as needed.
- `Resources` - This is where you should store any non-source-code files, such as library files or scripts.

## Using external libraries

To link the plugin to external libraries, it is necessary to manually edit the Build/CMakeLists.txt file. The code for linking libraries is located in comments at the end.
For most common libraries, the `find_package` option is recommended. An example would be

```cmake
find_package(ZLIB)
target_link_libraries(${PLUGIN_NAME} ${ZLIB_LIBRARIES})
target_include_directories(${PLUGIN_NAME} PRIVATE ${ZLIB_INCLUDE_DIRS})
```

If there is no standard package finder for cmake, `find_library`and `find_path` can be used to find the library and include files respectively. The commands will search in a variety of standard locations For example

```cmake
find_library(ZMQ_LIBRARIES NAMES libzmq-v120-mt-4_0_4 zmq zmq-v120-mt-4_0_4) #the different names after names are not a list of libraries to include, but a list of possible names the library might have, useful for multiple architectures. find_library will return the first library found that matches any of the names
find_path(ZMQ_INCLUDE_DIRS zmq.h)

target_link_libraries(${PLUGIN_NAME} ${ZMQ_LIBRARIES})
target_include_directories(${PLUGIN_NAME} PRIVATE ${ZMQ_INCLUDE_DIRS})
```

### Providing libraries for Windows

Since Windows does not have standardized paths for libraries, as Linux and macOS do, it is sometimes useful to pack the appropriate Windows version of the required libraries alongside the plugin.
To do so, a _libs_ directory has to be created **at the top level** of the repository, alongside this README file, and files from all required libraries placed there. The required folder structure is:

```
    libs
    ├─ include           #library headers
    ├─ lib
        ├─ x64           #64-bit compile-time (.lib) files
        └─ x86           #32-bit compile time (.lib) files, if needed
    └─ bin
        ├─ x64           #64-bit runtime (.dll) files
        └─ x86           #32-bit runtime (.dll) files, if needed
```

DLLs in the bin directories will be copied to the open-ephys GUI _shared_ folder when installing.
