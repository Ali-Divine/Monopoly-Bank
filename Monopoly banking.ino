#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Preferences.h>

Adafruit_SSD1306 screen(128, 64, &Wire, -1);
MFRC522 reader(5, 4); // SS=5, RST=4
Preferences mem;

// Button pins
const int btn100 = 13;
const int btn10  = 12; // Strapping pin....... do not press while powering on
const int btn1   = 14;
const int mode   = 27; // Switch deposit / deduct / transfer
const int ok     = 26;
const int go     = 25; // Pass go shortcut (+200)
const int reset  = 33; // Side button: clear tray / hold to wipe / cancel

// 0 = Deposit, 1 = Deduct, 2 = Transfer
int opMode = 0; 
const long goreward = 200;

struct Player {
  String card;
  String name;
  long money;
  bool isout;
};

Player players[4] = {
  {"", "Player 1", 1500, false},
  {"", "Player 2", 1500, false},
  {"", "Player 3", 1500, false},
  {"", "Player 4", 1500, false}
};

int activeplayer = -1;
long amount = 0;
unsigned long lastpress = 0;
unsigned long lastActionTime = 0;
const unsigned long wait = 180;
const unsigned long TIMEOUT_MS = 25000;

void popup(const char* title, const char* line1, const char* line2, int ms) {
  screen.clearDisplay();
  screen.drawRect(0, 0, 128, 64, SSD1306_WHITE);
  screen.fillRect(0, 0, 128, 14, SSD1306_WHITE);
  
  screen.setTextColor(SSD1306_BLACK);
  screen.setTextSize(1);
  screen.setCursor(6, 3);
  screen.print(title);

  screen.setTextColor(SSD1306_WHITE);
  screen.setTextSize(1);
  screen.setCursor(10, 24);
  screen.print(line1);
  if (line2 != nullptr) {
    screen.setCursor(10, 42);
    screen.print(line2);
  }

  screen.display();
  delay(ms);
}

void idlescreen() {
  activeplayer = -1;
  amount = 0;
  opMode = 0;

  screen.clearDisplay();
  screen.fillRect(0, 0, 128, 14, SSD1306_WHITE);
  screen.setTextColor(SSD1306_BLACK);
  screen.setTextSize(1);
  screen.setCursor(14, 3);
  screen.print("MONOPOLY BANK");

  screen.setTextColor(SSD1306_WHITE);
  screen.drawRoundRect(6, 20, 116, 40, 4, SSD1306_WHITE);
  screen.setCursor(20, 28);
  screen.print("TAP RFID CARD");
  screen.setCursor(26, 42);
  screen.print("TO DEPOSIT/PAY");
  
  screen.display();
}

void bankscreen() {
  screen.clearDisplay();

  screen.fillRect(0, 0, 128, 14, SSD1306_WHITE);
  screen.setTextColor(SSD1306_BLACK);
  screen.setTextSize(1);
  screen.setCursor(4, 3);
  screen.print(players[activeplayer].name);

  screen.setTextColor(SSD1306_WHITE);
  screen.setCursor(4, 18);
  screen.print("Bal: $");
  screen.setTextSize(2);
  screen.setCursor(44, 16);
  screen.print(players[activeplayer].money);

  screen.drawFastHLine(0, 34, 128, SSD1306_WHITE);

  screen.setTextSize(1);
  screen.setCursor(4, 38);
  if (opMode == 0) screen.print("[+] DEPOSIT");
  else if (opMode == 1) screen.print("[-] DEDUCT");
  else screen.print("[>] TRANSFER");

  screen.setTextSize(2);
  screen.setCursor(4, 48);
  screen.print("$");
  screen.print(amount);

  screen.display();
  screen.setTextSize(1);
}

void saveplayer(int p) {
  char key[8];
  snprintf(key, sizeof(key), "bal%d", p);
  mem.putLong(key, players[p].money);

  snprintf(key, sizeof(key), "bkr%d", p);
  mem.putBool(key, players[p].isout);
}

void saveCardUID(int p) {
  char key[8];
  snprintf(key, sizeof(key), "uid%d", p);
  mem.putString(key, players[p].card);
}

void loaddata() {
  for (int i = 0; i < 4; i++) {
    char key[8];
    snprintf(key, sizeof(key), "uid%d", i);
    String savedcard = mem.getString(key, "");
    if (savedcard != "") {
      players[i].card = savedcard;
    }
    snprintf(key, sizeof(key), "bal%d", i);
    players[i].money = mem.getLong(key, 1500);

    snprintf(key, sizeof(key), "bkr%d", i);
    players[i].isout = mem.getBool(key, false);
  }
}

