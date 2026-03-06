#include "Particle.h"

PRODUCT_ID(690);
PRODUCT_VERSION(9);

#define pTemperature    WKP
#define pHumidity       D3
#define pPressure       D2
#define pPrecipProb     D1
#define pWindSpeed      D0
#define pAlert1         D4
#define pAlert2         D5

// --- Network & UDP Setup ---
UDP udp;
const int TEMPEST_PORT = 50222;
unsigned char packetBuffer[1024];

// --- Timers ---
int refresh;
double lastRefresh = 0;            // Webhook publish timer
double lastGotWeather = 0;       // Webhook received timer
double lastTempestData = 0;        // UDP stale data tracker
bool staleTempest = true;
bool staleCloud = true;
unsigned long lastBlinkTime = 0;   // Non-blocking LED blinker
bool alertLedState = false;
bool lightningAlertActive = false;
unsigned long lastLightningTime = 0;
const unsigned long LIGHTNING_TIMEOUT = 15 * 60 * 1000; // 15 minutes

// --- Weather Variables ---
// Tempest (Current Conditions)
float temperature;
float humidity;
float pressure;
float windSpeed;

// DarkSky (Forecast & Alerts)
float precipProbability;
float precipLogic[5] = {0,0,0,0,0};
float precipInLogic[5] = {0,0,0,0,0};
float ifAlert = 0.0;

// Dial Outputs
int mTemperature;
int mHumidity;
int mPressure;
int mPrecipProb;
int mWindSpeed;

void setup() {
    // Enable next two lines to assist with debugging
    //Particle.variable("Refresh", lastRefresh);
    //Particle.variable("GotWeather", lastGotWeather);

    //Listen for replies to the particle.publish() called below, and put the replied data through gotWeatherData()
    Particle.subscribe(System.deviceID() + "/hook-response/weather/", gotWeatherData, MY_DEVICES);
    
    //Set up the output pins for Gauges and LEDs
    pinMode(pTemperature,   OUTPUT);
    pinMode(pHumidity,      OUTPUT);
    pinMode(pPressure,      OUTPUT);
    pinMode(pPrecipProb,    OUTPUT);
    pinMode(pWindSpeed,     OUTPUT);
    pinMode(pAlert1,        OUTPUT);
    pinMode(pAlert2,        OUTPUT);

    //Set time zone as Eastern - note that this won't adjust for daylight savings.
    Time.zone(-5);
    udp.begin(TEMPEST_PORT);

    //Dial sweep to check for calibration

    analogWrite(pTemperature,   255);
    delay(250);
    analogWrite(pHumidity,      255);
    delay(250);
    analogWrite(pPressure,      255);
    digitalWrite(pAlert1, HIGH);
    digitalWrite(pAlert2, HIGH);
    delay(250);
    analogWrite(pWindSpeed,     255);
    delay(250);
    analogWrite(pPrecipProb,    255);
    delay(1000);
    digitalWrite(pAlert1, LOW);
    digitalWrite(pAlert2, LOW);
    analogWrite(pTemperature,   0);
    analogWrite(pHumidity,      0);
    analogWrite(pPressure,      0);
    analogWrite(pPrecipProb,    0);
    analogWrite(pWindSpeed,     0);
    
    //Get initial values, then set the refresh to now
    Particle.publish("weather", PRIVATE);
    lastRefresh = Time.now();
    Particle.publish("Photon_bootup");
}

