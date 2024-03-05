## SMART TRAY DEPOSIT Project Code Documentation

### Overview
The SMART TRAY DEPOSIT project is an innovative solution designed to streamline the process of tray return in cafeterias, food courts, and similar dining environments. This project introduces an automated system that encourages diners to return their trays efficiently, thereby promoting cleanliness and operational efficiency. The implementation of this system not only aids in maintaining a cleaner dining area but also reduces the workload on the cleaning staff, making the dining experience more pleasant for everyone involved.

### Credits
This project is credited to the talented team from Tampines Secondary School. The idea was conceived and developed by the students as part of their initiative to address the common issue of unreturned trays in dining areas. As a mentor, I have had the privilege of guiding and assisting the team in bringing their innovative idea to life.

### System Description

The SMART TRAY DEPOSIT system incorporates a suite of hardware components, such as Wi-Fi connectivity, servo motors, a color sensor, and a load cell. These components work together to identify the tray's color and weight, automatically segregating food waste based on predefined criteria.

### Key Components and Setup

- **Wi-Fi Connectivity:**
  Establishes network connectivity to enable remote monitoring and control.
  ```cpp
  const char* ssid = "YourSSID"; // Replace with your network credentials
  const char* password = "YourPassword";
  WiFiServer server(80); // Set web server port number to 80
  ```

- **Servo Motors:**
  Four servo motors are used for the mechanism that segregates food waste.
  ```cpp
  Servo servos[4]; // Array to hold servo objects
  ```

- **Color Sensor:**
  Determines the tray's color, aiding in data collection.
  ```cpp
  #define S2 33
  #define S3 32
  #define sensorOut 35
  ```

- **Load Cell (HX711):**
  Measures the weight on the tray, triggering the waste removal process if a threshold is exceeded.
  ```cpp
  HX711 scale;
  uint8_t dataPin = 18;
  uint8_t clockPin = 19;
  ```

### Program Flow

1. **Initialization:** Configures the hardware components and network settings.
   ```cpp
   void setup() {
     // Setup Wi-Fi, servo motors, color sensor, and load cell here
   }
   ```

2. **Web Server:** A simple web server allows live monitoring of system status.
   ```cpp
   void loop() {
     WiFiClient client = server.available(); // Listen for incoming clients
     if (client) {
       // Process client requests here
     }
   }
   ```

3. **Color and Weight Detection:** The system reads the tray's color and weight to decide on waste removal.
   ```cpp
   long measureWeight() {
     // Measure and return the weight
   }
   void readColour(int colour[3]) {
     // Read and store the tray's color
   }
   ```

4. **Servo Control:** Controls the servo motors to open/close the gates for waste segregation.
   ```cpp
   void _N1_BaseOpen() {
     // Open the -1 base gate
   }
   void _N1_BaseClose() {
     // Close the -1 base gate
   }
   ```

5. **Waste Removal Process:** Activates upon detecting a weight above a certain threshold.
   ```cpp
   if(plate_weight >= 10){ // Threshold check
     _removeFood_(); // Activates the waste removal mechanism
   }
   ```

### Modifications and Improvements

Significant changes were made to the initial design to enhance its functionality:

- **Stepper Motors Removed:** Opted for servo motors for their simplicity and reliability.
- **Switch to Weight Detection:** Transitioned from an ultrasonic sensor to a load cell for precise weight measurement.

### Code Snippets

- **Wi-Fi Setup and Web Server Initialization:**
  ```cpp
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  server.begin();
  ```

- **Servo Motor Setup:**
  ```cpp
  for(int i = 0; i < 4; ++i) {
    servos[i].attach(servosPins[i]);
  }
  ```

- **Color Detection Setup:**
  ```cpp
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(sensorOut, INPUT);
  ```

- **Load Cell Calibration and Weight Measurement:**
  ```cpp
  scale.begin(dataPin, clockPin);
  scale.set_scale(482.643219); // Calibration value
  long plate_weight = measureWeight(); // Get weight measurement
  ```

