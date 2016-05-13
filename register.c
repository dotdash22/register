/* kassa.c - Embedded Electronics IE1206  */
/* B Knudsen Cc5x C-compiler - not ANSI-C */
/* ****************************************** */
/* ****************************************** */

#include "16f690.h"
#include "int16Cxx.h"
#pragma config |= 0x00D4
#pragma char X @ 0x115

/* ****************************************** */
/*              init functions                */
/* ****************************************** */
void delay( char);
void delay10( char );
void init_serial( void );
void init_interrupt( void );
void init_io_ports( void );
void putchar( char);
void printf(const char *string, char variable);
char getchar_eedata( char adress );
void putchar_eedata( char data, char adress );




/* ****************************************** */
/*            interrupt routines              */
/* ****************************************** */ 
char receiver_byte;  /* Transfer Byte used by interrupt routine */
bit receiver_flag;   /* Signal-flag used by interrupt routine   */

#pragma origin 4
interrupt int_server( void ) /* the place for the interrupt routine */
{
  int_save_registers
  /* New interrupts are automaticaly disabled            */
  /* "Interrupt on change" at pin RA1 from PK2 UART-tool */
  
  if( PORTA.1 == 0 )  /* Interpret this as the startbit  */
    {  /* Receive one full character   */
      char bitCount, ti;
      /* delay 1,5 bit 156 usec at 4 MHz         */
      /* 5+28*5-1+1+2+9=156 without optimization */
      ti = 28; do ; while( --ti > 0); nop(); nop2();
      for( bitCount = 8; bitCount > 0 ; bitCount--)
       {
         Carry = PORTA.1;
         receiver_byte = rr( receiver_byte);  /* rotate carry */
         /* delay one bit 104 usec at 4 MHz       */
         /* 5+18*5-1+1+9=104 without optimization */ 
         ti = 18; do ; while( --ti > 0); nop(); 
        }
      receiver_flag = 1; /* A full character is now received */
    }
  RABIF = 0;    /* Reset the RABIF-flag before leaving   */
  int_restore_registers
  /* New interrupts are now enabled */
}




/* ****************************************** */
/*                Start of main               */
/* ****************************************** */
void main( void)
{
	
	char choice;
	init_io_ports();
	init_serial();
	init_interrupt();

	/* You should "connect" PK2 UART-tool in one second after power on! */
	delay10(1000); 
	printf("Hello Slave! \r\n",0);
	printf("a for light on, b for light off \r\n",0);
  
  
  

// loop for interrupt/menu
	while(1)
	{
		if( receiver_flag ) /* Character received? */ 
		{
		choice = receiver_byte; /* get Character from interrupt routine */
		receiver_flag = 0;      /* Character now taken - reset the flag */
		
		switch (choice)
		{
			case 'a':
			PORTC.0 = 1;
			printf("light on \r\n", choice);
			break;
			case 'b':
			PORTC.0 = 0;
			printf("light off \r\n", choice);
			break;
			default:
			printf("Default, scurb.\r\n", choice);
		}
	}     	
	/* if no Character is received we always loop here */
	//printf("Idle \r\n",0);
	}
}



/* *********************************** */
/*            FUNCTIONS                */
/* *********************************** */
void delay( char millisec)
{
    OPTION = 2;  /* prescaler divide by 8        */
    do  {  TMR0 = 0;
           while ( TMR0 < 125)   /* 125 * 8 = 1000  */ ;
        } while ( -- millisec > 0);
}

void delay10( char n )
/*
  Delays a multiple of 10 milliseconds using the TMR0 timer
  Clock : 4 MHz   => period T = 0.25 microseconds
  1 IS = 1 Instruction Cycle = 1 microsecond
  error: 0.16 percent
*/
{
    char i;

    OPTION = 7;
    do  {
        i = TMR0 + 39; /* 256 microsec * 39 = 10 ms */
        while ( i != TMR0)
            ;
    } while ( --n > 0);
}


void init_serial( void )  /* initialise PIC16F690 bitbang serialcom */
{
   ANSEL.0 = 0; /* No AD on RA0             */
   ANSEL.1 = 0; /* No AD on RA1             */
   PORTA.0 = 1; /* marking line             */
   TRISA.0 = 0; /* output to PK2 UART-tool  */
   TRISA.1 = 1; /* input from PK2 UART-tool */
   //TRISC = 0; /* Enable output RC0		*/
   receiver_flag = 0 ;
   return;      
}

void init_interrupt( void )
{
  IOCA.1 = 1; /* PORTA.1 interrupt on change */
  RABIE =1;   /* interrupt on change         */
  GIE = 1;    /* interrupt enable            */
  return;
}

