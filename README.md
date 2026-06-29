# 3D-game-Space-Potatoes-for-Arduino-Uno

## 1. The name and idea of the game

The idea of the game itself and the name "Space Potatoes" is a reference to Stanislav Lem's series of stories about Ijon Tichy, namely "The Twenty-Fifth Journey", in which the main character recalls how, not far from the planet Tairia, he was attacked by a predatory potato.

The project's engine was based on the Doom for Arduino project "Uno" by daveruiz ([github.com/daveruiz/doom-nano](https://github.com/daveruiz/doom-nano)).

<img width="320" height="158" alt="image" src="https://github.com/user-attachments/assets/e003de37-29c5-4dc1-9792-904a7e7a8947" />
<img width="320" height="158" alt="TEST1" src="https://github.com/user-attachments/assets/e1023332-b913-45c6-ba09-da68d4767b14" />
<img width="320" height="158" alt="TEST2" src="https://github.com/user-attachments/assets/2a2b6044-0d91-4b2e-bedc-44ef35d48744" />

<img width="272" height="127" alt="image" src="https://github.com/user-attachments/assets/6b8d6ad2-22d0-4c1b-8cd9-6b6ffcaffbd2" />
<img width="272" height="127" alt="image" src="https://github.com/user-attachments/assets/e5d5406c-0502-446b-9f4b-5f5ce42838f6" />
<img width="272" height="127" alt="image" src="https://github.com/user-attachments/assets/4964702f-4ead-455a-ad03-a9bd5d7bd127" />
<img width="272" height="127" alt="SP4_cheat" src="https://github.com/user-attachments/assets/5eb60fc9-355a-49d2-92d2-672f099539ec" />
<img width="272" height="127" alt="SP5_4level" src="https://github.com/user-attachments/assets/d4fb8c1e-295f-4634-8e76-23f246da237e" />


## 2. Project composition

* **Arduino Uno** (ATmega328P)
* **OLED 0.96"** 128x64 (SSD1306)
* 6 tact buttons
* Piezoelectric buzzer
* 6 buttons
* Connecting wires
* Breadboard

### Arduino Specifications Uno (ATmega328P)

| Parameter | Value |
| :--- | :--- |
| Clock frequency | 16 MHz |
| Flash memory | 32 KB (program) |
| RAM | 2 KB (data) |
| EEPROM | 1 KB |

### System resources used
* **RAM:** 88.9% (used 1821 bytes from 2048 bytes)
* **Flash:** 99.7% (used 32174 bytes from 32256 bytes)

## 3. Connection diagram and control

### Pinout Configuration

| Component | Component pin | Pin Arduino | Note |
| :--- | :--- | :--- | :--- |
| OLED | VCC | 5V | 5V power supply |
| OLED | GND | GND | Common land |
| OLED | SCL | A5 | I2C clock signal |
| OLED | SDA | A4 | I2C data |
| Squeaker | + | D9 | PWM output for audio |
| UP button | 1 | D3 | Entrance with a lift |
| DOWN button | 1 | D4 | Entrance with a lift |
| LEFT button | 1 | D2 | Entrance with a lift |
| Right button | 1 | D5 | Entrance with a lift |
| FIRE button | 1 | D7 | Entrance with a lift |
| USE button | 1 | D8 | Entrance with a lift |

<img width="556" height="717" alt="image" src="https://github.com/user-attachments/assets/2ee83c34-dbc9-4cff-9cfb-391099e63709" />


### Controls

* **LEFT** — Turn left
* **RIGHT** — Turn right
* **UP** — Forward movement
* **DOWN** — Backward movement
* **FIRE** — Shooting / Punching
* **WEAPON** — Change weapon

## 4. Key Features

* **Real-time 3D:** Rendering a 3D scene with textures in real time.
* **Game world:** 4 levels (3 main, 1 secret).
* **Interactivity:** Opening doors, textured and animated walls.
* **Combat system:** 4 types of weapons, 4 types of ordinary enemies, 3 types of bosses.
* **Gameplay:** 3 levels of coordination (difficulty), hidden Easter eggs.
* **Interface & Visuals:** Animated HUD, animated intros for the beginning of the game, good and bad endings.
* **Audio:** Option to turn on/off background music.
* **Konami cheat code:** `↑` `↑` `↓` `↓` `←` `→` `←` `→` `FIRE` `WEAPON`

## 5. Game references and Easter eggs

### Weapons
* `E_WEAPON_BLASTER` — blaster (Star Wars)
* `E_WEAPON_PLASMA_CUTTER` — laser cutter (Dead Space)
* `E_WEAPON_BFG9000` — weapon BFG9000 (Doom)

### Items
* `E_SEPULKI` — sepulki (books by Stanislaw Lem about Ijon Tichy)
* `E_METAL_BOX` — companion cube (Portal)

### Text quotes in intros
* **Intro:** `"FAR FAR AWAY NEAR TAIRIYA"` (reference to Star Wars)
* **Good ending:** `"TO INFINITY AND BEYOND"` (phrase Buzz Lightyear, Toy Story)
* **Bad Ending:** `"WASTED"` (GTA Death Screen)

<img width="3958" height="2359" alt="11IMG_20260628_191202 (1)" src="https://github.com/user-attachments/assets/16ed63d0-0427-4978-85fd-36eba97a45ec" />

<img width="320" height="255" alt="ezgif-1694aa8d8801bca8" src="https://github.com/user-attachments/assets/e40cf2d8-4355-425e-8955-2eb37d3d5695" />