- **Food Waste Removal Activation:**
  ```cpp
  if(plate_weight >= 10) { // Threshold for waste presence
    _removeFood_(); // Engage waste removal mechanism
  }
  ```

### Proposed Improvements for the SMART TRAY DEPOSIT Project

#### 1. Power Supply Stabilization

**Issue:** The current motor driver does not provide a stable power supply for the system's needs, affecting the performance and reliability of the servo motors.

**Proposed Solution:** Implement a proper power converter designed to handle the system's power requirements efficiently. A dedicated power supply unit (PSU) with a higher current rating and voltage stability should be considered. This PSU should offer regulated output to ensure consistent performance across all components, especially the servo motors which are sensitive to power fluctuations.

**Implementation Suggestion:**
- Research and select a PSU with sufficient current output and voltage stability.
- Ensure the PSU has short-circuit and over-current protection for safety.
- Integrate the new PSU into the system and test with the existing load to confirm stability improvements.

#### 2. Servo Motor Functionality

**Issue:** Unstable power supply has led to the malfunctioning of one of the servo motors, impacting the system's operational efficiency.

**Proposed Solution:** After stabilizing the power supply, assess the affected servo motor for potential damage. If the servo is damaged, replace it with a new one of the same model to ensure compatibility and performance consistency. Additionally, consider using servo motors with higher tolerance to voltage fluctuations for future resilience.

**Implementation Suggestion:**
- Test each servo motor individually with the new power supply to identify any damage or performance issues.
- Replace any damaged or non-functional servo motors.
- Optionally, upgrade all servo motors to models known for robust performance under varying power conditions.

#### 3. Website Aesthetics Improvement

**Issue:** The current web interface lacks visual appeal and may not provide the best user experience.

**Proposed Solution:** Enhance the website's appearance and usability by employing modern CSS techniques. Focus on responsive design to ensure the interface is accessible on various devices, including smartphones, tablets, and desktop computers.

**Implementation Suggestion:**
- Use CSS frameworks such as Bootstrap or Tailwind CSS for rapid development and a polished look.
- Incorporate interactive elements using JavaScript for a dynamic user experience.
- Test the website on multiple devices to ensure compatibility and responsiveness.

#### 4. Plate Drop Mechanism Efficiency

**Issue:** The mechanism for dropping the uppermost plate (0 plate) requires improvement to ensure reliable operation.

**Proposed Solution:** Reevaluate the design and mechanism used for dropping the plates. Consider implementing a more reliable actuation method that provides consistent plate release under varying load conditions.

**Implementation Suggestion:**
- Explore alternative mechanisms such as a more powerful servo motor, a solenoid actuator, or a cam mechanism for reliable plate release.
- Prototype and test different designs to identify the most effective solution.
- Ensure the chosen solution integrates seamlessly with the existing system architecture.

#### 5. Servo Connection Reliability

**Issue:** The current servo connections, specifically the screw connections, are prone to loosening, affecting the system's reliability.

**Proposed Solution:** Upgrade the servo connections to more robust and reliable alternatives. Use lock nuts, thread-locking fluid, or soldered connections where applicable to prevent loosening due to vibration or frequent use.

**Implementation Suggestion:**
- Assess each servo connection point and identify those most susceptible to loosening.
- Apply thread-locking fluid to screw connections or replace them with lock nuts where feasible.
- Consider soldering connections that do not require regular disassembly for maintenance, ensuring a secure and permanent connection.

### Conclusion

The SMART TRAY DEPOSIT system, developed by the Tampines Secondary School team with mentorship support, presents an innovative approach to managing tray returns and waste segregation. This documentation outlines the technical framework of the system, providing insights into its operation and the rationale behind design decisions. Through careful integration of hardware and software components, the project exemplifies how technology can be leveraged to solve practical problems in public dining settings.
