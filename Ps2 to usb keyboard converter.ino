/* Advanced keyboard example test to serial port at 115200 with command setting

  PS2KeyAdvanced library example

  Advanced support PS2 Keyboard to get every key code byte from a PS2 Keyboard
  for testing purposes. Enables capture of all keys output to serial monitor.

  IMPORTANT WARNING

    If using a DUE or similar board with 3V3 I/O you MUST put a level translator
    like a Texas Instruments TXS0102 or FET circuit as the signals are
    Bi-directional (signals transmitted from both ends on same wire).

    Failure to do so may damage your Arduino Due or similar board.

  Test History
    September 2014 Uno and Mega 2560 September 2014 using Arduino V1.6.0
    January 2016   Uno, Mega 2560 and Due using Arduino 1.6.7 and Due Board
                    Manager V1.6.6

  This is for a LATIN style keyboard using Scan code set 2. See various
  websites on what different scan code sets use. Scan Code Set 2 is the
  default scan code set for PS2 keyboards on power up.

  Will support most keyboards even ones with multimedia keys or even 24 function keys.

  Also allows keys entered to send commands to keyboard as follows
    R = Reset       Send reset command to keyboard and get responses
    E = Echo        Send protocol ECHO command (0xEE) and get same back
    I = ReadID      Read the keyboard ID code ( should 2 bytes 0xAb 0x83)
    S = Scancode    Get current Scancode set in use on keyboard should be 2
    T = Typematic rate Set to slowest Rate and delay
    G = Current Lock Get the current lock status (num, scroll, caps)
    B = Break codes Toggle if receiving break codes is on or off
    N = No repeat   Ignore repeats of make codes for Shift, Ctrl, ALT, ALT-GR, GUI

  The circuit:
   * KBD Clock (PS2 pin 1) to an interrupt pin on Arduino ( this example pin 3 )
   * KBD Data (PS2 pin 5) to a data pin ( this example pin 4 )
   * +5V from Arduino to PS2 pin 4
   * GND from Arduino to PS2 pin 3

   The connector to mate with PS2 keyboard is a 6 pin Female Mini-Din connector
   PS2 Pins to signal
    1       KBD Data
    3       GND
    4       +5V
    5       KBD Clock

   Keyboard has 5V and GND connected see plenty of examples and
   photos around on Arduino site and other sites about the PS2 Connector.

 Interrupts

   Clock pin from PS2 keyboard MUST be connected to an interrupt
   pin, these vary with the different types of Arduino

  PS2KeyAdvanced requires both pins specified for begin( )

  keyboard.begin( data_pin, irq_pin );

  Valid irq pins:
     Arduino Uno:  2, 3
     Arduino Due:  All pins, except 13 (LED)
     Arduino Mega: 2, 3, 18, 19, 20, 21
     Teensy 2.0:   All pins, except 13 (LED)
     Teensy 2.0:   5, 6, 7, 8
     Teensy 1.0:   0, 1, 2, 3, 4, 6, 7, 16
     Teensy++ 2.0: 0, 1, 2, 3, 18, 19, 36, 37
     Teensy++ 1.0: 0, 1, 2, 3, 18, 19, 36, 37
     Sanguino:     2, 10, 11

  Read method Returns an UNSIGNED INT containing
        Make/Break status
        Caps status
        Shift, CTRL, ALT, ALT GR, GUI keys
        Flag for function key not a displayable/printable character
        8 bit key code

  Code Ranges (bottom byte of unsigned int)
        0       invalid/error
        1-1F    Functions (Caps, Shift, ALT, Enter, DEL... )
        1A-1F   Functions with ASCII control code
                    (DEL, BS, TAB, ESC, ENTER, SPACE)
        20-61   Printable characters noting
                    0-9 = 0x30 to 0x39 as ASCII
                    A to Z = 0x41 to 0x5A as upper case ASCII type codes
                    8B Extra European key
        61-A0   Function keys and other special keys (plus F2 and F1)
                    61-78 F1 to F24
                    79-8A Multimedia
                    8B NOT included
                    8C-8E ACPI power
                    91-A0 and F2 and F1 - Special multilingual
        A8-FF   Keyboard communications commands (note F2 and F1 are special
                codes for special multi-lingual keyboards)

    By using these ranges it is possible to perform detection of any key and do
    easy translation to ASCII/UTF-8 avoiding keys that do not have a valid code.

    Top Byte is 8 bits denoting as follows with defines for bit code

        Define name bit     description
        PS2_BREAK   15      1 = Break key code
                   (MSB)    0 = Make Key code
        PS2_SHIFT   14      1 = Shift key pressed as well (either side)
                            0 = NO shift key
        PS2_CTRL    13      1 = Ctrl key pressed as well (either side)
                            0 = NO Ctrl key
        PS2_CAPS    12      1 = Caps Lock ON
                            0 = Caps lock OFF
        PS2_ALT     11      1 = Left Alt key pressed as well
                            0 = NO Left Alt key
        PS2_ALT_GR  10      1 = Right Alt (Alt GR) key pressed as well
                            0 = NO Right Alt key
        PS2_GUI      9      1 = GUI key pressed as well (either)
                            0 = NO GUI key
        PS2_FUNCTION 8      1 = FUNCTION key non-printable character (plus space, tab, enter)
                            0 = standard character key

  Error Codes
     Most functions return 0 or 0xFFFF as error, other codes to note and
     handle appropriately
        0xAA   keyboard has reset and passed power up tests
               will happen if keyboard plugged in after code start
        0xFC   Keyboard General error or power up fail

  See PS2Keyboard.h file for returned definitions of Keys

  Note defines starting
            PS2_KEY_* are the codes this library returns
            PS2_*     remaining defines for use in higher levels

  To get the key as ASCII/UTF-8 single byte character conversion requires use
  of PS2KeyMap library AS WELL.

  Written by Paul Carpenter, PC Services <sales@pcserviceselectronics.co.uk>
*/

