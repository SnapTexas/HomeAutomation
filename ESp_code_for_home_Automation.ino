//Importing the essential lib
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
//above libs are required to communicate with the dht11(Temprature and humidity) Sensor
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
//above libs are required for i2c communication with the lcd display (16x2)
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
//above libs are reuired for communicating using MQTT(Message Queuing Telemetry Transport) protocol

// Set the LCD address to 0x27 or 0x3F (most common), 16 columns and 2 rows
LiquidCrystal_I2C lcd(0x27, 16, 2); 
//defiing Pins and Parameters
#define DHTPIN 14 //pin on which dht11 sensor is connected on esp8266 D5
//Pins of the Output devices In my case I have used led to resemble fan and humdifier on the output there are leds which rep them ,they can be replaced by actual things
#define light_pin 2
#define fan_pin 15
#define humidity_pin 12
//defining the type of sensor which we are using from dhtxx series
#define DHTTYPE DHT11
//Pins and Parameter for ppm calculation
#define MQ_PIN A0
#define RL 10.0         // kΩ
#define R0 76.63 
//Creates an array of size 100 which is used to store the data which is being send to the user via MQTT broker
char msg_send[100];
//               15 fan 12 humidty 2 light            
int devices[]={fan_pin,humidity_pin,light_pin};
char mode_selected='a';//by default the mode select is set to auto as the user has not interacted with devices
//                     fan,humidity,light
char devices_status[]={'0' ,'0' ,'0'};//These are the initial states of the devices
const char* ssid="Shaikh";//These are the feilds/paramter that require the user to put their wifi name and password
const char* passwd="Tplink7241";//IF not correctly put the values of ssid and password the micro controller will be stuck in trying to connect to server but it can't in a loop
//display will also show nothing

int publish_interval=10000;//Change the interval of time as required the esp8266 will publish data to esp->broker->user in 10 s time is stored in millis 

unsigned long lastPublish=0;//Var used in the time interval calculations 
// MQTT Broker
const char* mqtt_server = "broker.hivemq.com";//MQTT broker server which we are using 
const int mqtt_port = 1883; //Port of the server which we want to connect to
const char* mqtt_sub_topic = "HomeAutomationPrjctFostride/Esp/Pubdata";// topic to subscribe to
const char* mqtt_pub_topic = "HomeAutomationPrjctFostride/Esp/Subdata";// topic to publish to


//creating a wifi client to connect with the internet 
WiFiClient espClient;
PubSubClient client(espClient);//the PubSubClient cannot on its own connect to the internet it reuires other objects to handel the lower level stuff

//setting up the dht obj with the pin and type
DHT_Unified dht(DHTPIN, DHTTYPE);
uint32_t delayMS = 2000; // default delay in case sensor fails
// This function handles the Wi-Fi connection
 
