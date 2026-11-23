#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Preferences.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_ADDRESS 0x3C
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define SS_PIN 5
#define RST_PIN 4
MFRC522 rfid(SS_PIN, RST_PIN);

const int btn100Pin = 13;
const int btn10Pin = 12;
const int btn1Pin = 14;
const int btnModePin = 27;
const int btnOkPin = 26;
const int btnGoPin = 25;
const int btnResetPin = 33;

bool isDeductMode = false;
const long goReward = 200;

Preferences prefs;

struct Player {
  String uid;
  String name;
  long balance;
  bool isBankrupt;
};

Player players[4] = {
  {"", "Player 1", 1500, false},
  {"", "Player 2", 1500, false},
  {"", "Player 3", 1500, false},
  {"", "Player 4", 1500, false}
};

int activePlayerIndex = -1;
long pendingDelta = 0;
unsigned long lastDebounce = 0;
const unsigned long debounceDelay = 180;

void showIdleScreen();
void updateTransactionScreen();
void showPopup(const char* title, const char* body1, const char* body2, int delayMs);
void handleRFID();
void handleBankButtons();
void handlePassGo();
void handleGameReset();
void savePlayerData(int index);
void loadAllPlayerData();

void setup() {
  Serial.begin(115200);

  Wire.begin(21, 22);

  if (!oled.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    Serial.println("OLED init failed!");
  }

  SPI.begin();
  rfid.PCD_Init();

  pinMode(btn100Pin, INPUT_PULLUP);
  pinMode(btn10Pin, INPUT_PULLUP);
  pinMode(btn1Pin, INPUT_PULLUP);
  pinMode(btnModePin, INPUT_PULLUP);
  pinMode(btnOkPin, INPUT_PULLUP);
  pinMode(btnGoPin, INPUT_PULLUP);
  pinMode(btnResetPin, INPUT_PULLUP);

  loadAllPlayerData();

  showIdleScreen();
  Serial.println("Monopoly Banking Terminal Ready.");
}

void loop() {
  if (activePlayerIndex == -1) {
    if (millis() - lastDebounce >= debounceDelay) {
      if (digitalRead(btnGoPin) == LOW) {
        lastDebounce = millis();
        handlePassGo();
        return;
      }
      if (digitalRead(btnResetPin) == LOW) {
        lastDebounce = millis();
        handleGameReset();
        return;
      }
    }
    handleRFID();
  } else {
    handleBankButtons();
  }
}

void showIdleScreen() {
  oled.clearDisplay();
  
  oled.fillRect(0, 0, 128, 14, SSD1306_WHITE);
  oled.setTextColor(SSD1306_BLACK);
  oled.setTextSize(1);
  oled.setCursor(14, 3);
  oled.print("MONOPOLY BANKING");

  oled.setTextColor(SSD1306_WHITE);
  oled.drawRoundRect(6, 20, 116, 40, 4, SSD1306_WHITE);
  oled.setCursor(20, 28);
  oled.print("TAP RFID CARD");
  oled.setCursor(26, 42);
  oled.print("TO DEPOSIT/PAY");
  
  oled.display();
}

void updateTransactionScreen() {
  oled.clearDisplay();

  oled.fillRect(0, 0, 128, 14, SSD1306_WHITE);
  oled.setTextColor(SSD1306_BLACK);
  oled.setTextSize(1);
  oled.setCursor(4, 3);
  oled.print(players[activePlayerIndex].name);

  oled.setTextColor(SSD1306_WHITE);
  oled.setCursor(4, 18);
  oled.print("Bal: $");
  oled.setTextSize(2);
  oled.setCursor(44, 16);
  oled.print(players[activePlayerIndex].balance);

  oled.drawFastHLine(0, 34, 128, SSD1306_WHITE);

  oled.setTextSize(1);
  oled.setCursor(4, 38);
  oled.print(isDeductMode ? "[-] DEDUCT" : "[+] DEPOSIT");

  oled.setTextSize(2);
  oled.setCursor(4, 48);
  oled.print("$");
  oled.print(pendingDelta);

  oled.display();
}

