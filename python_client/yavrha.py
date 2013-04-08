#!/usr/bin/env python3

# This code connect to a serial port and send data to a MQTT server

import serial
import json
import time
import mosquitto

# Global Variables
REFRESH_DELAY = 10           # Delay in seconds to send new values only
FULL_REFRESH_DELAY = 10     # Will send all the configuration in REFRESH_DELAY * FULL_REFRESH_DELAY seconds
ACTIVE_NODES = []
NODES = {}          # Nested Dictionary in {"node1":{"name":"blabla", "type":1, etc.}, "node2":{"name":"blabla", etc...
MQTT_SERVER = "test.mosquitto.org"
MQTT_TOPIC = "yavrha"	

# Serial Port Configuration
ser = serial.Serial('/dev/ttyACM0',19200, timeout=.1)


# Create Mosquitto Instance
client = mosquitto.Mosquitto("yavrha-client")
client.connect(MQTT_SERVER)


# Load Node Configuration
command = bytes("print_cfg \r\n","utf-8")
# make sure input buffer is empty
ser.flushInput()
ser.write(command)
# read with the intention of delete the command sent
ser.readline()
Node_Cfg = (ser.read(1000))
Node_Cfg_string = "[" + Node_Cfg.decode() +"]"
#print(Node_Cfg_string,"\n******\n")
Node_Cfg_Obj = json.loads(Node_Cfg_string)
#print(Node_Cfg_Obj[0]['channel'])
#print(Node_Cfg_Obj[0]['home_addr'])
#print(Node_Cfg_Obj[0]['node0'])
i=0
while 'node'+str(i) in Node_Cfg_Obj[0]: 
    if Node_Cfg_Obj[0]['node'+str(i)]['enable'] == 1:
        ACTIVE_NODES.append(i)

#   Load information in NODES object
        if 'node'+str(i) not in NODES:
            NODES["node"+str(i)] = {}       
        else:
            NODES["node"+str(i)]["name"] = str(Node_Cfg_Obj[0]['node'+str(i)]['name'])
            NODES["node"+str(i)]["type"] = str(Node_Cfg_Obj[0]['node'+str(i)]['type'])
            NODES["node"+str(i)]["address"] = str(Node_Cfg_Obj[0]['node'+str(i)]['address'])
            NODES["node"+str(i)]["number"] = str(Node_Cfg_Obj[0]['node'+str(i)]['number'])


#       print(Node_Cfg_Obj[0]['node'+str(i)])
        client.publish(MQTT_TOPIC + "/node"+str(i)+"/name",str(Node_Cfg_Obj[0]['node'+str(i)]['name']), 1, True)
        client.publish(MQTT_TOPIC + "/node"+str(i)+"/type",str(Node_Cfg_Obj[0]['node'+str(i)]['type']), 1, True)
        client.publish(MQTT_TOPIC + "/node"+str(i)+"/address",str(Node_Cfg_Obj[0]['node'+str(i)]['address']), 1, True)
        client.publish(MQTT_TOPIC + "/node"+str(i)+"/number",str(Node_Cfg_Obj[0]['node'+str(i)]['number']), 1, True)

# This print a emptly message the same a delete old messages from MQTT server in deactivated nodes
    else:
        client.publish(MQTT_TOPIC + "/node"+str(i)+"/name",None , 1, True)
        client.publish(MQTT_TOPIC + "/node"+str(i)+"/type",None , 1, True)
        client.publish(MQTT_TOPIC + "/node"+str(i)+"/address",None, 1, True)
        client.publish(MQTT_TOPIC + "/node"+str(i)+"/number",None, 1, True)

   
    i += 1
#print(ACTIVE_NODES)        


# ********************************************************
# Get data from serial port and publish it in MQTT Server
# Argument "forced", force to publish everything even if it hasn't change
# ********************************************************
def Send_Data(forced):
    ser.flushInput()    # make sure input buffer is empty
    command = bytes("get \r\n","utf-8")
    ser.write(command)
    ser.readline()          # read with the intention of delete the command sent
    Node_Data = (ser.read(500))
    Node_Data_string = "[" + Node_Data.decode() +"]"
