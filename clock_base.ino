/*******************************************************************************************
 * /                        Arduino Clock with DS 1307, Rocker Switch
 * /                             3 Push Buttons, and 20x4 LCD 
 * /
 * /                            Garth Vander Houwen, July. 2016
 * /
/********************************************************************************************/

#define build 1
#define revision 5

#include <LiquidCrystal_I2C.h>       // library for I@C interface
#include <RTClib.h>                  // Realtime Clock 

// pin configuration for LCM1602 interface module
// set the LCD I2C_ADDR to 0x27 for a 20 chars 4 line display
#define I2C_ADDR  0x27
#define RS_pin    0                  
#define RW_pin    1
#define EN_pin    2
#define BACK_pin  3
#define D4_pin    4
#define D5_pin    5
#define D6_pin    6
#define D7_pin    7

// Set the pins on the I2C chip used for LCD connections:
//                    addr, en,rw,rs,d4,d5,d6,d7,bl,blpol
LiquidCrystal_I2C lcd(I2C_ADDR, EN_pin, RW_pin, RS_pin, D4_pin, D5_pin, D6_pin, D7_pin, BACK_pin, POSITIVE);

// Custom Big Number Characters
byte x10[8] = {0x00,0x00,0x00,0x00,0x00,0x07,0x07,0x07};   
byte x11[8] = {0x00,0x00,0x00,0x00,0x00,0x1C,0x1C,0x1C};
byte x12[8] = {0x00,0x00,0x00,0x00,0x00,0x1F,0x1F,0x1F};
byte x13[8] = {0x07,0x07,0x07,0x07,0x07,0x1F,0x1F,0x1F};
byte x14[8] = {0x1C,0x1C,0x1C,0x1C,0x1C,0x1F,0x1F,0x1F};
byte x15[8] = {0x1C,0x1C,0x1C,0x1C,0x1C,0x1C,0x1C,0x1C};
byte x16[8] = {0x07,0x07,0x07,0x07,0x07,0x07,0x07,0x07};
byte x17[8] = {0x00,0x00,0x0E,0x0E,0x0E,0x00,0x00,0x00};

// The different states of the clock
enum states
{
    SHOW_SPLASH,         // Displays the Splash Screen
    SHOW_TIME,           // Displays the time and date
    SHOW_CUSTOM_TEXT,    // Displays the time and custom text from the serial port
    SHOW_SYSTEM_INFO,    // Displays System Information
};

// Holds the current state of the clock
states state;  

// Time and date variables
const char* dayName[] = { "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT" };
const char* monthName[] = { "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC" };
String ampm;
int hr_24, hr_12;
RTC_Millis RTC;
//RTC_DS1307 RTC;
DateTime now;

// Buttons
int button1Pin = 2;
int button2Pin = 3;
int button3Pin = 4;

// Time Display options
bool showSeconds = false;
byte row = 0,col = 0;

// Setup
// run once
void setup() {

    // Used to type in characters in custom text mode
    Serial.begin(9600); 
//    RTC.begin();
    RTC.begin(DateTime(__DATE__, __TIME__));
    
    // Setup Button Pins
    pinMode(button1Pin, INPUT_PULLUP);
    pinMode(button2Pin, INPUT_PULLUP);
    pinMode(button3Pin, INPUT_PULLUP);

    lcd.begin(20,4); 
    // setup LCD  for 20 columns and 4 rows
    lcd.createChar(0, x10);                      // digit piece
    lcd.createChar(1, x11);                      // digit piece
    lcd.createChar(2, x12);                      // digit piece
    lcd.createChar(3, x13);                      // digit piece
    lcd.createChar(4, x14);                      // digit piece
    lcd.createChar(5, x15);                      // digit piece
    lcd.createChar(6, x16);                      // digit piece
    lcd.createChar(7, x17);                      // digit piece (colon)
//    if(!RTC.isrunning())
//    {
//        RTC.adjust(DateTime(__DATE__,__TIME__));
//    }
    
    state = SHOW_SPLASH;  // Initial state
}


