

import paho.mqtt.client as mqtt
import time

# MQTT broker settings
broker = "broker.hivemq.com"
port = 1883
publish_topic_data = "HomeAutomationPrjctFostride/Esp/Pubdata"

subscribe_topic="HomeAutomationPrjctFostride/Esp/Subdata"

# Create MQTT client
client = mqtt.Client()

# Define callback for received messages
def on_message(client, userdata, msg):
    with open('logs.txt','a') as f:
         f.write('\n')
         f.write(msg.payload.decode())
         
    

client.on_message = on_message  # Set the callback

# Connect to broker
client.connect(broker, port)


client.subscribe(subscribe_topic)
client.loop_start()  # Start the loop in the background

# Main control loop
while True:
    print("\nDevices: Fan, Light, Humidity Controller")
    print("Select mode: auto (a) or manual (m)")
    mode=input("seletect mode auto or manual : (a/m)")
    
    
    device_names = ["Fan",  "Humidity","Light"]

    

    if mode == 'a':
        print(f"AUTO mode")
        client.publish(publish_topic_data,'a000')
        print("published mode a")
                                          
    elif mode == 'm':
            fan = input("Fan press 1 to turn on 0 to turn off: ")
            light = input("Humidity press 1 to turn on 0 to turn off:")
            humid_s = input("Light press 1 to turn on 0 to turn off:")
            client.publish(publish_topic_data,f'm{fan+light+humid_s}')
            
            
           
            
    else:
            print(f"Invalid mode  Skipping...")
    show_data=input("want to see data (y/n):")
    if show_data=='y' :
        with open('logs.txt','r') as f:
            print(f.read())
         
    continue_or_not = input("\nPress 'q' to quit or Enter to continue: ").lower()
    if continue_or_not == 'q':
        break

client.loop_stop()     # Stop background thread
client.disconnect()    # Disconnect cleanly
