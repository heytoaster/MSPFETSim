## Overview

MSPFETSim lets you use generic hardware (like FTDI or STM32 chips) to debug MSP430 targets, replacing proprietary debug probes like the [TI MSP-FET](https://www.ti.com/tool/MSP-FET). With MSPFETSim, you can:

- Use generic probes (like a [FTDI cable](https://ftdichip.com/products/c232hm-ddhsl-0-2/)) to debug MSP430 chips

- Debug a MSP430 in-system without external hardware, as long as the system has a means to toggle the MSP430 `TEST` and `RST` pins

MSPFETSim simulates the TI eZ-FET firmware and appears to the host system as a real eZ-FET USB debug probe. MSPFETSim captures the Spy-Bi-Wire signals generated by the simulated firmware and delivers them to whatever hardware is wired to the MSP430 `TEST` and `RST` pins.

MSPFETSim supports any MSP430 chip that the real eZ-FET probe supports, and works with existing MSP430 tools such as [TI Code Composer Studio](https://www.ti.com/tool/CCSTUDIO) and [mspdebug](https://github.com/dlbeer/mspdebug). MSPFETSim only supports 2-wire Spy-Bi-Wire, not 4-wire JTAG.

MSPFETSim currently supports Linux.


## Motivation

This project was motivated by the need to flash/debug a MSP430 in-system through a STM32, where the STM32 is connected to a host computer via USB. With MSPFETSim, flashing/debugging the MSP430 was accomplished by simply: (1) creating a new MSPFETSim driver to send GPIO toggling commands to the STM32, and (2) implementing GPIO-toggling commands on the STM32.


## Supported Debug Probe Hardware

MSPFETSim has these hardware drivers:

- FTDI driver
    - Supports MPSSE FTDI chips, such as:
        - FT232H
        - FT2232H
        - FT4232H
    
    - Supports the [FTDI C232HM cable](https://ftdichip.com/products/c232hm-ddhsl-0-2/)
        - TCK (orange) <-> MSP TEST
        - TDO (green) <-> MSP RST


## Supported MSP430 Devices

In theory MSPFETSim should support any MSP430 supported by the real eZ-FET hardware, but in practice there are surely bugs and unimplemented functionality.

Flashing and general debugging (with both TI CCS and mspdebug) has been verified working with:

- MSP430FR2422
- MSP430FR2433
- MSP430G2452
- MSP430G2553
- MSP430I2041


## Usage

### Host Requirements

MSPFETSim supports Linux and requires these packages:

    sudo apt install libudev-dev
    sudo apt install libusb-1.0-0-dev
    sudo apt install libftdi1-dev

MSPFETSim was developed and tested on Ubuntu 20.04.1.

### Clone Repository
    git clone --recurse-submodules git@github.com:heytoaster/MSPFETSim.git

### Disable ModemManager Probing

Because the eZ-FET appears to the host as a modem, the ModemManager daemon may attempt to probe the device. For correct operation, a rule needs to be created to disable this behavior:

    echo 'ATTRS{idVendor}=="2047", ATTRS{idProduct}=="0014", ENV{ID_MM_DEVICE_IGNORE}="1"' | sudo tee /etc/udev/rules.d/42-mspfetsim.rules > /dev/null
    sudo udevadm control -R

TI Code Composer Studio will also do this, so it may not be necessary if you've installed TI CCS.

### Build

    cd MSPFETSim
    make

### Run

Load the virtual host controller kernel module (necessary to create virtual USB devices):

    sudo modprobe vhci-hcd

Make sure your FTDI-based debug probe is plugged in, and finally run MSPFETSim:

    sudo ./MSPFETSim

At this point `lsusb` should list a eZ-FET device (`Texas Instruments MSP Tools Driver`), and your MSP430 debug tools should see a eZ-FET attached to the system.


## Writing New Drivers

Adding support for new debug probe hardware requires implementing a minimal driver interface. This driver interface, declared in `MSPDebugDriver.h`, consists of 5 functions with semantics that amount to toggling the MSP430's `TEST` and `RST` pins:

- `sbwTestSet()`/`sbwRstSet()`: Set the output value of a pin
- `sbwTestPulse()`: Pulse TEST=[0,1]
- `sbwIO()`: Perform a Spy-Bi-Wire IO cycle
- `sbwRead()`: Retrieve data previously stored via `sbwIO()`


## Tips

- If you're using MSPFETSim from a virtual machine like VirtualBox, USB performance can be improved by configuring the VM to use a USB 3.0 (xHCI) controller.


## Caveats

- MSPFETSim is highly dependent on the version of the MSP Debug Stack/`libmsp430.so` that's being used to talk to it.
    - MSPFETSim is currently known to be compatible with:
        - MSP Debug Stack/`libmsp430.so` version 3.15.1.001 (as noted in MSPDS's `revisions.txt`)
        - Code Composer Studio version 10.4.0.00006 (as noted in Help > About Code Composer Studio)
    
    - If your debug software (such as TI CCS or mspdebug) says that it needs to update the eZ-FET's firmware, it's probably using an incompatible version of `libmsp430.so`. Try using the versions specified above.

- Programming flash-based (ie non-FRAM devices) devices correctly requires strobing the `TEST` signal within a certain frequency range. Although the MSPFETSim FTDI driver sets its clock frequency to be in this range, and flashing with MSPFETSim has been tested on the listed hardware (see Supported MSP430 Devices section), adherence to this requirement hasn't been thoroughly investigated.


## Development Notes

As TI adds support for new MSP430 devices, MSPFETSim will need to be updated to reflect the changes made to the eZ-FET firmware. To minimize this maintenance burden, as much of the eZ-FET firmware has been copy-pasted as possible, so that future changes to the eZ-FET firmware can be easily ported to MSPFETSim. This repository also stores firmware separately in Src/Firmware to maintain a clean divide between eZ-FET firmware and the rest of MSPFETSim.
