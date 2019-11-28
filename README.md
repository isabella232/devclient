# Conclusive Developer Cable Client

## Building and running on Debian and Ubuntu

1. Install the needed prerequisites:

```
sudo apt-get install build-essential cmake libgtkmm-3.0-dev libftdipp1-dev
```

2. Build:

```
$ cd devclient
$ mkdir build
$ cd build
$ cmake ..
$ make
$ make package
```

3. Install and run:

```
sudo dpkg -i devclient-0.1.1-Linux.deb
sudo /opt/conclusive/devclient/bin/devclient
```

## Building and running on macOS

1. You'll need `homebrew` to get all of the required dependencies. Run: `brew install cmake libtool automake libusb libusb-compat hidapi libftdi gtkmm3 telnet`

2. Build as for Linux.

3. If you encounter an error like this: `Failed to open device: unable to claim usb device. Make sure the default FTDI driver is not in use`, run `sudo kextunload -b com.FTDI.driver.FTDIUSBSerialDriver`

## Usage

### Device selection

### Serial console tab

### JTAG tab

### EEPROM tab
