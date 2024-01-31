#include <WiFi.h>

const char* ssid = "TP-Link_D8F2"; // Replace with your network credentials
const char* password = "53717221";

WiFiServer server(80); // Set web server port number to 80

String header; // Variable to store the HTTP request

const int pingPin = 19; // Trigger Pin of Ultrasonic Sensor
const int echoPin = 18; // Echo Pin of Ultrasonic Sensor

int count = 0;
long distance;

void setup() {
  Serial.begin(115200);
  pinMode(pingPin, OUTPUT);
  pinMode(echoPin, INPUT);

  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
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
              count++; // increment the count
              
              client.println("<html><body>");
              
              client.println("<h1>SMART TRAY DEPOSIT SERVER - TPSS</h1>");
              
              // send the sensor data
              client.print("Distance: ");
              client.print(distance);
              client.print(" cm\n");
              client.print("Count: ");
              client.print(count);

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
