// Receiver part use servo SG90 and nRF24L01 as receiver/transmitter
// include the library code:
#include <Servo.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
RF24 radio(7, 8); // CE vert, CSN violet
Servo monServomoteur;
// SPI pins
//13 SCK 13 gris
//11 MOSI
//12 MISO
// CE,CSN/SCK/MISO/MOSI 5V tolerant

// setup values
// for radio
const byte address[6] = "00001"; // channel transmitter, must be same than receiver
int timeprocess = 1000; // time to process images at receiver side in ms
// for sonar
/* Constantes pour les broches */
const byte TRIGGER_PIN = 2; // Broche TRIGGER
const byte ECHO_PIN = 3;    // Broche ECHO

/* Constantes pour le timeout */
const unsigned long MEASURE_TIMEOUT = 25000UL; // 25ms = ~8m Ã  340m/s
/* Vitesse du son dans l'air en mm/us */
const float SOUND_SPEED = 340.0 / 1000;
// for servo
int minAngle = 10; // minimum angle of sonar
int maxAngle = 170; // maximum angle of sonar, must be below 180
int deltaAngle = 10 ;// define the step for servo in degrees

// internal variables
float distance_mm;
long measure;
long Variable1;  // Create a variable to have something dynamic to show on the display

/** Fonction setup() */

void caldistance(void) {
  // 1. Lance une mesure de distance en envoyant une impulsion HIGH de 10Âµs sur la broche TRIGGER
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);

  // Mesure le temps entre l'envoi de l'impulsion ultrasonique et son Ã©cho (si il existe)
  measure = pulseIn(ECHO_PIN, HIGH, MEASURE_TIMEOUT);

  //. Calcul la distance Ã  partir du temps mesurÃ©
  distance_mm = measure / 2.0 * SOUND_SPEED;
  Variable1 = distance_mm / 10.0;

  // Affiche les rÃ©sultats en mm, cm et m
  Serial.print(F("Distance: "));
  //  Serial.print(distance_mm);
  // Serial.print(F("mm ("));
  Serial.print(distance_mm / 10.0, 2);
  Serial.println(F("cm, "));
  //Serial.print(distance_mm / 1000.0, 2);
  //Serial.println(F("m)"));

}
void setup() {
  radio.begin();
  radio.openWritingPipe(address); // start radio as transmitter
  radio.setPALevel(RF24_PA_MIN); // min power
  radio.stopListening();

  monServomoteur.attach(9);
  /* Initialise le port sÃ©rie */
  Serial.begin(9600);

  /* Initialise les broches */
  pinMode(TRIGGER_PIN, OUTPUT);
  digitalWrite(TRIGGER_PIN, LOW); // La broche TRIGGER doit Ãªtre Ã  LOW au repos
  pinMode(ECHO_PIN, INPUT);
}

/** Fonction loop() */
void loop() {
  // increase angles
  for (int position = minAngle; position < maxAngle; position = position + deltaAngle) {
    monServomoteur.write(position); // position servo
    caldistance(); // calculate distance
    // create data to be sent
    // distance = 3 left digits in cm
    // angle = 3 right digits in degrees
    Variable1 = (1000 * Variable1 ) + position;
    
        Serial.print("data: ");
        Serial.println(Variable1);
    
    radio.write(&Variable1, sizeof(Variable1)); // send data
    delay(timeprocess); // wait before sending new data, depends on speed of receiver display
  }

  // decrease angle
  for (int position = maxAngle; position > minAngle; position = position - deltaAngle) {
    monServomoteur.write(position);
    caldistance();
    Variable1 = 1000 * Variable1 + position;
    /*
          Serial.print("data: ");
          Serial.println(Variable1);
    */
    radio.write(&Variable1, sizeof(Variable1));
    delay(timeprocess);
  }
}
