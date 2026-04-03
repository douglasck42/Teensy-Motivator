#include <Arduino.h>
#include "sbus/sbus_config.h"
//#include "can_startup_scanner.h"
//#include "vesc_can_detector.h"

// Hand off the serial port here - Serial1 = pins 0/1 on Teensy 4.1
SbusDriverType sbusHandler(&SBUS_SERIAL);

// ========================= CONFIG =========================
#define HEARTBEAT_INTERVAL 10000  // 10 seconds
#define SBUS_DEADBAND 5           // minimum change to report

// 0 = auto-detect, 16 = SBUS-16, 24 = SBUS-24
uint8_t SBUS_CHANNELS = 0;

static uint32_t lastRead = 0;
static uint32_t lastPrintAll = 0;
const uint32_t SBUS_INTERVAL_MS = 15; // Slightly over 14ms SBUS frame rate

// --- State ---
int16_t lastChannels[24] = {0};

// ========================= GLOBALS =========================
unsigned long lastHeartbeat = 0;

// Wait for USB Serial connection at startup?
const bool WAIT_FOR_SERIAL = true;


// Serial-channel debbugging
bool serialDebugEnabled = true;
const int16_t DEADBAND = 10;
const uint32_t PRINT_ALL_INTERVAL_MS = 30000;

// ========================= SETUP =========================
void setup() {
    Serial.begin(115200);
    if (WAIT_FOR_SERIAL) {
        while (!Serial) {
            delay(100);
        }
    }

#if SBUS_DRIVER == SBUS_DRIVER_BOLDERFLIGHT
    Serial.println("Driver: Bolderflight SBUS");
#elif SBUS_DRIVER == SBUS_DRIVER_NATIVE16
    Serial.println("Driver: Native SBUS-16");
#endif

    Serial.println("Locking SBUS-24");
    SBUS_CHANNELS = 24;

    Serial.println("SBUS-" + String(SBUS_CHANNELS) + " Raw Reader Started");

    sbusHandler.begin();
    Serial.println("SBUS handler ready!");

    // run CAN scanner at startup (will no-op if CAN_STARTUP_SCANNER == 0)
    //CanStartupScanner::runIfEnabled();

    //vesc_setup();  // Initialize VESC CAN detector

    //sbusHandler.analyze();
    //sbusHandler.analyze_timegap();
    //sbusHandler.analyze_startstop();
}

// ========================= FUNCTIONS =========================

void printChannel(uint8_t index, int16_t value) {
    Serial.printf("CH%d: %d\n", index + 1, value);
}

void printAllChannels() {
    Serial.print("--- ");
    for (uint8_t i = 0; i < SBUS_CHANNELS; i++) {
        Serial.printf("CH%d:%d ", i + 1, sbusHandler.channel(i));
    }
    Serial.println("---");
}

void handleSerialOutput() {
    if (!serialDebugEnabled) return;

    for (uint8_t i = 0; i < SBUS_CHANNELS; i++) {
        int16_t current = sbusHandler.channel(i);
        if (abs(current - lastChannels[i]) > DEADBAND) {
            printChannel(i, current);
            lastChannels[i] = current;
        }
    }
}

// ========================= LOOP =========================
void loop() {
    
    // Heartbeat
    unsigned long now = millis();
    if (now - lastHeartbeat >= HEARTBEAT_INTERVAL) {
        Serial.println("Heartbeat: Teensy alive");
        lastHeartbeat = now;
    }
    if (now - lastPrintAll >= PRINT_ALL_INTERVAL_MS) {
        printAllChannels();
        lastPrintAll = now;
    }

    if (millis() - lastRead >= SBUS_INTERVAL_MS) {
        lastRead = millis();
        if (sbusHandler.read()) {
            handleSerialOutput();
            if (sbusHandler.failsafe())  Serial.println("** FAILSAFE **");
            if (sbusHandler.lostFrame()) Serial.println("** LOST FRAME **");
        }
    }

    // VESC CAN detection and monitoring
    //vesc_update();  // Process incoming CAN messages
    //
    //static uint32_t last_ping = 0;
    //if (millis() - last_ping > 1000) {
    //    vesc_sendPing(1);  // Ping motor ID 1 every 1 second
    //    last_ping = millis();
    //}
    //
    //static uint32_t last_print = 0;
    //if (millis() - last_print > 2000) {
    //    vesc_printStatus();
    //    last_print = millis();
    //}

}