// EZFET: OK

/* Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 *
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Types.h"

#define eZ_FET
#define TOOL_ID eZ_FET_NO_DCDC

// Make sure the C basic types aren't used.
// They must be replaced by explicit-width types (eg uint16_t) to ensure
// execution that's consistent with the embedded device.
#define char    BADTYPE
#define short   BADTYPE
#define int     BADTYPE
#define long    BADTYPE

// static variables/functions aren't allowed since the entirety of firmware
// exists within the MSPFETSim class, and class instances shouldn't share
// variables, while functions should only affect their respective instances.
// Globals should just be member variables, and static functions should just
// be normal functions.
#define static  BADSTATIC

#define extern  BADEXTERN

#include "USB.h"
#include "Descriptor.h"

void dummy_Scheduler(void){};
void dummy_UsbLoop(void){};

int16_t debuggerOff = 0;

struct LOOP_INFOS
{
    void (MSPFETSim::*Scheduler)(void);
    void (MSPFETSim::*UsbLoop)(void);
};
typedef struct LOOP_INFOS LOOP_INFOS_t;

LOOP_INFOS_t loopInfos_ =
{
    MEMBER_FN_PTR(V3OP_Scheduler),
    MEMBER_FN_PTR(dummy_UsbLoop),
};

void Main(void)
{
    // MSPFETSIM: no watchdog
//    WDTCTL = WDTPW + WDTHOLD;       // Stop watchdog timer
//
//    if(!V3OP_SystemOk())
//    {
//        WDTCTL = 0;
//    }

    // MSPFETSIM: these functions would end up being empty after
    // removing hardware-specific stuff
//    init_BiosPorts();
//    init_Clock();

    BIOS_LedOff(BIOS_LED_MODE);
    BIOS_LedOn(BIOS_LED_POWER);

    // MSPFETSIM: we only implement the USBCDC_ USB functions
//    // Initialize USB and enable various events
//    USB_init();
//    WORD USBEVIE = kUSB_VbusOnEvent+kUSB_VbusOffEvent+kUSB_dataReceivedEvent+kUSB_UsbSuspendEvent+kUSB_UsbResumeEvent+kUSB_UsbResetEvent;
//    USB_setEnabledEvents(USBEVIE);
//    // See if we're already attached physically to USB, and if so, connect to it
//    // Normally applications don't invoke the event handlers, but this is an exception.
//    if (USB_connectionInfo() & kUSB_vbusPresent)
//    {
//        if (USB_enable() == kUSB_succeed)
//        {
//            USB_reset();
//            USB_connect();
//        }
//        else
//        {
//            BIOS_LedOff(BIOS_LED_POWER);
//            BIOS_LedOn(BIOS_LED_MODE);
//            while(1);
//        }
//    }
//    else
//    {
//        BIOS_LedOff(BIOS_LED_POWER);
//        BIOS_LedOn(BIOS_LED_MODE);
//        while(1);
//    }

    // init Bios
    BIOS_InitSystem();
    BIOS_InitCom();

    V3OP_ComInterfaceClear();
    V3OP_DcdcInterfaceClear();

    // int Sw Layers
    V3OP_DcdcInterfaceInit();
    V3OP_HalInterfaceInit();
    V3OP_ComInterfaceInit();

    IccMonitor_StartVoltageSupervision();
    _EINT_FET();

    IccMonitor_TriggerAdc();

    while(1)
    {
        CALL_MEMBER_FN_PTR(loopInfos_.UsbLoop)();
        CALL_MEMBER_FN_PTR(loopInfos_.Scheduler)();
    }
}

#undef extern
#undef static
#undef char
#undef short
#undef int
#undef long
