#!/usr/bin/env python3
'''
               Yavrha
     Copyright (C) Carlos Silva, 2013
      (csaleman [at] gmail [dot] com)

      http://code.google.com/p/yavrha/
'''

'''
    This file is part of Yavrha.

    Yavrha is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Yavrha is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Yavrha.  If not, see <http://www.gnu.org/licenses/>.

'''
# This code connect to a serial port and send data to a MQTT server

import serial
import json
import time
import mosquitto

# Global Variables
REFRESH_DELAY = 1           # Delay in seconds to send new values only
ACTIVE_NODES = []
NODES = {}          # Nested Dictionary in {"node1":{"name":"blabla", "type":1, etc.}, "node2":{"name":"blabla", etc...
MQTT_SERVER = "test.mosquitto.org"
MQTT_TOPIC = "yavrha"	

# Serial Port Configuration
ser = serial.Serial('/dev/ttyACM0',19200, timeout=.1)


# Create Mosquitto Instance
client = mosquitto.Mosquitto("yavrha-client")
client.connect(MQTT_SERVER)

# ********************************************************
# Get Configuration data from serial port and publish it in MQTT Server
# ********************************************************

def Send_Cfg_Data():
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

#   This is a loop through all elements in Node_Cfg_Obj
    i=0
    while 'node'+str(i) in Node_Cfg_Obj[0]: 
#   Add active Nodes in the ACTIVE_NODES Global variable
        if Node_Cfg_Obj[0]['node'+str(i)]['enable'] == 1:
            ACTIVE_NODES.append(i)

#   Load information in NODES object
#   First check if the NODE object exist in NODES before setting properties, if not it create an emptly object 
            if 'node'+str(i) not in NODES:
                NODES["node"+str(i)] = {}     

#   Copy Nodes properties from Node_Cfg_Obj to NODES
            NODES["node"+str(i)]["name"] = str(Node_Cfg_Obj[0]['node'+str(i)]['name'])
            NODES["node"+str(i)]["type"] = str(Node_Cfg_Obj[0]['node'+str(i)]['type'])
            NODES["node"+str(i)]["address"] = str(Node_Cfg_Obj[0]['node'+str(i)]['address'])
            NODES["node"+str(i)]["number"] = str(Node_Cfg_Obj[0]['node'+str(i)]['number'])
            
#   Publish Nodes properties in MQTT Server with retain flag
            client.publish(MQTT_TOPIC + "/node"+str(i)+"/name",str(Node_Cfg_Obj[0]['node'+str(i)]['name']), 1, True)
            client.publish(MQTT_TOPIC + "/node"+str(i)+"/type",str(Node_Cfg_Obj[0]['node'+str(i)]['type']), 1, True)
            client.publish(MQTT_TOPIC + "/node"+str(i)+"/address",str(Node_Cfg_Obj[0]['node'+str(i)]['address']), 1, True)
            client.publish(MQTT_TOPIC + "/node"+str(i)+"/number",str(Node_Cfg_Obj[0]['node'+str(i)]['number']), 1, True)

# If Node was deactivated, this publish a empty message with retain flag to delete old retained messages if any
        else:
            client.publish(MQTT_TOPIC + "/node"+str(i)+"/name",None , 1, True)
            client.publish(MQTT_TOPIC + "/node"+str(i)+"/type",None , 1, True)
            client.publish(MQTT_TOPIC + "/node"+str(i)+"/address",None, 1, True)
            client.publish(MQTT_TOPIC + "/node"+str(i)+"/number",None, 1, True)
        i += 1


# ********************************************************
# Get data from serial port and publish it in MQTT Server
# ********************************************************
def Send_Data():
    time.sleep(0.1)    
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
        print("Value Error with json.loads(Node_Data_string)\n")
        print(Node_Data_string)
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

#        if (NODES['node'+str(item)]["msgid"] != str(Node_Data_Obj[0]['node'+str(item)]['msgid'])):        
#            NODES['node'+str(item)]["msgid"] = str(Node_Data_Obj[0]['node'+str(item)]['msgid'])
#            client.publish(MQTT_TOPIC + "/node"+str(item)+"/msgid",str(Node_Data_Obj[0]['node'+str(item)]['msgid']), 1, True)
            
        if (NODES['node'+str(item)]["data0"] != str(Node_Data_Obj[0]['node'+str(item)]['data0'])):    
            NODES['node'+str(item)]["data0"] = str(Node_Data_Obj[0]['node'+str(item)]['data0'])
            client.publish(MQTT_TOPIC + "/node"+str(item)+"/msgid",str(Node_Data_Obj[0]['node'+str(item)]['msgid']), 1, True)
            client.publish(MQTT_TOPIC + "/node"+str(item)+"/data0",str(Node_Data_Obj[0]['node'+str(item)]['data0']), 1, True)
      
        if (NODES['node'+str(item)]["data1"] != str(Node_Data_Obj[0]['node'+str(item)]['data1'])):
            NODES['node'+str(item)]["data1"] = str(Node_Data_Obj[0]['node'+str(item)]['data1'])
            client.publish(MQTT_TOPIC + "/node"+str(item)+"/msgid",str(Node_Data_Obj[0]['node'+str(item)]['msgid']), 1, True)            
            client.publish(MQTT_TOPIC + "/node"+str(item)+"/data1",str(Node_Data_Obj[0]['node'+str(item)]['data1']), 1, True)
      
        if (NODES['node'+str(item)]["data2"] != str(Node_Data_Obj[0]['node'+str(item)]['data2'])):
            NODES['node'+str(item)]["data2"] = str(Node_Data_Obj[0]['node'+str(item)]['data2'])
            client.publish(MQTT_TOPIC + "/node"+str(item)+"/msgid",str(Node_Data_Obj[0]['node'+str(item)]['msgid']), 1, True)            
            client.publish(MQTT_TOPIC + "/node"+str(item)+"/data2",str(Node_Data_Obj[0]['node'+str(item)]['data2']), 1, True)

        if (NODES['node'+str(item)]["data3"] != str(Node_Data_Obj[0]['node'+str(item)]['data3'])):
            NODES['node'+str(item)]["data3"] = str(Node_Data_Obj[0]['node'+str(item)]['data3'])
            client.publish(MQTT_TOPIC + "/node"+str(item)+"/msgid",str(Node_Data_Obj[0]['node'+str(item)]['msgid']), 1, True)
            client.publish(MQTT_TOPIC + "/node"+str(item)+"/data3",str(Node_Data_Obj[0]['node'+str(item)]['data3']), 1, True)

		
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

# Send Configuration Data
Send_Cfg_Data()


# Main Loop 
while True:
    start_time = int(time.time())   
    Send_Data()

# Inner loop to call mosquitto client in order to keep connection alive.
    while (int(time.time()) < start_time + REFRESH_DELAY):
        client.loop()
        
