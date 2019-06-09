/* Receiver part use ILI9341 display and nRF24L01 as receiver/transmitter

  This is an example on how to use the 2.2" and 2.4" TFT 240x320 SPI ILI9341 display using the Adafruit library.

  ILI9341 TFT SPI display pins for Arduino Uno/Nano:
  This is a 3.3V device so you need to put a level shifter on all 5 data pins.
  An easy way to do this is to use a 2.2K resistor between the data pin on the display to the pin on arduino, and
  another resistor on the side of the display to a 3.3K resistor to ground. This creates a voltage divider to
  convert the 5V output of arduino into a 3V logic. Do this for all 5 data pins.
   LED =   10Ω (or more) resistor to 5V
   SCK =   13 D13
   SDI =   11 D11
   D/C =   9  D9
   RESET = 6 or RST or 3.3V  D6
   CS =    10  D10
   GND =   GND
   VCC =   5V or 3.3V (the display has it's own 3.3V regulator)

  Hardware SPI Pins:
   Arduino Uno   SCK=13, SDA=11
   Arduino Nano  SCK=13, SDA=11
   Arduino Due   SCK=76, SDA=75
   Arduino Mega  SCK=52, SDA=51

  SPI pin names can be confusing. These are the alternative names for the SPI pins:
  MOSI = DIN = R/W = SDO = DI = SI = MTSR = SDA = D1 = SDI
  CS = CE = RS = SS
  DC = A0 = DO = DOUT = SO = MRST
  RESET = RST
  SCLK = CLK = E = SCK = SCL = D0

  Ucglib library: https://github.com/olikraus/ucglib
  Reference Page: https://github.com/olikraus/ucglib/wiki/reference
  List of fonts: https://github.com/olikraus/ucglib/wiki/fontsize
               https://github.com/olikraus/ucglib/wiki/fontgroup

  Font Purpose Description:
  t = Transparent font, use together with UCG_FONT_MODE_TRANSPARENT. t is identical to h but font size is smaller.
  h = All glyphs have common height, use together with UCG_FONT_MODE_TRANSPARENT and UCG_FONT_MODE_SOLID.
  m = All glyphs have common height and width (monospace), use together with UCG_FONT_MODE_TRANSPARENT and UCG_FONT_MODE_SOLID.
  8 = All glyphs have common height and width, use together with UCG_FONT_MODE_TRANSPARENT and UCG_FONT_MODE_SOLID. Height and width are devisible by 8.

  Font Char Set Description:
  f = The font includes all glyphs of the original font (up to 256).
  r = Only glyphs on the range of the ASCII codes 32 to 127 are included in the font.
  n = Only numbers and extra glyphs for writing date and time strings are included in the font.

  Recommended fonts end with _mr or _mf
    _mr lack some special characters but uses less memory
    _mf supports more special characters but uses more memory


  Sketch made by: InterlinkKnight
  Last modification: 01/29/2018
*/
// libraries
#include "Ucglib.h"  // Include Ucglib library to drive the display
#include <nRF24L01.h> //radio library
#include <RF24.h> // radio library

// radio share SCK, MOSI,MISO with display but they are 5Volt tolerant
RF24 radio(7, 8); // CE vert, CSN violet radio receiver

// Create display and define the pins:
Ucglib_ILI9341_18x240x320_HWSPI ucg(9, 10, 6);  // (A0=9, CS=10, RESET=6)
// The rest of the pins are pre-selected as the default hardware SPI for Arduino Uno (SCK = 13 and SDA = 11)
//13 SCK 13 bleu
//11 MOSI jaune
//12 MISO orange

const byte address[6] = "00001";
const float degrad = 0.0174533; // t0o convert degree in radiant angles

// setup values
int minAngle = 10; // minimum angle of sonar
int maxAngle = 170; // maximum angle of sonar, must be below 180
// internal variables
int posX;
int posY;
int coeff;
float maxRange; // define the maximum distance to view on radar display in centimeters
float interval; // define the interval for marks on display, by default maxrange/5
int x;
int y;
int distance = 0;
long measure;
int angle = 0;
char string[10];  // Create a character array of 10 characters


