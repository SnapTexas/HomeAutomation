1. Sensor Reading Strategy
This system uses environmental sensors to monitor a room’s conditions in near real-time:

•	Temperature & Humidity Sensor: A DHT11 sensor is connected to GPIO14 (D5). It is initialized using the DHT_Unified library for consistent access to temperature() and humidity() values.

•	Light Detection: A digital HIGH/LOW signal from an Arduino which reads the analog signals and interprets them as digital signal based upon a certain threshold is read on GPIO13 (D7) to determine if the room is dark ,if it is dark then the output of the Arduino is high else it is low(An Arduino was used as there is only one digital pin on the esp so Arduino handles that and behaves like an ldr sensor module with digital output).

•	Air Quality Monitoring: An MQ135 analog gas sensor is connected to A0. Voltage is read and converted into an approximate CO₂ PPM using a formula involving the sensor's resistance ratio.

•	All sensors are polled inside the loop() function every few seconds (based on delayMS, with publishing every 10 seconds). Simple error checking (e.g., isnan()) is used to ensure valid readings.

2. Control Logic
User decides the mode in which the system should operate by default it operates on Auto Mode.
Once the user decides to change the behaviour of system from autonomously to manual mode user specifies the parameters like M which tells the system which mode to behave in if user selects manual mode then it asks the user to specify the output of the devices attached to it and their states changes to the specified state by the user .Whiile in manual mode user has full control when exiting the manual mode user has to give control to back to the system for it to behave autonomously user has to specify mode ‘a’(autonomously) which gives control back to the system.

•	Manual Mode (m): Device states are explicitly set by the user through MQTT messages like "m101" (Fan ON, Humidifier OFF, Light ON).

•	Auto Mode (a): The system autonomously controls devices based on sensor values:

Device	Trigger Condition
Fan	ON if temperature > 30°C
Humidifier	ON if humidity < 40%
Light	ON if dark == true
The devices_status[] array holds the states of each device, which are updated either from the MQTT message or logic in the auto functions (control_fan(), control_humid(), etc.).

4. Communication Handling
Communication is handled using the MQTT protocol via the PubSubClient library:

•	Broker: HiveMQ public broker (broker.hivemq.com, port 1883)

•	Topics:

o	Subscribe: HomeAutomationPrjct/Esp/Pubdata – receives mode/device commands
o	Publish: HomeAutomationPrjct/Esp/Subdata – sends sensor data periodically.
Commands format:
The command the user mode which user selects and the choices that user has made on the states of the devices are combined into a single string the first character of the string specifies to the system which mode user has selected ,the 3 remaining characters are the states of the devices in this case there were 3 devices being used so it had 3 states devices re indexed each device has a index according to which is known by both user and system indexing allows for proper state control (ON/OFF) of devices . 
Ex:-“m001” This command tells microcontroller that user wants to use manual mode and the states of devices are (0=OFF & 1=ON) OFF ,OFF, ON the third device will turn on while other devices will turn off.
“axxx” (where x is 0 or 1)This command tells the system to gain back control and behave autonomously 

•	Wi-Fi: Configured via wifi_setup() using SSID and password.

•	A unique ESP ID ("ThisIsPrivateIdForEspHomeAutomation") or can be dynamically genrated using random numbers is used for MQTT connection.

•	Data is published in a compact string format every 10 seconds using millis() timing.

4. Power Considerations
While running on 3.7 v lipo battery in series with another 3.7v lipo battery for powering the system the system has 5v regulated power supply for devices and sensors and a 3.3 v regulated  power supply for powering (IC7805 5v regulator & AMS1117 3.3v regulator ).

•	5V regulated power supply for ESP8266 and sensors.

•	Voltage dividers are used to safely drop the 5V signal from the MQ-135 and Arduino light sensor to 3.3V.

•	Power Efficiency:

o	Delay-based sensor polling for simplicity.
o	Could be improved later using sleep modes for power-saving.


