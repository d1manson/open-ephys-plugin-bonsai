
#include "OSCServer.h"

namespace Bonsai {
    OSCServer::OSCServer(int port_, String address_) : 
        Thread("OscListener Thread"), port(port_), address(address_)
    {
        LOGC("Creating OSC server - Port:", port, " Address:", address);

        try{
            socket = std::make_unique<UdpListeningReceiveSocket>(
                IpEndpointName(IpEndpointName::ANY_ADDRESS, port), this);
            CoreServices::sendStatusMessage("OSC Server ready!");
            LOGC("OSC Server started!");
        } catch (const std::exception& e) {
            CoreServices::sendStatusMessage("OSC Server failed to start!");
            LOGE("Exception in creating OSC Server: ", String(e.what()));
        }
    }

    OSCServer::~OSCServer()
    {
        // stop the OSC Listener thread running
        stop();
        stopThread(-1);
        waitForThreadToExit(-1);
    }

    void OSCServer::FlushBuffer(std::vector<std::array<int16_t, 4>>& dest) {
        const std::lock_guard<std::mutex> lock(bufferMutex);
        // TODO: copy the buffer data into dest, and clear the buffer
        
    }


    void OSCServer::ProcessMessage(const osc::ReceivedMessage& receivedMessage,
        const IpEndpointName&)
    {

        LOGD("Message received on ", receivedMessage.AddressPattern());

        try {

            if (!String(receivedMessage.AddressPattern()).equalsIgnoreCase(getOSCAddress())) {
                LOGC("Ignoring message with address: ", receivedMessage.AddressPattern());
                return;
            }

            if (receivedMessage.ArgumentCount() != 4) {
                LOGE("Expected exactly 4 args over OSC, but found: ", receivedMessage.ArgumentCount());
                return;
            }

            osc::ReceivedMessageArgumentStream args = receivedMessage.ArgumentStream();

            {
                const std::lock_guard<std::mutex> lock(bufferMutex);
                // TODO: add values into the buffer
                int v0, v1, v2, v3;
                args >> v0 >> v1 >> v1 >> v3;
            }
        } catch (osc::Exception& e) {
            // any parsing errors such as unexpected argument types, or
            // missing arguments get thrown as exceptions.
            LOGE("error while parsing message: ", String(receivedMessage.AddressPattern()), ": ", String(e.what()));
        }
    }

    void OSCServer::run()
    {
        LOGC("OSCServer::run() called"); // i'm presuming this is somehow called by a base class?

        // Start the oscpack OSC Listener Thread
        // TODO (FIX): Hits assertion in the JUCE::Thread class because listener's
        // 'Run()' method is throwing expection in some cases.
        if (socket) {
            socket->Run();
        }
    }

    bool OSCServer::isBound()
    {
        if (socket) {
            return socket->IsBound();
        } else {
            return false;
        }
    }

    void OSCServer::stop()
    {
        // Stop the oscpack OSC Listener Thread
        if (!isThreadRunning()){
            return;
        }

        if (socket){
            socket->AsynchronousBreak();
        }
    
     }


}