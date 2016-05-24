/* This is standalone code meant to run on a second Particle Photon.
 It listens for "weather*" and intercepts inches to throw up on an OLED screen
 */
// This #include statement was automatically added by the Particle IDE.
//#include "Adafruit_GFX/Adafruit_GFX.h"

// This #include statement was automatically added by the Particle IDE.
#include "Adafruit_SSD1306/Adafruit_SSD1306.h"
/* ============== MAIN =====================*/

 //Use I2C with OLED RESET pin on D4
 #define OLED_RESET D4
 Adafruit_SSD1306 oled(OLED_RESET);


unsigned long previousMillis;
unsigned long interval = 30000;
float temperature;
float humidity;
float pressure;
float precipProbability;
float precipLogic[5] = {0,0,0,0,0};
float precipInLogic[5] = {0,0,0,0,0};
float windSpeed;
float ifAlert;
float precipInSum = -1;

void setup() {
    oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)
    oled.display(); // show splashscreen
    Particle.subscribe("hook-response/weather", gotWeatherData, MY_DEVICES);
    Time.zone(-5);
}

void loop() {
  oled.clearDisplay();
  //delay(200);
  oled.setTextSize(1);
  oled.setTextColor(WHITE);
  oled.setCursor(0,0);
  oled.println("5-hr rain forecast:");
  oled.println("Units: Inches");
  oled.println();
  oled.setTextSize(3);
  oled.print(precipInSum);
  //oled.println(" in.");
  oled.display();
  //delay(800);
}

void gotWeatherData(const char *name, const char *data) {
    
    String str = String(data);
    char strBuffer[400] = "";
    str.toCharArray(strBuffer, 400);

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

    precipInSum = 0;
    for (auto& num : precipInLogic) { //Adds up everything in the PrecipInLogic array
        precipInSum += num;
    }
}
