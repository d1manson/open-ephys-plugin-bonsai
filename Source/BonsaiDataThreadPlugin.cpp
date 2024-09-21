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
    constexpr char DEFAULT_OSC_ADDRESS[] = "/red";


    DataThreadPlugin::DataThreadPlugin(SourceNode* sn) : DataThread(sn)
    {
        /*addIntParameter(Parameter::GLOBAL_SCOPE, "Port", "Tracking source OSC port", DEF_PORT, 1024, 49151);
        addStringParameter(Parameter::GLOBAL_SCOPE, "Address", "Tracking source OSC address", DEF_ADDRESS);*/
        server = std::make_unique<OSCServer>(getOSCPort(), getOSCAddress());
        if (server->isBound()) {
            server->startThread();
        }
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
        // todo: call theServer.FlushBuffer(...)
        return true;
    }

    bool DataThreadPlugin::foundInputSource()
    {
        return true;
    }

    bool DataThreadPlugin::startAcquisition()
    {
        return true;
    }

    bool DataThreadPlugin::stopAcquisition()
    {
        return true;
    }


    void DataThreadPlugin::updateSettings(OwnedArray<ContinuousChannel>* continuousChannels,
        OwnedArray<EventChannel>* eventChannels,
        OwnedArray<SpikeChannel>* spikeChannels,
        OwnedArray<DataStream>* sourceStreams,
        OwnedArray<DeviceInfo>* devices,
        OwnedArray<ConfigurationObject>* configurationObjects)
    {


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