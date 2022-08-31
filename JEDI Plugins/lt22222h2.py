#!/usr/bin/python3
# Change the path above to where python3 is on system - use "which python3" to find out
#
# NOTE: this code is heavily modified by SBR but based on Machinechat example for below
#
# kuando-jedi-action.py - A custom action plugin for JEDI Pro to enqueue downlinks
# for the Plenum Kuando Busylight (LoRaWAN) via ChirpStack using their API:  
# https://www.chirpstack.io/application-server/api/python-examples/
#
# This can also be run from the command line to test
#
# v0.1 DRM 04/10/2022  (original Machinechat code)
# 
# Depends on Python package: chirpstack_api
# Install:  sudo python3 -m pip install chirpstack_api
#
# The original code by DRM has been heavily modified to work with Dragino LT-22222-L LoRa I/O controller
# similar to the original code to enqueue downlinks to set parameters and output settings
# per Dragino user manual at below link
# http://wiki.dragino.com/xwiki/bin/view/Main/User%20Manual%20for%20LoRaWAN%20End%20Nodes/LT-22222-L/
#
# Example: "./lt22222h.py 0x03 0x01 0x01"
# LT-22222 will turn relays 1 and 2 on
# Example: "./lt22222h.py 0x03 0x00 0x00"
# LT-22222 will turn relays 1 and 2 off"




import os
import sys

import grpc
from chirpstack_api.as_pb.external import api

#
# Kuando has the colors in a strange order: red blue green versus normal RGB
# SR modified for 3 parameters and Dragino LT22222 io controller

# change to 4 for lt22222
args = len(sys.argv)
print(args)
# create list of 10 integers
arg = list(range(10))

# check count of command line arguments to check if valid
if (args == 4 or args == 5 or args == 6 or args == 8):
        print("args is 4, 5, 6 or 8")
else:
        print("number of args not valid")
        print("program handles 4,5, 6 or 8 command line variables as shown below")
        print("usage: %s arg1 arg2 arg3" % sys.argv[0])
        print("example: %s 0x03 0x01 0x00" % sys.argv[0])
        sys.exit(1)

# check if format of command line arguments are valid and store as integer
for x in range(args - 1):
        if (sys.argv[x + 1].startswith("0x")):                
            print(sys.argv[x +1])
            arg[x+1] = int(sys.argv[x+1],16)
            print(arg[x+1])
        else:
                print("invald arg")
                sys.exit(1)

# debug
for x in range(args - 1):
        print(arg[x+1])



#DEBUG
#print (arg1)
#print (arg2)
#print (arg3)
#print (ont)
#print (offt)

# Configuration.
#
########## CHANGE THIS TO YOUR ENVIRONMENT
#
# This must point to the ChirpStack API interface.
#server = "192.168.1.55:8080"
server = "192.168.1.23:8080"
#
########## CHANGE THIS TO YOUR ENVIRONMENT
#
# The Kuando BusyLight DevEUI for which you want to enqueue the downlink.
#dev_eui = bytes([0x20, 0x20, 0x20, 0x41, 0x28, 0x13, 0x05, 0x02])
# for LT22222  eui a8 40 41 46 d1 84 84 96
dev_eui = bytes([0xa8, 0x40, 0x41, 0x46, 0xd1, 0x84, 0x84, 0x96])
#
########## CHANGE THIS TO YOUR ENVIRONMENT
#
# The API token (generated using the ChirpStack web-interface).
# make sure to copy the key when it is displayed - it is hidden after leaving page
# and it is impossible to retrieve the key again.  Have to create new one if that happens
#
api_token = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJhcGlfa2V5X2lkIjoiZTQ4NTQwNTMtZjU5OS00YjI3LTk4OGEtOTE3NDZkNjcwOTFkIiwiYXVkIjoiYXMiLCJpc3MiOiJhcyIsIm5iZiI6MTY2MTI2MTA0Niwic3ViIjoiYXBpX2tleSJ9.7i4bzCOaSbODNT22voFUgewkyvwiEnRTPYaglYO7jxo"

if __name__ == "__main__":
  # Connect without using TLS.
  channel = grpc.insecure_channel(server)

  # Device-queue API client.
  client = api.DeviceQueueServiceStub(channel)

  # Define the API key meta-data.
  auth_token = [("authorization", "Bearer %s" % api_token)]

  # Construct request.
  req = api.EnqueueDeviceQueueItemRequest()
  # Confirmed downlink
  req.device_queue_item.confirmed = True
  if (args == 4):
        req.device_queue_item.data = bytes([arg[1], arg[2], arg[3]])
  elif (args == 5):
        req.device_queue_item.data = bytes([arg[1], arg[2], arg[3], arg[4]])
  elif (args == 6):
        req.device_queue_item.data = bytes([arg[1], arg[2], arg[3], arg[4], arg[5]])
  elif (args == 8):
        req.device_queue_item.data = bytes([arg[1], arg[2], arg[3], arg[4], arg[5], arg[6], arg[7]])
  req.device_queue_item.dev_eui = dev_eui.hex()
  
  # 15 for Kuando Busylight
  # 2 for LT22222 io controller
  req.device_queue_item.f_port = 2

  resp = client.Enqueue(req, metadata=auth_token)
  
  # DEBUG
  # Print the downlink frame-counter value.
  #print(resp.f_cnt)
