# ESP_Firebase
Send data from a firebase to an STM32F7
This project used an Esp8266 microcontller, as well a STM32F7 and a firebase.
The code presented here was made in the Arduino IDE and is inteded to transmit blocks of data from the firebase to the Esp and in the end to the STM.
For the Firebase (Google's approach to data bases) is extremelly eazy to configure and there is only required for a very basic first setup to define the rules of access, something like this:
{
  "rules": {
    ".read": true,
    ".write": true
  }
}
There can be added extra layers of security depending on the necessities.
Then configure the data it self, example:

fire-esp-91105 (This first field is the name of the data base, and each tab is connected to the data above and only the above data)
  1
    FingerID: 1
    Songs: "1,2,3,4"
    UserName: "Jose Gomes"
  2
  3
  4
  Songs: "NEFFEX,Toxicity,Stitches,BeatIt"
  Users: "1,2,3,4"

Configuration needed for the ESP are mainly on the wifi connection, it will require the credentials and the name of the wifi to be connected. For testing porpurses and easier first deployments, I recommend the use of a hotspot with no password, looking like this:

#define WIFI_SSID "Valencia2Y100pro"  /*Name of the device to be connected to*/
#define WIFI_PASSWORD NULL            /*Password for connection*/

Dependencies:
 - ESP8266WiFi library:
https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WiFi

 - FirebaseArduino library:
https://github.com/FirebaseExtended/firebase-arduino