void showPopup(const char* title, const char* body1, const char* body2, int delayMs) {
  oled.clearDisplay();
  oled.drawRect(0, 0, 128, 64, SSD1306_WHITE);
  oled.fillRect(0, 0, 128, 14, SSD1306_WHITE);
  
  oled.setTextColor(SSD1306_BLACK);
  oled.setTextSize(1);
  oled.setCursor(6, 3);
  oled.print(title);

  oled.setTextColor(SSD1306_WHITE);
  oled.setTextSize(1);
  oled.setCursor(10, 24);
  oled.print(body1);
  if (body2 != nullptr) {
    oled.setCursor(10, 42);
    oled.print(body2);
  }

  oled.display();
  delay(delayMs);
}

void savePlayerData(int index) {
  prefs.begin("bank", false);
  String uidKey = "uid" + String(index);
  String balKey = "bal" + String(index);
  String bkrKey = "bkr" + String(index);

  prefs.putString(uidKey.c_str(), players[index].uid);
  prefs.putLong(balKey.c_str(), players[index].balance);
  prefs.putBool(bkrKey.c_str(), players[index].isBankrupt);
  prefs.end();

  Serial.print("Saved to Flash: ");
  Serial.print(players[index].name);
  Serial.print(" Balance: $");
  Serial.println(players[index].balance);
}

void loadAllPlayerData() {
  prefs.begin("bank", true);
  for (int i = 0; i < 4; i++) {
    String uidKey = "uid" + String(i);
    String balKey = "bal" + String(i);
    String bkrKey = "bkr" + String(i);

    String savedUID = prefs.getString(uidKey.c_str(), "");
    long savedBalance = prefs.getLong(balKey.c_str(), 1500);
    bool savedBankrupt = prefs.getBool(bkrKey.c_str(), false);

    if (savedUID != "") {
      players[i].uid = savedUID;
    }
    players[i].balance = savedBalance;
    players[i].isBankrupt = savedBankrupt;
  }
  prefs.end();
}

void handleRFID() {
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    return;
  }

  String scannedUID = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    scannedUID += (rfid.uid.uidByte[i] < 0x10 ? "0" : "");
    scannedUID += String(rfid.uid.uidByte[i], HEX);
    if (i < rfid.uid.size - 1) scannedUID += " ";
  }
  scannedUID.toUpperCase();

  Serial.print("Card Scanned: ");
  Serial.println(scannedUID);

  int foundIndex = -1;
  for (int i = 0; i < 4; i++) {
    if (players[i].uid == scannedUID) {
      foundIndex = i;
      break;
    }
  }

  if (foundIndex == -1) {
    for (int i = 0; i < 4; i++) {
      if (players[i].uid == "") {
        players[i].uid = scannedUID;
        foundIndex = i;
        savePlayerData(foundIndex);
        Serial.print("Bound to ");
        Serial.println(players[i].name);
        break;
      }
    }
  }

  if (foundIndex != -1) {
    if (players[foundIndex].isBankrupt) {
      showPopup("BANKRUPT!", players[foundIndex].name.c_str(), "Locked Out", 1800);
      showIdleScreen();
    } else {
      activePlayerIndex = foundIndex;
      pendingDelta = 0;
      updateTransactionScreen();
    }
  } else {
    showPopup("LIMIT REACHED", "All 4 card slots", "already mapped", 1500);
    showIdleScreen();
  }

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}

