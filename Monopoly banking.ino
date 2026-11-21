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

const int btnPlusPin = 13;
const int btnMinusPin = 12;
const int btnOkPin = 14;

Preferences prefs;

struct Player {
  String uid;
  String name;
  long balance;
};

Player players[4] = {
  {"", "Player 1", 1500},
  {"", "Player 2", 1500},
  {"", "Player 3", 1500},
  {"", "Player 4", 1500}
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

  pinMode(btnPlusPin, INPUT_PULLUP);
  pinMode(btnMinusPin, INPUT_PULLUP);
  pinMode(btnOkPin, INPUT_PULLUP);

  loadAllPlayerData();

  showIdleScreen();
  Serial.println("Monopoly Banking Terminal Ready.");
}

void loop() {
  if (activePlayerIndex == -1) {
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

  oled.drawFastHLine(0, 35, 128, SSD1306_WHITE);

  oled.setTextSize(1);
  oled.setCursor(4, 43);
  oled.print("Adj:");
  oled.setTextSize(2);
  oled.setCursor(40, 41);

  if (pendingDelta == 0) {
    oled.print("$0");
  } else if (pendingDelta > 0) {
    oled.print("+$");
    oled.print(pendingDelta);
  } else {
    oled.print("-$");
    oled.print(abs(pendingDelta));
  }

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

  prefs.putString(uidKey.c_str(), players[index].uid);
  prefs.putLong(balKey.c_str(), players[index].balance);
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

    String savedUID = prefs.getString(uidKey.c_str(), "");
    long savedBalance = prefs.getLong(balKey.c_str(), 1500);

    if (savedUID != "") {
      players[i].uid = savedUID;
    }
    players[i].balance = savedBalance;
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
    activePlayerIndex = foundIndex;
    pendingDelta = 0;
    updateTransactionScreen();
  } else {
    showPopup("LIMIT REACHED", "All 4 card slots", "already mapped", 1500);
    showIdleScreen();
  }

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}

void handleBankButtons() {
  if (millis() - lastDebounce < debounceDelay) return;

  if (digitalRead(btnPlusPin) == LOW) {
    pendingDelta += 100;
    lastDebounce = millis();
    updateTransactionScreen();
  }

  if (digitalRead(btnMinusPin) == LOW) {
    pendingDelta -= 100;
    lastDebounce = millis();
    updateTransactionScreen();
  }

  if (digitalRead(btnOkPin) == LOW) {
    lastDebounce = millis();

    
    if (players[activePlayerIndex].balance + pendingDelta < 0) {
      showPopup("BANKRUPT!", "Insufficient funds", "Transaction denied", 1600);
      pendingDelta = 0;
      updateTransactionScreen();
      return;
    }

    
    players[activePlayerIndex].balance += pendingDelta;

    
    savePlayerData(activePlayerIndex);

    char balStr[20];
    sprintf(balStr, "New: $%ld", players[activePlayerIndex].balance);
    showPopup("TRANSACTION OK", players[activePlayerIndex].name.c_str(), balStr, 1500);

    
    activePlayerIndex = -1;
    pendingDelta = 0;
    showIdleScreen();
  }
}