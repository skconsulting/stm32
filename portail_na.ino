// pins
int   countrot  = PD2; // counter of rotation
int   trig      = PD3; // change mode
int   EMA       = PD5; // pwm motor
int   IN1       = PD6; // command motor 1
int   IN2       = PD7; // command motor 2
int   sensor    = A7; // overloadsensor


int maxrotationopen, maxrotationclose, calibre, inrotation, minCC, maxCC;
int halfopen, halfclose;
int opendelay, closedelay, minmotorpos, maxmotorpos, minmotor, delayloop;
int countrotation = 0;
int new_pb, old_pb = 1;
int new_pb1, old_pb1 = 1;
int stateportail = 0; // 0 repos ferme, 1 ouvre, 2 repos ouvert, 3 ferme,4 ouvert a moitie 5 ferme a moitie
double meas, measold ;
int varCompteur = 0; // La variable compteur
int mVperAmp = 185; // use 100 for 20A Module and 66 for 30A Module
int ACSoffset = 2500;
double Voltage = 0;
double Amps = 0;

void pressedcountrot (void)
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
    }
  }
  //Serial.println(countrotation);
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
{
  meas = analogRead(sensor); // Converts and read the analog input value (value from 0.0 to 1.0)
  if ((meas - measold > 15) || (measold - meas > 15)) {
    Serial.print("measure = : ");
    Serial.println(meas);
    Voltage = (meas / 1024.0) * 5000; // Gets you mV
    Amps = ((Voltage - ACSoffset) / mVperAmp);
    Serial.print("mV = "); // shows the voltage measured
    Serial.println(Voltage, 3);
    // the '3' after voltage allows you to display 3 digits after decimal point
    Serial.print("Amps = "); // shows the voltage measured
    Serial.println(Amps, 3);
    // the '3' after voltage allows you to display 3 digits after decimal point
    //Serial.print(" old: ");
    //Serial.println(measold);
    measold = meas;
  }
  if ((meas < minCC) || (meas > maxCC)) {
    if (meas < minCC) {
      Serial.print("overdrive ouvre: ");
      Serial.println(meas);
      Serial.print("mincc: ");
      Serial.print(minCC);
    }
    if (meas > maxCC) {
      Serial.print("overdrive ferme: ");
      Serial.println(meas);
      Serial.print("maxcc: ");
      Serial.println(maxCC);
    }
    if (calibre == 2) {
      maxrotationclose = countrotation;
      halfclose = int(0.5 * maxrotationclose);
      //minmotorpos = int(0.9 * maxrotationopen);
      minmotorpos = maxrotationclose - 1;
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
void setup() {
  Serial.println("***** portail*********");
  pinMode (countrot, INPUT);
  pinMode (trig, INPUT);
  pinMode (EMA, OUTPUT);
  pinMode (IN1, OUTPUT);
  pinMode (IN2, OUTPUT);
  Serial.begin(9600);
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

  minmotor = 150;

  minCC = 450;//ouvre
  //average 510
  maxCC = 570;//ferme
  Serial.print("min voltage overdrive: ");
  Serial.println(minCC);
  Serial.print("max voltage overdrive: ");
  Serial.println(maxCC);

  opendelay = 500; // 0.5 second
  closedelay = 0;
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
  delayloop = 10;
}

void loop() {
  /// icurrent read after 500ms
  if (varCompteur == 0) {
    anaread();
  }
  if (varCompteur > 0 ) {
    varCompteur++;
  }
  if (varCompteur > 500.0 / delayloop) { //500ms
    varCompteur = 0;
  }
  // stop rotation after 20 second
  if (inrotation > 0 ) {
    inrotation++;
  }
  if (inrotation > 18000.0 / delayloop) { //18 second
    inrotation = 0;
    Serial.println("stop motor after 18s");
    if (stateportail == 1) {
      stateportail = 2;
    }
    if (stateportail == 3) {
      stateportail = 0;
    }
    actionportail();
  }
  // count rotation
  new_pb = digitalRead(countrot);
  if ((new_pb == 0) && (old_pb == 1)) {
    //Serial.println("count rotation pressed");
    pressedcountrot();
    //delay(10); // 100 ms
  }
  old_pb = new_pb;

  // mode
  new_pb1 = digitalRead(trig);
  if ((new_pb1 == 0) && (old_pb1 == 1)) {
    Serial.println(" action pressed!");
    delay(100); // 100 ms
    triggeraction();
  }
  old_pb1 = new_pb1;

  delay(delayloop); // 50 ms
}
