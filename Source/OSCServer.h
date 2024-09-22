/*------------------------------------------------------------------

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

#ifndef OSCSERVER_H
#define OSCSERVER_H

#include <ProcessorHeaders.h>
#include <stdio.h>
#include "oscpack/osc/OscOutboundPacketStream.h"
#include "oscpack/ip/IpEndpointName.h"
#include "oscpack/osc/OscReceivedElements.h"
#include "oscpack/osc/OscPacketListener.h"
#include "oscpack/ip/UdpSocket.h"
#include <DataThreadHeaders.h>

/*
	An OSC UDP Server that expects messages with 4 int16 values. It writes them into the dataBuffer.
	Note that DataBuffer is thread-safe, because it's managed via a JUCE AbstractFifo under the hood.
*/
namespace Bonsai {
	class OSCServer 
		: public osc::OscPacketListener
	{
	public:

		/** Constructor */
		OSCServer(int port, String address, DataBuffer* dataBuffer);

		/** Destructor*/
		~OSCServer();

		/** Blocking method, that should be called on its own thread. */
		void run();

		void stop();

		DataBuffer* dataBuffer;

		bool IsBound() { return socket && socket->IsBound(); };

	protected:
		/** OscPacketListener method*/
		virtual void ProcessMessage(const osc::ReceivedMessage& m, const IpEndpointName&);

	private:
		int port;
		String address;
		std::unique_ptr<UdpListeningReceiveSocket> socket;


		//JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Bonsai);
	};

}
#endif