void handleBankButtons() {
  if (millis() - lastDebounce < debounceDelay) return;

  if (digitalRead(btn100Pin) == LOW) {
    pendingDelta += 100;
    lastDebounce = millis();
    updateTransactionScreen();
  }

  if (digitalRead(btn10Pin) == LOW) {
    pendingDelta += 10;
    lastDebounce = millis();
    updateTransactionScreen();
  }

  if (digitalRead(btn1Pin) == LOW) {
    pendingDelta += 1;
    lastDebounce = millis();
    updateTransactionScreen();
  }

  if (digitalRead(btnModePin) == LOW) {
    isDeductMode = !isDeductMode;
    lastDebounce = millis();
    updateTransactionScreen();
  }

  if (digitalRead(btnOkPin) == LOW) {
    lastDebounce = millis();

    long finalDelta = isDeductMode ? -pendingDelta : pendingDelta;

    if (players[activePlayerIndex].balance + finalDelta < 0) {
      showPopup("BANKRUPT?", "RED=Confirm Fold", "OK=Cancel", 0);
      delay(300);

      while (true) {
        if (digitalRead(btnResetPin) == LOW) {
          players[activePlayerIndex].balance = 0;
          players[activePlayerIndex].isBankrupt = true;
          savePlayerData(activePlayerIndex);

          showPopup("BANKRUPT!", players[activePlayerIndex].name.c_str(), "Out of the Game!", 2000);
          break;
        }
        if (digitalRead(btnOkPin) == LOW) {
          break;
        }
        delay(20);
      }

      activePlayerIndex = -1;
      pendingDelta = 0;
      isDeductMode = false;
      showIdleScreen();
      return;
    }

    players[activePlayerIndex].balance += finalDelta;
    savePlayerData(activePlayerIndex);

    char balStr[20];
    sprintf(balStr, "New: $%ld", players[activePlayerIndex].balance);
    showPopup("TRANSACTION OK", players[activePlayerIndex].name.c_str(), balStr, 1500);

    activePlayerIndex = -1;
    pendingDelta = 0;
    isDeductMode = false;
    showIdleScreen();
  }
}
 
void handlePassGo() {
  showPopup("PASS GO (+200)", "Tap player card", "to collect", 300);

  rfid.PCD_Init();
  unsigned long startTime = millis();
  while (millis() - startTime < 8000) {
    if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
      String scannedUID = "";
      for (byte i = 0; i < rfid.uid.size; i++) {
        scannedUID += (rfid.uid.uidByte[i] < 0x10 ? "0" : "");
        scannedUID += String(rfid.uid.uidByte[i], HEX);
        if (i < rfid.uid.size - 1) scannedUID += " ";
      }
      scannedUID.toUpperCase();

      int foundIndex = -1;
      for (int i = 0; i < 4; i++) {
        if (players[i].uid == scannedUID) {
          foundIndex = i;
          break;
        }
      }

      if (foundIndex != -1) {
        if (players[foundIndex].isBankrupt) {
          showPopup("DENIED", players[foundIndex].name.c_str(), "Is Bankrupt!", 1600);
        } else {
          players[foundIndex].balance += goReward;
          savePlayerData(foundIndex);

          char balStr[20];
          sprintf(balStr, "New: $%ld", players[foundIndex].balance);
          showPopup("GO COLLECTED!", players[foundIndex].name.c_str(), balStr, 1500);
        }
      } else {
        showPopup("ERROR", "Unregistered card", nullptr, 1500);
      }

      rfid.PICC_HaltA();
      rfid.PCD_StopCrypto1();
      showIdleScreen();
      return;
    }
    delay(50);
  }
  showIdleScreen();
}

void handleGameReset() {
  prefs.begin("bank", false);
  for (int i = 0; i < 4; i++) {
    players[i].balance = 1500;
    players[i].isBankrupt = false;
    String balKey = "bal" + String(i);
    String bkrKey = "bkr" + String(i);
    prefs.putLong(balKey.c_str(), 1500);
    prefs.putBool(bkrKey.c_str(), false);
  }
  prefs.end();

  showPopup("GAME RESET", "All players", "Reset to $1500", 1600);
  showIdleScreen();
}