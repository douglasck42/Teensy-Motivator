#include <Arduino.h>
#include "dfp.h"
#include "settings.h"

#if (defined(ARDUINO_AVR_UNO) || defined(ESP8266))
SoftwareSerial softSerial(DF_RX_PIN, DF_TX_PIN);  // definition
#endif

DFRobotDFPlayerMini myDFPlayer; 


void dfpSetup() {
  FPSerial.begin(9600);

  // Additional DFPlayer configuration can be added here if needed in the future
  // DFPlayer setup and communication check, 5 tries with 3-second delay between attempts
  Serial.println(F("DFPlayer: Checking communication with DFPlayer..."));
  Serial.println(F("DFPlayer: Initializing... (May take 3~5 seconds)"));
    
  if (!myDFPlayer.begin(DF_SERIAL, /*isACK = */true, /*doReset = */true)) {  //Use serial to communicate with mp3.
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    while(true);
  }
  Serial.println(F("DFPlayer: Online"));

  myDFPlayer.setTimeOut(500); //Set serial communictaion time out 500ms
  
  //----Set volume----
  // Volume value can be 0~30, you can use this range to do as you wish, if you use 0 it will be the same as mute, and 30 will be the loudest.
  // Volume 20 = Volume 512 as reported by readVolume()
  myDFPlayer.volume(0);  //Set volume value (0~30).
  //myDFPlayer.volumeUp(); //Volume Up
  //myDFPlayer.volumeDown(); //Volume Down
  
  //----Set different EQ----
  //myDFPlayer.EQ(DFPLAYER_EQ_POP);            // 15 = Pop EQ as reported by readEQ()
  myDFPlayer.EQ(DFPLAYER_EQ_NORMAL);      // 20 = Normal EQ as reported by readEQ()
  // myDFPlayer.EQ(DFPLAYER_EQ_ROCK);            // 0 as reported by readEQ() so maybe broken?
  //myDFPlayer.EQ(DFPLAYER_EQ_JAZZ);            //0 as reported by readEQ() so maybe broken?
   //myDFPlayer.EQ(DFPLAYER_EQ_CLASSIC);        //0 as reported by readEQ() so maybe broken?
  //myDFPlayer.EQ(DFPLAYER_EQ_BASS);         //0 as reported by readEQ() so maybe broken?

  //----Set device we use SD as default----
  //  myDFPlayer.outputDevice(DFPLAYER_DEVICE_U_DISK);
  myDFPlayer.outputDevice(DFPLAYER_DEVICE_SD);
  //  myDFPlayer.outputDevice(DFPLAYER_DEVICE_AUX);
  //  myDFPlayer.outputDevice(DFPLAYER_DEVICE_SLEEP);
  //  myDFPlayer.outputDevice(DFPLAYER_DEVICE_FLASH);

  //  myDFPlayer.enableDAC();  //Enable On-chip DAC
  //  myDFPlayer.disableDAC();  //Disable On-chip DAC
  //  myDFPlayer.outputSetting(true, 15); //output setting, enable the output and set the gain to 15
    //----Read imformation----
  Serial.println("DFPlayer: Status");
  Serial.println("  MP3 State: " + String(myDFPlayer.readState())); //read mp3 state
  Serial.println("  Volume: " + String(myDFPlayer.readVolume())); //read current volume
  Serial.println("  EQ Setting: " + String(myDFPlayer.readEQ())); //read EQ setting
  Serial.println("  File Counts: " + String(myDFPlayer.readFileCounts())); //read all file counts in SD card
  Serial.println("  Current File Number: " + String(myDFPlayer.readCurrentFileNumber())); //read current play file number
  //Serial.println("  File Counts in Folder 3: " + String(myDFPlayer.readFileCountsInFolder(3))); //read file counts in folder SD:/03

}

// ========================= FUNCTIONS =========================
void dfpPrintDetail(uint8_t type, int value){
  switch (type) {
    case TimeOut:
      Serial.println(F("DFPlayer: Time Out!"));
      break;
    case WrongStack:
      Serial.println(F("DFPlayer: Stack Wrong!"));
      break;
    case DFPlayerCardInserted:
      Serial.println(F("DFPlayer: Card Inserted!"));
      break;
    case DFPlayerCardRemoved:
      Serial.println(F("DFPlayer: Card Removed!"));
      break;
    case DFPlayerCardOnline:
      Serial.println(F("DFPlayer: Card Online!"));
      break;
    case DFPlayerUSBInserted:
      Serial.println(F("DFPlayer: USB Inserted!"));
      break;
    case DFPlayerUSBRemoved:
      Serial.println(F("DFPlayer: USB Removed!"));
      break;
    case DFPlayerPlayFinished:
      Serial.print(F("DFPlayer: Number:"));
      Serial.print(value);
      Serial.println(F("DFPlayer: Play Finished!"));
      break;
    case DFPlayerError:
      Serial.print(F("DFPlayer: Error:"));
      switch (value) {
        case Busy:
          Serial.println(F("DFPlayer: Card not found"));
          break;
        case Sleeping:
          Serial.println(F("DFPlayer: Sleeping"));
          break;
        case SerialWrongStack:
          Serial.println(F("DFPlayer: Get Wrong Stack"));
          break;
        case CheckSumNotMatch:
          Serial.println(F("DFPlayer: Check Sum Not Match"));
          break;
        case FileIndexOut:
          Serial.println(F("DFPlayer: File Index Out of Bound"));
          break;
        case FileMismatch:
          Serial.println(F("DFPlayer: Cannot Find File"));
          break;
        case Advertise:
          Serial.println(F("DFPlayer: In Advertise"));
          break;
        default:
          Serial.println(F("DFPlayer: Unknown Error"));
          break;
      }
      break;
    default:
      break;
  }
  
}

void dfpStop() {
  if (settings.audio.dfp_debug) {
    Serial.printf("DFPlayer: Stopping playback\n");
  }
  myDFPlayer.stop(); // Stop any currently playing audio to prevent overlapping audio files when changing pages or pressing buttons that are set to stop further processing of button inputs
}

void dfpPlay(int fileNumber) {
  if (settings.audio.dfp_debug) {
    Serial.printf("DFPlayer: Playing file number %d\n", fileNumber);
  }
  myDFPlayer.playMp3Folder(fileNumber); // Play the selected audio file for this button
}

void dfpVolume(int volume) {
  if (settings.audio.dfp_debug) {
    Serial.printf("DFPlayer: Setting volume to %d\n", volume);
  }
  myDFPlayer.volume(volume);
}