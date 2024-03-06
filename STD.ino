/****
1. No longer has Stepper structure, as more structure the system has, the more prone to error
2. Changing the ultrasonic sensor to weight detection, once a weight is detected, it will automatically get rid of the food waste (TBC)
****/

#include <WiFi.h>
#include <Servo.h>
#include "HX711.h"

const char* ssid = "YOUR NETWORK SSID"; // Replace with your network credentials
const char* password = "YOUR NETWORK PASSWORD";

WiFiServer server(80); // Set web server port number to 80

String header; // Variable to store the HTTP request

// ATTRIBUTES
long distance, plate_weight;
String plate_colour;
int online_counter = 0;

// SAVED DATA
String saved_plate_colour;
long saved_counter = 999;
int saved_colour[3] = {255,255,255};
long saved_weight = 999;

// COLOUR SENSOR
#define S2 33
#define S3 32
#define sensorOut 35

int colour[3] = {0,0,0};
int frequency = 0;

// LOAD CELL
HX711 scale;
uint8_t dataPin = 18;
uint8_t clockPin = 19;
long w1, w2, previous = 0;

// SERVO 
// 12,14,27,26
// -2R, -2L , -1R, -1L
static const int servosPins[4] = {4, 16, 22, 23};
static const int servosOffset[4] = {65, 69, 0, 0}; 
//static const int servosOffset[4] = {20, 24, 0, 0}; 
Servo servos[4];

void setup() {
  Serial.begin(115200);

  // SETUP WIFI
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  // SETUP SERVO
  for(int i = 0; i < 4; ++i) {
        if(!servos[i].attach(servosPins[i])) {
            Serial.print("Servo ");
            Serial.print(i);
            Serial.println("attach error");
        }
  }

  // SETUP COLOUR SENSOR
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(sensorOut, INPUT);

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

              // LIVE DATA
              readColour(colour); // update colour array
              online_counter += 1;
              plate_colour = String(colour[0]) + " " + String(colour[1]) + " " + String(colour[2]);
              plate_weight = measureWeight()      ; // weight (in g) of the food waste, to be integrated with the weight detector 

              // SAVED DATA
              saved_plate_colour = String(saved_colour[0]) + " " + String(saved_colour[1]) + " " + String(saved_colour[2]);

              client.println("<html><body>");
              client.println("<h1>SMART TRAY DEPOSIT SERVER - TPSS</h1>");
              
              client.println("<h2>LIVE - DATA</h2>");
              client.print("<p> Online Counter: " + String(online_counter) + " </p>");
              client.print("<p> Plate Colour: " + String(plate_colour) + " </p>");
              client.print("<p> Plate Weight: " + String(plate_weight) + " </p>");

              client.println("<h2>SAVED - DATA</h2>");
              client.print("<p> Saved Counter: " + String(saved_counter) + " </p>");
              client.print("<p> Saved Colour: " + String(saved_plate_colour) + " </p>");
              client.print("<p> Saved Weight: " + String(saved_weight) + " </p>");

              client.println("</body></html>");

              if(plate_weight >= 10){ 
                saved_counter = online_counter;
                saved_colour[0] = colour[0];
                saved_colour[1] = colour[1];
                saved_colour[2] = colour[2];
                saved_weight = plate_weight;
                _removeFood_();
              }
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
              client.println("<script>function refreshData() {var xhttp = new XMLHttpRequest();xhttp.onreadystatechange = function() {if (this.readyState == 4 && this.status == 200) {document.body.innerHTML = this.responseText;}};xhttp.open(\"GET\", \"/sensor\", true);xhttp.send();}setInterval(refreshData, 1000);</script>");
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

long measureWeight()
{
  // read until stable
  w1 = scale.get_units(10);
  delay(50);
  w2 = scale.get_units();
  while (abs(w1 - w2) > 10)
  {
     w1 = w2;
     w2 = scale.get_units();
     delay(50);
  }
  return w1;
} 

void readColour(int Array[3]){
  // Setting BLUE filtered photodiodes to be read
  digitalWrite(S2,LOW);
  digitalWrite(S3,LOW);
  frequency = pulseIn(sensorOut, LOW);
  Array[2] = frequency;
  delay(100);

  // Setting GREEN filtered photodiodes to be read
  digitalWrite(S2,LOW);
  digitalWrite(S3,HIGH);
  frequency = pulseIn(sensorOut, LOW);
  Array[1] = frequency; 
  delay(100);

  // Setting RED filtered photodiodes to be read
  digitalWrite(S2,HIGH);
  digitalWrite(S3,HIGH);
  frequency = pulseIn(sensorOut, LOW);
  Array[0] = frequency;
  delay(100);
}

void set_N2BaseServos(int degrees, int max_degrees) {
    for(int i = 0; i < 2; ++i) {
      if(i %2 == 0){ // Even-Indexed Servo
        servos[i].write(((degrees + servosOffset[i]) + (35 * i)) % 180);
      }else{ // Odd-Indexed Servo
        servos[i].write(((max_degrees - degrees - servosOffset[i]) + (35 * i)) % 180);
      }
    }
}

void set_N1BaseServos(int degrees, int max_degrees) {
    for(int i = 2; i < 4; ++i) {
      if(i %2 == 0){ // Even-Indexed Servo
        servos[i].write(((degrees + servosOffset[i]) + (35 * i)) % 180);
      }else{ // Odd-Indexed Servo
        servos[i].write(((max_degrees - degrees - servosOffset[i]) + (35 * i)) % 180);
      }
    }
}

void _N2_BaseOpen(){
  int sweep_range = 90;
  for(int posDegrees = 0; posDegrees <= sweep_range; posDegrees++) {
      set_N2BaseServos(posDegrees,sweep_range);
      Serial.println(posDegrees);
      delay(20);
  }
  Serial.println("-2 opened");
}

void _N2_BaseClose(){
  int sweep_range = 90;
  for(int posDegrees = sweep_range; posDegrees >= 0; posDegrees--) {
    set_N2BaseServos(posDegrees,sweep_range);
    Serial.println(posDegrees);
    delay(20);
  }
  Serial.println("-2 closed");
}

void _N1_BaseOpen(){
  int sweep_range = 45;
  for(int posDegrees = 0; posDegrees <= sweep_range; posDegrees++) {
      set_N1BaseServos(posDegrees,sweep_range);
      Serial.println(posDegrees);
      delay(20);
  }
  Serial.println("-1 opened");
}

void _N1_BaseClose(){
  int sweep_range = 45;
  for(int posDegrees = sweep_range; posDegrees >= 0; posDegrees--) {
    set_N1BaseServos(posDegrees,sweep_range);
    Serial.println(posDegrees);
    delay(20);
  }
  Serial.println("-1 closed");
}


void _removeFood_(){
  _N1_BaseOpen();
  delay(2000);
  _N1_BaseClose();
  delay(1000);
  _N2_BaseOpen();
  delay(2000);
  _N2_BaseClose();
}
