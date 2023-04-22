#include <Arduino.h>
#include <Preferences.h>

#include "WiFiManager.h"
#include "blockClockClient.h"
#include "blockClockTypes.h"
#include "blockClockUtils.h"
#include "powerManager.h"
#include "prefsManager.h"
#include "screen.h"
#include "timeManager.h"
#include "userBoardDefines.h"

ApiClient apiClient("3602a548384fe25c");
ScreenState stateInScreen;
String blockHeightGlobal;
PriceData priceGlobal;
RecommendedFees recommendedFeesGlobal;

uint8_t globalMinute = 61;
int globalBatteryLevel = -1;
unsigned long lastMinuteCheck = 60001;

void setup() {
  initScreen();
  setCpuMaxPowerSave();

  drawStringPush("BLOCKCLOCK", 10, 10, 2);

  initWiFi();

  drawStringPush("Configuring clock", 10, 70, 1);
  timeManagerbegin();

  drawStringPush("Getting current block height", 10, 80, 1);
  firstTimeInit();
  initButtons();
}

void initButtons() {
  pinMode(BUTTON1PIN, INPUT_PULLUP);
  pinMode(BUTTON2PIN, INPUT_PULLUP);
}

void loop() {
  if (digitalRead(BUTTON1PIN) == 0) {
    buttonActionScreen();
  }

  if (digitalRead(BUTTON2PIN) == 0) {
    buttonBPressed();
  }

  if (isIntervalElapsed()) {
    updateScreen();
  }

  if (currentScreenState == DATEANDTIME) {
    callDateTimeScreen();
  }

  delay(200);
}

void buttonBPressed() {
  changeScreenState();
  updateScreen();
}

void buttonActionScreen() {
  switch (stateInScreen) {
    case PRICE:
      priceGlobal.price = "";
      changeCurrencyState();
      callPriceScreen();
      break;
    case BTC_CHANGE:
      priceGlobal.price = "";
      changeCurrencyState();
      callChangeScreen();
      break;
    case WIFIDATA:
      callWiFiDataScreen(true);
    default:
      break;
  }
}

void updateScreen() {
  switch (currentScreenState) {
    case BLOCKHEIGHT:
      callBlockHeightScreen();
      break;
    case RECOMMENDED_FEES:
      callTransactionFeesScreen();
      break;
    case PRICE:
      callPriceScreen();
      break;
    case BTC_CHANGE:
      callChangeScreen();
      break;
    case WIFIDATA:
      callWiFiDataScreen(false);
      break;
    default:
      break;
  }

  if (isIntervalElapsed()) {
    lastMinuteCheck = millis();
  }
}

void firstTimeInit() {
  if (isWiFiConnected()) {
    blockHeightGlobal = apiClient.getBlockHeight();
    clearScreen();
    drawBlockHeightScreen(blockHeightGlobal);
  }
}

void callBlockHeightScreen() {
  String blockheight;
  if (isWiFiConnected()) {
    if (isIntervalElapsed()) {
      blockheight = apiClient.getBlockHeight();
    } else {
      blockheight = blockHeightGlobal;
    }
    if (blockheight != blockHeightGlobal || stateInScreen != BLOCKHEIGHT) {
      stateInScreen = BLOCKHEIGHT;
      blockHeightGlobal = blockheight;
      drawBlockHeightScreen(blockHeightGlobal);
    }
  }
}

void callTransactionFeesScreen() {
  RecommendedFees recommendedFees;

  if (isWiFiConnected()) {
    if (isIntervalElapsed() || recommendedFeesGlobal.high == 0) {
      recommendedFees = apiClient.getRecommendedFees();
    } else {
      recommendedFees = recommendedFeesGlobal;
    }
    if (recommendedFees.high != recommendedFeesGlobal.high ||
        stateInScreen != RECOMMENDED_FEES) {
      stateInScreen = RECOMMENDED_FEES;
      recommendedFeesGlobal = recommendedFees;
      clearScreenExceptBattery();
      drawRecommendedFeesScreen(recommendedFees);
    }
  }
}

