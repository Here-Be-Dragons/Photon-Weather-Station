# Photon-Weather-Station
Wifi-enabled weather display using panel meters

Queries darksky.net's (previously called forecast.io) API for current weather conditions

### Parts
Particle Photon: https://www.adafruit.com/product/2721  
Gauges: http://www.allelectronics.com/make-a-store/item/pm-376/panel-meter-full-scale-1ma-dc/1.html  
Trimpots: http://www.allelectronics.com/make-a-store/item/tps-5k/5k-single-turn-trimpot/1.html  

### Sketches
**weather.ino:** The actual weather station Photon sketch  
**weather-screen.ino:** Something I wrote to toy around with a 128x64 OLED screen I bought.  Subscribes to the same webhook as the primary weather station, and outputs total expected precipitation in the next 5 hours to the oled screen.  Will require some modification to weather.ino to work.  
**gauge-tester.ino:** Cycles gauges between 0% and 100% for calibration purposes.  

### Images
![Alt text](/images/DSC08575.JPG?raw=true "Finished Product")
![Alt text](/images/pinout.jpg?raw=true "Pinout")

### Copyright
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
