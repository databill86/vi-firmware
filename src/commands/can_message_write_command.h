#ifndef __CAN_MESSAGE_WRITE_COMMAND_H__
#define __CAN_MESSAGE_WRITE_COMMAND_H__

#include "commands/commands.h"

namespace openxc {
namespace commands {

bool handleRaw(openxc_VehicleMessage* message);

bool validateRaw(openxc_VehicleMessage* message);

} // namespace commands
} // namespace openxc

#endif // __CAN_MESSAGE_WRITE_COMMAND_H__