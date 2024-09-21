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
#include <mutex>
#include <vector>


/*
	An OSC UDP Server running its own thread. It expects messages with 4 int16 values.
	Call .flushBuffer() to get the available data and wipe the buffer. This method is
	thread safe (which is important because the server runs on another thread).
*/
namespace Bonsai {
	class OSCServer 
		: public osc::OscPacketListener, public Thread
	{
	public:

		/** Constructor */
		OSCServer(int port, String address);

		/** Destructor*/
		~OSCServer();

		/** Run thread */
		void run();

		/** Stop listening */
		void stop();

		/** Check if server was bound successfully */
		bool isBound();

		/** Thread-safe copy the data from the buffer and reset the buffer to empty */
		void FlushBuffer(std::vector<std::array<int16_t, 4>>& dest);


	protected:
		/** OscPacketListener method*/
		virtual void ProcessMessage(const osc::ReceivedMessage& m, const IpEndpointName&);

	private:

		int port;
		String address;

		std::unique_ptr<UdpListeningReceiveSocket> socket;

		std::vector<std::array<int16_t, 4>> buffer;
		std::mutex bufferMutex;

		//JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Bonsai);
	};

}
#endif