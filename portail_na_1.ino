// pins
int   countrot  = PD2; // counter of rotation
int   trig      = PD3; // change mode
int   EMA       = PD5; // pwm motor
int   IN1       = PD6; // command motor 1
int   IN2       = PD7; // command motor 2
int   sensor    = A7; // overloadsensor

int lowMotor=0;
int maxrotationopen, maxrotationclose, calibre, inrotation;
int halfopen, halfclose;
int opendelay, closedelay, minmotorpos, maxmotorpos, minmotor, delayloop;
int countrotation = 0;
int new_pb, old_pb = 1;
int new_pb1, old_pb1 = 1;
int stateportail = 0; // 0 repos ferme, 1 ouvre, 2 repos ouvert, 3 ferme,4 ouvert a moitie 5 ferme a moitie
double meas, mesa0, meas1, meas2, measold, maxVCC ;
int varCompteur = 0; // La variable compteur

double Voltage = 0;
double Amps = 0;
int dg = 0; // 1 for left portail, 0 for right, seen from inside court
int mVperAmp;
int ACSoffset;

// for debug, normally 1
double coefd = 1;


void pressedcountrot (void) // each time there is a couner increase
{
  if (stateportail == 3) {//ferme
    countrotation++;
    if (countrotation == maxrotationclose) {
      Serial.print("min rotation reached! completement ferme: ");
      Serial.println(countrotation);
      stateportail = 0;
      actionportail();
    }
    if (countrotation == minmotorpos) {
      Serial.print("min motor reached!: ");
      Serial.println( minmotorpos, DEC);
      Serial.print("countrotation : ");
      Serial.println(countrotation);
      analogWrite(EMA, minmotor);
      lowMotor=1;
    }
  }
  if (stateportail == 1) {//ouvre
    countrotation++;
    if (countrotation == maxrotationopen) {
      Serial.print("maxrotation reached! completement ouvert!: ");
      Serial.println(maxrotationopen);
      stateportail = 2;
      actionportail();
    }
    if (countrotation == maxmotorpos) {
      Serial.print("max motor reached!: ");
      Serial.println(maxmotorpos);
      Serial.print("countrotation : ");
      Serial.println(countrotation);
      analogWrite(EMA, minmotor);
      lowMotor=1;
    }
  }
  Serial.println(countrotation);
}

void ouvreportail()
{
  varCompteur = 1;
  inrotation = 1;
  digitalWrite(IN1, 1);
  digitalWrite(IN2, 0);
  analogWrite(EMA, 255);
}
void fermeportail()
{
  varCompteur = 1;
  inrotation = 1;
  digitalWrite(IN1, 0);
  digitalWrite(IN2, 1);
  analogWrite(EMA, 255);
}
void reposportail() {
  digitalWrite(IN1, 0);
  digitalWrite(IN2, 0);
  analogWrite(EMA, 0);
  inrotation = 0;
  varCompteur = 0;
}

void actionportail() {// 0 repos ferme, 1 ouvre, 2 repos ouvert, 3 ferme,4 ouvert a moitie 5 ferme a moitie
  switch (stateportail) {
    case 0: {
        Serial.println("repos ferme!");
        countrotation = 0;
        reposportail();
        break;
      }
    case 1: {
        Serial.println("ouvre!");
        delay(opendelay);
        ouvreportail();
        break;
      }
    case 2: {
        Serial.println("repos ouvert!");
        reposportail();
        countrotation = 0;
        break;
      }
    case 3: {
        Serial.println("ferme!");
        delay(closedelay);
        fermeportail();
        break;
      }
    case 4: {
        if (countrotation < halfopen)
        {
          Serial.println("a moitie ouvert!");
          reposportail();
          countrotation = maxrotationclose - countrotation;
        }
        else {
          Serial.println(" a moitie ouvert et ferme!");
          stateportail = 3;
          fermeportail();
          countrotation = maxrotationclose - countrotation;
        }
        break;
      }
    case 5: {
        if (countrotation < halfclose) {
          Serial.println("a moitie ferme!");
          reposportail();
          countrotation = maxrotationopen - countrotation;
        }
        else {
          Serial.println(" a moitie ferme et ouvre!");
          stateportail = 1;
          ouvreportail();
          countrotation = maxrotationclose - countrotation;
        }
        break;
      }
  }
}

void triggeraction (void) { // 0 repos ferme, 1 ouvre, 2 repos ouvert, 3 ferme,4 ouvert a moitie, 5 ferme a moitie
  // run each time action button is pressed
  digitalWrite(LED_BUILTIN, LOW);
  switch (stateportail) {
    case 0: {
        stateportail = 1;
        break;
      }
    case 1: {
        stateportail = 4;
        break;
      }
    case 2 : {
        stateportail = 3;
        break;
      }
    case 3: {
        stateportail = 5;
        break;
      }
    case 4: {
        stateportail = 3;
        break;
      }
    case 5: {
        stateportail = 1;
        break;
      }
  }
  Serial.print("new stateportail: ");
  Serial.println( stateportail);
  actionportail();
}

