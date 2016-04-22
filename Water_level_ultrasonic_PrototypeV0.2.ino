#include <LiquidCrystal.h>
#include <EEPROM.h>
#include <Wire.h>
#include <OneWire.h>
// Version 0.2 by Bob
// Includes water hight and Temperature compensated
// EEPROM stuff
//******************************************************
int address = 0                 ;      //start of EEPROM adress
byte nulstand;

// Rangefinder stuff
//******************************************************
const int echo = 6              ;
const int trigger = 7           ;
long Distance                   ;      // Global Var for distance
int offset = 0                  ;      // 0 cm 
long lastrangefindertime = 0    ;      // the last time the range was checked
long refresh_rangefinder = 5000 ;      // at least 1000 ms before next rangefinder call or you will break it
boolean newDist = FALSE         ;

//DS18B20 stuff
//******************************************************
#define MAX_DS1820_SENSORS 1
int DS18S20_Pin = 10            ;      //DS18S20 Signal pin on digital 10
OneWire ds(DS18S20_Pin)         ;      // announce on which digital pin the onewire is connected
long refresh_temperature = 4000 ;      // at least 5000 ms before next temperature refresh
long lasttemperaturetime = 0    ;      // the last time the temperature was checked
float Temperature = 15          ;      // Define global temperature var and set it to a default
byte addr[MAX_DS1820_SENSORS][8];
char buf[20]                    ;
boolean newTemp = FALSE         ;


// Button 1 and 2 stuff
//******************************************************
const int button1 = 8;
const int button2 = 9;
int buttonState1;             // the current reading from the input pin
int lastButtonState1 = LOW;   // the previous reading from the input pin
int buttonState2;             // the current reading from the input pin
int lastButtonState2 = LOW;   // the previous reading from the input pin
// the following variables are long's because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
long lastDebounceTime = 0;    // the last time the output pin was toggled
long debounceDelay = 50;      // the debounce time; increase if the output flickers

//Menucontext stuff
//******************************************************
int menucontext = 1 ;
// 1 : main screen
// 2 : program new offset to memory


//LCD stuff
//******************************************************
/* LiquidCrystal Library - display() and noDisplay() The LiquidCrystal library works with all LCD displays that are compatible with the
   Hitachi HD44780 driver. There are many of them out there, and you  can usually tell them by the 16-pin interface.
 * LCD RS pin to digital pin 12  * LCD Enable pin to digital pin 11
 * LCD D4 pin to digital pin 5   * LCD D5 pin to digital pin 4
 * LCD D6 pin to digital pin 3   * LCD D7 pin to digital pin 2
 * LCD R/W pin to ground         * 10K resistor: ends to +5V and ground and wiper to LCD VO pin   */
// set up the LCD's number of columns and rows:
#define LCD_WIDTH 16
#define LCD_HEIGHT 2
// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);


//SETUP
//******************************************************
void setup() {
//DEBUG only
// initialize serial communication at 9600 bits per second:
//Serial.begin(9600);
  
  //LCD SETUP
  lcd.begin(LCD_WIDTH, LCD_HEIGHT);
  welcomescreen()                 ;
  
//RANGEFINDER SETUP
  pinMode (trigger, OUTPUT);  //Define output pin - was important addition to make it work
  pinMode (echo, INPUT)    ;  //Define input pin - was important addition to make it work
  lastrangefindertime = millis();
  
//BUTTON SETUP
  pinMode (button1, INPUT);
  pinMode (button2, INPUT);

//EEPROM SETUP
// EEPROM.write(address, 40); // 40 cm offset for debug only hardcoded
   nulstand = EEPROM.read(address);   // read installation standard hight from the current address of the EEPROM

//DS18S20 Temperature chip io for address 
  if (!ds.search(addr[0]))
  {
    ds.reset_search();
    delay(1000);
    return;
  }

//CLOSURE OF SETUP
  delay(2000);                //Keep welcome text up
  mainscreen();               //prepare mainscreen
}


//MAIN LOOP
//******************************************************
void loop() {
  if ((millis() - lasttemperaturetime) > refresh_temperature) {
    Temperature = getTemp();
    newTemp = TRUE ;
    lasttemperaturetime = millis();  //update last rangefindertime
  }
 
  if ((millis() - lastrangefindertime) > refresh_rangefinder) {
    Distance = range();              //call rangefinder
    newDist = TRUE ;
    lastrangefindertime = millis();  //update last rangefindertime
  }
  
  if (menucontext == 1) {            //Are we in the main menu and not in submenus?
     if(newTemp == TRUE){
        PrintTemp();                 //Print the found temperature on the LCD 
        newTemp = FALSE; }           
     if(newDist == TRUE) {   
        PrintRange();                //Print the found range on the LCD 
        newDist = FALSE; }                  
 }
 
  buttonState1 = readbutton(button1);
  buttonState2 = readbutton(button2);


 if ( (menucontext == 1) && (buttonState1 == HIGH) && (lastButtonState1 == LOW ) ) { 
       PrintNewOffset();
       menucontext = 2;
      {goto NextLoop;} 
 }
 
 if ( (menucontext == 1) && (buttonState2 == HIGH) && (lastButtonState2 == LOW ) ) { 
       gimmicscreen();
       delay(2500);
       //menucontext = 2;
       mainscreen();
      {goto NextLoop;}       
 }

 if ( (menucontext == 2) && (buttonState1 == HIGH)&& (lastButtonState1 == LOW ) ) { 
       //NEE keuze
       mainscreen();
       menucontext = 1;
      {goto NextLoop;}
 }

 if ( (menucontext == 2) && (buttonState2 == HIGH)&& (lastButtonState2 == LOW ) ) { 
       //JA keuze
       EEPROM.write(address, Distance); //Write new offset to EEPROM
       nulstand=Distance;                //adjust offset for running program
       mainscreen();
       menucontext = 1;
      {goto NextLoop;}
 }


 
  NextLoop:
  lastButtonState1 = buttonState1;
  lastButtonState2 = buttonState2;  
}
//END MAIN LOOP*****************************************


