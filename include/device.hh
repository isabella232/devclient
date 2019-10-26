//
// Created by jakub on 25.10.2019.
//

#ifndef DEVCLIENT_DEVICE_HH
#define DEVCLIENT_DEVICE_HH

#include <optional>
#include <vector>

struct Device
{
	uint16_t vid;
	uint16_t pid;
	std::string serial;
	std::string description;
};

class DeviceEnumerator
{
public:
	static std::vector<Device> enumerate();
	static std::optional<Device> find_by_serial(const std::string &serial);
};


#endif //DEVCLIENT_DEVICE_HH
