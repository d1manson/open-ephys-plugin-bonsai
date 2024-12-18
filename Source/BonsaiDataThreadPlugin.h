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

#ifndef DATATHREADPLUGIN_H_DEFINED
#define DATATHREADPLUGIN_H_DEFINED

#include <DataThreadHeaders.h>
#include "oscpack/osc/OscOutboundPacketStream.h"
#include "oscpack/ip/IpEndpointName.h"
#include "oscpack/osc/OscReceivedElements.h"
#include "oscpack/osc/OscPacketListener.h"
#include "oscpack/ip/UdpSocket.h"
#include "OSCServer.h"
#include "QualityInfo.h"

namespace Bonsai {

    class DataThreadPlugin : public DataThread
    {
    public:
        /** The class constructor, used to initialize any members. */
        DataThreadPlugin(SourceNode* sn);

        /** The class destructor, used to deallocate memory */
        ~DataThreadPlugin();

        // ------------------------------------------------------------
        //                  PURE VIRTUAL METHODS
        //     (must be implemented by all DataThreads)
        // ------------------------------------------------------------

        /** Called repeatedly to add any available data to the buffer */
        bool updateBuffer() override;

        /** Returns true if the data source is connected, false otherwise.*/
        bool foundInputSource() override;

        /** Initializes data transfer.*/
        bool startAcquisition() override;

        /** Stops data transfer.*/
        bool stopAcquisition() override;

        /* Passes the processor's info objects to DataThread, to allow them to be configured */
        void updateSettings(OwnedArray<ContinuousChannel>* continuousChannels,
            OwnedArray<EventChannel>* eventChannels,
            OwnedArray<SpikeChannel>* spikeChannels,
            OwnedArray<DataStream>* sourceStreams,
            OwnedArray<DeviceInfo>* devices,
            OwnedArray<ConfigurationObject>* configurationObjects) override;

        // ------------------------------------------------------------
        //                   VIRTUAL METHODS 
        //       (can optionally be overriden by sub-classes)
        // ------------------------------------------------------------

        /** Called when the chain updates, to add, remove or resize the sourceBuffers' DataBuffers as needed*/
        void resizeBuffers() override;

        /** Create the DataThread custom editor */
        std::unique_ptr<GenericEditor> createEditor(SourceNode* sn) override;

        // ** Allows the DataThread plugin to respond to messages sent by other processors */
        void handleBroadcastMessage(String msg) override;

        // ** Allows the DataThread plugin to handle a config message while acquisition is not active. */
        String handleConfigMessage(String msg) override;

        void run() override;

        /* Reference to this is provided to the server and the sampleQualityComponent within the editor. Uses a lock
           for simplicity. */
        QualityInfo qualityInfo;


    private:
        std::unique_ptr<OSCServer> server;

        SourceNode* sourceNode;



    };

}
#endif