#include <PS2KeyAdvanced.h> //i have made some changes on this i raccomand to use mine


/* Keyboard constants  Change to suit your Arduino
   define pins used for data and clock from keyboard */
#define DATAPIN 2
#define IRQPIN  3

uint16_t c;
uint8_t breaks = 0;
uint8_t repeats = 0;
uint8_t buf[8]={0};

int s=0;
#define shift 0x02;
#define altgr 0x05;
PS2KeyAdvanced keyboard;


void setup( )
{
Serial.begin( 9600 );
//Serial.println( "PS2 Advanced Key - Advanced Test:" );

// Configure the keyboard library
keyboard.begin( DATAPIN, IRQPIN );
keyboard.echo( );              // ping keyboard to see if there
delay( 6 );
char c = keyboard.read( );
/*if( (c & 0xFF) == PS2_KEY_ECHO 
    || (c & 0xFF) == PS2_KEY_BAT )
  Serial.println( "Keyboard OK.." );    // Response was Echo or power up
else
  if( ( c & 0xFF ) == 0 )
    Serial.println( "Keyboard Not Found" );
  else
    {
    Serial.print( "Invalid Code received of " );
    Serial.println( c, HEX );
    } */
}


void loop( )
{
if( keyboard.available( ) )
  {  s=0;
  // read the next key
  c = keyboard.read( );
  
  if( ( c & 0xFF ) > 0 ){ //any number or letter is pressed
  
  



 /* if(((c-j)==0x8000)||((c-j)==0x8100)){
   releaseKey();
   c=0;
  
   }*/
   if ((c&0b1000000000000000)==0b1000000000000000){
    releaseKey(); //remove the key that is released
    c=0;
    
   }
   
   if((c&0b11111111)==0b11111010)//remove bugged key(nothing strange)
   c=0;

 
  
 if ((c&0b100000000000000)== 0b100000000000000) s+=0x02; //shif
 if ((c&0b10000000000000)==0b10000000000000)s+=0x01; // control
 
 if((c&0b100000000000)==0b100000000000)s+=0x04; // alt
 
 if ((c&0b1000000000)==0b1000000000)s+=0x08; //windows
 //Serial.print(c&0b1000000000,BIN);
 if((c&0b10000000000)==0b10000000000) s+=0x05; //altgr  
 
 
 buf[0]=s; 
 

}
char j=c&0b1111111; //removing all the funcion key as well as funtion keys, for the letters and numbers
//Serial.println(c,BIN);
 if( ((c&0b1000000000000)==0b1000000000000)&&((j>='A')&&(j<='Z'))) { //controlling if it is a capital letter  and shift is pressed

  if ((c&0b100000000000000)== 0b100000000000000) 
   buf[0]-=0x02; //if shift is pressed remove the shift that goes to the pc (so that the letter is read as a small letter)
else 
buf[0]+=0x02; 
 }

 buf[2]=encript(j); //convert from ps2 to usb
 
 if ((c&0xFF)==0b0000000010001011) // this is bugged but like this it works lol
 buf[2]=0x64;//< key between shif and z present on some keyboard
 
  if ((c&0b0000000100000000)==0b0000000100000000){ //control if it is a function key
    c=c&0b1111111111; //remove all bit that don't refear to keys
 
 if((c>=0x161)&&(c<=0x16c)){
 buf[2]=((c-0x161)+0x3A);//f1 to f12

  }


    
    switch(c){
  
  
  case 0x11E: buf[2]=(0x28); //enter
  break;
  case 0x11b: buf[2]=(0x29);//escape
  break;
  case 0x11d: buf[2]=(0x2b);//tab
  break;
  case 0x11f: buf[2]=(0x2c);//space
  break;
  case 0x11c: buf[2]=(0x2a);//backspace
  break;
  case 0x115: buf[2]= (0x50);//left arrow
  break;
  case 0x116: buf[2]=(0x4F);//right arrow
  break;
  case 0x118: buf[2]= (0x51);//down arrow
  break;
  case 0x117: buf[2]=(0x52);//up arrow
  break;
  case 0x11a: buf[2]= (0x4c);//delete
  break; 
  case 0x104: buf[2]=0x46;//stamp
  break;
  case 0x10E: buf[2]= (0x65);//menù
  break;
  case 0x119: buf[2]=0x49;//ins
  break;
  case 0x113: buf[2]= 0x4b;//page up
  break;
  case 0x114: buf[2]=0x4e;//page down
  break;
  case 0x112: buf[2]=0x4d;//end
  break;
  case 0x111: buf[2]=0x4a;//home
  break;
  
  
}
}

//Serial.println(c,HEX);
   
 // Serial.print(buf[2],HEX);
 //Serial.println(buf[2],BIN);
Serial.write(buf, 8); //sent to usb



  
 }



//else releaseKey();
  
}


   void releaseKey() 
{ if (((c&0b100000000000000)== 0b000000000000000)&&((c&0b10000000000000)==0b00000000000000)&&((c&0b100000000000)==0b000000000000)&&((c&0b10000000000)==0b00000000000)&&((c&0b1000000000)==0b0000000000))
 
 buf[0]=0; //if nothing is pressed send 0

 
  buf[2] = 0;
  
 // Serial.write(buf, 8); // Release key  
  delay(50);
}


