#define pTemperature    WKP
#define pHumidity       D3
#define pPressure       D2
#define pPrecipProb     D1
#define pWindSpeed      D0
#define pAlert1         D4
#define pAlert2         D5

//Refresh time in seconds. Min is about 87 seconds (1000 API calls per day are allowed by darksky.net).
int refresh;
//Unix time of last refresh
double lastRefresh      = 0;
//Unix time of last time the webhook data was returned
double lastGotWeather   = 0;
double currentTime = 0;

float temperature;
float humidity;
float pressure;
float precipProbability;
float precipLogic[5] = {0,0,0,0,0};
float precipInLogic[5] = {0,0,0,0,0};
float windSpeed;
float ifAlert;

int mTemperature;
int mHumidity;
int mPressure;
int mPrecipProb;
int mWindSpeed;
int mAlert;

void setup() {
    // Enable next two lines to assist with debugging
    //Particle.variable("Refresh", lastRefresh);
    //Particle.variable("GotWeather", lastGotWeather);

    //Listen for replies to the particle.publish() called below, and put the replied data through gotWeatherData()
    Particle.subscribe("hook-response/weather_" + System.deviceID(), gotWeatherData, MY_DEVICES);

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
    Particle.publish("weather_" + System.deviceID(), PRIVATE);
    lastRefresh = Time.now();
    currentTime = Time.now();
    Particle.publish("Photon_bootup");
    delay(10000);
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
    
    //Display an error code if webhook data has not been received within the past 20 minutes
    if ((Time.now() - lastGotWeather) > 1200) { 
        digitalWrite(pAlert2, HIGH);
        digitalWrite(pAlert1, HIGH);
        delay(500);
        digitalWrite(pAlert2, LOW);
        digitalWrite(pAlert1, LOW);
        delay(500);
        digitalWrite(pAlert2, HIGH);
        digitalWrite(pAlert1, HIGH);
        delay(500);
        digitalWrite(pAlert2, LOW);
        digitalWrite(pAlert1, LOW);
        delay(500);
        digitalWrite(pAlert2, HIGH);
        digitalWrite(pAlert1, HIGH);
        delay(500);
        digitalWrite(pAlert2, LOW);
        digitalWrite(pAlert1, LOW);
    }
    if ((lastRefresh - lastGotWeather ) > 3600 && lastGotWeather != 0 ) { //hack to fix Particle.subscribe losing connection
        Particle.publish("Photon_bootup","FAULT: AUTOMATED RESTART");
        System.reset();
    }
    if ((Time.now() - lastRefresh) > refresh) {
        Particle.publish("weather_" + System.deviceID(), PRIVATE);
        lastRefresh = Time.now();
    }
    delay(1000);
    currentTime = Time.now();
}

void gotWeatherData(const char *name, const char *data) {
    
    String str = String(data);
    char strBuffer[400] = "";
    str.toCharArray(strBuffer, 400);
    lastGotWeather = Time.now();
    
    temperature         = atof(strtok(strBuffer, "~"));
    humidity            = atof(strtok(NULL, "~"));
    pressure            = atof(strtok(NULL, "~"));
    windSpeed           = atof(strtok(NULL, "~"));
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

    //Get the highest chance of rain for the next 5 hours    
    precipProbability   = precipLogic[0];
    for ( int i = 0; i < 5; i++ ) {
        if ( precipProbability < precipLogic[i] ) {
            precipProbability = precipLogic[i];
        }
    }

    mTemperature        = (int) constrain( (255.0) * (temperature       - 0.0)      / (100.0    - 0.0),     0, 255 );
    mHumidity           = (int) constrain( (255.0) * (humidity          - 0.0)      / (1.0      - 0.0),     0, 255 );
    mPressure           = (int) constrain( (255.0) * (pressure          - 960.0)    / (1060.0   - 960.0),   0, 255 );
    mPrecipProb         = (int) constrain( (255.0) * (precipProbability - 0.0)      / (1.0      - 0.0),     0, 255 );
    mWindSpeed          = (int) constrain( (255.0) * (windSpeed         - 0.0)      / (30.0     - 0.0),     0, 255 );
    
    analogWrite(pTemperature,   mTemperature);
    analogWrite(pHumidity,      mHumidity   );
    analogWrite(pPressure,      mPressure   );
    analogWrite(pPrecipProb,    mPrecipProb );
    analogWrite(pWindSpeed,     mWindSpeed  );
    
    if (ifAlert > 0.0 && ifAlert > Time.now()) {
        digitalWrite(pAlert1, HIGH);
        digitalWrite(pAlert2, HIGH);
    }
    else {
        digitalWrite(pAlert1, LOW);
        digitalWrite(pAlert2, LOW);
    }
}
