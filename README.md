# SYSGLITCH
A tool for glitching the on-chip debugger rom located in RL78 devices in order to dump full flash contents

Only compatible with version 3.03 of the OCD rom.

Based on the attack outlined by Fail0verflow https://fail0verflow.com/blog/2018/ps4-syscon/

## Setup
- A Teensy 4.0 and the Arduino IDE
- A usb serial cable wired to a PC, capturing raw data on RX with Realterm https://sourceforge.net/projects/realterm/
- An RL78 with version 3.03 of the OCD rom
- A small diode and ~4K ohm resistor for the RX line pulldown(needed to stabilise signal on syscon TOOL0)

***Glitching Pinout***
![SYSCON_PINOUT](syscon_glitcher_pinout.png)

***Vita RL78 Pinout***
![VITA_PINOUT](vita_pinout.png)

## Credits:
- Fail0verflow for the initial Writeup on the attack.
- droogie for early syscon investigations.
- juansbeck for his findings on identifying the chip and pinout. 
- Zecoxao, M4j0r, and SSL for their support in all syscon related work.

![HELL](hell.jpg)
