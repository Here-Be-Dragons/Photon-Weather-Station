# Photon-Weather-Station

Wifi-enabled weather display using panel meters

## Data Sources

* Scrapes weather from any Tempest Weather station on the local network via UDP broadcast packets.
* Gets forecast data from PirateWeather (https://github.com/Pirate-Weather/pirateweather) or via Tempest's Cloud API.

Optionally, you could get all data from PirateWeather with some rework. Look at the `PirateWeather` branch to see this version.

## Parts

Particle Photon: https://www.adafruit.com/product/2721  
Gauges: http://www.allelectronics.com/make-a-store/item/pm-376/panel-meter-full-scale-1ma-dc/1.html  
Trimpots: http://www.allelectronics.com/make-a-store/item/tps-5k/5k-single-turn-trimpot/1.html  

## Sketches

`weather.ino`: The actual weather station Photon sketch  
`weather-screen.ino`: Something I wrote to toy around with a 128x64 OLED screen I bought.  Subscribes to the same webhook as the primary weather station, and outputs total expected precipitation in the next 5 hours to the oled screen.  Will require some modification to weather.ino to work.  
`gauge-tester.ino`: Cycles gauges between 0% and 100% for calibration purposes.  

## Webhooks

The Particle device will send a request to the Particle Cloud for a webhook response. It will then listen for this response. You can use one of the following templates to get this set up in the particle cloud (via the Integrations -> Custom Webhook GUI interface)

* `tempest_webhook.json`: Update the values \<in brackets\> and this will pull your values from the Tempest cloud using local weather stations.
* `pirateweather_webhook.json`: Update the values \<in brackets\>. A drop-in replacement for forecast.io or darksky.net (both defunct). This API was what this weather station was originally designed to use.

## Other Useful Scripts

`simulate_tempest_broadcast.py`: A python script that will broadcast UDP packets for wind and weather conditions on your local network in the same way a Tempest weather station does. Useful for getting up and running before you have your weather station functional or for troubleshooting.  
`simulate_tempest_lightning.py`: A python script that will broadcast UDP packets for lightning strikes on your local network in the same way a Tempest weather station does.  
`monitor_tempest.py`: A python script that will listen for UDP broadcast packets and write the raw output to your terminal.

Both can be run from a python3 terminal environment via `python3 <the file.py>`.

## Images

![Alt text](/images/DSC08575.JPG?raw=true "Finished Product")
![Alt text](/images/schematic.png?raw=true "Pinout")

## Copyright

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
