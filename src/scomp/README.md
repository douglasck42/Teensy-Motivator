Frame format: [0xAB][type:1][len:1][payload:N][crc8:1][0xCD] — CRC-8 (Dallas/Maxim) covers type+len+payload. Corrupt or partial frames are silently dropped and the parser resets.

Messages implemented:

Direction	ID	Struct	Payload
T→E	0x01	ScompHeartbeat	uptime, SBUS flags (6 B)
T→E	0x02	ScompInputChannels	24 SBUS values + enabled mask (51 B)
T→E	0x03	ScompOutputChannels	48 µs values + enabled mask (102 B)
T→E	0x04	ScompAudioState	volume, playing, file# (4 B)
T→E	0x05	ScompLedTrigger	animation ID + RGB + brightness (5 B)
E→T	0x81	(none)	request state dump
E→T	0x82	ScompSetVolume	volume (1 B)
E→T	0x83	ScompTriggerAudio	file number (2 B)
E→T	0x84	ScompSetSetting	key/index/value (6 B)
Integration in main.cpp: scomp.update() each loop, channel state pushed at 20 Hz (SCOMP_SEND_INTERVAL_MS=50, overridable via build flag), heartbeat piggybacks on the existing 60s timer.

On the ESP32 side: copy the three scomp/ files over, call scomp.begin(Serial2) (or whichever UART connects to Teensy pin 20/21), register onMessage(), and call update() from your loop. The only ESP32-side difference will be which serial port you pass in.