void loop() {
    if (Particle.connected() == true) {
        //Take control of the breathing status LED
        RGB.control(true);
        RGB.brightness(20);
        RGB.color(0, 255, 255);
    } else {
        //Take control of the breathing status LED
        RGB.control(true);
        RGB.brightness(100);
        RGB.color(255, 0, 0);
    }
    //API calls are limited so maximize their use during the normal waking hours.
    //This totals to 499 calls per day (2 weather stations on my account)
    //10 hours slow (36000 seconds) with a refresh of 1000 seconds gives 36 refreshes
    //14 hours fast (50400 seconds) with a refresh of 109 seconds gives 463 refreshes
    if (Time.hour() < 6 || Time.hour() > 20) {
        refresh = 1000; //~17 Minutes
    }
    else {
        refresh = 109; //70 seconds
    }
    if ((Time.now() - lastRefresh) > refresh) {
        Particle.publish("weather", PRIVATE);
        lastRefresh = Time.now();
    }

    // UDP Listener Logic (Tempest Current Conditions)
    int size = udp.receivePacket(packetBuffer, sizeof(packetBuffer) - 1);
    if (size > 0) {
        packetBuffer[size] = 0;
        String jsonString = String((char*)packetBuffer);
        JSONValue root = JSONValue::parseCopy(jsonString);
        
        String type = "";
        
        // Find the event "type"
        JSONObjectIterator iterType(root);
        while(iterType.next()) {
            if (iterType.name() == "type") {
                // Extract const char* from spark::JSONString to build a standard String
                type = String(iterType.value().toString().data());
            }
        }

        // Parse full observation (published every 60 seconds)
        if (type == "obs_st") {
            JSONObjectIterator iterObs(root);
            while(iterObs.next()) {
                if (iterObs.name() == "obs") {
                    JSONArrayIterator outerArray(iterObs.value());
                    if (outerArray.next()) { // Enter the outer array [[...]]
                        JSONArrayIterator innerArray(outerArray.value()); // Enter the inner array
                        int index = 0;
                        while(innerArray.next()) {
                            if (index == 6) pressure = innerArray.value().toDouble();
                            if (index == 7) {
                                float tempC = innerArray.value().toDouble();
                                temperature = (tempC * 9.0 / 5.0) + 32.0; // C to F
                            }
                            if (index == 8) humidity = innerArray.value().toDouble();
                            index++;
                        }
                        lastTempestData = Time.now();
                        updateDials();
                    }
                }
            }
        }

        // Parse rapid wind (published every 60 seconds)
        if (type == "rapid_wind") {
            JSONObjectIterator iterWind(root);
            while(iterWind.next()) {
                if (iterWind.name() == "ob") {
                    JSONArrayIterator obArray(iterWind.value()); // Enter the array [...]
                    int index = 0;
                    while(obArray.next()) {
                        if (index == 1) { // Wind Speed is index 1
                            float windMPS = obArray.value().toDouble();
                            windSpeed = windMPS * 2.23694; // m/s to mph
                        }
                        index++;
                    }
                    lastTempestData = Time.now();
                    updateDials();
                }
            }
        }
        // Parse lightning strikes (published ad-hoc, we will filter for strikes within 6 miles to trigger an alert)
        if (type == "evt_strike") {
            JSONObjectIterator iterStrike(root);
            while(iterStrike.next()) {
                if (iterStrike.name() == "evt") {
                    JSONArrayIterator evtArray(iterStrike.value());
                    int index = 0;
                    while(evtArray.next()) {
                        if (index == 1) { // Distance
                            float distanceKM = evtArray.value().toDouble();
                            float distanceMiles = distanceKM * 0.621371;
                            
                            if (distanceMiles <= 6.0) {
                                lightningAlertActive = true;
                                lastLightningTime = Time.now();
                            }
                        }
                        index++;
                    }
                }
            }
        }
    }

    // Alert & Stale Data LED Logic
    // Local UDP Stale
    if (lastTempestData != 0 && (Time.now() - lastTempestData) > 300) {
        staleTempest = true;
    } else {
        staleTempest = false;
    }
    // Cloud Data Stale
    if ((Time.now() - lastGotWeather) > 300) {
        staleCloud = true;
    } else {
        staleCloud = false;
    }
    
    if (lightningAlertActive && (Time.now() - lastLightningTime) < LIGHTNING_TIMEOUT) {
        // Data is fresh, and there is an active lightning alert: SOLID LEDs
        digitalWrite(pAlert1, HIGH);
        digitalWrite(pAlert2, HIGH);
    } else if (ifAlert > 0.0 && ifAlert > Time.now()) {
        // Data is fresh, and there is an active Weather Alert: SOLID LEDs
        digitalWrite(pAlert1, HIGH);
        digitalWrite(pAlert2, HIGH);
    } else if (staleTempest || staleCloud) {
        if (millis() - lastBlinkTime > 500) { 
            alertLedState = !alertLedState;
            // Tempest Data is Stale (> 5 mins): Blink Left LED
            if (staleTempest) {
                digitalWrite(pAlert1, alertLedState ? HIGH : LOW);
            } else {
                digitalWrite(pAlert1, LOW);
            }
            // Cloud Data is Stale (> 5 mins): Blink Right LED
            if (staleCloud) {
                digitalWrite(pAlert2, alertLedState ? HIGH : LOW);
            } else {
                digitalWrite(pAlert2, LOW);
            }
            lastBlinkTime = millis();
        }
    } else {
        // All clear
        digitalWrite(pAlert1, LOW);
        digitalWrite(pAlert2, LOW);
    }
}