//SCREENS
//******************************************************
void welcomescreen() {
      lcd.setCursor(0, 0);
      lcd.print("<<Hoogte-meter>>");
      lcd.setCursor(0, 1);
      lcd.print("<< versie 0.2 >>");
}

void gimmicscreen() {
      lcd.setCursor(0, 0);
      lcd.print("BobdeBouwer job!");
      lcd.setCursor(0, 1);
      lcd.print("in Arduino Mini5");
}

void mainscreen() {
     lcd.setCursor(0, 0)             ;
     lcd.print("Afst:       Tmp")    ;
     lcd.print((char)223)            ; // the degree symbol..
     lcd.setCursor(0, 1)             ;
     lcd.print("Hoog:           ")   ;
}

void PrintTemp(){
      lcd.setCursor(12, 1);
      lcd.print(Temperature);
}

void PrintRange() {
      lcd.setCursor(5, 0);       // position 5 line number 0
      lcd.print(Distance);
      lcd.print("   ");
      lcd.setCursor(9, 0);       // position 8 line number 0
      lcd.print("Cm");
      lcd.setCursor(5, 1);       // position 8 line number 1
      lcd.print(nulstand- Distance);
      lcd.print("   ");
      lcd.setCursor(9, 1);       // position 8 line number 0
      lcd.print("Cm");
}

void PrintNewOffset() {
      lcd.setCursor(0, 0);
      lcd.print("Nulpunt->Memory?");
      lcd.setCursor(0, 1);
      lcd.print("<nee>       <ja>");
}

//RANGEFINDER
//******************************************************
int range() {
  long start;
//  long watchdog;
  long elapsed;
  float speedofsound;
  float dist ;
  // trigger the rangefinder
  digitalWrite(trigger, HIGH);
  delay(1);
  digitalWrite(trigger, LOW);

  while (!digitalRead(echo)); // wait for the echo to start
  start = micros();           // start timer
  while (digitalRead(echo));  // wait for echo to return
  elapsed = micros() - start; // stop timer

  // The speed of sound is 331 m/s or 29 microseconds per centimeter at zero degrees
  // The ping travels out and back, so to find the distance of the
  // object we take half of the distance travelled.
  speedofsound = 331.3 + (0.606 * Temperature); 
  dist = (elapsed * speedofsound /20000)+ offset ;
return dist;
//  return ( elapsed / (58*(25/Temperature)) / 2 ) ; // convert to centimeters
}

//TEMPERATURE
//******************************************************
float getTemp() {
  int HighByte, LowByte;
  float TReading;
  byte i, sensor;
  byte present = 0;
  byte data[12];
  for ( sensor = 0; sensor < MAX_DS1820_SENSORS; sensor++ )
  {
    if ( OneWire::crc8( addr[sensor], 7) != addr[sensor][7])
    {
      lcd.setCursor(0, 0);
      lcd.print("CRC not valid");
      delay (8000);
      return 99;
    }
    if ( addr[sensor][0] != 0x10)
    {
      lcd.setCursor(0, 0);
      lcd.print("No DS18S20 chips");
      delay (8000);
      return 98;
    }
    // IF we came here we have a good reading from a DS18B20 sensor
    ds.reset();
    ds.select(addr[sensor]);
    ds.write(0x44, 1);        // start conversion, with parasite power on at the end
    delay(900);               // maybe 750ms is enough, maybe not

    // we might do a ds.depower() here, but the reset will take care of it.
    present = ds.reset();
    ds.select(addr[sensor]);
    ds.write(0xBE);           // Read Scratchpad

    for ( i = 0; i < 9; i++)  // we need 9 bytes
    {                         
      data[i] = ds.read();
    }

    LowByte = data[0];
    HighByte = data[1];
    TReading = ((HighByte << 8) | LowByte);
  }
  return TReading /2 ;
}

int readbutton (int button) {
    start:
    int reading1 = digitalRead(button);
    delay(debounceDelay);
    int reading2 = digitalRead(button);
    if (reading2 != reading1) {
         if (debounceDelay < 150) {        //put a max on extending delays    
             debounceDelay = debounceDelay +10 ;}
      {goto start;} }                      //Again until we have equal readings, to help we keep adding 10 miliseconds to delaytime whenever we have a difference
    return reading2;   
 }



