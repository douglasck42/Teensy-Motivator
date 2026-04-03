#pragma once

#define SBUS_DRIVER_BOLDERFLIGHT   1
#define SBUS_DRIVER_NATIVE16       2
#define SBUS_DRIVER_NATIVE24       3

#if !defined(SBUS_DRIVER)
    #define SBUS_DRIVER SBUS_DRIVER_BOLDERFLIGHT  // default if not set
#endif
#if !defined(SBUS_SERIAL)
    #define SBUS_SERIAL Serial5  // default if not set in platformio.ini; Pin 21 on Teensy 4.1
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