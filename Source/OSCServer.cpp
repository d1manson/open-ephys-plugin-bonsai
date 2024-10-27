
#include "OSCServer.h"

namespace Bonsai {
    constexpr size_t maxMessageNumValues = 8;

    OSCServer::OSCServer(int port_, String address_, DataBuffer* dataBuffer_, bool messageHasTimestamp_, int messageNumValues_, float sampleRate_) :
        port(port_), address(address_), dataBuffer(dataBuffer_), nSamples(0), messageHasTimestamp(messageHasTimestamp_),
        messageNumValues(messageNumValues_), sampleRate(sampleRate_)
    {
        
        if (messageNumValues > maxMessageNumValues) {
            LOGE("OSCServer only designed to support up to ", maxMessageNumValues, " values per message.");
            return;
        }

        if(messageHasTimestamp){
            buffer.resize(10 * sampleRate + 1);
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

    void OSCServer::stepBuffer(){
        bufferWriteIdx = (bufferWriteIdx + 1) % buffer.size();
        buffer[bufferWriteIdx] = {};
    }

    void OSCServer::copyBuffer(std::vector<BonsaiSampleProblems>& buffer_, size_t& startIndex_){
        const ScopedLock sl(lock);
        buffer_.assign(buffer.begin(), buffer.end());
        startIndex_ = (bufferWriteIdx + 1) % buffer.size();
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

            if (receivedMessage.ArgumentCount() != (messageHasTimestamp ? 1 : 0) + messageNumValues) {
                const MessageManagerLock mmLock(Thread::getCurrentThread());
                LOGE("Expected exactly ", (messageHasTimestamp ? 1 : 0) + messageNumValues, " args over OSC, but found : ", receivedMessage.ArgumentCount());
                return;
            }

            osc::ReceivedMessageArgumentStream args = receivedMessage.ArgumentStream();

            uint64 eventCode = 0;
            double timestamp = 0; // see note in readme about timestamps

            float vals[maxMessageNumValues + 1] = {};
            if (messageHasTimestamp) {
                // see note in readme about timestamp hackiness
                double timestamp;
                args >> timestamp;
                if (nSamples == 0) {
                    firstTimestamp = timestamp;
                } else {
                    const ScopedLock sl(lock);

                    double error = (timestamp - firstTimestamp) * sampleRate - nSamples;

                    if (error < -0.5) {
                        buffer[bufferWriteIdx].dropped_super_early = 1;
                        return; // more than 50% too early, drop sample entirely
                    }
                    if (error > 1000){
                        LOGE("Bonsai data gap of ", error, " samples is way too bad, stopping acquisition. (timestamp on latest sample is ", timestamp, "; the very first timestamp was ", firstTimestamp, ").");
                        CoreServices::setAcquisitionStatus(false);
                        return;
                    }
                    if (error > 0.5){
                        // this sample is too late, fill the gap with 1 or more nan-timestamped / zero valued samples
                        timestamp = std::nan("");
                        size_t filled_samples = static_cast<int>(std::ceil(error));
                        std::vector<float> zeros(messageNumValues * filled_samples, 0.0f);
                        dataBuffer->addToBuffer(zeros.data(), &nSamples, &timestamp, &eventCode, filled_samples, 1);
                        while (error > 0.5) {
                            nSamples++;
                            buffer[bufferWriteIdx].filled_too_late = 1;
                            stepBuffer();
                            error -= 1;
                        }
                    }

                    buffer[bufferWriteIdx].used_value = 1;
                    buffer[bufferWriteIdx].error_is_negative = error < 0;
                    buffer[bufferWriteIdx].error_size = (error < -0.1 || error > 0.1) + (error < -0.25 || error > 0.25);
                    stepBuffer();
                }

                vals[0] = timestamp - firstTimestamp;

                for (int i = 0; i < maxMessageNumValues && i < messageNumValues; i++) {
                    args >> vals[i+1];
                }
            } else {
                for (int i = 0; i < maxMessageNumValues && i < messageNumValues; i++) {
                    args >> vals[i];
                }
            }
            
            dataBuffer->addToBuffer(vals, &nSamples, &timestamp, &eventCode, 1, 1);
            nSamples++;
            
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