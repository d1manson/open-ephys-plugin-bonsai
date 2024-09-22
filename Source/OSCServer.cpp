
#include "OSCServer.h"

namespace Bonsai {
    OSCServer::OSCServer(int port_, String address_, DataBuffer* dataBuffer_) :
        port(port_), address(address_), dataBuffer(dataBuffer_), nSamples(0)
    {
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
            
            float vals[4];
            args >> vals[0] >> vals[1] >> vals[2] >> vals[3];
            nSamples++;
            double timestamps[4] = { nSamples, nSamples, nSamples, nSamples }; // not especially helpful, but need something
            int64 sampleNums[4] = { nSamples * 4 + 0, nSamples * 4 + 1, nSamples * 4 + 2, nSamples * 4 + 3 };
            uint64 eventCodes[4] = { 0,0,0,0 };
            dataBuffer->addToBuffer(vals, sampleNums, timestamps, eventCodes, 4, 1);
            
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