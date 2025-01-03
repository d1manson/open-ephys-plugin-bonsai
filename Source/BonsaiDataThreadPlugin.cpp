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


    DataThreadPlugin::DataThreadPlugin(SourceNode* sn) : DataThread(sn), sourceNode(sn)
    {
        sourceNode->addIntParameter(Parameter::GLOBAL_SCOPE, "Port", "Bonsai OSC port", DEFAULT_OSC_PORT, 1024, 49151, true);
        sourceNode->addStringParameter(Parameter::GLOBAL_SCOPE, "Address", "Bonsai source OSC address", DEFAULT_OSC_ADDRESS, true);
        sourceNode->addBooleanParameter(Parameter::GLOBAL_SCOPE, "Timestamp", "First value within message is timestamp", true, true);
        sourceNode->addIntParameter(Parameter::GLOBAL_SCOPE, "Values", "Number of values within messages (after timestamp)", 6, 1, 8, true);
        sourceNode->addIntParameter(Parameter::GLOBAL_SCOPE, "SampleRate", "Sample Rate (Hz) to show on data stream.", 50, 1, 1000, true);
    }


    DataThreadPlugin::~DataThreadPlugin()
    {
       
    }


    bool DataThreadPlugin::updateBuffer()
    {
        return true; // we don't actually use the updateBuffer method in this case, rather data is written within OSCServer.ProcessMessage()
    }

    bool DataThreadPlugin::foundInputSource()
    {
        return true;
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

        bool hasTimestamp = sourceNode->getParameter("Timestamp")->getValue();
        int sampleRate = sourceNode->getParameter("SampleRate")->getValue();
        qualityInfo.initialise(sampleRate, hasTimestamp);

        server = std::make_unique<OSCServer>(
            sourceNode->getParameter("Port")->getValue(),
            sourceNode->getParameter("Address")->getValue(),
            sourceBuffers.getFirst(),
            hasTimestamp,
            sourceNode->getParameter("Values")->getValue(),
            qualityInfo
        );

        if (!server || !server->IsBound()) {
            return false;
        }
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
        sourceBuffers.getFirst()->clear();
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

        int sampleRate = sourceNode->getParameter("SampleRate")->getValue();
        DataStream* stream = new DataStream({
           "bonsai",
           "float32 values from bonsai over UDP/OSC",
           "bonsai",
           static_cast<float>(sampleRate),
        });

        sourceStreams->add(stream);

        int messageNumValues = sourceNode->getParameter("Values")->getValue();
        bool hasTimestamp = sourceNode->getParameter("Timestamp")->getValue();

        if (hasTimestamp) {
            messageNumValues++;
        }

        sourceBuffers.add(new DataBuffer(messageNumValues, 1024));

        for (int i = 0; i < messageNumValues; i++) {
            continuousChannels->add(new ContinuousChannel({
                ContinuousChannel::Type::AUX,
                "BONS" + std::to_string(i+1),
                i == 0 && hasTimestamp ? "float32 timestamp from bonsai" : "float32 value from bonsai",
                "BONS" + std::to_string(i+1),
                6.,  // channel bitvolts scaling, not especially relevant here
                stream
             }));
        }


        // we don't currently use the event channel for anything but the RecordEngine requires one (or it crashes on startup)
        eventChannels->add(new EventChannel({
            EventChannel::Type::TTL,
            "Bonsai Event Channel",
            "Dummy channel, required for RecordEngine to not crash",
            "bonsai_ttl_events",
            stream,
            1
         }));
    }


    void DataThreadPlugin::resizeBuffers()
    {

    }


    std::unique_ptr<GenericEditor> DataThreadPlugin::createEditor(SourceNode* sn)
    {
        std::unique_ptr<DataThreadPluginEditor> editor = std::make_unique<DataThreadPluginEditor>(sn, qualityInfo);
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
