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

### Enclosure v2: Full Overhaul and Mechanical Switch Plate

The original v1 shell was fine for a first pass, but having everything trapped in a single deep unibody made mounting and wiring miserable. I completely scrapped that structure and rebuilt the casing from the ground up as a two piece modular assembly: a deep bathtub base to house all the raw electronics, and a flat faceplate lid that drops cleanly over top.

#### What Changed in v2:
* **Mechanical Switch Grid (3x2):** Ditched the generic circular push buttons. The front plate now holds a 3x2 matrix of standard MX mechanical switches for real tactile feedback. Each cutout is measured to 14.1 x 14.1 mm (giving a 0.1 mm tolerance for 3D printing so switches snap straight in without filing).
* **Under-Plate Retention Pockets:** MX switches need a 1.5 mm plate thickness for their side clips to actually spring out and lock. Because the front plate needs to be thick and rigid, I cut 1.5 mm deep clearance pockets on the underside around every switch cutout. The clips bite onto a 1.5 mm lip from the back and lock tight without glue.
* **Handheld Ergonomics:** Instead of clumping the keys together like a keyboard numpad, columns are widened to 34.0 mm center to center and rows sit at 22.0 mm vertical pitch. The whole cluster starts at the horizontal center line and steps downward, keeping your thumbs in a natural grip without fat-fingering keys.
* **OLED Window:** Centered 24.0 x 13.0 mm display cutout positioned in the upper half of the faceplate for the 0.96 inch SSD1306 display, keeping the delicate screen glass isolated from direct thumb pressure.
* **Screw Mounting and Base Tub:** Added four corner M3 screw mounts with countersinks on the top plate and reinforced standoff cylinders inside the base tub. The base tub is 23.0 mm deep with 2.5 mm thick walls, leaving plenty of room for the ESP32, RFID module, and internal wiring harness.

CAD models (STEP and STL) are available in the `/cad` directory.
More updates coming soon...after my exam prep probably hehe

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