// --- Parse the Webhook ---
void gotWeatherData(const char *name, const char *data) {
    lastGotWeather = Time.now();
    String str = String(data);
    char strBuffer[400] = "";
    str.toCharArray(strBuffer, 400);
    
    // We must parse the current conditions to advance the strtok pointer, 
    // but we use side variables so we don't overwrite the Tempest data if it exists.
    float cloudTemp  = atof(strtok(strBuffer, "~"));
    float cloudHum   = atof(strtok(NULL, "~"));
    float cloudPress = atof(strtok(NULL, "~"));
    float cloudWind  = atof(strtok(NULL, "~"));
    
    // Now extract the precipitation forecast we actually want
    precipInLogic[0]    = atof(strtok(NULL, "~"));
    precipLogic[0]      = atof(strtok(NULL, "~"));
    precipInLogic[1]    = atof(strtok(NULL, "~"));
    precipLogic[1]      = atof(strtok(NULL, "~"));
    precipInLogic[2]    = atof(strtok(NULL, "~"));
    precipLogic[2]      = atof(strtok(NULL, "~"));
    precipInLogic[3]    = atof(strtok(NULL, "~"));
    precipLogic[3]      = atof(strtok(NULL, "~"));
    precipInLogic[4]    = atof(strtok(NULL, "~"));
    precipLogic[4]      = atof(strtok(NULL, "~"));
    ifAlert             = atof(strtok(NULL, "~"));
    
    // Get highest chance of precipitation for next 5 hours
    precipProbability   = precipLogic[0];
    for ( int i = 0; i < 5; i++ ) {
        if ( precipProbability < precipLogic[i] ) {
            precipProbability = precipLogic[i];
        }
    }
    if (Time.now() - lastTempestData > 300) {
        // Failover: Copy cloud data into the main variables
        temperature = cloudTemp;
        humidity = cloudHum;
        pressure = cloudPress;
        windSpeed = cloudWind;
    }
    updateDials();
}

// --- Unified Dial Output ---
// This will try to dynamically catch humidity and temperature values on a scale of 0-1 (PirateWeather) and 0-100 (Tempest)
void updateDials() {
    mTemperature = (int) constrain((255.0) * (temperature - 0.0) / (100.0 - 0.0), 0, 255);
    
    // Note: Tempest Humidity is 0-100%, PirateWeather is 0.0-1.0. 
    float normalizedHum = (humidity > 1.0) ? (humidity / 100.0) : humidity;
    mHumidity = (int) constrain((255.0) * normalizedHum, 0, 255);

    mPressure = (int) constrain((255.0) * (pressure - 960.0) / (1060.0 - 960.0), 0, 255);
    mWindSpeed = (int) constrain((255.0) * (windSpeed - 0.0) / (30.0 - 0.0), 0, 255);
    
    // Same as Humidity - Tempest gives 0-100%, PirateWeather gives 0.0-1.0
    float normalizedPrecip = (precipProbability > 1.0) ? (precipProbability / 100.0) : precipProbability;
    mPrecipProb = (int) constrain((255.0) * normalizedPrecip, 0, 255);

    analogWrite(pTemperature, mTemperature);
    analogWrite(pHumidity, mHumidity);
    analogWrite(pPressure, mPressure);
    analogWrite(pPrecipProb, mPrecipProb);
    analogWrite(pWindSpeed, mWindSpeed);
}