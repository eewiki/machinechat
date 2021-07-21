# file name mqtt_jedi_display.py
# application: use pyportal titano to subscribe/publish to JEDI One mqtt broker and display data on pyportal titano display
# SR initial6/24/2021
# modified 6/25 to add additional mqtt fields to subscribe serial message print
# update 7/1 add in code for display, add json send data from pyportal
# update 7/21 add in code to add second MQTT subscribe for outdoor sensor and display
# below code updated from original Adafruit minimqtt code example
# SPDX-FileCopyrightText: 2021 ladyada for Adafruit Industries
# SPDX-License-Identifier: MIT

import time
import board
import busio
from digitalio import DigitalInOut
import neopixel
from adafruit_esp32spi import adafruit_esp32spi
from adafruit_esp32spi import adafruit_esp32spi_wifimanager
import adafruit_esp32spi.adafruit_esp32spi_socket as socket
import adafruit_minimqtt.adafruit_minimqtt as MQTT
import json
import sys
from adafruit_pyportal import PyPortal
import displayio
import terminalio
from adafruit_bitmap_font import bitmap_font
from adafruit_display_text import label

unique_id = "ppt10:52:1C:88:B6:F8" # set unique id based on pyportal MAC
latest_msg = "test"                # set initial message

display = board.DISPLAY
main_group = displayio.Group(max_size=10)
MEDIUM_FONT = bitmap_font.load_font("fonts/Arial-16.bdf")
BIG_FONT = bitmap_font.load_font("fonts/Arial-Bold-24.bdf")

### WiFi ###

# Get wifi details and more from a secrets.py file
try:
    from secrets import secrets
except ImportError:
    print("WiFi secrets are kept in secrets.py, please add them there!")
    raise

# If you are using a board with pre-defined ESP32 Pins:
esp32_cs = DigitalInOut(board.ESP_CS)
esp32_ready = DigitalInOut(board.ESP_BUSY)
esp32_reset = DigitalInOut(board.ESP_RESET)

# If you have an externally connected ESP32:
# esp32_cs = DigitalInOut(board.D9)
# esp32_ready = DigitalInOut(board.D10)
# esp32_reset = DigitalInOut(board.D5)

spi = busio.SPI(board.SCK, board.MOSI, board.MISO)
esp = adafruit_esp32spi.ESP_SPIcontrol(spi, esp32_cs, esp32_ready, esp32_reset)
"""Use below for Most Boards"""
status_light = neopixel.NeoPixel(
    board.NEOPIXEL, 1, brightness=0.2
)  # Uncomment for Most Boards

wifi = adafruit_esp32spi_wifimanager.ESPSPI_WiFiManager(esp, secrets, status_light)

### Feeds ###

# Setup a feed named 'photocell' for publishing to a feed
photocell_feed = "datacache/photocell"  # modify feed for JEDI One MQTT broker


# Setup a feed named 'pump' for subscribing to changes
pump_feed = "datacache/T960981B2D"     # map feed to JEDI One MQTT broker pumphouse data stream

# Setup a feed named 'outdoor' for subscribing to changes
outdoor_feed = "datacache/MS58981B2D"     # map feed to JEDI One MQTT broker outdoor data stream


### Code ###

# Define callback methods which are called when events occur
# pylint: disable=unused-argument, redefined-outer-name
def connected(client, userdata, flags, rc):
    # This function will be called when the client is connected
    # successfully to the broker.
    print("Connected to JEDI One MQTT broker ! Listening for topic changes on pump_feed and outdoor_feed")
    # Subscribe to all changes on the pump_feed.
    client.subscribe(pump_feed)
    client.subscribe(outdoor_feed)


def disconnected(client, userdata, rc):
    # This method is called when the client is disconnected
    print("Disconnected from JEDI One!")


def message(client, topic, message):
    # This method is called when a topic the client is subscribed to
    # has a new message.
    global latest_msg
    print("New message on topic {0}: {1}".format(topic, message))
    latest_msg = message
    parsed = json.loads(latest_msg)
    data1 = "Humidity " + str(parsed["data1"])
    print(data1)
    print(parsed["data2"])
    print(parsed["data3"])
    print(parsed["timestamp"])
    print(latest_msg)