#    print(Node_Data_string)

    try:    
        Node_Data_Obj = json.loads(Node_Data_string)
    except	ValueError:
        print("Value Error with json.loads(Node_Data_string)")
        return 0
# Loop through all active nodes

    for item in ACTIVE_NODES:

# This check if the key exist in the object if not it create the key, this is to avoid error in the following if
        if 'msgid' not in  NODES['node'+str(item)]:
            NODES['node'+str(item)]["msgid"] = " "

        if 'data0' not in  NODES['node'+str(item)]:
            NODES['node'+str(item)]["data0"] = " "

        if 'data1' not in  NODES['node'+str(item)]:
            NODES['node'+str(item)]["data1"] = " "

        if 'data2' not in  NODES['node'+str(item)]:
            NODES['node'+str(item)]["data2"] = " "

        if 'data3' not in  NODES['node'+str(item)]:
            NODES['node'+str(item)]["data3"] = " "

# Load new data in NODES object
# Check if new data is different that old one, if so it update the value in the NODES object and publish the value.


        if (NODES['node'+str(item)]["msgid"] != str(Node_Data_Obj[0]['node'+str(item)]['msgid']) or forced ):          
            NODES['node'+str(item)]["msgid"] = str(Node_Data_Obj[0]['node'+str(item)]['msgid'])
            client.publish(MQTT_TOPIC + "/node"+str(item)+"/msgid",str(Node_Data_Obj[0]['node'+str(item)]['msgid']), 1)
        
        if (NODES['node'+str(item)]["data0"] != str(Node_Data_Obj[0]['node'+str(item)]['data0']) or forced ): 
            print(NODES['node'+str(item)]["data0"])            
            NODES['node'+str(item)]["data0"] = str(Node_Data_Obj[0]['node'+str(item)]['data0'])
            client.publish(MQTT_TOPIC + "/node"+str(item)+"/data0",str(Node_Data_Obj[0]['node'+str(item)]['data0']), 1)

        if (NODES['node'+str(item)]["data1"] != str(Node_Data_Obj[0]['node'+str(item)]['data1']) or forced ): 
            NODES['node'+str(item)]["data1"] = str(Node_Data_Obj[0]['node'+str(item)]['data1'])
            client.publish(MQTT_TOPIC + "/node"+str(item)+"/data1",str(Node_Data_Obj[0]['node'+str(item)]['data1']), 1)

        if (NODES['node'+str(item)]["data2"] != str(Node_Data_Obj[0]['node'+str(item)]['data2']) or forced ): 
            NODES['node'+str(item)]["data2"] = str(Node_Data_Obj[0]['node'+str(item)]['data2'])
            client.publish(MQTT_TOPIC + "/node"+str(item)+"/data2",str(Node_Data_Obj[0]['node'+str(item)]['data2']), 1)

        if (NODES['node'+str(item)]["data3"] != str(Node_Data_Obj[0]['node'+str(item)]['data3']) or forced ): 
            NODES['node'+str(item)]["data3"] = str(Node_Data_Obj[0]['node'+str(item)]['data3'])
            client.publish(MQTT_TOPIC + "/node"+str(item)+"/data3",str(Node_Data_Obj[0]['node'+str(item)]['data3']), 1)

		
def Received_Cmd(msg):
    Node = str(msg.topic).lstrip(MQTT_TOPIC+'/').rstrip("/cmd").strip("node")
    command = bytes("send "+Node+" "+msg.payload.decode()+" \r\n","utf-8")
    ser.write(command)
#    print(ser.readline().decode())

def on_connect(mosq, obj, rc):
    if rc == 0:
        print("Connected successfully.")

def on_message(mosq, obj, msg):
#    print("Message received on topic "+str(msg.topic)+" with QoS "+str(msg.qos)+" and payload "+msg.payload.decode())
    Received_Cmd(msg)


# Subscribe to nodes/cmd.
client.subscribe(MQTT_TOPIC+"/+/cmd", 0)

# Set Call back functions
client.on_connect = on_connect
client.on_message = on_message


# Main Loop 
while True:
    start_time = int(time.time())   
    Send_Data(False)

# Inner loop to call mosquitto client in order to keep connection alive.
    while (int(time.time()) < start_time + REFRESH_DELAY):
        client.loop()
        
