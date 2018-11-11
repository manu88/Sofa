//
//  DriverKit_UnitTests.c
//  DevelSofaInit
//
//  Created by Manuel Deneu on 05/11/2018.
//  Copyright © 2018 Manuel Deneu. All rights reserved.
//

#include <assert.h>
#include "DriverKit_UnitTests.h"
#include "DriverKit.h"

typedef struct
{
    IOBaseDevice super;
    int value; // guard value to check overflows
    
} TestDriver;

static int DriverinitReturn = 0;
static int Driverinit(IOBaseDevice *device)
{
    TestDriver* self = (TestDriver*) device;
    assert(self);
    self->value = 10;
    return DriverinitReturn;
}

static int DriverDeInitDevice (IOBaseDevice *device)
{
    return 0;
}

static int DriverHandleIRQ (IOBaseDevice *device, int irqNum)
{
    return 0;
}

int DriverKit_UnitTests()
{
    DriverKitInit( NULL);
    
    
    TestDriver driver;
    driver.value = 100;
    assert( IOBaseDeviceInit(&driver.super));
    assert(driver.super.InitDevice == NULL);
    assert(driver.super.DeInitDevice == NULL);
    assert(driver.value == 100);
    
    // fails cause no callbacks
    assert(DriverKitRegisterDevice(1, (IOBaseDevice*) &driver) == 0);
    driver.super.InitDevice =  Driverinit;
    
    // fails cause no callbacks
    assert(DriverKitRegisterDevice(1, (IOBaseDevice*) &driver) == 0);
    
    driver.super.DeInitDevice = DriverDeInitDevice;
    driver.super.HandleIRQ = DriverHandleIRQ;
    
    // fails 1st time cause init returns 0
    assert(DriverKitRegisterDevice(1, (IOBaseDevice*) &driver) == 0);
    
    // now does not fail
    DriverinitReturn = 1;
    assert(DriverKitRegisterDevice(1, (IOBaseDevice*) &driver));
    assert(driver.value == 10);
    
    // MUST fail cause badge 0 is reserved
    assert(DriverKitRegisterDevice(0, (IOBaseDevice*) &driver) == 0);
    
    assert(DriverKitGetDeviceForBadge(1) == (IOBaseDevice*) &driver);
    assert(DriverKitGetDeviceForBadge(10000) == NULL);
    assert(DriverKitGetDeviceForBadge(0) == NULL);
    
    assert(DriverKitRemoveDevice( (IOBaseDevice*) &driver) );
    assert(DriverKitGetDeviceForBadge(1) == NULL);
    
    // unimplemented
    //assert(DriverKitRemoveDevice( (IOBaseDevice*) &driver ));
    
    return 1;
}