void wifi_setup(){
  delay(10);
  WiFi.begin(ssid,passwd);
  while(WiFi.status()!=WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected");

}
//This function updates the States of the devices store in devices status array
void update_devices_states(int device_index,char state){
   
    devices_status[device_index]=state;
}
//this function is called when a message is recieved on from the broker 
//This function reads the incomming data and processes it and stores in the designated places
//The message being sent by the user is in the format of mxxx where m is mode which to operate and xxx are the states of the fan,humidity,light ('0' OFF/ON '1')
void msg_recieved(char* topic,byte* payload, unsigned int length){
  String msg="";
  for (unsigned int i=0;i<length;i++){
    msg+=(char)payload[i];

  }
   if (msg.length() > 0) {
    if (msg[0] == 'a') {
      mode_selected = 'a';
      Serial.println("Mode set to AUTO");
    } else if (msg[0] == 'm') {
      mode_selected = 'm';
      Serial.println("Mode set to MANUAL");
      if (msg.length() >= 4) {
        devices_status[0] = msg[1];
        devices_status[1] = msg[2];
        devices_status[2] = msg[3];
      } else {
        Serial.println("Manual command too short.");
      }
    }
  

  Serial.print("recieved msg:");
  Serial.print(msg);
  
  
  
}}
//handels the reconnection to the server(broker) 
void reconnect(){
  while(!client.connected()){
    Serial.print("Attempting MQTT connection...");
    if(client.connect("ThisIsPrivateIdForEspHomeAutomation")){
      Serial.println("Connected");
      client.subscribe(mqtt_sub_topic);
    }
    else{
      Serial.println("failed ,rc=");
      Serial.print(client.state());
      Serial.println("\n Trying in 5s ");
    }
    delay(5000);
  }
}
//used to control fan output in normal conditions(Mode:AUTO)
//takes the temprature reading and decides based on condition t>30.0 then turn on fan else not
void control_fan(float temperature){
    if(temperature>30.0){
        digitalWrite(devices[0],HIGH);
        Serial.println("FAN :ON");
    }
    else{
        digitalWrite(devices[0],LOW);
        Serial.println("FAN :OFF");
    }
}
bool Dark=digitalRead(13); //D7 on Esp8266
//Used to control light in Mode:AUTO
//takes an digital signal as input it is high when its dark (I have used an arduino as a sensor as sensor was not available and esp8266 doesn't have more than 1 analog pin)
//arduino produces high signal when its dark 5v is reduced to 3.3v by voltage divider  
void control_light(bool dark) {
  if (dark) {
    digitalWrite(devices[2], HIGH);
    Serial.println("Light : ON");
  } else {
    digitalWrite(devices[2], LOW);
    Serial.println("Light : OFF");
  }
}

//Used to control humidity level in Mode:AUTO
//takes a float as input it is high when its humdity is <40.0 (I have used an led on the output as i could not source a humidifier)
//arduino produces high signal when its dark 5v is reduced to 3.3v by voltage divider 
void control_humid(float humidity){
    if (humidity<40.0){
        digitalWrite(devices[1],HIGH);
        Serial.println("Humid :ON");
    }
    else{
        digitalWrite(devices[1],LOW);
        Serial.println("Humid :OFF");
    }
}
//Code runs once used for initializations
void setup() {
  Serial.begin(115200);
  
  wifi_setup();//func call
  client.setServer(mqtt_server,mqtt_port);
  client.setCallback(msg_recieved);//binding the msg_recieved fun to call back when ever the broker sends some data this function will be called
  //Sets up the sensor
  dht.begin();
  
  lcd.init();        // Initialize LCD
  lcd.backlight();   // Turn on backlight
  lcd.setCursor(0, 0);


  // Get delay from sensor
  sensor_t sensor;//sensor obj is used to store the data from dht sensor
  dht.temperature().getSensor(&sensor);
  delayMS = sensor.min_delay / 1000;
  //looping through the devices and defining them as O/P
  for (int i=0;i<sizeof(devices)/sizeof(devices[0]);i++){
    pinMode(devices[i],OUTPUT);
  }
  pinMode(16, INPUT);  // Light sensor pin where data is being read different from light_pin
  pinMode(A0, INPUT);  // Air quality sensor
}
void loop() {
  int index = 0;//var used to acces the devices using indexing
  delay(delayMS);
  lcd.clear();//clears display
  msg_send[0] = '\0';//initializes array to empty 
  String toSenddata="";
  

  sensors_event_t temp_event, humid_event;
  dht.temperature().getEvent(&temp_event);
  dht.humidity().getEvent(&humid_event);

  // Temperature
  if (!isnan(temp_event.temperature)) {
    float value_t = temp_event.temperature;
    toSenddata+="temp:"+String(value_t)+"C ";
    //Checking for which mode it is to work accordingly
    if (mode_selected == 'm') {
      digitalWrite(devices[index], devices_status[index] == '1' ? HIGH : LOW);
    } else {

      control_fan(value_t);
    }
  } 
// Incrementing the index so that we can access the next device
  index++;//index =1 

  // Humidity
  if (!isnan(humid_event.relative_humidity)) {
    float value_h = humid_event.relative_humidity;
    toSenddata+="humid:"+String(value_h)+"% ";
    //appending 
    

 
    if (mode_selected == 'm') {
      digitalWrite(devices[index], devices_status[index] == '1' ? HIGH : LOW);
    } else {
      control_humid(value_h);
    }
  } else {
    //Serial.println("Humidity Error");
  }

  index++;//index=2

  // Light Sensor
  Dark = digitalRead(13);  
  Serial.println("Light sensror pin oup:");
  Serial.println(Dark);
  if (mode_selected == 'm') {
    digitalWrite(devices[index], devices_status[index] == '1' ? HIGH : LOW);
    Serial.print("LIGHT:");
    Serial.println(devices_status[index] == '1' ? HIGH : LOW);
    toSenddata+="light:"+String(devices_status[index] == '1' ? "ON " : "OFF ");
  } else {
    control_light(Dark);
  }
 

  // Air Quality

  int adc = analogRead(MQ_PIN);
  float v = adc * 3.3 / 1023.0;
  float sensorRs = (3.3 - v) * RL / v;
  float ratio = sensorRs / R0;
  float ppm = 116.6 * pow(ratio, -2.769);  // Approx. for CO₂
  toSenddata+="aq:"+String(ppm)+" ";

  strcat(msg_send, toSenddata.c_str());

  //check if client is connected or not 
  if (!client.connected()) {
    reconnect();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Reconnecting...");
  }
  // for timing publishing data 
  unsigned long now = millis();
  if (now - lastPublish > publish_interval) {
    lastPublish = now;
    client.publish(mqtt_pub_topic, msg_send);
    //Serial.print("Published: ");
    //Serial.println(msg_send);
  }
//overruding dark var when necessary 
if(mode_selected=='m' && devices_status[2]=='1'){
  Dark=true;
}
//Show all data to the lcd
lcd.setCursor(0, 0);
lcd.print("T:");
lcd.print((int)temp_event.temperature);
lcd.print(" H:");
lcd.print((int)humid_event.relative_humidity);

// Line 2: Light & AQ
lcd.setCursor(0, 1);
lcd.print("L:");
lcd.print(Dark ? "ON " : "OFF");
lcd.print(" AQ:");
lcd.print((int)ppm);
  //maintains connection and handles call back
  client.loop();
  delay(1000);
}
