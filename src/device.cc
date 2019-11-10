//
// Created by jakub on 25.10.2019.
//

#include <optional>
#include <ftdi.hpp>
#include <device.hh>

#define USB_VID		0x0403
#define USB_PID		0x6011

std::vector<Device>
DeviceEnumerator::enumerate()
{
	Ftdi::Context ctx;
	Ftdi::List *devices = Ftdi::List::find_all(ctx, USB_VID, USB_PID);
	std::vector<Device> result;

	for (auto &i: *devices) {
		result.push_back({
			USB_VID,
			USB_PID,
			i.serial(),
			i.description()
		});
	}

	delete devices;
	return (result);
}


std::optional<Device>
DeviceEnumerator::find_by_serial(const std::string &serial)
{
	for (const auto &i: enumerate()) {
		if (i.serial == serial)
			return (i);
	}

	return std::nullopt;
}
