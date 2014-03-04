#include <stdlib.h>
#include <canutil/read.h>
#include <pb_encode.h>
#include "can/canread.h"
#include "config.h"
#include "util/log.h"
#include "util/timer.h"

using openxc::util::log::debug;
using openxc::pipeline::MessageClass;
using openxc::pipeline::Pipeline;
using openxc::config::getConfiguration;

namespace time = openxc::util::time;
namespace pipeline = openxc::pipeline;

float openxc::can::read::preTranslate(CanSignal* signal, uint64_t data,
        bool* send) {
    float value = eightbyte_parse_float(data, signal->bitPosition,
            signal->bitSize, signal->factor, signal->offset);

    if(time::tick(&signal->frequencyClock) ||
            (value != signal->lastValue && signal->forceSendChanged)) {
        if(send && (!signal->received || signal->sendSame ||
                    value != signal->lastValue)) {
            signal->received = true;
        } else {
            *send = false;
        }
    } else {
        *send = false;
    }
    return value;
}

void openxc::can::read::postTranslate(CanSignal* signal, float value) {
    signal->lastValue = value;
}

float openxc::can::read::passthroughHandler(CanSignal* signal,
        CanSignal* signals, int signalCount, Pipeline* pipeline, float value,
        bool* send) {
    return value;
}

bool openxc::can::read::booleanHandler(CanSignal* signal, CanSignal* signals,
        int signalCount, Pipeline* pipeline, float value, bool* send) {
    return value == 0.0 ? false : true;
}

float openxc::can::read::ignoreHandler(CanSignal* signal, CanSignal* signals,
        int signalCount, Pipeline* pipeline, float value, bool* send) {
    *send = false;
    return value;
}

const char* openxc::can::read::stateHandler(CanSignal* signal,
        CanSignal* signals, int signalCount, Pipeline* pipeline, float value,
        bool* send) {
    const CanSignalState* signalState = lookupSignalState(value, signal,
            signals, signalCount);
    if(signalState != NULL) {
        return signalState->name;
    }
    *send = false;
    return NULL;
}

void openxc::can::read::sendNumericalMessage(const char* name, float value,
        Pipeline* pipeline) {
    openxc_VehicleMessage message = {0};
    message.has_type = true;
    message.type = openxc_VehicleMessage_Type_TRANSLATED;
    message.has_translated_message = true;
    message.translated_message = {0};
    message.translated_message.has_name = true;
    strcpy(message.translated_message.name, name);
    message.translated_message.has_type = true;
    message.translated_message.type = openxc_TranslatedMessage_Type_NUM;
    message.translated_message.has_numeric_value = true;
    message.translated_message.numeric_value = value;

    sendVehicleMessage(&message, pipeline);
}

void openxc::can::read::sendBooleanMessage(const char* name, bool value,
        Pipeline* pipeline) {
    openxc_VehicleMessage message = {0};
    message.has_type = true;
    message.type = openxc_VehicleMessage_Type_TRANSLATED;
    message.has_translated_message = true;
    message.translated_message = {0};
    message.translated_message.has_name = true;
    strcpy(message.translated_message.name, name);
    message.translated_message.has_type = true;
    message.translated_message.type = openxc_TranslatedMessage_Type_BOOL;
    message.translated_message.has_boolean_value = true;
    message.translated_message.boolean_value = value;

    sendVehicleMessage(&message, pipeline);
}

void openxc::can::read::sendStringMessage(const char* name, const char* value,
        Pipeline* pipeline) {
    openxc_VehicleMessage message = {0};
    message.has_type = true;
    message.type = openxc_VehicleMessage_Type_TRANSLATED;
    message.has_translated_message = true;
    message.translated_message = {0};
    message.translated_message.has_name = true;
    strcpy(message.translated_message.name, name);
    message.translated_message.has_type = true;
    message.translated_message.type = openxc_TranslatedMessage_Type_STRING;
    message.translated_message.has_string_value = true;
    strcpy(message.translated_message.string_value, value);

    sendVehicleMessage(&message, pipeline);
}

void openxc::can::read::sendEventedFloatMessage(const char* name,
        const char* value, float event,
        Pipeline* pipeline) {
    openxc_VehicleMessage message = {0};
    message.has_type = true;
    message.type = openxc_VehicleMessage_Type_TRANSLATED;
    message.has_translated_message = true;
    message.translated_message = {0};
    message.translated_message.has_name = true;
    strcpy(message.translated_message.name, name);
    message.translated_message.has_type = true;
    message.translated_message.type = openxc_TranslatedMessage_Type_EVENTED_NUM;
    message.translated_message.has_string_value = true;
    strcpy(message.translated_message.string_value, value);
    message.translated_message.has_numeric_event = true;
    message.translated_message.numeric_event = event;

    sendVehicleMessage(&message, pipeline);
}

void openxc::can::read::sendEventedBooleanMessage(const char* name,
        const char* value, bool event, Pipeline* pipeline) {
    openxc_VehicleMessage message = {0};
    message.has_type = true;
    message.type = openxc_VehicleMessage_Type_TRANSLATED;
    message.has_translated_message = true;
    message.translated_message = {0};
    message.translated_message.has_name = true;
    strcpy(message.translated_message.name, name);
    message.translated_message.has_type = true;
    message.translated_message.type =
            openxc_TranslatedMessage_Type_EVENTED_BOOL;
    message.translated_message.has_string_value = true;
    strcpy(message.translated_message.string_value, value);
    message.translated_message.has_boolean_event = true;
    message.translated_message.boolean_event = event;

    sendVehicleMessage(&message, pipeline);
}

