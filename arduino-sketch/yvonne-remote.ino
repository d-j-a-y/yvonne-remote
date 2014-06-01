/*
 yvonne-remote Arduino 
 
 Few buttons to control over serial communication few aspects of the yvonnne shooting :
 - start/stop the shoot
 - signal to generate a video
 - quit the shooting
  
 
 The circuit:
 --> Visual Output
 * video LED attached from pin 6 to 220ohm to ground 
 * start state LED attached from pin 8 to 220ohm to ground
 --> Command
 * quit pushbutton attached to pin 14 from +5V
 * 120ohm push down resistor attached to pin 14 from ground
 * start/stop switch attached to pin 2 from +5V
 * 120ohm push down resistor attached to pin 2 from ground
 * video pushbutton attached to pin 4 from +5V
 * 120ohm push down resistor attached to pin 31 from ground
 
 created 2014
 by Jérôme Blanchi aka d.j.a.y  <http://github.com/d-j-a-y/yvonne-remote>
 
 based on Tutorial/Button example
 by DojoDave <http://www.0j0.org>
 modified 30 Aug 2011
 by Tom Igoe

 based on TODO Tutorial/Debounce example
 by DojoDave <http://www.0j0.org> TODO
 modified 30 Aug 2011
 by Tom Igoe

 
 */

// constants won't change. They're used here to 
// set pin numbers:
const int quitPushButtonPin = 14;     // the number of the quit push button pin
const int videoPushButtonPin = 4;     // the number of the video push button pin
const int stopButtonPin = 2;     // the number of the start/stop button pin

const int videoLedPin =  6;      // the number of the video LED pin
const int startLedPin = 8;     // the number of the start state LED pin

// quit variables will change:
int quitPushButtonState = 0;     // variable for reading the pushbutton status
int quitPushPreviousState = 0;     // variable for reading the pushbutton status
long quitLastDebounceTime = 0;

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
  pinMode(videoLedPin, OUTPUT);
  pinMode(startLedPin, OUTPUT); 

  // initialize the quit pushbutton pin as an input:
  pinMode(quitPushButtonPin, INPUT);  
  // initialize the video pushbutton pin as an input:
  pinMode(videoPushButtonPin, INPUT);
  // initialize the stop pushbutton pin as an input:
  pinMode(stopButtonPin, INPUT);  
  
  // Open serial (default 8N1) communications and wait for port to open:
  Serial.begin(57600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  // Signal we are quite ready to go!
  ReadyToLoop();  
  
  // init stop button and led state
  stopButtonState = digitalRead(stopButtonPin);
  stopPreviousState = stopButtonState;
  
  digitalWrite(startLedPin, stopButtonState);
}

void loop(){
  // read the state of the pushbuttons value:
  int videoReading = digitalRead(videoPushButtonPin);
  int stopReading = digitalRead(stopButtonPin);  
  int quitReading = digitalRead(quitPushButtonPin);    

  //quit Push button   
  if(quitReading != quitPushPreviousState){
      quitLastDebounceTime = millis();
  }
  
  if ( (millis() - quitLastDebounceTime) > debounceDelay)
  {
    if (quitReading != quitPushButtonState)
    {
      quitPushButtonState = quitReading;
      // check if the pushbutton is pressed.
      // if it is, the buttonState is HIGH:
      if (quitPushButtonState == HIGH) {     
        Serial.print("QUIT");
      } 
    }
  }
  quitPushPreviousState = quitReading;

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
        digitalWrite(videoLedPin, HIGH);
        Serial.print("VIDEO");
      } 
      else {
        // turn LED off:
        digitalWrite(videoLedPin, LOW);
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
        digitalWrite(startLedPin, HIGH);   
      } 
      else {
        Serial.print("STOP");
        digitalWrite(startLedPin, LOW);   
        
      }
    }
  }
  stopPreviousState = stopReading;  
}

// Visual confirmation of the start
void ReadyToLoop()
{
  short nbloop = 0;
  
  for (;nbloop < 4 ; nbloop++)
  {
    digitalWrite(startLedPin, HIGH);
    digitalWrite(videoLedPin, HIGH);
    delay(50);    
    digitalWrite(startLedPin, LOW);
    digitalWrite(videoLedPin, LOW);
    delay(50); 
  }    
}
