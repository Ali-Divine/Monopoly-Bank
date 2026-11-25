# Monopoly Bank

[![Simulate in Wokwi](https://img.shields.io/badge/Wokwi-Simulate%20Online-059669?style=for-the-badge&logo=espressif&logoColor=white)](https://wokwi.com/projects/481561425176914945)

An electronic banking terminal for Monopoly built on an ESP32. It replaces paper money with RFID cards, an OLED display, and physical buttons to add, subtract, and track player balances.

## Features

* Supports up to 4 players with individual starting balances ($1500)
* Auto detects and binds new RFID tags on first tap
* Persistent flash memory storage using ESP32 Preferences (balances stay saved after power loss or reset)
* Flexible keypad transaction staging (+100, +10, +1) with a MODE toggle for deposits and deductions
* Pass GO button 
* Game Reset button 
* Full bankruptcy mechanic with fold confirmation, automatic card lockout, and balance wipe to $0
* Negative balance safety checks preventing accidental overdrafts during standard play

## Components

* 1x ESP32 DevKitC v4 (38 pin)
* 1x SSD1306 0.96" I2C OLED Display (128x64)
* 1x RC522 RFID Reader (SPI)
* 7x Buttons (+100, +10, +1, MODE, OK, PASS GO, RESET)
* 4x RFID Cards or Key Fobs

## Pinout

### SSD1306 OLED Display

* OLED(SDA) = ESP32(GPIO 21)
* OLED(SCL) = ESP32(GPIO 22)
* OLED(VCC) = ESP32(3V3)
* OLED(GND) = ESP32(GND)

### RC522 RFID Reader

* RC522(SDA) = ESP32(GPIO 5)
* RC522(SCK) = ESP32(GPIO 18)
* RC522(MOSI) = ESP32(GPIO 23)
* RC522(MISO) = ESP32(GPIO 19)
* RC522(RST) = ESP32(GPIO 4)
* RC522(3.3V) = ESP32(3V3)
* RC522(GND) = ESP32(GND)

### Push Buttons

* Button +100(Terminal 1) = ESP32(GPIO 13)
* Button +10(Terminal 1) = ESP32(GPIO 12)
* Button +1(Terminal 1) = ESP32(GPIO 14)
* Button MODE(Terminal 1) = ESP32(GPIO 27)
* Button OK(Terminal 1) = ESP32(GPIO 26)
* Button PASS GO(Terminal 1) = ESP32(GPIO 25)
* Button RESET(Terminal 1) = ESP32(GPIO 33)
* All Buttons(Terminal 2) = ESP32(GND)

## Enclosure / CAD (Onshape)

I didn't know CAD before starting this, so I picked up Onshape and learned the basics in about 2 hours to design the casing from scratch. 

### Enclosure v3: Full Internal Architecture & Hardware Mounts
v2 was basically just an empty box with screw holes. It looked cool from the outside, but inside there was nothing to hold the parts, meaning stuff would just bounce around or need hot glue. For v3, I built out the whole inside so every single piece has its own snap in spot.

##What Changed in v3:
* **ESP32 Bed & USB Port:** Made a snug little tray on the floor for the ESP32 so it can't slide around, and lined it up so the micro USB cable plugs right in through a neat cutout on the side.
* **RFID Mount & Card Slot:** Added posts to screw the RC522 reader upside down, plus a smooth side slot and a guide brace so cards slide right over the sensor.
* **OLED Screen Mount:** Added 4 little screw posts under the lid right around the screen window to hold the display tight without cracking the glass.
* **Casio-Style Reset Button:** Put a recessed dish on the left side with a custom 3D printed button plunger, looks like an old calculator button so nobody wipes the game balances by mistake.
* **Custom Keycaps:** Modeled the 1U keycaps from scratch to fit right on the switches and finish up the whole look.

CAD models (STEP and STL) are available in the `/cad` directory.

## How to Use

### Regular Transactions
1. Tap an RFID card on the RC522 reader to select the player.
2. Use **+100**, **+10**, and **+1** to set the transaction amount.
3. Press **MODE** to toggle between **[+] DEPOSIT** and **[-] DEDUCT**.
4. Press **OK** to save the transaction to memory.

### Declaring Bankruptcy
1. If a deduction exceeds a player's current balance, the screen prompts **BANKRUPT?**.
2. Press **RESET** to confirm folding:
   * Balance drops to **$0**.
   * Player is marked **Bankrupt** in flash storage.
   * The card is permanently locked out from transactions and Pass GO rewards until a full game reset.
3. Or press **OK** to cancel the deduction and return to the game without penalty.

### Pass GO (+$200)
1. Press the **PASS GO** button from the ready screen.
2. Tap your RFID card to instantly get $200.

### Game Reset
1. Press the **RESET** button from the ready screen.
2. All 4 player balances reset back to $1500 in flash memory and any bankrupt players are restored to active status.

## Demo & Testing

### Bank working
<img width="1138" height="849" alt="Screenshot 2026-11-23 003818" src="https://github.com/user-attachments/assets/4d2ed7fa-8152-4fff-bd41-a9ea37a5abac" />

### Testing video

https://github.com/user-attachments/assets/5004af7a-5906-44a6-af74-8d28106729cc