void anaread ()
{ int
  meas0 = analogRead(sensor); // Converts and read the analog input value (value from 0.0 to 1.0)
  delay(10); // 10 ms
  meas1 = analogRead(sensor); // Converts and read the analog input value (value from 0.0 to 1.0)
  delay(10); // 10 ms
  meas2 = analogRead(sensor); // Converts and read the analog input value (value from 0.0 to 1.0)
  meas = (meas0 + meas1 + meas2) / 3;
  if ((meas - measold > 5) || (measold - meas > 5)) {
    //delay(500);
    //Serial.print("measure = : ");
    //Serial.println(meas * (5.0 / 1023.0));
    Voltage = (meas / 1024.0) * 5000; // Gets you mV
    Serial.print("mV = "); // shows the voltage measured
    Serial.println(Voltage );

    measold = meas;

    if (Voltage < maxVCC) {
      delay(100); // 100 ms
      meas0 = analogRead(sensor); // Converts and read the analog input value (value from 0.0 to 1.0)
      delay(10); // 10 ms
      meas1 = analogRead(sensor); // Converts and read the analog input value (value from 0.0 to 1.0)
      delay(10); // 10 ms
      meas2 = analogRead(sensor); // Converts and read the analog input value (value from 0.0 to 1.0)
      meas = (meas0 + meas1 + meas2) / 3;
      Voltage = (meas / 1024.0) * 5000; // Gets you mV

      if (Voltage < maxVCC) {
        digitalWrite(LED_BUILTIN, HIGH);
        Serial.print("overdrive : ");
        Serial.println(maxVCC);
        Serial.print("under maxVCC: ");
        Serial.println(Voltage);

        if (calibre == 2) {
          maxrotationclose = countrotation;
          halfclose = int(0.5 * maxrotationclose);
          minmotorpos = int(0.9 * maxrotationclose);
          //minmotorpos = maxrotationclose - 1;
          stateportail = 0;
          actionportail();
          calibre = 0;
          Serial.print("calibration maxrotationoclose: ");
          Serial.println(maxrotationclose);
        }
        if (calibre == 1) {
          maxrotationopen = countrotation;
          //minmotorpos = int(0.1 * maxrot
          halfopen = int(0.5 * maxrotationopen);
          maxmotorpos = int(0.9 * maxrotationopen);
          stateportail = 2;
          actionportail();
          calibre = 2;
          Serial.print("calibration maxrotationopen: ");
          Serial.println(maxrotationopen);
        }
        if (stateportail == 1) {
          Serial.println("overdrive with a moitie ouvert!");
          Serial.print("countrotation : ");
          Serial.println(countrotation);
          stateportail = 4;
          countrotation = maxrotationclose - countrotation;
          reposportail();
        }
        if (stateportail == 3) {
          Serial.println("overdrive with a moitie ferme!");
          Serial.print("countrotation : ");
          Serial.println(countrotation);
          stateportail = 5;
          countrotation = maxrotationclose - countrotation;
          reposportail();
        }
      }
    }
  }
}
void setup() {

  minmotor = 150; // value when approaching end of run
  Serial.begin(9600);
  Serial.println(dg);
  if (dg == 1) {
    Serial.println("***** portail gauche*********");
    closedelay = 1000;
    opendelay = 0; // 0 second
    maxVCC = 600;
  }
  else {
    Serial.println("***** portail droit*********");
    opendelay = 1000; // 1.5 second
    closedelay = 0;
    maxVCC = 2500;
  }
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode (countrot, INPUT_PULLUP);
  pinMode (trig, INPUT_PULLUP);
  pinMode (EMA, OUTPUT);
  pinMode (IN1, OUTPUT);
  pinMode (IN2, OUTPUT);

  Serial.println("moniteur pret");
  countrotation = 0;
  maxrotationopen = 500;
  maxrotationclose = 500;
  minmotorpos = int(0.8 * maxrotationclose);
  maxmotorpos = int(0.8 * maxrotationopen);
  Serial.print("min motor: ");
  Serial.println(minmotorpos);
  Serial.print("max motor: ");
  Serial.println( maxmotorpos);

  Serial.print("min voltage overdrive: ");
  Serial.print(maxVCC);
  Serial.println("mV");

  Serial.print("opendelay ms: ");
  Serial.println(opendelay);
  Serial.print("closedelay ms: ");
  Serial.println(closedelay);
  stateportail = 0;
  Serial.println("completement ferme!");
  digitalWrite(IN1, 0);
  digitalWrite(IN2, 0);
  analogWrite(EMA, 0);
  calibre = 1;
  inrotation = 0;
  measold = 0;
  delayloop = 100;
}

void loop() {
  /// icurrent read after 1000ms
  if (varCompteur == 0 and inrotation > 0 and lowMotor==0 ) {
    anaread();
  }
  if (varCompteur > 0 ) {
    varCompteur++;
  }
  if (varCompteur > 1000.0 / delayloop) { //1000ms
    varCompteur = 0;
  }
  // stop rotation after 25 second
  if (inrotation > 0 ) {
    inrotation++;
  }
  if (inrotation > 20000.0 * coefd / delayloop) { //25 second =20000
    inrotation = 0;
    Serial.println("stop motor after 25s");
    if (stateportail == 1) {
      stateportail = 2;
    }
    if (stateportail == 3) {
      stateportail = 0;
    }
    actionportail();
  }
  // count rotation
  if (inrotation > 0 ) {
    new_pb = digitalRead(countrot);
    if ((new_pb == 0) && (old_pb == 1)) {
      //delay(10);//delay(10); // 10 ms
      //new_pb = digitalRead(countrot);
      //if (new_pb == 0) {
      //Serial.println("count rotation pressed");
      pressedcountrot();
      //}
    }
    old_pb = new_pb;
  }

  // change mode by pressing remote controller
  new_pb1 = digitalRead(trig);
  //Serial.println(new_pb1);
  if ((new_pb1 == 0) && (old_pb1 == 1)) {
    Serial.println(" action pressed!");
    lowMotor=0;
    triggeraction();
    delay(100); // 100 ms

  }
  old_pb1 = new_pb1;

  delay(delayloop); // 100 ms

}
