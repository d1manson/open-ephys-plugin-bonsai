
#include "OSCServer.h"

namespace Bonsai {
    constexpr size_t maxMessageNumValues = 8;

    OSCServer::OSCServer(int port_, String address_, DataBuffer* dataBuffer_, bool messageHasTimestamp_, int messageNumValues_) :
        port(port_), address(address_), dataBuffer(dataBuffer_), nSamples(0), messageHasTimestamp(messageHasTimestamp_), messageNumValues(messageNumValues_)
    {
        
        if (messageNumValues > maxMessageNumValues) {
            LOGE("OSCServer only designed to support up to ", maxMessageNumValues, " values per message.");
            return;
        }

        LOGC("Creating OSC server - Port:", port, " Address:", address);
        
        try{
            socket = std::make_unique<UdpListeningReceiveSocket>(IpEndpointName("localhost", port), this);
            CoreServices::sendStatusMessage("OSC Server ready!");
            LOGC("OSC Server started!");
        } catch (const std::exception& e) {
            CoreServices::sendStatusMessage("OSC Server failed to start!");
            LOGE("Exception in creating OSC Server: ", String(e.what()));
        }
    }

    OSCServer::~OSCServer()
    {
        stop();
    }

 
    void OSCServer::ProcessMessage(const osc::ReceivedMessage& receivedMessage,
        const IpEndpointName&)
    {
        
        try {

            if (!String(receivedMessage.AddressPattern()).equalsIgnoreCase(address)) {
                const MessageManagerLock mmLock(Thread::getCurrentThread());
                LOGC("Ignoring message with address: ", receivedMessage.AddressPattern());
                return;
            }

            if (receivedMessage.ArgumentCount() != 4) {
                const MessageManagerLock mmLock(Thread::getCurrentThread());
                LOGE("Expected exactly 4 args over OSC, but found: ", receivedMessage.ArgumentCount());
                return;
            }

            osc::ReceivedMessageArgumentStream args = receivedMessage.ArgumentStream();
            
            // fill timestamp
            double timestamps[maxMessageNumValues];
            double ts;
            if (messageHasTimestamp) {
                args >> ts;
            } else {
                // no idea if this is a good value to use
                ts = std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count();
            }
            std::fill(timestamps, timestamps + maxMessageNumValues, ts);

            // actual message values
            float vals[maxMessageNumValues];
            for (int i = 0; i < maxMessageNumValues; i++) {
                if (i < messageNumValues) {
                    args >> vals[i];
                }
            }
  
            // increment nSamples and repeat
            int64 sampleNums[maxMessageNumValues];
            std::fill(sampleNums, sampleNums + maxMessageNumValues, nSamples++); // not sure if smaple number should start at 0 or 1

            // eventCodes are always zeros
            static uint64 eventCodes[maxMessageNumValues] = {};

            dataBuffer->addToBuffer(vals, sampleNums, timestamps, eventCodes, messageNumValues, 1);
            
        } catch (osc::Exception& e) {
            // any parsing errors such as unexpected argument types, or
            // missing arguments get thrown as exceptions.
            const MessageManagerLock mmLock(Thread::getCurrentThread());
            LOGE("error while parsing message: ", String(receivedMessage.AddressPattern()), ": ", String(e.what()));
        }
    }

    void OSCServer::run()
    {
        // This whole method should be called from a separate thread to the main thread as this is blocking.
        {
            const MessageManagerLock mmLock(Thread::getCurrentThread());
            LOGC("OSCServer::run() called");
        }

        if (!socket) {
            const MessageManagerLock mmLock(Thread::getCurrentThread());
            LOGE("Socket not initialised.");
            return;
        }
        if (nSamples) {
            const MessageManagerLock mmLock(Thread::getCurrentThread());
            LOGE("nSamples should be 0 when run() is called, found:", nSamples);
            return;
        }
        socket->Run();
    }

    void OSCServer::stop()
    {
        if (socket) {
            // docs say to use this if not called from the same thread as .Run(), which is the case here.
            // what i'm not sure of though is if this call waits for run to stop or if it returns immediately.
            // if it doesn't wait then one imagines that the ProcessMessage() method might use memory after it 
            // was freed, which is bad (the dataBuffer specifically).
            socket->AsynchronousBreak(); 
        }
    }

}