void init_io_ports( void )
{
  TRISC = 0xF8; /* 11111000 0 is for outputbit  */
  PORTC = 0b000;    /* initial value */

  ANSEL =0;     /* not AD-input      */
 // TRISA.5 = 1;  /* input rpgA        */
 // TRISA.4 = 1;  /* input rpgB        */
  /* Enable week pullup's            */
 // OPTION.7 = 0; /* !RABPU bit        */
 // WPUA.5   = 1; /* rpgA pullup       */
 // WPUA.4   = 1; /* rpgB pullup       */
  X.6 = 1;
  //TRISB.6 = 1;  /* PORTB pin 6 input */
  
 return;
}

void putchar( char ch )  /* sends one char */
{
  char bitCount, ti;
  PORTA.0 = 0; /* set startbit */
  for ( bitCount = 10; bitCount > 0 ; bitCount-- )
   {
     /* delay one bit 104 usec at 4 MHz       */
     /* 5+18*5-1+1+9=104 without optimization */ 
     ti = 18; do ; while( --ti > 0); nop(); 
     Carry = 1;     /* stopbit                    */
     ch = rr( ch ); /* Rotate Right through Carry */
     PORTA.0 = Carry;
   }
  return;
}

void printf(const char *string, char variable)
{
  char i, k, m, a, b;
  for(i = 0 ; ; i++)
   {
     k = string[i];
     if( k == '\0') break;   // at end of string
     if( k == '%')           // insert variable in string
      {
        i++;
        k = string[i];
        switch(k)
         {
           case 'd':         // %d  signed 8bit
             if( variable.7 ==1) putchar('-');
             else putchar(' ');
             if( variable > 127) variable = -variable;  // no break!
           case 'u':         // %u unsigned 8bit
             a = variable/100;
             putchar('0'+a); // print 100's
             b = variable%100;
             a = b/10;
             putchar('0'+a); // print 10's
             a = b%10;
             putchar('0'+a); // print 1's
             break;
           case 'b':         // %b BINARY 8bit
             for( m = 0 ; m < 8 ; m++ )
              {
                if (variable.7 == 1) putchar('1');
                else putchar('0');
                variable = rl(variable);
               }
              break;
           case 'c':         // %c  'char'
             putchar(variable);
             break;
           case '%':
             putchar('%');
             break;
           default:          // not implemented
             putchar('!');
         }
      }
      else putchar(k);
   }
}

void putchar_eedata( char data, char adress )
{
/* Put char in specific EEPROM-adress */
      /* Write EEPROM-data sequence                          */
      EEADR = adress;     /* EEPROM-data adress 0x00 => 0x40 */
      EEPGD = 0;          /* Data, not Program memory        */  
      EEDATA = data;      /* data to be written              */
      WREN = 1;           /* write enable                    */
      EECON2 = 0x55;      /* first Byte in comandsequence    */
      EECON2 = 0xAA;      /* second Byte in comandsequence   */
      WR = 1;             /* write                           */
      while( EEIF == 0) ; /* wait for done (EEIF=1)          */
      WR = 0;
      WREN = 0;           /* write disable - safety first    */
      EEIF = 0;           /* Reset EEIF bit in software      */
      /* End of write EEPROM-data sequence                   */
}


char getchar_eedata( char adress )
{
/* Get char from specific EEPROM-adress */
      /* Start of read EEPROM-data sequence                */
      char temp;
      EEADR = adress;  /* EEPROM-data adress 0x00 => 0x40  */ 
      EEPGD = 0;       /* Data not Program -memory         */      
      RD = 1;          /* Read                             */
      temp = EEDATA;
      RD = 0;
      return temp;     /* data to be read                  */
      /* End of read EEPROM-data sequence                  */  
}


/* *********************************** */
/*            HARDWARE                 */
/* *********************************** */

/*
   Use "PICkit2 UART Tool" as a 9600 Baud terminal.
   PIC internal USART is not used. BitBanging routines.
   Measure unknown frequency at CCP-pin.
   Testfrequencies from 4040 chip, Clocked by 16F690.
           ___________  ___________ 
          |           \/           |
    +5V---|Vdd      16F690      Vss|---GND 
          |RA5            RA0/(PGD)|bbTx->-PK2Rx
          |RA4            RA1/(PGC)|bbRx-<-PK2Tx 
          |RA3/!MCLR/(Vpp)  RA2/INT|
          |RC5/CCP              RC0|H/L DB4
          |RC4                  RC1|H/L DB5
   H/L DB7|RC3                  RC2|H/L DB6
          |RC6                  RB4|H/L Register Select Signal
          |RC7               RB5/Rx|
          |RB7/Tx               RB6|Enable signal
          |________________________| 
		  
*/
