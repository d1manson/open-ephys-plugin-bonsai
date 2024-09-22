/*
 ------------------------------------------------------------------

 This file is part of the Open Ephys GUI
 Copyright (C) 2022 Open Ephys

 ------------------------------------------------------------------

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#include "BonsaiDataThreadPlugin.h"

#include "BonsaiDataThreadPluginEditor.h"

#include <ProcessorHeaders.h>

namespace Bonsai {

    constexpr size_t DEFAULT_OSC_PORT = 27020;
    constexpr char DEFAULT_OSC_ADDRESS[] = "/bonsai";


    DataThreadPlugin::DataThreadPlugin(SourceNode* sn) : DataThread(sn)
    {
        
    }


    DataThreadPlugin::~DataThreadPlugin()
    {
       
    }


    int DataThreadPlugin::getOSCPort() const
    {
       // todo: provide way to configure this in the interface, see https://github.com/open-ephys-plugins/osc-io/blob/main/Source/OSCEvents.cpp
       return DEFAULT_OSC_PORT; 
    }

    String DataThreadPlugin::getOSCAddress() const
    {
        return DEFAULT_OSC_ADDRESS;
    }

    bool DataThreadPlugin::updateBuffer()
    {
        return true; // we don't actually use the updateBuffer method in this case, rather data is written within OSCServer.ProcessMessage()
    }

    bool DataThreadPlugin::foundInputSource()
    {
        return server && server->IsBound();
    }

    bool DataThreadPlugin::startAcquisition()
    {
        if (server && server->IsBound()) {
            LOGE("OSC Server should not be running yet when calling startAcquisition(), but it is.");
            return false;
        }
        if (!sourceBuffers.getFirst()) {
            LOGE("sourceBuffer should be allocated when calling startAcquisition(), but it is.");
            return false;
        }
        sourceBuffers.getFirst()->clear();
        server = std::make_unique<OSCServer>(getOSCPort(), getOSCAddress(), sourceBuffers.getFirst());
        startThread(); // will call run() on this class, which in turn calls server->run()
        return true;
    }

    void DataThreadPlugin::run()
    {
        // this function is called by the base JUCE Thread class, on a dedicated thread
        if (!server) {
            LOGE("OSC server not created yet.");
            return;
        }
        server->run();
    }

    bool DataThreadPlugin::stopAcquisition()
    {
        if (!server || !server->IsBound()) {
            LOGE("OSC Server should be running when calling stopAcquisition(), but it isn't.");
            return false;
        }
        server->stop();
        if (!stopThread(500)) {
            return false;
        }
        server = nullptr;
        return true;
    }


    void DataThreadPlugin::updateSettings(OwnedArray<ContinuousChannel>* continuousChannels,
        OwnedArray<EventChannel>* eventChannels,
        OwnedArray<SpikeChannel>* spikeChannels,
        OwnedArray<DataStream>* sourceStreams,
        OwnedArray<DeviceInfo>* devices,
        OwnedArray<ConfigurationObject>* configurationObjects)
    {
        if (server && server->IsBound()) {
            LOGE("OSC Server should not be running when updating Settings, but it is.");
            return;
        }

        sourceStreams->clear();
        sourceBuffers.clear();
        continuousChannels->clear();

        DataStream* stream = new DataStream({
           "bonsai",
           "4x int32 values from bonsai over UDP/OSC",
           "bonsai",
           1000.0  // stream sample rate; not sure how this is actually used, the value here was chosen arbitrarily
        });

        sourceStreams->add(stream);

        sourceBuffers.add(new DataBuffer(1, 4 * 1024));

        continuousChannels->add(new ContinuousChannel({
            ContinuousChannel::Type::AUX,
            "CH1",
            "4 int32s packed into 1D float array",
            "CH1",
            0,  // channel bitvolts scaling, not relevant here
            stream
        }));

    }


    void DataThreadPlugin::resizeBuffers()
    {

    }


    std::unique_ptr<GenericEditor> DataThreadPlugin::createEditor(SourceNode* sn)
    {

        std::unique_ptr<DataThreadPluginEditor> editor = std::make_unique<DataThreadPluginEditor>(sn);

        return editor;

    }

    void DataThreadPlugin::handleBroadcastMessage(String msg)
    {


    }

    String DataThreadPlugin::handleConfigMessage(String msg)
    {

        return "";
    }


}