// Loop
// run over and over again
void loop() {                   

    //timeRef = millis();
    // Get the time from the RTC
    now = RTC.now();

    // Uses the current state to decide what to process
    switch (state)
    {
        case SHOW_SPLASH:
            showSplash();
            delay(5000);
            lcd.clear();
            state = SHOW_TIME;
            break;
        case SHOW_TIME:
            showTime();
            showDate();
            break;
        case SHOW_CUSTOM_TEXT: 
            showSerialText();
            showTime();
            break;
        case SHOW_SYSTEM_INFO:   
            showSystemInfo();
            break;
    }

    // variables to hold the pushbutton states
    int button1State, button2State, button3State;  
    
    // Read Button States
    button1State = digitalRead(button1Pin);
    button2State = digitalRead(button2Pin);
    button3State = digitalRead(button3Pin);

    // Handle Button Presses
    if (button1State == LOW || button2State == LOW || button3State == LOW)
    {
        // Button 1 - Red
        if (button1State == LOW) {
          // Cycle to the next mode
          switch (state)
          {
              case SHOW_TIME:
                  state = SHOW_SYSTEM_INFO;
                  Serial.println("Button 1 Pressed - Mode Changed to SHOW_SYSTEM_INFO");
                  break;
              case SHOW_SYSTEM_INFO:
                  lcd.clear();
                  lcd.setCursor(0, 0);
                  lcd.print("Awaiting Serial Text"); 
                  state = SHOW_CUSTOM_TEXT;
                  Serial.println("Button 1 Pressed - Mode Changed to SHOW_CUSTOM_TEXT");
                  break;
              case SHOW_CUSTOM_TEXT:
                  state = SHOW_SPLASH;
                  Serial.println("Button 1 Pressed - Mode Changed to SHOW_SPLASH");
                  break;
          }
          delay(500);
        }
        // Button 2 - Black
        if (button2State == LOW) { 
          Serial.println("Button 2 Pressed - Black");
          delay(500);
        }
        // Button 3 - White
        if (button3State == LOW) { 
          Serial.println("Button 3 Pressed - White");
          delay(500);
        }
    }
}

// Show System Information Software Version
// and available RAM
void showSplash()
{  
  lcd.setCursor(0, 0);
  lcd.print("  GarthVH.com Labs  "); 
  lcd.setCursor(0, 1);
  lcd.print(" Arduino Uno DS1307 ");
  lcd.setCursor(0, 2);
  lcd.print("  Custom LCD Clock  "); 
  lcd.setCursor(0, 3); 
  lcd.print(F("  [V")); 
  lcd.print(build); 
  lcd.print(F(".")); 
  lcd.print(revision);
  lcd.print(F(" RAM: "));  
  lcd.print(freeRam()); 
  lcd.print(F("B]")); 
}


// Show System Information Software Version
// and available RAM
void showSystemInfo()
{  
  lcd.setCursor(0, 0);
  lcd.print("Arduino DS1307 Clock"); 
  lcd.setCursor(0, 1);
  lcd.print(" With Rocker Switch ");
  lcd.setCursor(0, 2);
  lcd.print("3 Buttons & 20X4 LCD"); 
  lcd.setCursor(0, 3); 
  lcd.print(F("  [V")); 
  lcd.print(build); 
  lcd.print(F(".")); 
  lcd.print(revision);
  lcd.print(F(" RAM: "));  
  lcd.print(freeRam()); 
  lcd.print(F("B]")); 
}

// Show Text received
// over serial connection
void showSerialText()
{
    // when characters arrive over the serial port...
    if (Serial.available()) {
      // wait a bit for the entire message to arrive
      delay(100);
      // clear the top row
      lcd.setCursor(0,0); 
      lcd.print("                    ");
      lcd.setCursor(0,0); 

      String txtMsg;
      // add any incoming characters to the String:
      while (Serial.available() > 0) {
        char inChar = Serial.read();
        txtMsg += inChar;

        if(txtMsg.length() < 21)
        {
          lcd.write(inChar);
        }
      } 
    }
}

// Displays the current date in the top row 
// e.g. SAT 04 JAN 2014
void showDate()
{
   now = RTC.now();
   lcd.setCursor(0, 0); 
   String dateString = "* " + String(dayName[now.dayOfTheWeek()]) + " " + String(monthName[now.month()-1]) + " " + String(now.day()) + ", " + String(now.year()) + " *";
   lcd.print(dateString);
}

// displays time in large 3 row letters
// starting on the second row
// 10:59 AM
void showTime()
{   
    now = RTC.now();
    lcd.setCursor(0, 0);
    
    // Set Times for 12 Hour Clock
    // Set AMPM Indicator Values
    hr_24 = now.hour();
    ampm = "AM";
    if (hr_24 == 0) { // 12 AM
      hr_12 = 12;
    }
    else if(hr_24 == 12) { // 12 PM
      hr_12 = hr_24;
      ampm = "PM";
    }
    else if (hr_24 > 12) { // 13-23 PM
       hr_12 = (hr_24 - 12);
       ampm = "PM";
    }
    else { // 1-11 AM
      hr_12 = hr_24;
    }
    if(!showSeconds){
      lcd.setCursor(18, 2);
      lcd.print(ampm);
    }

   // Display the time using bigNumber
   row = 1;
   col = 3;
   if (hr_12 < 10) {  
      // bigNumber(0,row,col);
      //col = col + 3;
      bigNumber(hr_12,row,col);
      col = col + 3;
   }
   else {
     bigNumber(getDigit(hr_12,2) ,row,col);
     col = col + 3;
     bigNumber(getDigit(hr_12,1) ,row,col);
     col = col + 3;
   }
   bigNumber(11,row,col);
   col = col + 2;
   if (now.minute() < 10) {  
      bigNumber(0,row,col);
      col = col + 3;
      bigNumber(now.minute(),row,col);
      col = col + 2;
   }
   else {
     bigNumber(getDigit(now.minute(),2) ,row,col);
     col = col + 3;
     bigNumber(getDigit(now.minute(),1) ,row,col);
     
   }
   if(showSeconds){
     col = col + 2;
     bigNumber(11,row,col);
     col = col + 2;
     if (now.second() < 10) {  
        bigNumber(0,row,col);
        col = col + 3;
        bigNumber(now.second(),row,col);
     }
     else {
       bigNumber(getDigit(now.second(),2) ,row,col);
       col = col + 3;
       bigNumber(getDigit(now.second(),1) ,row,col);
     }
   }
}