void setup(void)  // Start of setup
{
  maxRange = 200.  ; // max range in cm
  interval = maxRange / 5;// tick interval on screen
  radio.begin();
  radio.openReadingPipe(0, address); // open channel to listen
  radio.startListening();
  /* Initialise le port sÃ©rie */
  Serial.begin(9600);


  // Select a type of text background:
  ucg.begin(UCG_FONT_MODE_SOLID);  // It writes a background for the text. This is the recommended option
  ucg.clearScreen();  // Clear the screen
  // Set display orientation:
  ucg.setRotate270();  // Put 90, 180 or 270, or comment to leave default
  ucg.setColor(1, 0, 0, 0);  // Set color of text background (1,R,G,B)


}  // End of setup

void loop(void)  // Start of loop
{
  if (radio.available()) {
    radio.read(&measure, sizeof(measure)); // get data from radio
    distance = measure / 1000; // distance = 3 left digits in cm
    angle = measure - distance * 1000; // angle = 3 right digits in degrees
    Serial.print("data: ");
    Serial.println(measure);
    Serial.print("angle: ");
    Serial.println(angle);
    Serial.print("distance: ");
    Serial.println(distance);

    // Draw horizontal line:
    ucg.setColor(255, 255, 255);  // Set color (0,R,G,B)
    ucg.drawHLine(0, 50, 320);  // Start from left pixel (x,y,lenght)
    ucg.setColor(0, 255, 255, 0);  // Set color (0,R,G,B) yellow
    ucg.setFont(ucg_font_inr16_mr);  // Set font
    ucg.setPrintPos(0, 40); // Set position (x,y) for distance text
    ucg.print("D:");  // Print distance text
    ucg.setPrintPos(25, 40); // Set position (x,y) for distance
    if(distance < 10)  // If distance is less than 10...
  {
    // Fill the other digit with background color:
    ucg.print(" ");
  }
  if(distance < 100)  // If Variable1 is less than 100...
  {
    // Fill the other digit with background color:
    ucg.print(" ");
  }
    ucg.print(distance);  // Print distance

    // prepare for distance and range
    dtostrf(maxRange, 3, 0, string);
    ucg.setPrintPos(75, 40); // Set position (x,y)
    ucg.print("maxR:");  // Print text
    ucg.setPrintPos(145, 40); // Set position (x,y)
    ucg.print(string);  // Print text or value maxRange
    // prepare for ticks
    dtostrf(interval, 3, 0, string);
    ucg.setPrintPos(210, 40); // Set position (x,y)
    ucg.print("Div:");  // Print text or value
    ucg.setPrintPos(260, 40); // Set position (x,y)
    ucg.print(string);  // Print text or value

    // print X wher distance is
    ucg.setColor(0, 255, 255, 0);  // Set color (0,R,G,B)
    ucg.setFont(ucg_font_5x8_mr);;
    coeff = int(200.0 * distance / maxRange);
    posX = 160 - (cos(angle * degrad) * coeff);
    posY = 240 - (sin(angle * degrad) * coeff);
    ucg.setPrintPos(posX, posY); // Set position (x,y)
    ucg.print("X");  // Print text or value


    // convert data in position x, y on screen
    x = 160 - (cos(angle * degrad) * 200);
    y = 240 - (sin(angle * degrad) * 200);

    ucg.setColor(100, 100, 100);  // Set color (0,R,G,B)
    ucg.drawLine(160, 240, x, y);  // Start from boottom middle pixel to x, y in low white

    // write ticks
    ucg.setColor(0, 255, 255, 255);  // Set color (0,R,G,B) white

    // print 5 ticks (default)
    ucg.setFont(ucg_font_profont10_mr);
    for (int i = 1; (i - 1) * interval < maxRange; i++) {
      coeff = int(200.0 * i * interval / maxRange);
      posX = 160 - (cos(angle * degrad) * coeff);
      posY = 240 - (sin(angle * degrad) * coeff);
      //dtostrf(coeff, 3, 0, string);
      ucg.setPrintPos(posX, posY); // Set position (x,y)
      ucg.print("+");  // Print text or value
    }

    ucg.setColor(255, 255, 255);  // Set color (0,R,G,B)
    ucg.drawLine(posX, posY, x, y);  // Start from pos X pos Y up to maxrange in heavy white
    if ((angle == maxAngle) or (angle == minAngle) ) { // clear the screen when reach min or max angle
      ucg.clearScreen();  // Clear the screen
    };
  }
}  // End of loop
