/*-
 * SPDX-License-Identifier: BSD-2-Clause-FreeBSD
 *
 * Copyright (c) 2019 Conclusive Engineering
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#include <iostream>
#include <memory>
#include <sysexits.h>
#include <getopt.h>
#include <fmt/format.h>
#include <string>

#include <log.hh>
#include <device.hh>
#include <uart.hh>
#include <i2c.hh>
#include <eeprom/24c.hh>
#include <gpio.hh>
#include <utils.hh>
#include <mainwindow.hh>
#include <gtkmm/application.h>

static const struct option long_options[] = {
    { "list", no_argument, nullptr, 'l' },
    { "device", optional_argument, nullptr, 'd' },
    { "serial", required_argument, nullptr, 's' },
    { "help", no_argument, nullptr, 'h' },
    { nullptr, 0, nullptr, 0}
};

static void
usage(const std::string &argv0)
{

}

int
main(int argc, char *const argv[])
{
	Glib::RefPtr<Gtk::Application> app;
	Device dev;
	std::unique_ptr<Uart> uart;
	std::string uart_listen_addr;
	std::string serial;
	uint8_t gpio_value;
	bool list = false;
	bool eeprom_read = false;
	bool gpio = false;
	int ch;

	for (;;) {
		ch = getopt_long(argc, argv, "ld:s:rg:h", long_options, nullptr);
		if (ch == -1)
			break;

		switch (ch) {
		case 'l':
			list = true;
			break;

		case 'd':
			serial = optarg;
			break;

		case 's':
			uart_listen_addr = optarg;
			break;

		case 'h':
			usage(argv[0]);
			return (EX_USAGE);

		case 'r':
			eeprom_read = true;
			break;

		case 'g':
			gpio = true;
			gpio_value = std::stoi(optarg, 0, 16);
			break;

		default:
			fmt::print(stderr, "Unrecognized option: {}", ch);
			usage(argv[0]);
			return (EX_USAGE);
		}
	}

	Gio::init();

	if (list) {
		fmt::print("Available devices:\n");
		for (const auto &i: DeviceEnumerator::enumerate()) {
			fmt::print("{:#04x}:{:#04x} - {} ({})\n", i.vid, i.pid,
			    i.description, i.serial);
		}
	}

	if (gpio) {
		dev = *DeviceEnumerator::find_by_serial(serial);
		Gpio gpio(dev);
		gpio.set(gpio_value);
		return (0);
	}

	if (eeprom_read) {
		dev = *DeviceEnumerator::find_by_serial(serial);
		I2C i2c(dev, 300000);
		Eeprom24c eeprom(i2c);
		std::vector<uint8_t> data;

		eeprom.read(0, 4096, data);
		hex_dump(data.data(), data.size(), std::cout, 16);

		return (0);
	}

	if (!uart_listen_addr.empty()) {
		auto saddr = Gio::InetSocketAddress::create(
		    Gio::InetAddress::create("127.0.0.1"), 1234);

		dev = *DeviceEnumerator::find_by_serial(serial);
		uart = std::unique_ptr<Uart>(new Uart(dev, saddr, 115200));
	}

	app = Gtk::Application::create("pl.conclusive.devclient");

	MainWindow window;
	app->run(window);
	return 0;
}
