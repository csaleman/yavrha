#!/usr/bin/env python3

import serial
import json
import time
import mosquitto

NumberofNodes = 10

ser = serial.Serial('/dev/ttyACM0',19200, timeout=.1)

command = bytes("get \r\n","utf-8")
#mosquitto setting
client = mosquitto.Mosquitto("test-client")
client.connect("127.0.0.1")

while True:
    ser.write(command)
    ser.readline()
    data = (ser.read(500))
    data_string = "[" + data.decode() +"]"
#   print(data_string)
    jObject = json.loads(data_string)
    i = 0
    while (i < NumberofNodes):
        if 'node'+str(i) in jObject[0]:
#           print(jObject[0]['node'+str(i)])
            print(jObject[0]['node'+str(i)])
            client.publish("my/node"+str(i),str(jObject[0]['node'+str(i)]), 1, retain=True)
        i = i+1
    time.sleep(5)
