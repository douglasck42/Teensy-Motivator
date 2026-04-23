#pragma once

#define SBUS_DRIVER_BOLDERFLIGHT   1
#define SBUS_DRIVER_NATIVE16       2
#define SBUS_DRIVER_NATIVE24       3
#ifndef SBUS_SERIAL
    #define SBUS_SERIAL Serial8  // Hardware v1.0
#endif
#ifndef SBUS_DRIVER
    #define SBUS_DRIVER SBUS_DRIVER_NATIVE24  // default if not set
#endif

#if SBUS_DRIVER == SBUS_DRIVER_BOLDERFLIGHT
    #include "sbus_bolderflight.h"
    using SbusDriverType = SbusHandler;

#elif SBUS_DRIVER == SBUS_DRIVER_NATIVE16
    #include "sbus16_native.h"
    using SbusDriverType = Sbus16Native;
#elif SBUS_DRIVER == SBUS_DRIVER_NATIVE24
    #include "sbus24_native.h"
    using SbusDriverType = Sbus24Native;

#else
    #error "Unknown SBUS_DRIVER value. Check platformio.ini build_flags."
#endif