void openxc::can::read::sendEventedStringMessage(const char* name,
        const char* value, const char* event, Pipeline* pipeline) {
    openxc_VehicleMessage message = {0};
    message.has_type = true;
    message.type = openxc_VehicleMessage_Type_TRANSLATED;
    message.has_translated_message = true;
    message.translated_message = {0};
    message.translated_message.has_name = true;
    strcpy(message.translated_message.name, name);
    message.translated_message.has_type = true;
    message.translated_message.type =
            openxc_TranslatedMessage_Type_EVENTED_STRING;
    message.translated_message.has_string_value = true;
    strcpy(message.translated_message.string_value, value);
    message.translated_message.has_string_event = true;
    strcpy(message.translated_message.string_event, event);

    sendVehicleMessage(&message, pipeline);
}

/* Public: Serialize the message to a bytestream (conforming to the OpenXC
 * standard and the currently selected payload format) and send it out to the
 * pipeline.
 *
 * This will accept both raw and translated typed messages.
 *
 * message - A message structure containing the type and data for the message.
 * pipeline - The pipeline to send on.
 */
void openxc::can::read::sendVehicleMessage(openxc_VehicleMessage* message,
        Pipeline* pipeline) {
    uint8_t payload[MAX_OUTGOING_PAYLOAD_SIZE] = {0};
    size_t length = payload::serialize(message, payload, sizeof(payload),
            getConfiguration()->payloadFormat);
    MessageClass messageClass;
    switch(message->type) {
        case openxc_VehicleMessage_Type_TRANSLATED:
            messageClass = MessageClass::TRANSLATED;
            break;
        case openxc_VehicleMessage_Type_RAW:
            messageClass = MessageClass::RAW;
            break;
        case openxc_VehicleMessage_Type_DIAGNOSTIC:
            messageClass = MessageClass::DIAGNOSTIC;
            break;
        default:
            debug("Trying to serialize unrecognized type: %d", message->type);
            break;
    }
    pipeline::sendMessage(pipeline, payload, length, messageClass);
}

void openxc::can::read::passthroughMessage(CanBus* bus, CanMessage* message,
        CanMessageDefinition* messages, int messageCount, Pipeline* pipeline) {
    bool send = true;
    CanMessageDefinition* messageDefinition = lookupMessageDefinition(bus,
            message->id, messages, messageCount);
    if(messageDefinition == NULL) {
        debug("Adding new message definition for message %d on bus %d",
                message->id, bus->address);
        send = registerMessageDefinition(bus, message->id, messages, messageCount);
    } else if(time::tick(&messageDefinition->frequencyClock) ||
            (message->data != messageDefinition->lastValue &&
                 messageDefinition->forceSendChanged)) {
        send = true;
    } else {
        send = false;
    }

    if(send) {
        openxc_VehicleMessage vehicleMessage = {0};
        vehicleMessage.has_type = true;
        vehicleMessage.type = openxc_VehicleMessage_Type_RAW;
        vehicleMessage.has_raw_message = true;
        vehicleMessage.raw_message = {0};
        vehicleMessage.raw_message.has_message_id = true;
        vehicleMessage.raw_message.message_id = message->id;
        vehicleMessage.raw_message.has_bus = true;
        vehicleMessage.raw_message.bus = bus->address;
        vehicleMessage.raw_message.has_data = true;
        vehicleMessage.raw_message.data.size = message->length;

        union {
            uint64_t whole;
            uint8_t bytes[8];
        } combined;
        combined.whole = message->data;
        memcpy(vehicleMessage.raw_message.data.bytes, combined.bytes,
                message->length);

        sendVehicleMessage(&vehicleMessage, pipeline);
    }

    if(messageDefinition != NULL) {
        messageDefinition->lastValue = message->data;
    }
}

void openxc::can::read::translateSignal(Pipeline* pipeline, CanSignal* signal,
        uint64_t data, NumericalHandler handler, CanSignal* signals,
        int signalCount) {
    bool send = true;
    float value = preTranslate(signal, data, &send);
    float processedValue = handler(signal, signals, signalCount, pipeline,
            value, &send);
    if(send) {
        sendNumericalMessage(signal->genericName, processedValue, pipeline);
    }
    postTranslate(signal, value);
}

void openxc::can::read::translateSignal(Pipeline* pipeline, CanSignal* signal,
        uint64_t data, StringHandler handler, CanSignal* signals,
        int signalCount) {
    bool send = true;
    float value = preTranslate(signal, data, &send);
    const char* stringValue = handler(signal, signals, signalCount, pipeline,
            value, &send);
    if(stringValue != NULL && send) {
        sendStringMessage(signal->genericName, stringValue, pipeline);
    }
    postTranslate(signal, value);
}

void openxc::can::read::translateSignal(Pipeline* pipeline, CanSignal* signal,
        uint64_t data, BooleanHandler handler, CanSignal* signals,
        int signalCount) {
    bool send = true;
    float value = preTranslate(signal, data, &send);
    bool booleanValue = handler(signal, signals, signalCount, pipeline, value,
            &send);
    if(send) {
        sendBooleanMessage(signal->genericName, booleanValue, pipeline);
    }
    postTranslate(signal, value);
}

void openxc::can::read::translateSignal(Pipeline* pipeline, CanSignal* signal,
        uint64_t data, CanSignal* signals, int signalCount) {
    translateSignal(pipeline, signal, data, passthroughHandler, signals,
            signalCount);
}
