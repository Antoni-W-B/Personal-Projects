#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiAP.h>

//Pins for Ultrasonic Sensors
const int pingPinStraight = A2; // Trigger Pin of Ultrasonic Sensor Facing Straight
const int echoPinStraight = A3; // Echo Pin of Ultrasonic Sensor Facing Straight
long durationStraight;

const int pingPinRight = A0; // Trigger Pin of Ultrasonic Sensor Facing Right
const int echoPinRight = A1; // Echo Pin of Ultrasonic Sensor Facing Right
long durationRight;

const float c = 3.45e5;    // approximate speed of sound (m/s)
const int freq = 5000;     // PWM frequency in Hz
const int resolution = 8;  // PWM resolution in bits

//Pins for controlling the wheels
const int LeftWheelsForwardPin= 11;
const int LeftWheelsReversePin= 12 ;

const int RightWheelsForwardPin= 13;
const int RightWheelsReversePin= 14;

const int SleepPin= 9;

// Wifi Credentials for connecting to the car:
const char *ssid = "MazeCar1";
const char *password = "mazecar1";

WiFiServer server(80);

void setup() {
// US100 Echo Sensor For straight distance
pinMode(pingPinStraight, OUTPUT);
pinMode(echoPinStraight, INPUT);
digitalWrite(pingPinStraight, LOW);
Serial.begin(115200);

// US100 Echo Sensor For Right distance
pinMode(pingPinRight, OUTPUT);
pinMode(echoPinRight, INPUT);
digitalWrite(pingPinRight, LOW);
Serial.begin(115200);

//Setup for wheel control
pinMode(LeftWheelsForwardPin, OUTPUT);
pinMode(LeftWheelsReversePin, OUTPUT);
pinMode(RightWheelsForwardPin, OUTPUT);
pinMode(RightWheelsReversePin, OUTPUT);
Serial.begin(115200);
ledcSetup(0, freq, resolution);
ledcAttachPin(LeftWheelsForwardPin, 0);
ledcAttachPin(LeftWheelsReversePin, 0);
ledcAttachPin(RightWheelsForwardPin, 0);
ledcAttachPin(RightWheelsReversePin, 0);

// Setting up the sleep pin to disable driving
pinMode(SleepPin, OUTPUT);

// Further code for the Wifi Server:
if (!WiFi.softAP(ssid, password)) {
    log_e("Soft AP creation failed.");
    while(1);
  }
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.begin();

  Serial.println("Server started");
}

void loop() {
// *** US100 Echo Sensor Straigth distance value ***
digitalWrite(pingPinStraight, HIGH);
delayMicroseconds(10);
digitalWrite(pingPinStraight, LOW);
durationStraight = pulseIn(echoPinStraight, HIGH); // returns length of high pulse
float distanceStraight = (durationStraight * 1.e-6 * c / 2);
delay(300);
Serial.println(distanceStraight);

// *** US100 Echo Sensor Right distance value ***
digitalWrite(pingPinRight, HIGH);
delayMicroseconds(10);
digitalWrite(pingPinRight, LOW);
durationRight = pulseIn(echoPinRight, HIGH); // returns length of high pulse
float distanceRight = (durationRight * 1.e-6 * c / 2);
delay(300);

//Code For the behavior of the car for different cases
if (distanceStraight>distanceRight){
ledcWrite(LeftWheelsForwardPin,255);   //This loop causes car to go STRAIGHT as long as
ledcWrite(LeftWheelsReversePin,0);     //distance STRAIGHT is the further distance
ledcWrite(RightWheelsForwardPin,255);
ledcWrite(RightWheelsReversePin,0);
}

if (distanceStraight<distanceRight){
ledcWrite(LeftWheelsForwardPin,255);  //This loop causes the car to turn RIGHT when
ledcWrite(LeftWheelsReversePin,0);    //distance to right is greater than straight
ledcWrite(RightWheelsForwardPin,0);   //turn should be 90 degrees
ledcWrite(RightWheelsReversePin,255); 
delay(3000);                          //Turning occurs for 3 seconds, then car stops until new measurement is taken
ledcWrite(LeftWheelsForwardPin,0);  
ledcWrite(LeftWheelsReversePin,0);   
ledcWrite(RightWheelsForwardPin,0);   
ledcWrite(RightWheelsReversePin,0);
}

if (distanceStraight < 10 && distanceRight < 10 ){
ledcWrite(LeftWheelsForwardPin,0);    //This loop causes the car to turn LEFT when
ledcWrite(LeftWheelsReversePin,255);  //distance to RIGHT AND STRAIGHT is smaller than 10cm. 
ledcWrite(RightWheelsForwardPin,255); //Car Sees it is stuck in Corner
ledcWrite(RightWheelsReversePin,0); 
delay(3000);                          //Turning occurs for 3 seconds, then car stops until new measurement is taken
ledcWrite(LeftWheelsForwardPin,0);  
ledcWrite(LeftWheelsReversePin,0);   
ledcWrite(RightWheelsForwardPin,0);   
ledcWrite(RightWheelsReversePin,0);
}

if (distanceStraight < 1 ){
ledcWrite(LeftWheelsForwardPin,0);    //This loop causes the car to Reverse when
ledcWrite(LeftWheelsReversePin,255);  //distance to STRAIGHT wall is smaller than 1cm. 
ledcWrite(RightWheelsForwardPin,0);   //Car hit wall straight on.
ledcWrite(RightWheelsReversePin,255); 
delay(3000);                          //Turning occurs for 3 seconds, then car stops until new measurement is taken
ledcWrite(LeftWheelsForwardPin,0);  
ledcWrite(LeftWheelsReversePin,0);   
ledcWrite(RightWheelsForwardPin,0);   
ledcWrite(RightWheelsReversePin,0);
}


// The rest of the code is for the Wifi server
WiFiClient client = server.available();   // listen for incoming clients

  if (client) {                             // if you get a client,
    Serial.println("New Client.");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        if (c == '\n') {                    // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            // the content of the HTTP response follows the header:
            client.print("Click <a href=\"/H\">here</a> to turn ON the car.<br>");
            client.print("Click <a href=\"/L\">here</a> to turn OFF the car.<br>");

            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          } else {    // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }

        // Check to see if the client request was "GET /H" or "GET /L":
        if (currentLine.endsWith("GET /H")) {
          digitalWrite(SleepPin, HIGH);               // GET /H turns the Car on
        }
        if (currentLine.endsWith("GET /L")) {
          digitalWrite(SleepPin, LOW);                // GET /L turns the Car off
        }
      }
    }
    // close the connection:
    client.stop();
    Serial.println("Client Disconnected.");
  }

}









