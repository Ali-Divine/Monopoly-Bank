# Monopoly Bank

[![Simulate in Wokwi](https://img.shields.io/badge/Wokwi-Simulate%20Online-059669?style=for-the-badge&logo=espressif&logoColor=white)](https://wokwi.com/projects/481771578590168065)

An electronic banking terminal for Monopoly built on an ESP32. It replaces paper money with RFID cards, an OLED display, and physical buttons to add, subtract, transfer, and track player balances.

## Features

* **Up to 4 Players:** Automatic tag detection with standard starting balances ($1500).
* **Player-to-Player Direct Transfers:** Pay rent, buy properties, or settle trades directly between players without intermediate bank steps.
* **Persistent Flash Storage (NVS):** Player balances and status persist across power cycles using ESP32 Preferences, optimized for flash wear leveling.
* **Full Bankruptcy Engine:** Overdraft safety catch, manual fold confirmation, automatic card lockout, and account termination.
* **Pass GO Shortcut:** Dedicated hardware key adds $200 with an RFID scan verification.
* **Typo & Overdraft Protection:** Dedicated amount-tray wipe during a turn, plus a 2.5-second hold gate to prevent accidental full-game wipes.
* **Inactivity Watchdog:** Automatically returns to the idle screen after 25 seconds of silence if a player steps away mid-turn.


## Hardware Components

* 1x ESP32 DevKitC v4 (38-pin)
* 1x SSD1306 0.96" I2C OLED Display (128x64)
* 1x RC522 RFID Reader (SPI)
* 7x Tactile pushbuttons (+100, +10, +1, MODE, OK, PASS GO, RESET)
* 4x RFID cards or key fobs


## Pinout

### SSD1306 OLED Display (I2C)

* `SDA`  $\rightarrow$ ESP32 **GPIO 21**
* `SCL`  $\rightarrow$ ESP32 **GPIO 22**
* `VCC`  $\rightarrow$ **3V3**
* `GND`  $\rightarrow$ **GND**

### RC522 RFID Reader (SPI)

* `SDA (SS)` $\rightarrow$ ESP32 **GPIO 5**
* `SCK`      $\rightarrow$ ESP32 **GPIO 18**
* `MOSI`     $\rightarrow$ ESP32 **GPIO 23**
* `MISO`     $\rightarrow$ ESP32 **GPIO 19**
* `RST`      $\rightarrow$ ESP32 **GPIO 4**
* `3.3V`     $\rightarrow$ **3V3**
* `GND`      $\rightarrow$ **GND**

### Push Buttons

* **+100:** GPIO 13 $\rightarrow$ Button Terminal 1
* **+10:** GPIO 12 $\rightarrow$ Button Terminal 1 *(Strapping pin: do not hold down during boot)*
* **+1:** GPIO 14 $\rightarrow$ Button Terminal 1
* **MODE:** GPIO 27 $\rightarrow$ Button Terminal 1 *(Cycle: Deposit $\rightarrow$ Deduct $\rightarrow$ Transfer)*
* **OK:** GPIO 26 $\rightarrow$ Button Terminal 1 *(Short tap = confirm/next, 2s hold = complete transfer)*
* **PASS GO:** GPIO 25 $\rightarrow$ Button Terminal 1
* **RESET:** GPIO 33 $\rightarrow$ Button Terminal 1 *(Turn = clear amount; Idle = hold 2.5s to reset game)*
* **All Buttons (Terminal 2):** Tied to common **GND** (utilizing ESP32 internal `INPUT_PULLUP`)


## Firmware Rewrite & Bug Fixes

While testing the terminal in the simulator, things completely broke down. The simulation kept acting weird and crashing, player balances were glitching out, and a few sneaky loopholes in the logic made the whole system laggy and unresponsive during games.

Instead of patching broken logic, the code was rewritten from scratch to run smooth and reliably:

1. **Simulator Crashes & Random Freezes:** The old code was creating and destroying text variables way too fast every time a button was tapped, eating up memory until the simulator gave up and froze. Swapped all that out for simple, fixed text buffers so it stops crashing.
2. **Glitchy Balances & Lag:** The board was trying to re-save entire player profiles and card IDs every time a single dollar changed hands. This slowed the system down and made balances glitch. Now it only saves the actual numbers when money moves.
3. **Menu Loopholes & Getting Stuck:** Buttons were occasionally double-registering or getting stuck in loops—especially with long presses and menu screens—leaving players stranded mid-turn. Fixed the button detection logic and added an auto-cancel timer so nobody gets locked out.
4. **Broken Screen Text:** The display wasn't clearing font sizes properly between menus, causing text to clip, stretch, or render off-screen. Cleaned up the screen refresh code so menus stay readable.

## Enclosure / CAD (Onshape)

I didn't know CAD before starting this, so I picked up Onshape and learned the basics in about 8 hours to design the casing from scratch.

### Enclosure v3: Full Internal Architecture & Hardware Mounts

v2 was basically just an empty box with screw holes. It looked cool from the outside, but inside there was nothing to hold the parts, meaning stuff would just bounce around or need hot glue. For v3, I built out the whole inside so every single piece has its own snap-in spot.

**What Changed in v3:**

* **ESP32 Bed & USB Port:** Snug tray on the floor for the ESP32 preventing board shifting, with a flush cutout for the micro USB connector.
* **RFID Mount & Card Slot:** Dedicated posts for the inverted RC522 board, matched with an external side slot and guide rails to ensure close coil-to-card coupling.
* **OLED Screen Mount:** 4 corner mounting posts under the top bezel to hold the 0.96" display flush without placing point pressure on the glass.
* **Recessed Reset Pocket:** Calculator style side dish with a custom 3D-printed plunger prevents accidental game resets mid-match.
* **Custom 1U Keycaps:** Modeled switch caps designed specifically for tactile switches.

CAD models (`.STEP` and `.STL`) are available in the `/cad` directory.

## How to Use

### Initial Card Setup
Before the first turn, register each player's RFID card by tapping them one by one at the idle screen. The terminal automatically assigns them in order (Player 1 through Player 4) with their starting balance ($1500).

### Deposits & Deductions (Bank $\leftrightarrow$ Player)

1. Tap your RFID card on the reader to open your account.
2. Press **+100**, **+10**, and **+1** to dial in the amount. (Tap the side **RESET** button to clear a typo back to $0).
3. Press **MODE** to set `[+] DEPOSIT` or `[-] DEDUCT`.
4. Press **OK** to commit.

### Direct Player-to-Player Transfers (Rent & Trading)

1. Tap the **Payer's** RFID card to unlock their bank screen.
2. Input the transfer amount using the value buttons.
3. Press **MODE** until the display reads `[>] TRANSFER`.
4. Press **OK** to open the recipient selector menu.
5. **Tap OK** repeatedly to cycle through active players.
6. **Hold OK for 2 seconds** to execute the transfer. Funds move atomically between both balances in flash storage.

### Bankruptcy & Elimination

1. If a deduction or transfer drives a balance below $0, the screen triggers a `BANKRUPT?` warning.
2. Press **RESET** to confirm elimination:
* Balance zeroes out.
* Player is marked bankrupt in flash storage.
* Tag is locked out from all future turns and GO bonuses.


3. Or press **OK** to cancel and return to play without balance penalties.

### Collecting GO (+$200)

1. Press **PASS GO** from the idle screen.
2. Tap an active player card within 8 seconds (or press **OK** / **RESET** to abort).
3. $200 is automatically credited and committed to flash.

### Master Game Reset

1. Ensure the terminal is on the main idle screen.
2. **Hold the recessed RESET button for 2.5 seconds**.
3. All player accounts reset back to $1500 and bankrupt cards are cleared for a new match.

---

## Demo & Testing

### Bank working
<img width="1138" height="849" alt="Screenshot 2026-11-23 003818" src="https://github.com/user-attachments/assets/4d2ed7fa-8152-4fff-bd41-a9ea37a5abac" />

### Testing video

https://github.com/user-attachments/assets/5004af7a-5906-44a6-af74-8d28106729cc
