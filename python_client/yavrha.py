#!/usr/bin/env python3

# This code connect to a serial port and send data to a MQTT server

import serial
import json
import time
import mosquitto

REFRESH_DELAY = 5   # in seconds
ACTIVE_NODES = []
MQTT_TOPIC = "yavrha"	
ser = serial.Serial('/dev/ttyACM0',19200, timeout=.1)


#mosquitto setting
client = mosquitto.Mosquitto("yavrha-client")
client.connect("test.mosquitto.org")

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
#        print(Node_Cfg_Obj[0]['node'+str(i)])
        client.publish(MQTT_TOPIC + "/node"+str(i)+"/name",str(Node_Cfg_Obj[0]['node'+str(i)]['name']), 1, True)
        client.publish(MQTT_TOPIC + "/node"+str(i)+"/type",str(Node_Cfg_Obj[0]['node'+str(i)]['type']), 1, True)
        client.publish(MQTT_TOPIC + "/node"+str(i)+"/address",str(Node_Cfg_Obj[0]['node'+str(i)]['address']), 1, True)
        client.publish(MQTT_TOPIC + "/node"+str(i)+"/number",str(Node_Cfg_Obj[0]['node'+str(i)]['number']), 1, True)
    else:
        client.publish(MQTT_TOPIC + "/node"+str(i)+"/name",None , 1, True)
        client.publish(MQTT_TOPIC + "/node"+str(i)+"/type",None , 1, True)
        client.publish(MQTT_TOPIC + "/node"+str(i)+"/address",None, 1, True)
        client.publish(MQTT_TOPIC + "/node"+str(i)+"/number",None, 1, True)
    i += 1
#print(ACTIVE_NODES)        

def Send_Data():
    ser.flushInput()    # make sure input buffer is empty
    command = bytes("get \r\n","utf-8")
    ser.write(command)
    ser.readline()          # read with the intention of delete the command sent
    Node_Data = (ser.read(500))
    Node_Data_string = "[" + Node_Data.decode() +"]"
#    print(Node_Data_string)

    try:    
        Node_Data_Obj = json.loads(Node_Data_string)
    except ValueError:
        print("Value Error with json.loads(Node_Data_string)")
        return 0

    for item in ACTIVE_NODES:
#        print(Node_Data_Obj[0]['node'+str(item)])
        client.publish(MQTT_TOPIC + "/node"+str(item)+"/msg",str(Node_Data_Obj[0]['node'+str(item)]), 1)
        client.publish(MQTT_TOPIC + "/node"+str(item)+"/msgid",str(Node_Data_Obj[0]['node'+str(item)]['msgid']), 1)
        client.publish(MQTT_TOPIC + "/node"+str(item)+"/data0",str(Node_Data_Obj[0]['node'+str(item)]['data0']), 1)
        client.publish(MQTT_TOPIC + "/node"+str(item)+"/data1",str(Node_Data_Obj[0]['node'+str(item)]['data1']), 1)
        client.publish(MQTT_TOPIC + "/node"+str(item)+"/data2",str(Node_Data_Obj[0]['node'+str(item)]['data2']), 1)
        client.publish(MQTT_TOPIC + "/node"+str(item)+"/data3",str(Node_Data_Obj[0]['node'+str(item)]['data3']), 1)
		
def Received_Cmd(msg):
    Node = str(msg.topic).lstrip(MQTT_TOPIC+'/').rstrip("/cmd").strip("node")
    command = bytes("send "+Node+" "+msg.payload.decode()+" \r\n","utf-8")
    ser.write(command)
#    print(ser.readline().decode())

def on_connect(mosq, obj, rc):
    if rc == 0:
        print("Connected successfully.")

client.on_connect = on_connect

    
def on_message(mosq, obj, msg):
#    print("Message received on topic "+str(msg.topic)+" with QoS "+str(msg.qos)+" and payload "+msg.payload.decode())
    Received_Cmd(msg)
client.on_message = on_message



# Subcription
client.subscribe("yavrha/+/cmd", 0)

while True:
    start_time = int(time.time())
    Send_Data()
    while (int(time.time()) < start_time + REFRESH_DELAY):
        client.loop()
        
