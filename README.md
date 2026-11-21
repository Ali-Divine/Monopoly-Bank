# Monopoly Bank

An electronic banking terminal for Monopoly built on an ESP32. It replaces paper money with RFID cards, an OLED display, and physical buttons to add, subtract, and track player balances.

## Features

* Supports up to 4 players with individual starting balances ($1500)
* Auto detects and binds new RFID tags on first tap
* Persistent flash memory storage using ESP32 Preferences (balances stay saved after power loss or reset)
* On screen transaction staging (+100, minus 100, and OK to confirm)
* Bankruptcy and insufficient funds protection

## Components

* 1x ESP32 DevKit v1 (30 pin)
* 1x SSD1306 0.96" I2C OLED Display (128x64)
* 1x RC522 RFID Reader (SPI)
* 3x Push Buttons (+100, minus 100, OK)
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
* Button +100(Terminal 2) = ESP32(GND)
* Button minus 100(Terminal 1) = ESP32(GPIO 12)
* Button minus 100(Terminal 2) = ESP32(GND)
* Button OK(Terminal 1) = ESP32(GPIO 14)
* Button OK(Terminal 2) = ESP32(GND)

## How to Use

1. Tap an RFID card against the RC522 reader.
2. The OLED displays the player's name and current balance.
3. Press **+100** or **minus 100** to stage the transaction amount.
4. Press **OK** to commit the transaction to memory.
5. The display shows the updated balance and returns to the ready screen.
