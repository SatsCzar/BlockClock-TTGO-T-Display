# Blockclock for TTGO T-Display

This is a versatile Bitcoin blockclock for the TTGO T-Display (ESP32) that connects to Wi-Fi and fetches data from mempool.space every minute. The updated version now supports multiple screens, displaying additional information such as price, transaction fees, date and time, and more.

## Installing

1. Download the latest version of Arduino IDE.
2. Follow this [tutorial](https://github.com/Xinyuan-LilyGO/TTGO-T-Display/blob/master/README.md) to install the required libraries:
   - TFT_eSPI
   - ArduinoJson (check the ArduinoJson [tutorial](https://arduinojson.org/v6/doc/installation/#option-1-use-the-arduino-library-manager) for more information)
3. Install the ESP32 Bitcoin BlockClock Library as described in its [README](https://github.com/SatsCzar/ESP32-Bitcoin-BlockClock-Library).
4. Clone the repository and open the blockclock.ino file with the Arduino IDE.
5. Make sure to edit the `userBoardDefines.h` file in the ESP32-Bitcoin-BlockClock-Library to define your device type. For example:
```arduino
#define GENERIC_ESP32 1
/* #define M5STACK 1 */
#ifdef GENERIC_ESP32
#define BUTTON1PIN 35
#define BUTTON2PIN 0
#endif
```
6. Select the COM port corresponding to your TTGO T-Display and the model.
7. Compile and upload the code using these commands:
```bash
# Command to compile
arduino-cli compile --fqbn esp32:esp32:esp32:JTAGAdapter=default,PSRAM=disabled,PartitionScheme=default,CPUFreq=240,FlashMode=qio,FlashFreq=80,FlashSize=4M,UploadSpeed=921600,LoopCore=1,EventsCore=1,DebugLevel=none,EraseFlash=none blockclock/blockclock.ino

# Command to upload
arduino-cli upload --fqbn esp32:esp32:esp32:JTAGAdapter=default,PSRAM=disabled,PartitionScheme=default,CPUFreq=240,FlashMode=qio,FlashFreq=80,FlashSize=4M,UploadSpeed=921600,LoopCore=1,EventsCore=1,DebugLevel=none,EraseFlash=none --port COM18 blockclock/blockclock.ino
```

## Screens

| #   | Screen          | Function                            |
| --- | --------------- | ----------------------------------- |
| 1   | BlockHeight     | Show current blockheight            |
| 2   | TransactionFees | Show recommended fees in Sats/Byte  |
| 3   | DateTime        | Show current date and time          |
| 4   | Price           | Show current price of BTC           |
| 5   | Change          | Show BTC change in 1h, 24h, 7d, 30d |
| 6   | WiFiData        | Show information about Wi-Fi        |

## Features

- Uses the TTGO T-Display's real-time clock to manage time and control on-screen information updates.
- Fetches current time and date from NTP server and sets this information in the real-time clock.
- Stores Bitcoin price and price changes in persistent storage, valid for 5 minutes, reducing API fetches.
- Supports multiple currencies for price display, including BRL and USD.
- Improved battery percentage display.
- Stores Wi-Fi credentials (SSID and password) persistently after successful smartconfig.

## Connecting to Wi-Fi

The code uses smartconfig from ESP32 to connect to Wi-Fi, so you need the app [ESP-TOUCH](https://www.espressif.com/en/products/software/esp-touch/resources), which can be found on the EspressIF website.

![SmartConfig](https://www.espressif.com/sites/default/files/faq/screen_shot_2016-04-27_at_1.30.27_pm_0.png)