# Connect to WiFi
print("Connecting to WiFi...")
wifi.connect()
print("Connected!")


# Initialize MQTT interface with the esp interface
MQTT.set_socket(socket, esp)


# Set up a MiniMQTT Client with JEDI One MQTT broker
#  set client_id to unique id for connecting to mqtt broker
mqtt_client = MQTT.MQTT(
    broker="192.168.1.7",
    port = 1883,
    client_id = unique_id,
)

# Setup the callback methods above
mqtt_client.on_connect = connected
mqtt_client.on_disconnect = disconnected
mqtt_client.on_message = message

# Connect the client to the MQTT broker.
print("Connecting to JEDI One MQTT broker...")
mqtt_client.connect()

# print header message on display
text_area1 = label.Label(BIG_FONT, text="Pump House Monitor", max_glyphs=40)
text_area1.x = 10
text_area1.y = 10
main_group.append(text_area1)
display.show(main_group)


text_area = label.Label(MEDIUM_FONT, text="123456", max_glyphs=40)
text_area.x = 10
text_area.y = 60
main_group.append(text_area)
display.show(main_group)

text_area2 = label.Label(MEDIUM_FONT, text="abcde", max_glyphs=40)
text_area2.x = 10
text_area2.y = 100
main_group.append(text_area2)
display.show(main_group)

text_area3 = label.Label(MEDIUM_FONT, text="loony", max_glyphs=40)
text_area3.x = 10
text_area3.y = 140
main_group.append(text_area3)
display.show(main_group)

text_area4 = label.Label(MEDIUM_FONT, text="sandhill", max_glyphs=40)
text_area4.x = 10
text_area4.y = 180
main_group.append(text_area4)
display.show(main_group)

text_area5 = label.Label(MEDIUM_FONT, text="timestamp", max_glyphs=50)
text_area5.x = 10
text_area5.y = 220
main_group.append(text_area5)
display.show(main_group)


# photocell is a simulated sensor used by the pyportal to test publishing to JEDI One MQTT broker
photocell_val = 0  # set initial photocell value

data1 = "data1" # set initial values
data2 = "data2"
data3 = "data3"
data4 = "data4"
data5 = "data5"

# map photocell data dictionary object in prep to be compatible with JEDI One mqtt broker
photocell_dict = {}
photocell_dict["deviceType"] = "photocell"
photocell_dict["value"] = photocell_val
print(photocell_dict)

while True:
    # Poll the message queue
    mqtt_client.loop()
    if latest_msg != "test":  # check to see if new mqtt subscribe message
        print("received mqtt subscribe message from:")
        parsed = json.loads(latest_msg)
        nodeID = "Node ID " + str(parsed["nodeID"])
        print(nodeID)
        if str(parsed["nodeID"]) == "T960981B2D":
            print("in T960981B2D node loop")
            data1 = "PH Humidity " + str(parsed["data1"]) + " %"
            data2 = "PH Temperature " + str(parsed["data2"]) + " F"
        print("past 1st MQTT node check")
        if str(parsed["nodeID"]) == "MS58981B2D":
            print("in MS58981B2D node loop")
            data3 = "Outdoor Temperature " + str(parsed["data2"]) + " F"
            data4 = "Barometric Pressure " + str(parsed["data1"])
        print("past 2nd MQTT node check")
        data5 = "TS: " + str(parsed["timestamp"]) # most recent timestamp
        text_area.text = data1   # update pump house humidity value for display
        text_area2.text = data2  # update pump house temperature value for display
        text_area3.text = data3  # update outdoor temperature value for display
        text_area4.text = data4  # update barometric pressure value for display
        text_area5.text = data5  # update latest timestamp for display
        latest_msg = "test"  # reset latest message

    photocell_dict["value"] = photocell_val       # update to latest photocell value
    photocell_json = json.dumps(photocell_dict)   # convert photocell data to json format for JEDI One mqtt broker
    print(photocell_json)
    # Send a new message
    print("Sending photocell json string: " + photocell_json)
    mqtt_client.publish(photocell_feed, photocell_json)
    print("Sent!")
    photocell_val += 1
    if photocell_val > 100: # reset counter for simulated sensor data
        photocell_val = 0
    print(latest_msg)
    print(data1)
    time.sleep(11)