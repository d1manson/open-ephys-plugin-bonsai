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
	An OSC UDP Server that expects messages with 4 float32 values. It packs the 4 the float32 dataBuffer.
	Note that DataBuffer is thread-safe, because it's managed via a JUCE AbstractFifo under the hood.
*/
namespace Bonsai {

#pragma pack(push, 1)
/*
  When timestamps are provided in samples, we check the timestamp on a given sample relative to what it should be
  if we'd been seeing perfect sampling frequency relative to the first timestamp seen. If a given sample is earlier by
  over 50% of the sampling interval then we drop the sample entirely. If a sample is more than 50% late, then we fill
  the gap with one or more samples with nan-timestamp and zeros for the other values. Note that for a single sample it's
  possible that both of these things happened, i.e. we dropped a prior sample that was super early but then had to fill
  the gap. If a sample does arrive within +-50% of the expected time we record the error.
*/
struct BonsaiSampleProblems {
    uint8_t dropped_super_early: 1;
    uint8_t filled_too_late: 1;
    uint8_t used_value: 1; // filled_too_late=1 and used_value=1 are mutually exclusive
    uint8_t error_is_negative: 1; // only set if used_value=1
    uint8_t error_size: 2;  // only set if used_value=1. within 10%: 0, within 25%: 1, within 50%: 2
};
#pragma pack(pop)


	class OSCServer
		: public osc::OscPacketListener
	{
	public:

		/** Constructor */
		OSCServer(int port, String address, DataBuffer* dataBuffer, bool messageHasTimestamp, int messageNumValues, float sampleRate_);

		/** Destructor*/
		~OSCServer();

		/** Blocking method, that should be called on its own thread. */
		void run();

		void stop();

		DataBuffer* dataBuffer;

		bool IsBound() { return socket && socket->IsBound(); };

        void copyBuffer(std::vector<BonsaiSampleProblems>& buffer_, size_t& startIndex_);

	protected:
		/** OscPacketListener method*/
		virtual void ProcessMessage(const osc::ReceivedMessage& m, const IpEndpointName&) override;

	private:
		const int port;
		const String address;
		std::unique_ptr<UdpListeningReceiveSocket> socket;
		const bool messageHasTimestamp;
		const size_t messageNumValues;
		double firstTimestamp; // see readme for info on timestamp hackiness

        const double sampleRate;

        /* track every sample for 10s in a circular buffer. it's sized dynamically as it depends on sample rate
           the BonsaiDataThreadPluginEditor can use copyBuffer() to get the buffer. */
        CriticalSection lock;
        std::vector<BonsaiSampleProblems> buffer;
        size_t bufferWriteIdx = 0;
        void stepBuffer();

        // TODO: should really track how long it's been since any data so we can show the gap building up

		int64 nSamples;
		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OSCServer);


	};

}
#endif