void handlecard() {
  if (!reader.PICC_IsNewCardPresent() || !reader.PICC_ReadCardSerial()) return;

  String uid = "";
  for (byte i = 0; i < reader.uid.size; i++) {
    uid += (reader.uid.uidByte[i] < 0x10 ? "0" : "");
    uid += String(reader.uid.uidByte[i], HEX);
    if (i < reader.uid.size - 1) uid += " ";
  }
  uid.toUpperCase();

  int slot = -1;
  for (int i = 0; i < 4; i++) {
    if (players[i].card == uid) {
      slot = i;
      break;
    }
  }

  if (slot == -1) {
    for (int i = 0; i < 4; i++) {
      if (players[i].card == "") {
        players[i].card = uid;
        slot = i;
        saveCardUID(slot);
        saveplayer(slot);
        break;
      }
    }
  }

  if (slot != -1) {
    if (players[slot].isout) {
      popup("BANKRUPT!", players[slot].name.c_str(), "Locked Out", 1800);
      idlescreen();
    } else {
      activeplayer = slot;
      amount = 0;
      opMode = 0;
      lastActionTime = millis();
      bankscreen();
    }
  } else {
    popup("LIMIT REACHED", "All 4 card slots", "already mapped", 1500);
    idlescreen();
  }

  reader.PICC_HaltA();
  reader.PCD_StopCrypto1();
}

void renderTransferMenu(int targets[], int count, int selIndex) {
  screen.clearDisplay();
  screen.fillRect(0, 0, 128, 14, SSD1306_WHITE);
  screen.setTextColor(SSD1306_BLACK);
  screen.setTextSize(1);
  screen.setCursor(4, 3);
  screen.print("SEND: $");
  screen.print(amount);

  screen.setTextColor(SSD1306_WHITE);
  int y = 17;
  for (int i = 0; i < count; i++) {
    screen.setCursor(4, y);
    screen.print(i == selIndex ? "> " : "  ");
    screen.print(players[targets[i]].name);
    screen.print(" ($");
    screen.print(players[targets[i]].money);
    screen.print(")");
    y += 11;
  }
  
  screen.setCursor(4, 53);
  screen.print("Tap:Next | Hold:Send");
  screen.display();
}

void handleTransfer() {
  if (amount <= 0) {
    popup("ERROR", "Amount is $0", nullptr, 1200);
    bankscreen();
    return;
  }

  if (players[activeplayer].money < amount) {
    popup("NO FUNDS", "Not enough cash!", nullptr, 1500);
    bankscreen();
    return;
  }

  int targets[3];
  int count = 0;
  for (int i = 0; i < 4; i++) {
    if (i != activeplayer && players[i].card != "" && !players[i].isout) {
      targets[count++] = i;
    }
  }

  if (count == 0) {
    popup("NO TARGETS", "No other players", "registered yet", 1600);
    bankscreen();
    return;
  }

  int sel = 0;
  renderTransferMenu(targets, count, sel);
  delay(250);

  unsigned long menuAction = millis();

  while (true) {
    if (millis() - menuAction > TIMEOUT_MS) {
      idlescreen();
      return;
    }

    if (digitalRead(reset) == LOW || digitalRead(mode) == LOW) {
      delay(200);
      lastActionTime = millis();
      bankscreen();
      return;
    }

    if (digitalRead(ok) == LOW) {
      unsigned long pressStart = millis();
      bool held = false;

      while (digitalRead(ok) == LOW) {
        if (millis() - pressStart >= 2000) {
          held = true;
          break;
        }
        delay(20);
      }

      if (held) {
        while (digitalRead(ok) == LOW) delay(10); // Wait for button release

        int targetPlayer = targets[sel];
        players[activeplayer].money -= amount;
        players[targetPlayer].money += amount;

        saveplayer(activeplayer);
        saveplayer(targetPlayer);

        char msgTarget[24];
        snprintf(msgTarget, sizeof(msgTarget), "Sent to %s", players[targetPlayer].name.c_str());

        char balmsg[20];
        snprintf(balmsg, sizeof(balmsg), "New: $%ld", players[activeplayer].money);
        popup("TRANSFER OK!", msgTarget, balmsg, 1800);

        idlescreen();
        return;
      } else {
        sel = (sel + 1) % count;
        menuAction = millis();
        renderTransferMenu(targets, count, sel);
        delay(150);
      }
    }
    delay(20);
  }
}