// Position a custom big font digit
// position number 'num' starting top left at row 'r', column 'c'
void bigNumber(byte num, byte r, byte c) {
    lcd.setCursor(c,r);
    switch(num) {
      case 0: lcd.write(byte(2)); lcd.write(byte(2)); 
              lcd.setCursor(c,r+1); lcd.write(byte(5)); lcd.write(byte(6)); 
              lcd.setCursor(c,r+2); lcd.write(byte(4)); lcd.write(byte(3)); break;
            
      case 1: lcd.write(byte(0)); lcd.write(byte(1)); 
              lcd.setCursor(c,r+1); lcd.print(" "); lcd.write(byte(5));
              lcd.setCursor(c,r+2); lcd.write(byte(0)); lcd.write(byte(4)); break;
            
      case 2: lcd.write(byte(2)); lcd.write(byte(2)); 
              lcd.setCursor(c,r+1); lcd.write(byte(2)); lcd.write(byte(3)); 
              lcd.setCursor(c,r+2); lcd.write(byte(4)); lcd.write(byte(2)); break;  
            
      case 3: lcd.write(byte(2)); lcd.write(byte(2)); 
              lcd.setCursor(c,r+1); lcd.write(byte(0)); lcd.write(byte(3)); 
              lcd.setCursor(c,r+2); lcd.write(byte(2)); lcd.write(byte(3)); break;  
            
      case 4: lcd.write(byte(1)); lcd.write(byte(0)); 
              lcd.setCursor(c,r+1); lcd.write(byte(4)); lcd.write(byte(3)); 
              lcd.setCursor(c,r+2); lcd.print(" "); lcd.write(byte(6)); break;  
            
      case 5: lcd.write(byte(2)); lcd.write(byte(2)); 
              lcd.setCursor(c,r+1); lcd.write(byte(4)); lcd.write(byte(2)); 
              lcd.setCursor(c,r+2); lcd.write(byte(2)); lcd.write(byte(3)); break;  
      case 6: lcd.write(byte(1)); lcd.print(" ");     
              lcd.setCursor(c,r+1); lcd.write(byte(4)); lcd.write(byte(2)); 
              lcd.setCursor(c,r+2); lcd.write(byte(4)); lcd.write(byte(3)); break;  

      case 7: lcd.write(byte(2)); lcd.write(byte(2));
              lcd.setCursor(c,r+1); lcd.print(" "); lcd.write(byte(6)); 
              lcd.setCursor(c,r+2); lcd.print(" "); lcd.write(byte(6)); break;  

      case 8: lcd.write(byte(2)); lcd.write(byte(2)); 
              lcd.setCursor(c,r+1); lcd.write(byte(4)); lcd.write(byte(3)); 
              lcd.setCursor(c,r+2); lcd.write(byte(4)); lcd.write(byte(3)); break;  
   
      case 9: lcd.write(byte(2)); lcd.write(byte(2)); 
              lcd.setCursor(c,r+1); lcd.write(byte(4)); lcd.write(byte(3)); 
              lcd.setCursor(c,r+2); lcd.print(" "); lcd.write(byte(6)); break;  

       case 11: lcd.setCursor(c,r+1); lcd.write(byte(7)); lcd.setCursor(c,r+2); lcd.write(byte(7)); break; 
    } 
}

// Count the number of digits in a number
// used to draw the individual big numbers
byte countDigits(int num){
  byte count=0;
  while(num){
    num=num/10;
    count++;
  }
  return count;
}

// Get a digit of a number 
// used to draw the individual big numbers one at a time
int getDigit(unsigned int number, int digit) {
    for (int i=0; i<digit-1; i++) { 
      number /= 10; 
    }
    return number % 10;
}

// FREERAM: Returns the number of bytes currently free in RAM  
int freeRam(void) {
  extern int  __bss_end, *__brkval; 
  int free_memory; 
  if((int)__brkval == 0) {
    free_memory = ((int)&free_memory) - ((int)&__bss_end); 
  } 
  else {
    free_memory = ((int)&free_memory) - ((int)__brkval); 
  }
  return free_memory; 
}

