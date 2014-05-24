/*****************************************************
* This file is part of yvonne-remote.
* (Arduino part)
*
* 2014, Jérôme Blanchi aka d.j.a.y
* http://github.com/d-j-a-y/yvonne-remote
*
*    yvonne-remote is free software: you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation, either version 3 of the License, or
*    any later version.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
* 
* descriptif....TODO
* 
* 
* The circuit:
* * LED attached from pin 13 to ground 
* * start/stop switch attached to pin 2 from +5V
* * 10K push down resistor attached to pin 2 from ground
* * video pushbutton attached to pin 31 from +5V
* * 10K push down resistor attached to pin 31 from ground
* * TODO
* 
* * Note: on most Arduinos there is already an LED on the board
* attached to pin 13.
* 
* created 2014
* by Jérôme Blanchi aka d.j.a.y  <http://github.com/d-j-a-y/yvonne-remote>
* 
* based on Tutorial/Button example
* by DojoDave <http://www.0j0.org>
* modified 30 Aug 2011
* by Tom Igoe
*
* based on Tutorial/Debounce example
* created 21 November 2006
* by David A. Mellis
* modified 30 Aug 2011
* by Limor Fried
* modified 28 Dec 2012
* by Mike Walters 
*
*****************************************************/


// constants won't change. They're used here to 
// set pin numbers:
const int videoPushButtonPin = 31;     // the number of the video push button pin
const int stopButtonPin = 2;     // the number of the start/stop button pin
const int ledPin =  13;      // the number of the LED pin

// video variables will change:
int videoPushButtonState = 0;     // variable for reading the pushbutton status
int videoPushPreviousState = 0;     // variable for reading the pushbutton status
long videoLastDebounceTime = 0;

// start/stop variables will change:
int stopButtonState = 0;     // variable for reading the pushbutton status
int stopPreviousState = 0;     // variable for reading the pushbutton status
long stopLastDebounceTime = 0;

long debounceDelay = 50;

void setup() {
  // initialize the LED pin as an output:
  pinMode(ledPin, OUTPUT);
  
  // initialize the video pushbutton pin as an input:
  pinMode(videoPushButtonPin, INPUT);
  // initialize the stop pushbutton pin as an input:
  pinMode(stopButtonPin, INPUT);  
  
  // Open serial (default 8N1) communications and wait for port to open:
  Serial.begin(57600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
}

void loop(){
  // read the state of the pushbuttons value:
  int videoReading = digitalRead(videoPushButtonPin);
  int stopReading = digitalRead(stopButtonPin);  

  //Video Push button   
  if(videoReading != videoPushPreviousState){
      videoLastDebounceTime = millis();
  }
  
  if ( (millis() - videoLastDebounceTime) > debounceDelay)
  {
    if (videoReading != videoPushButtonState)
    {
      videoPushButtonState = videoReading;
      // check if the pushbutton is pressed.
      // if it is, the buttonState is HIGH:
      if (videoPushButtonState == HIGH) {     
        // turn LED on:    
        digitalWrite(ledPin, HIGH);
        Serial.print("VIDEO");
      } 
      else {
        // turn LED off:
        digitalWrite(ledPin, LOW);
      }
    }
  }
  videoPushPreviousState = videoReading;
  
  //Start/Stop Push button   
  if(stopReading != stopPreviousState){
      stopLastDebounceTime = millis();
  }
  
  if ( (millis() - stopLastDebounceTime) > debounceDelay)
  {
    if (stopReading != stopButtonState)
    {
      stopButtonState = stopReading;
      // check if the pushbutton is pressed.
      // if it is, the buttonState is HIGH:
      if (stopButtonState == HIGH) {     
        Serial.print("START");
      } 
      else {
        Serial.print("STOP");    
      }
    }
  }
  stopPreviousState = stopReading;  
}