void handlebuttons() {
  if (millis() - lastActionTime > TIMEOUT_MS) {
    idlescreen();
    return;
  }

  if (millis() - lastpress < wait) return;

  bool actionTaken = false;

  if (digitalRead(btn100) == LOW) {
    amount += 100;
    actionTaken = true;
    bankscreen();
  }
  else if (digitalRead(btn10) == LOW) {
    amount += 10;
    actionTaken = true;
    bankscreen();
  }
  else if (digitalRead(btn1) == LOW) {
    amount += 1;
    actionTaken = true;
    bankscreen();
  }
  else if (digitalRead(mode) == LOW) {
    opMode = (opMode + 1) % 3;
    actionTaken = true;
    bankscreen();
  }
  else if (digitalRead(reset) == LOW) {
    amount = 0;
    actionTaken = true;
    bankscreen();
  }
  else if (digitalRead(ok) == LOW) {
    lastpress = millis();
    lastActionTime = millis();

    if (opMode == 2) {
      handleTransfer();
      return;
    }

    long delta = (opMode == 1) ? -amount : amount;

    if (players[activeplayer].money + delta < 0) {
      popup("BANKRUPT?", "RED=Confirm Fold", "OK=Cancel", 0);
      delay(300);

      while (true) {
        if (digitalRead(reset) == LOW) {
          while (digitalRead(reset) == LOW) delay(10);
          players[activeplayer].money = 0;
          players[activeplayer].isout = true;
          saveplayer(activeplayer);
          popup("BANKRUPT!", players[activeplayer].name.c_str(), "Out of the Game!", 2000);
          break;
        }
        if (digitalRead(ok) == LOW) {
          while (digitalRead(ok) == LOW) delay(10);
          break;
        }
        delay(20);
      }

      idlescreen();
      return;
    }

    players[activeplayer].money += delta;
    saveplayer(activeplayer);

    char balmsg[20];
    snprintf(balmsg, sizeof(balmsg), "New: $%ld", players[activeplayer].money);
    popup("TRANSACTION OK", players[activeplayer].name.c_str(), balmsg, 1500);

    idlescreen();
    return;
  }

  if (actionTaken) {
    lastpress = millis();
    lastActionTime = millis();
  }
}

void passgo() {
  popup("PASS GO (+200)", "Tap card (OK=Exit)", nullptr, 300);

  reader.PCD_Init();
  unsigned long start = millis();
  while (millis() - start < 8000) {
    if (digitalRead(ok) == LOW || digitalRead(reset) == LOW) {
      break;
    }

    if (reader.PICC_IsNewCardPresent() && reader.PICC_ReadCardSerial()) {
      String uid = "";
      for (byte i = 0; i < reader.uid.size; i++) {
        uid += (reader.uid.uidByte[i] < 0x10 ? "0" : "");
        uid += String(reader.uid.uidByte[i], HEX);
        if (i < reader.uid.size - 1) uid += " ";
      }
      uid.toUpperCase();

      int slot = -1;
      for (int i = 0; i < 4; i++) {
        if (players[i].card == uid) {
          slot = i;
          break;
        }
      }

      if (slot != -1) {
        if (players[slot].isout) {
          popup("DENIED", players[slot].name.c_str(), "Is Bankrupt!", 1600);
        } else {
          players[slot].money += goreward;
          saveplayer(slot);

          char balmsg[20];
          snprintf(balmsg, sizeof(balmsg), "New: $%ld", players[slot].money);
          popup("GO COLLECTED!", players[slot].name.c_str(), balmsg, 1500);
        }
      } else {
        popup("ERROR", "Unregistered card", nullptr, 1500);
      }

      reader.PICC_HaltA();
      reader.PCD_StopCrypto1();
      idlescreen();
      return;
    }
    delay(40);
  }
  idlescreen();
}

void wipegame() {
  for (int i = 0; i < 4; i++) {
    players[i].money = 1500;
    players[i].isout = false;
    saveplayer(i);
  }

  popup("GAME RESET", "All players", "Reset to $1500", 1600);
  idlescreen();
}

void setup() {
  Serial.begin(115200);

  Wire.begin(21, 22);
  screen.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  SPI.begin();
  reader.PCD_Init();

  pinMode(btn100, INPUT_PULLUP);
  pinMode(btn10,  INPUT_PULLUP);
  pinMode(btn1,   INPUT_PULLUP);
  pinMode(mode,   INPUT_PULLUP);
  pinMode(ok,     INPUT_PULLUP);
  pinMode(go,     INPUT_PULLUP);
  pinMode(reset,  INPUT_PULLUP);

  mem.begin("bank", false);
  loaddata();

  idlescreen();
}

void loop() {
  if (activeplayer == -1) {
    if (millis() - lastpress >= wait) {
      if (digitalRead(go) == LOW) {
        lastpress = millis();
        passgo();
        return;
      }
      
      if (digitalRead(reset) == LOW) {
        unsigned long holdStart = millis();
        while (digitalRead(reset) == LOW) {
          if (millis() - holdStart > 2500) {
            while (digitalRead(reset) == LOW) delay(10);
            lastpress = millis();
            wipegame();
            return;
          }
          delay(20);
        }
      }
    }
    handlecard();
  } else {
    handlebuttons();
  }
}