void callPriceScreen() {
  if (isWiFiConnected()) {
    stateInScreen = PRICE;

    PriceData pricePrefs = getBitcoinDataInPrefs(currentCurrencyState);

    if (pricePrefs.price != "" && pricePrefs.error == false) {
      time_t timestampFromRTC = getTimestampFromRTC();
      int64_t difference = timestampFromRTC - pricePrefs.timestamp;

      if (difference >= 300) {
        priceGlobal = apiClient.getBitcoinPrice(currentCurrencyState);

        if (priceGlobal.price != "") {
          saveBitcoinDataInPrefs(priceGlobal);
        }
      } else {
        priceGlobal.price = pricePrefs.price;
        priceGlobal.change1h = pricePrefs.change1h;
        priceGlobal.change24h = pricePrefs.change24h;
        priceGlobal.change30d = pricePrefs.change30d;
        priceGlobal.change7d = pricePrefs.change7d;
        priceGlobal.currency = pricePrefs.currency;
      }

      lastMinuteCheck = millis();
    }

    if (isIntervalElapsed() || priceGlobal.price == "" ||
        priceGlobal.error == true) {
      priceGlobal = apiClient.getBitcoinPrice(currentCurrencyState);
      saveBitcoinDataInPrefs(priceGlobal);
    }

    clearScreenExceptBattery();
    drawnPriceScreen(priceGlobal);
  }
}

void callChangeScreen() {
  if (isWiFiConnected()) {
    stateInScreen = BTC_CHANGE;

    PriceData pricePrefs = getBitcoinDataInPrefs(currentCurrencyState);

    if (pricePrefs.price != "" && pricePrefs.error == false) {
      time_t timestampFromRTC = getTimestampFromRTC();
      int64_t difference = timestampFromRTC - pricePrefs.timestamp;

      if (difference >= 300) {
        priceGlobal = apiClient.getBitcoinPrice(currentCurrencyState);
        if (priceGlobal.price != "") {
          saveBitcoinDataInPrefs(priceGlobal);
        }
      } else {
        priceGlobal = pricePrefs;
      }

      lastMinuteCheck = millis();
    }

    if (isIntervalElapsed() || priceGlobal.price == "" ||
        priceGlobal.error == true) {
      priceGlobal = apiClient.getBitcoinPrice(currentCurrencyState);
      saveBitcoinDataInPrefs(priceGlobal);
    }

    clearScreenExceptBattery();
    drawnChangeScreen(priceGlobal);
  }
}

void callDateTimeScreen() {
  BlockClockDateAndTime currentDateAndTime = getDateAndTime();

  String minutes = String(currentDateAndTime.minutes);
  String hours = String(currentDateAndTime.hour);

  String day;
  String month;

  if (currentDateAndTime.hour < 10) {
    hours = "0" + String(currentDateAndTime.hour);
  }

  if (currentDateAndTime.minutes < 10) {
    minutes = "0" + String(currentDateAndTime.minutes);
  }

  if (currentDateAndTime.day < 10) {
    day = "0" + String(currentDateAndTime.day);
  } else {
    day = String(currentDateAndTime.day);
  }

  if (currentDateAndTime.month < 10) {
    month = "0" + String(currentDateAndTime.month);
  } else {
    month = String(currentDateAndTime.month);
  }

  String ddmmyyyy = day + "/" + month + "/" + String(currentDateAndTime.year);

  if (stateInScreen == DATEANDTIME) {
    if (currentDateAndTime.minutes != globalMinute) {
      drawnDateAndTimeScreen(hours, minutes, ddmmyyyy);
      globalMinute = currentDateAndTime.minutes;
    }

  } else {
    stateInScreen = DATEANDTIME;
    drawnDateAndTimeScreen(hours, minutes, ddmmyyyy);
    globalMinute = currentDateAndTime.minutes;
  }
}

void callWiFiDataScreen(bool forceRender) {
  if (stateInScreen != WIFIDATA || forceRender == true) {
    stateInScreen = WIFIDATA;
    WiFiData wifiData = getWiFiData();

    drawnWiFiDataScreen(wifiData);
  }
}

bool isIntervalElapsed() {
  const unsigned long BLOCK_HEIGHT_CHECK_INTERVAL = 60000;
  const unsigned long PRICE_CHANGE_CHECK_INTERVAL = 300000;
  unsigned long checkInterval;

  if (currentScreenState == BLOCKHEIGHT) {
    checkInterval = BLOCK_HEIGHT_CHECK_INTERVAL;
  } else {
    checkInterval = PRICE_CHANGE_CHECK_INTERVAL;
  }

  unsigned long currentTime = millis();
  if (currentTime - lastMinuteCheck >= checkInterval) {
    return true;
  }

  return false;
}

/* void callPrintbattery() {
  int batteryLevel = getBatteryLevel();
  if (globalBatteryLevel == -1) {
    globalBatteryLevel = batteryLevel;
    clearBatteryScreen();
    printBattery(batteryLevel);
  }
  if (batteryLevel < globalBatteryLevel) {
    globalBatteryLevel = batteryLevel;
    clearBatteryScreen();
    printBattery(batteryLevel);
  }
} */