int encript(char c){
  
 if ((c>='A')&&(c<='Z'))
   {
      c = tolower(c); //convert into lower caracter
 
   }
   
   if ((c>='a')&&(c<='z'))
   { 
      return(((c-='a')+4)); //a to z
   }
   
   if ((c>='1')&&(c<='9'))
   {
      return((c-='0')+0x1D); //1 to 9
   }
   
   if((c>=0x21)&&(c<=0x29)){
     
      return((c-=0x21)+0x59); //keypad 1 to 9

   }
  switch(c){//keyboard caracter ar referred to italian keyboard but should work just fine if you set the coorect nation of the keyboard, in the computer
    case 0x5b: return(0x33); //ò
    case 0x5d: return(0x2F); //è
    case 0x5e: return(0x30); //+
    case 0x3A: return(0x34); //à
    case 0x5c: return(0x32); //ù
    case 0x3b: return(0x36); //,
    case 0x2f: return(0x54); //keypad /
    case 0x2e: return(0x55); //keypad*
    case 0x2d: return(0x56); //keypad-
    case 0x2b: return(0x58); //keypad enter
    case 0x2a: return(0x63); //kepad .
    case 0x20: return(0x62); //keypad 0
    case 0b110:return(0x48); //pause
    case 0x2c: return(0x57); //keypad +
    case 0x3c: return(0x2d); //'
    case 0x5f: return(0x2e); //ì
    case 0x30: return(0x27); //0
    case 0x40: return(0x35); // \ //
    case 0x3e: return(0x38); //-
    case 0x3d: return(0x37); //.
    case 0x21: return(0x59); //keypad 1
  //case     : return(0x31);//key between backspace and enter 
    //0x31 key is missing, if you have an american keyboard it should still work with no problem



    
  //else 
  //Serial.println(c, HEX);
  }
  
  return 0;
}
