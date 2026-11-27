# Monopoly Bank

[![Simulate in Wokwi](https://img.shields.io/badge/Wokwi-Simulate%20Online-059669?style=for-the-badge&logo=espressif&logoColor=white)](https://wokwi.com/projects/481771578590168065)

I had a cousins sleepover a while ago, well it was more of a monopoly night....and yuh even tho i had the most money, my cousins stole all of it making me lose the freaking game...I thought if we never had money in the first place, my cousins would never be able to steal it...
PRESENTING MONOPOLY BANK..and ESP32 project that handles all the finances in the game....no more stealing money guys....thank me later :P

## Main features

* **4 Player support:** all 4 players can play and all start with an initial balance of 1500M
* **Money transfer among players:** now u can pay rent, buy properties and do trades between 2 players....also a plus point no one will see you giving away bribe money hehehe!
* **Bankruptcy included:** once a players balance reaches 0...HE COOKED, now he cant join the game cuz he OUTTTT...and yuh u cant transfer him money to get him back if he gone..he gone you will have to move on man 
* **Salary:** when u pass GO u get 200M added into ur balance
* **Removing money and game reset:** if u wanna wipe ur typed in ammount, u can click reset and if u wanna reset the game, u hold reset for 2.5 seconds so no more ragequitting by giving u enough time to beat the shit out of the person johning (elite ball knowledge required)
* **AFK feature:** the screen returns to idle position if a palyer is inactive for 25 seconds


## Hardware Components

* 1x ESP32 DevKitC v4 (38pin)
* 1x SSD1306 0.96" I2C OLED Display (128x64)
* 1x RC522 RFID Reader (SPI)
* 6x Cherry mx switches (Go, Ok, Mode, +1, +10, +100)
* 1x pushbotton (reset)
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
* **+10:** GPIO 12 $\rightarrow$ Button Terminal 1 (dont hold this down while booting or else ur esp is fried)
* **+1:** GPIO 14 $\rightarrow$ Button Terminal 1
* **MODE:** GPIO 27 $\rightarrow$ Button Terminal 1 (cycle thru deposit, deduct and transfer)
* **OK:** GPIO 26 $\rightarrow$ Button Terminal 1 (click = cycle thru players, hold = confirm transfer)
* **PASS GO:** GPIO 25 $\rightarrow$ Button Terminal 1
* **RESET:** GPIO 33 $\rightarrow$ Button Terminal 1 (click = clear amount, hold 2.5s = reset game)
* **All Buttons (Terminal 2):** Tied to common **GND** (using ESP32 `INPUT_PULLUP`)


## Firmware Rewrite & Bug Fixes

While testing in the simulator, things completely broke down. The simulation kept acting weird and crashing, player balances were glitching out, and a few probelems in the logic made the whole system laggy and unresponsive during games....and the code was spaghetti af, so i just said fukkit and rewrote the entire thing from scratch to run smoothly (ik m dumb, but guess what my cat walked on my keyboard and typed sumthing in, thats why it wasnt working....I only found out after replacing my code so yuh....FAHHHH)

1. **Simulator Crashes:** the old code was writing stuff too fast like literally on every single button press, and yuh it ate up all the memory so i fixed it
2. **unlimited ram paradox:** the code thought m sam altman or sum, so it was saving up all player data after every money change..i removed that part hehehe
3. **button fixes:** there was no going back once u typed sumthing in so i added a backspace feature to the reset button.....now u can catch if ur cousins tryna charge u extra and just remove ts
4. **screen goof:** the screen wasnt outputing the fonts correctly giving the display a messed up look, so i did the most sensible thing....REWRITE THE WHOLE SCREEN LOGIC idk if m too unemplyed or just dumb af 

## Enclosure / CAD (Onshape)

I didn't know CAD before starting this, so I picked up Onshape and learned the basics in about 8 hours to design the casing from scratch. very big shout out to myself for not doing ts instead of preparing for my exam lol(got highest even then)

### Enclosure v3: Full Internal Architecture & Hardware Mounts

v2 was basically just an empty box with screw holes. It looked cool from the outside, but inside it was empty, cuz i didnt knwo how to add the component clamps, so i locked in sum more and yuh NOW WE HAVE CLAMPS so no hot glue needed..use that hotglue to glue ur cousins hands together if they try stealing again MWAHAHAHHA

**What Changed in v3:**

* **ESP32 Mount & USB Port:** added a tray for the esp32 and a port hole cuz how elese we gonna connect it?..tbh i kinda hate this part cuz the usb port is not flush with the outside so its hard to connect, might fix it in v4 if im not lazy
* **RFID Mount & Card Slot:** screw posts for the inverted RFID board.the card slot is good tho, u can easily slide the card in and out
* **OLED Screen Mount:** 4 corner mounting posts under the top bezel to hold the 0.96" display flush without placing point pressure on the glass.this part took me a while cuz the display was bigger than i expected...
* **Reset button(casio style):** Calculator style side dish with a custom 3D printed plunger prevents accidental game resets mid match.
* **Custom 1U Keycaps:** Made switch caps designed for cherry mx switches

CAD models (`.STEP` and `.STL`) are available in the `/cad` directory

## How to Use

### Initial Card Setup
Before the first turn, tap all of ur RFID cards by tapping them one by one at the idle screen. The terminal assigns them in order (Player 1 thru Player 4) with their starting balance ($1500)

### Banking (between player and da bank)

1. Tap your RFID card on the reader to open ur account
2. Press **+100**, **+10**, and **+1** to put in the amount. (click the side **RESET** button to clear a typo back to $0)
3. Press **MODE** to set `[+] DEPOSIT` or `[-] DEDUCT`
4. Press **OK** to confirm

### rent and trading (between the players)

1. Tap the **Payer** RFID card to open their bank screen
2. Input the transfer amount using the **+100**, **+10**, and **+1** buttons
3. tap **reset** if u wanna clear the amount
4. Press **MODE** until the display reads `[>] TRANSFER`
5. Press **OK** to open the recipient selector menu
6. Tap **OK** repeatedly to cycle thru active players
7. Hold **OK** for 2 seconds to execute the transfer (Money laundering done lmao)

### Bankruptcy & Elimination

1. If a deduction or transfer takes ur balance below $0, the screen triggers a `BANKRUPT?` warning
2. Press **RESET** to confirm elimination.....
* Balance zeroes out
* Player is marked bankrupt in flash storage
* Tag is locked out from all future turns and GO bonuses

3. press **OK** to cancel and return to play without balance penalties

### Collecting salary (+$200)

1. Press **PASS GO** from the idle screen
2. Tap an active player card within 8 seconds (or press **OK** / **RESET** to abort)
3. $200 is automatically added into ur balance

### Game Reset

1. Ensure the terminal is on the main idle screen
2. Hold the **RESET** button for 2.5 seconds
3. All player accounts reset back to $1500 and bankrupt cards are cleared for a new match

## Demo & Testing

### Bank working
<img width="1138" height="849" alt="Screenshot 2026-11-23 003818" src="https://github.com/user-attachments/assets/4d2ed7fa-8152-4fff-bd41-a9ea37a5abac" />

### Testing video

https://github.com/user-attachments/assets/5004af7a-5906-44a6-af74-8d28106729cc
