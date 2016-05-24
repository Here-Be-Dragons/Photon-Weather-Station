//Loops Gauges from 0% to 100% and back to dial in max (trimpot) and min (gauge knob) adjustments

int pTemperature    = WKP;
int pHumidity       = D3;
int pPressure       = D2;
int pPrecipProb     = D1;
int pWindSpeed      = D0;
int pAlert1         = D4;
int pAlert2         = D5;

void setup() {

    pinMode(pTemperature,   OUTPUT);
    pinMode(pHumidity,      OUTPUT);
    pinMode(pPressure,      OUTPUT);
    pinMode(pPrecipProb,    OUTPUT);
    pinMode(pWindSpeed,     OUTPUT);
    pinMode(pAlert1,        OUTPUT);
    pinMode(pAlert2,        OUTPUT);
}

void loop() {
    digitalWrite(pAlert1, LOW);
    digitalWrite(pAlert2, LOW);
    analogWrite(pTemperature,   0);
    analogWrite(pHumidity,      0);
    analogWrite(pPressure,      0);
    analogWrite(pPrecipProb,    0);
    analogWrite(pWindSpeed,     0);
    delay(1000);
    digitalWrite(pAlert1, HIGH);
    digitalWrite(pAlert2, HIGH);
    analogWrite(pTemperature,   255);
    analogWrite(pHumidity,      255);
    analogWrite(pPressure,      255);
    analogWrite(pPrecipProb,    255);
    analogWrite(pWindSpeed,     255);
    delay(1000);
}
