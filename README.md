# Smart-House-Danger-Detector
Embedded System project 

This project involves an ESP32 microcontroller-based system designed for environmental monitoring and security alerting. The system utilizes an MQ2 gas sensor and a sound sensor to detect gas levels and sound intensity, respectively. The ESP32 processes the sensor data and transmits it to a web server via WebSockets for real-time monitoring. Additionally, the system can trigger a buzzer and send alerts when certain thresholds are exceeded, enhancing security by detecting unusual activities based on sound levels.

Inputs:

•	MQ2 gas sensor (CH4_PIN)

•	Sound sensor (sound_PIN)

Outputs:

•	Buzzer (buzzer)

•	WebSocket messages to a web server

Processing:

•	The ESP32 reads data from the gas and sound sensors.

•	It processes the data to check for threshold breaches.

•	If thresholds are exceeded, it triggers the buzzer and sends alert messages via WebSockets.

•	The system also supports setting security times through a web interface, allowing for scheduled monitoring.

Keywords:

•	Environmental Monitoring:  This refers to the process of continuously observing and measuring environmental parameters such as gas levels and sound intensity. In this project, the ESP32 microcontroller uses sensors to monitor these parameters in real-time.

•	Security Alerts:  These are notifications or warnings triggered when certain predefined conditions are met, such as high gas levels or loud noises. The system uses a buzzer and WebSocket messages to alert users.

•	Real-time Data:  This refers to the immediate processing and transmission of data as it is collected by the sensors. The ESP32 sends this data to a web server using WebSockets, allowing for instant monitoring and response.

•	ESP32: The ESP32 is a powerful microcontroller with built-in WiFi and Bluetooth capabilities. It is used in this project to read sensor data, process it, and communicate with a web server.

•	WebSockets: WebSockets are a communication protocol that provides full-duplex communication channels over a single TCP connection. In this project, WebSockets are used to send sensor data and alerts from the ESP32 to a web server.

# Design

Design:-
System Design
The system is designed to monitor environmental conditions and provide security alerts using an ESP32 microcontroller. It integrates various sensors to detect gas levels and sound intensity, processes the data, and communicates with a web server for real-time monitoring and alerting. The system also includes a buzzer for audible alerts and supports deep sleep mode to conserve power.
Components Used
1.	ESP32 Microcontroller:
o	Function: Acts as the central processing unit of the system. It reads data from sensors, processes the information, and communicates with the web server via WiFi.
o	Features: Built-in WiFi and Bluetooth, multiple GPIO pins, ADC channels, and support for FreeRTOS.
2.	MQ2 Gas Sensor (CH4_PIN):
o	Function: Detects the concentration of various gases, including methane (CH4). It provides an analog output proportional to the gas concentration.
o	Connection: Connected to an analog input pin (GPIO 36) on the ESP32.
3.	Sound Sensor (sound_PIN):
o	Function: Measures sound intensity. It provides an analog output that varies with the sound level.
o	Connection: Connected to an analog input pin (GPIO 34) on the ESP32.
4.	Buzzer (buzzer):
o	Function: Provides audible alerts when certain conditions are met, such as high gas levels or loud noises.
o	Connection: Connected to a digital output pin (GPIO 2) on the ESP32.
5.	WiFi Module (Built-in ESP32):
o	Function: Enables the ESP32 to connect to a WiFi network and communicate with a web server using WebSockets.
o	Configuration: WiFi credentials are provided in the code (ssid and password).
6.	WebSockets Server:
o	Function: Facilitates real-time communication between the ESP32 and a web server. It allows the ESP32 to send sensor data and alerts to the web interface.
o	Configuration: The WebSocket server is initialized on port 81.
7.	FreeRTOS Tasks:
o	Function: Manages concurrent operations on the ESP32. Different tasks handle sensor data collection, security checks, and power management.
o	Tasks: 
	opentask: Initializes the system and starts the data collection process.
	gastemp: Reads gas sensor data and checks for threshold breaches.
	security: Monitors sound levels and triggers security alerts.
	sleeptask: Manages power-saving by putting the ESP32 into deep sleep mode when appropriate.
8.	Semaphores:
o	Function: Synchronize tasks to ensure smooth operation and avoid conflicts.
o	Types: openSemaphore, gastempSemaphore, securitySemaphore, sleepSemaphore.
9.	ESP32Time Library:
o	Function: Manages time-related functions, such as getting the current hour for security checks.
o	Configuration: Time offset is set to GMT+3.
System Operation
1.	Initialization:
o	The ESP32 initializes the sensors, semaphores, and WiFi connection.
o	It starts the WebSocket server and creates FreeRTOS tasks.
2.	Data Collection:
o	The gastemp task reads data from the MQ2 gas sensor and sends it to the web server.
o	The security task reads data from the sound sensor, checks for security alerts, and sends data to the web server.
3.	Alerting:
o	If gas levels exceed a threshold, the system triggers the buzzer and sends an alert message.
o	If sound levels exceed a threshold during the specified security time, the system triggers the buzzer and sends a security alert.
4.	Power Management:
o	The sleeptask manages the power-saving mode by putting the ESP32 into deep sleep when no alerts are active.
