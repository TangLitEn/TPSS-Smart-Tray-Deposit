/****
1. No longer has Stepper structure, as more structure the system has, the more prone to error
2. Changing the ultrasonic sensor to weight detection, once a weight is detected, it will automatically get rid of the food waste (TBC)
****/

#include <WiFi.h>
#include <Servo.h>
#include "HX711.h"

const char* ssid = "TP-Link_D8F2"; // Replace with your network credentials
const char* password = "53717221";

WiFiServer server(80); // Set web server port number to 80

String header; // Variable to store the HTTP request

// ATTRIBUTES
long distance, plate_weight;
String plate_colour;

// ULTRASONIC
const int pingPin = 20; // Trigger Pin of Ultrasonic Sensor
const int echoPin = 21; // Echo Pin of Ultrasonic Sensor
int count = 0;

// COLOUR SENSOR
#define S0 25
#define S1 26
#define S2 27
#define S3 14
#define sensorOut 12

int colour[3] = {0,0,0};
int frequency = 0;

// LOAD CELL
HX711 scale;
uint8_t dataPin = 18;
uint8_t clockPin = 19;
long w1, w2, previous = 0;

// SERVO 
static const int servoPinBase_1 = 25;
static const int servoPinBase_2 = 26;
Servo servoBase_1;
Servo servoBase_2;

// INDICATOR
const int redIndicator = 22; 
const int greenIndicator = 23; 

void setup() {
  Serial.begin(115200);

  // SETUP THE PIN 
  pinMode(pingPin, OUTPUT);
  pinMode(echoPin, INPUT);

  pinMode(redIndicator,OUTPUT);
  pinMode(greenIndicator,OUTPUT);

  // SETUP WIFI
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  // SETUP SERVO
  servoBase_1.attach(servoPinBase_1);
  servoBase_2.attach(servoPinBase_2);

  servoBase_1.write(0);
  servoBase_2.write(180);

  // SETUP COLOUR SENSOR
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(sensorOut, INPUT);
  //// Setting frequency-scaling to 20%
  digitalWrite(S0,HIGH);
  digitalWrite(S1,LOW);

  // SETUP LOAD CELL (5KG)
  scale.begin(dataPin, clockPin);
  scale.set_offset(426839); // HX711 calibration
  scale.set_scale(482.643219);  // HX711 calibration

  // INIT
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
}

void loop() {
  WiFiClient client = server.available(); // Listen for incoming clients

  if (client) { // if you get a client,
    Serial.println("New Client."); // print a message out the serial port
    String currentLine = ""; // make a String to hold incoming data from the client
    while (client.connected()) { // loop while the client's connected
      if (client.available()) { // if there's bytes to read from the client,
        char c = client.read(); // read a byte, then
        header += c;
        if (c == '\n') { // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            
            // Check the request route
            if (header.indexOf("GET /sensor") >= 0) {
              // Sensor data route
              client.println("HTTP/1.1 200 OK");
              client.println("Content-type:text/plain");
              client.println("Connection: close");
              client.println();

              distance = ultrasonicSensor(); // get the distance
              readColour(colour); // update colour array
              plate_colour = String(colour[0]) + " " + String(colour[1]) + " " + String(colour[2]);
              //plate_colour = "RED"  ; // plate colour, to be integrated with the colour sensor
              plate_weight = measureWeight()      ; // weight (in g) of the food waste, to be integrated with the weight detector 

              client.println("<html><body>");
              client.println("<h1>SMART TRAY DEPOSIT SERVER - TPSS</h1>");
              
              client.print("<p> Plate Detection: " + String(distance) + "cm </p>");
              client.print("<p> Plate Count: " + String(count) + " </p>");
              client.print("<p> Plate Colour: " + String(plate_colour) + " </p>");
              client.print("<p> Plate Weight: " + String(plate_weight) + " </p>");
              
              client.println("</body></html>");
              break;
            } else {
              // Main page route
              client.println("HTTP/1.1 200 OK");
              client.println("Content-type:text/html");
              client.println("Connection: close");
              client.println();

              // Web page content
              client.println("<!DOCTYPE html><html>");
              client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
              client.println("<link rel=\"icon\" href=\"data:,\">");
              // CSS
              client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0 auto; text-align: center;}</style>");
              // JavaScript
              client.println("<script>function refreshData() {var xhttp = new XMLHttpRequest();xhttp.onreadystatechange = function() {if (this.readyState == 4 && this.status == 200) {document.body.innerHTML = this.responseText;}};xhttp.open(\"GET\", \"/sensor\", true);xhttp.send();}setInterval(refreshData, 500);</script>");
              // Body
              client.println("</head><body>");
              client.println("<h1> SMART TRAY DEPOSIT SERVER - TPSS </h1>");
              client.println("<p> SYSTEM BOOT UP </p>");
              client.println("</body></html>");
              client.println();
            }
            break;
          } else { // if you got anything else but a carriage return character,
            currentLine = ""; // clear currentLine
          }
        } else if (c != '\r') { // if you got anything else but a carriage return character,
          currentLine += c; // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}

long microsecondsToCentimeters(long microseconds) {
  return microseconds / 29 / 2;
}

long ultrasonicSensor() {
  digitalWrite(pingPin, LOW);
  delayMicroseconds(2);
  digitalWrite(pingPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(pingPin, LOW);
  
  long duration = pulseIn(echoPin, HIGH);
  return microsecondsToCentimeters(duration);
}

long measureWeight()
{
  // read until stable
  w1 = scale.get_units(10);
  delay(100);
  w2 = scale.get_units();
  while (abs(w1 - w2) > 10)
  {
     w1 = w2;
     w2 = scale.get_units();
     delay(100);
  }
  return w1;
} 

void removeFood(){
  for(int posDegrees = 0; posDegrees <= 180; posDegrees++) {
        servoBase_1.write(posDegrees);
        servoBase_2.write(180-posDegrees);
        Serial.println(posDegrees);
        delay(5);
    }

    for(int posDegrees = 180; posDegrees >= 0; posDegrees--) {
        servoBase_1.write(posDegrees);
        servoBase_2.write(180-posDegrees);
        Serial.println(posDegrees);
        delay(5);
    }
}

void systemBusy(){
  digitalWrite(redIndicator,HIGH);
  digitalWrite(greenIndicator,LOW);
}

void systemAvailable(){
  digitalWrite(redIndicator,LOW);
  digitalWrite(greenIndicator,HIGH);
}

void readColour(int Array[3]){
  // Setting RED filtered photodiodes to be read
  digitalWrite(S2,HIGH);
  digitalWrite(S3,HIGH);
  frequency = pulseIn(sensorOut, LOW);
  Array[0] = frequency;
  delay(100);

  // Setting GREEN filtered photodiodes to be read
  digitalWrite(S2,LOW);
  digitalWrite(S3,HIGH);
  frequency = pulseIn(sensorOut, LOW);
  Array[1] = frequency;
  delay(100);

  // Setting BLUE filtered photodiodes to be read
  digitalWrite(S2,LOW);
  digitalWrite(S3,LOW);
  frequency = pulseIn(sensorOut, LOW);
  Array[2] = frequency; 
  delay(100);
}
