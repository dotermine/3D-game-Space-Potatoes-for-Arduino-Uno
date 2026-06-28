# 3D-game-Space-Potatoes-for-Arduino-Uno

**1. The name and idea of the game**

The idea of the game itself and the name " Space Potatoes " is a reference to Stanislav Lem's series of stories about Ijon Tichy, namely "The Twenty-Fifth Journey", in which the main character recalls how, not far from the planet Tairia, he was attacked by a predatory potato.
The project's engine was based on the Doom for Arduino project. Uno " by daveruiz (https://github.com/daveruiz/doom-nano).

<img width="192" height="104" alt="image" src="https://github.com/user-attachments/assets/9945cf27-27a9-49a9-ab72-92c0e6b796e4" />

<img width="196" height="98" alt="spgif" src="https://github.com/user-attachments/assets/d9822549-abf3-43f0-a217-47cc5ac4918d" />


**2. Project composition**

•	Arduino Uno (ATmega328P)
•	OLED 0.96" 128x64 (SSD1306)
•	6 tact buttons
•	Piezoelectric buzzer
•	6 buttons
•	Connecting wires
•	Breadboard

Arduino Specifications Uno (ATmega328P):
Clock frequency 	16 MHz
Flash memory 	32 KB (program)
RAM 	2 KB (data)
EEPROM 	1 KB

System resources used:
RAM: 88.9% (used 1821 bytes from 2048 bytes)
Flash: 99.7% (used 32174 bytes from 32256 bytes)

3. Connection diagram and control

Component	Component pin	Pin Arduino	Note
OLED	VCC	5V	5V power supply
OLED	GND	GND	Common land
OLED	SCL	A5	I2C clock signal
OLED	SDA	A4	I2C data
Squeaker	+	D9	PWM output for audio
UP button	1	D3	Entrance with a lift
DOWN button	1	D4	Entrance with a lift
LEFT button	1	D2	Entrance with a lift
Right button	1	D5	Entrance with a lift
FIRE button	1	D7	Entrance with a lift
USE button	1	D8	Entrance with a lift

LEFT - Turn left
RIGHT (right) - Turn right
UP (upward) - Forward movement
DOWN - Backward movement
FIRE - Shooting / Punching
WEAPON - Change weapon

<img width="726" height="537" alt="SP_ptoteus" src="https://github.com/user-attachments/assets/1e026bad-78a9-4a68-9d99-8cec087cb5e5" />


4. Key Features

•	Rendering a 3D scene with textures in real time
•	Game world: 4 levels (3 main, 1 secret).
•	Interactivity: opening doors, textured and animated walls.
•	Combat system: 4 types of weapons, 4 types of ordinary enemies, 3 types of bosses.
•	Gameplay: 3 levels of coordination (difficulty), hidden Easter eggs .
•	Interface and visuals : animated HUD, animated intros for the beginning of the game, good and bad endings.
•	Audio: Option to turn on/off background music.
•	Konami cheat code  ( ↑, ↑, ↓, ↓, ←, →, ←, →, FIRE , WEAPON )

5. Game references and Easter eggs
Weapon:
•	E_WEAPON_BLASTER ( blaster , Star Wars)
•	E_WEAPON_PLASMA_CUTTER ( laser cutter , Dead Space)
•	E_WEAPON_BFG9000 ( weapon BFG9000, Doom)

Subjects:
•	E_SEPULKI ( sipulki , books by Stanislaw Lem about Jonah the Quiet)
•	E_METAL_BOX (companion cube from Portal )

Text quotes in intros:
•	Intro : "FAR FAR AWAY NEAR TAIRIYA" ( reference to Star Wars)
•	Good ending : "TO INFINITY AND BEYOND" ( phrase Bazza Lightyear , Toy Story)
•	Bad Ending: "WASTED" (GTA Death Screen)





