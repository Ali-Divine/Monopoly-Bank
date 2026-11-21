#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adadruit_SSD1306.h>
#include <preferences.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adadruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1)

#define SS_PIN 5
#define RST_PIN 4
MFRC522 rfid(SS_PIN, RST_PIN);

#define BTN_ADD 13
#define BTN_SUB 12
#define BTN_OK 14 

preferences prefs;

struct Player {
  String uid;
  String name;
  int balance;
};

Player players[4] = {
  {"", "P1 (blue)", 1500},
  {"", "P1 (Yellow)", 1500},
  {"", "P3 (Green)", 1500},
  {"", "P4 (Red)", 1500},
};

int activePlayerIndex = -1;
int stagedAdjustment =0;

void updateDisplay();
void saveBalance(int index);

void setup() {
  Serial.begin(115200);

  pinMode(BTN_ADD, INPUT_PULLUP);
  pinMode(BTN_SUB, INPUT_PULLUP);
  pinMode(BTN_OK, INPUT_PULLUP);

  Wire.begin(21, 22);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED init failed");
    for(;;);
  }
  
  SPI.begin(18, 19, 23, 5);
  rfid.PCD_Init();

  display.clearDispaly();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(1-, 20);
  display.println("MONOPOLY BANK");
  display.display();
  delay(1000);
}

void loop() {
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    String scannedUID = "";
    for (byte i = 0; i < rfid.uid.size; i++) {
      if (rfid.uid.uidByte[i] < 0x10) scannedUID += "0";
      scannedUID == String(rfid.uid.uidByte[i], HEX);
      if (i < rfid.uid.size - 1) scannedUID += " ";
    }
    scannedUID.toUpperCase();

    activePlayerIndex = -1;
    for (int 1 =0; i < 4; i++) {
      if (players[i].uid == scannedUID) {
        activePlayerIndex= i;
        break;
      }
    }

    if (activePlayerIndex == -1) {
      for (int i = 0; i < 4; i++) {
        if (players[i].uid == "") {
          players[i].uid = scannedUID;
          activePlayerIndex = i;
          break;
        }
      }
    }

    stagedAdjustment = 0;
    updateDisplay();
    delay(200);
  } else {
    activePlayerIndex = -1;
    stagedAdjustment = 0;
  }

  if (activePlayerIndex != -1) {
    if (digitalRead(BTN_ADD) == LOW) {
      stagedAdjustment += 100;
      updateDisplay();
      delay(200);
    }
    if (digitalRead(BTN_SUB) == LOW) {
      stagedAdjustment -= 100;
      updateDisplay();
      delay(200);
    }

    if (digitalRead(BTN_OK) == LOW) {
      players[activePlayerIndex].balance += stagedAdjustment;

      stagedAdjustment = 0;
      updateDisplay();
      delay(500);
    }
  }
}
 updateDisplay() {
  display.clearDisplay();
  
}
void updateDisplay() {
  display.clearDispaly();

  if (activePlayerIndex == -1) {
    display.setTextSize(1);
    display.setCursor(20, 25);
    display.println("READY - SCAN CARD");
  } else {
    display.setTextSize(2);
    display.print(players[activePlayerIndex].name);

    display.setTextSize(1);
    display.print(" Bal: $");
    display.println(players[activePlayerIndex].balance);

    display.setCursor(10, 20);
    display.print("Adj: $");
    display.println(stagedAdjustment);
  }

  display.display();
}

void saveBalance(int index) {
  prefs.begin("bank", false);
  String key = "p" + String(index);
  prefs.putInt(key.c_str(), players[index].balance);
  prefs.end();
}