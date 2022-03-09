# project: sensor_ms8607THV
# description: Xbee3 Zigbee Micropython MS8607 sensor project that sleeps x seconds, then wakes to send temperature,
#              humidity, and battery voltage readings to coordinator
# Default template for Digi Xbee3 projects

import utime
import ustruct
import machine
import time
import xbee
from machine import Pin


x = xbee.XBee()

# list of commands in hex for MS8607 pressure sensor
c_reset = 0x1E  # reset command
r_c1 = 0xA2  # read PROM C1 command
r_c2 = 0xA4  # read PROM C2 command
r_c3 = 0xA6  # read PROM C3 command
r_c4 = 0xA8  # read PROM C4 command
r_c5 = 0xAA  # read PROM C5 command
r_c6 = 0xAC  # read PROM C6 command
r_adc = 0x00  # read ADC command
r_d1 = 0x44  # convert D1 (OSR=1024)
r_d2 = 0x54  # convert D2 (OSR=1024)
p_address = 0x76  # pressure sensor i2c address

# list of commands in hex for MS8607 humidity sensor
h_address = 0x40  # humidty sensor i2c address
r_user = 0xE7  # read user register command
w_user = 0xE6  # write user register command
t_temp = 0xE3  # trigger temperature measurement, hold master
t_humi = 0xE5  # trigger humidity measurement, hold master
# set register format
REGISTER_FORMAT = '>h'  # ">" big endian, "h" 2 bytes
REGISTER_SHIFT = 4  # rightshift 4 for 12 bit resolution

# set i2c clock to 100KHz
i2c = machine.I2C(1, freq=100000)

#Zigbee coordinator adddress in house
TARGET_64BIT_ADDR = b'\x00\x13\xA2\x00\x41\xAA\xCB\xBA'



# slave_addr = 0x76

# reset pressure sensor
def reset_ps():
    slave_addr = p_address
    data = bytearray([c_reset])
    i2c.writeto(slave_addr, data)
    return data


# scan i2c bus for active addresses
def scan_I2C():
    devices = i2c.scan()
    return devices


def read_c1():  # read PROM value C1
    data = bytearray([r_c1])
    i2c.writeto(slave_addr, data)
    raw_c = i2c.readfrom(slave_addr, 2)  # raw C is 2 bytes
    value = int.from_bytes(raw_c, "big")  # use builtin to convert to integer
    return value


def read_c2():  # read PROM value C2
    data = bytearray([r_c2])
    i2c.writeto(slave_addr, data)
    raw_c = i2c.readfrom(slave_addr, 2)  # raw C is 2 bytes
    value = int.from_bytes(raw_c, "big")  # use builtin to convert to unsigned integer
    return value


def read_c3():  # read PROM value C3
    data = bytearray([r_c3])
    i2c.writeto(slave_addr, data)
    raw_c = i2c.readfrom(slave_addr, 2)  # raw C is 2 bytes
    value = int.from_bytes(raw_c, "big")  # use builtin to convert to unsigned integer
    return value


def read_c4():  # read PROM value C4
    data = bytearray([r_c4])
    i2c.writeto(slave_addr, data)
    raw_c = i2c.readfrom(slave_addr, 2)  # raw C is 2 bytes
    value = int.from_bytes(raw_c, "big")  # use builtin to convert to unsigned integer
    return value


def read_c5():  # read PROM value C5
    data = bytearray([r_c5])
    i2c.writeto(slave_addr, data)
    raw_c = i2c.readfrom(slave_addr, 2)  # raw C is 2 bytes
    value = int.from_bytes(raw_c, "big")  # use builtin to convert to unsigned integer
    return value


def read_c6():  # read PROM value C6
    data = bytearray([r_c6])
    i2c.writeto(slave_addr, data)
    raw_c = i2c.readfrom(slave_addr, 2)  # raw C is 2 bytes
    value = int.from_bytes(raw_c, "big")  # use builtin to convert to unsigned integer
    return value


# start D1 conversion - pressure (24 bit unsigned)
def start_d1():
    # print ('start D1 ')
    data = bytearray([r_d1])
    i2c.writeto(slave_addr, data)


# start D2 conversion - temperature (24 bit unsigned)
def start_d2():
    # print ('start D2 ')
    data = bytearray([r_d2])
    i2c.writeto(slave_addr, data)


# read pressure sensor ADC
def read_adc():  # read ADC 24 bits unsigned
    data = bytearray([r_adc])
    i2c.writeto(slave_addr, data)
    adc = i2c.readfrom(slave_addr, 3)  # ADC is 3 bytes
    value = int.from_bytes(adc, "big")  # use builtin to convert to integer
    return value


# read humidity sensor user register command 0xE7, default value = 0x02
# default resolution RH 12bit, T 14bit
def read_user():
    data = bytearray([r_user])
    i2c.writeto(slave_addr, data)
    value = i2c.readfrom(slave_addr, 1)
    return value


# read rh: send trigger rh command 0xE5
def read_rh():
    data = bytearray([t_humi])
    i2c.writeto(slave_addr, data)
    raw_rh = i2c.readfrom(slave_addr, 2)  # raw RH is 2 bytes
    raw_value = int.from_bytes(raw_rh, "big")  # use builtin to convert to integer
    # rh_tc = (-0.15)*(25 - read_temp())# RH temp compensation
    rh_value = (((raw_value / 65536) * 125) - 6)  # calculate RH
    return rh_value


# **************** Main Program *********************************


# set up variables d2, d3, d4 as pin outputs and set to 0
# use D4 to /EN TPS2042D power distribution switch, power MS8607 and voltage divider for ADC pin D0
d2 = Pin.board.D2
d2.mode(Pin.OUT)
d2.value(0)
d3 = Pin.board.D3
d3.mode(Pin.OUT)
d3.value(0)
d4 = Pin.board.D4
d4.mode(Pin.OUT)
d4.value(0)  # set low to enable TPS2042P to enable i2c during startup

# set DIO9 and DIO10 to outputs with value 0
d9 = Pin.board.D9
d9.mode(Pin.OUT)
d9.value(0)
d10 = Pin.board.D10
d10.mode(Pin.OUT)
d10.value(0)

# set up ADC with 2.5V reference for pin D0 as analog input
#x = xbee.Xbee()
xbee.atcmd('AV', 1)
apin = machine.ADC('D0')

# delay for 30 seconds (long delay to connect to device if needed after reset)
#print("start 90 seconds delay")
utime.sleep(90)
#print("end 90 seconds delay")



try:
    print('i2c scan addresses found: ', scan_I2C())
except:
    print('i2c scan addresses not found')
try:
    print('perform reset on pressure sensor, code = ', reset_ps())
except:
    print('cannot reset pressure sensor')


# read and print humidity sensor user register
# slave_addr = h_address  # set humidity sensor i2c address
# print('user register: ', read_user())

# read press sensor calibration PROM
slave_addr = p_address
try:
    C1 = read_c1()
    C2 = read_c2()
    C3 = read_c3()
    C4 = read_c4()
    C5 = read_c5()
    C6 = read_c6()
except:
    print('cannot read pressure sensor calibration')

#print('PROM C1 = ', C1)
#print('PROM C2 = ', C2)
#print('PROM C3 = ', C3)
#print('PROM C4 = ', C4)
#print('PROM C5 = ', C5)
#print('PROM C6 = ', C6)

# check zigbee connection
while xbee.atcmd("AI") != 0:
    print("#Trying to Connect...")
    utime.sleep(1)

print("#Online...")

# Main loop
while True:
    # set pin d4 low to enable TPS2042
#    d2.value(1)
    d4.value(0)
    # delay for 2 seconds to stabilize
    utime.sleep(2)

    # take ADC reading on pin D0 (voltage divider across supply/battery)
    raw_val = apin.read()
    val_mv = int((raw_val * 2500)/4095 * 2)
    print('supply voltage %d mV' % val_mv)

    # start on D1 conversion for pressure sensor
    try:
        slave_addr = p_address  # set i2c address to pressure sensor
        start_d1()  # start D1 conversion
        utime.sleep(1)  # short delay during conversion
        raw_d1 = read_adc()
        #print("D1= ", raw_d1)
    except:
        print("D1 conversion failed")

    # start D2 conversion for temperature
    try:
        start_d2()  # start D2 conversion
        utime.sleep(1)
        raw_d2 = read_adc()
        #print("D2= ", raw_d2)
    except:
        print("D2 conversion failed")

    # calulate pressure and temperature
    try:
        # difference between actual and ref P temp
        dT = raw_d2 - (C5 * 256)
        #print("dT= ", dT)
        #
        Temp = 2000 + (dT * (C6 / 8388608))
        if Temp < 2000:  # add 2nd order correction when temp < 20C
            T2 = 3 * dT ** 2 / 8589934592
            OFF2 = (61 * (Temp - 2000) ** 2) / 16
            SENS2 = (29 * (Temp - 2000) ** 2) / 16
        else:
            T2 = 5 * dT ** 2 / 274877906944
            OFF2 = 0
            SENS2 = 0
        if Temp < -1500:  # add 2nd order correction when temp < -15C
            OFF2 = OFF2 + (17 * (Temp + 1500) ** 2)
            SENS2 = SENS2 + (9 * (Temp + 1500) ** 2)
        # calculate corrected temp
        Temp = (2000 - T2 + (dT * (C6 / 8388608))) / 100
        fTemp = int(Temp * 9 / 5 + 32)
        OFF = (C2 * 131072) + (C4 * dT / 64) - OFF2  # offset at actual P temperature
        #print("OFF= ", OFF)
        SENS = (C1 * 65536) + (C3 * dT / 128) - SENS2  # pressure offset at actual temperature
        #print("SENS= ", SENS)
        Pres = (raw_d1 * SENS / 2097152 - OFF) / 3276800  # barometric pressure
        #print('P Temp = ', '%.1fC' % Temp)
        #print('P Temp = ', '%.1fF' % fTemp)
        #print('T2 = ', T2)
        #print('OFF2 = ', OFF2)
        #print('SENS2 = ', SENS2)
        #print('Pressure = ', '%.1f ' % Pres)
        utime.sleep(1)
    except:
        print("Temp and Pressure calculation failed")

    # start on humidity sensor
    try:
        slave_addr = h_address
        RH = int(read_rh() - 3.6 - (0.18 * Temp))
        print('relative humidity: ', '%.1f percent' % RH)  # temp compensated humidity
    except:
        print("humidity sensor conversion failed")

    # turn off TPS2042
    d4.value(1)
    # build PTH sensor data payload for battery voltage, temp and humidity
    try:
        time_snapshot = str(utime.ticks_cpu())
        # print_PTH = "PTH_sensor:" + time_snapshot + ":Press:" + str(Pres) + "mB:Temp:" + str(fTemp) + "F:Temp1:" + str(ftemp9) + "F:Humidity:" + str(RH) + "%:#"
        # print_PTH = "PTH_sensor:" + time_snapshot + ":Press:" + str(Pres) + "mB:Temp:" + str(fTemp) + "F:Humidity:" + str(RH) + "%:#" #remove MCP9808
        print_PTH = "PTH_sensor:" + time_snapshot + ":V_Ba:" + str(val_mv) + ":T_F:" + str(fTemp) + ":H_%:" + str(
            RH) + ":#"
        print(print_PTH)
    except:
        print("sensor payload build failed")
        Pres = 99
        fTemp = 99
        RH = 99

    # transmit PTH data over Zigbee to coordinator
    d3.value(1)
    try:
        xbee.transmit(TARGET_64BIT_ADDR, print_PTH)
        # xbee.transmit(ROUTER_64BIT_x1B2D, print_PTH)
        #
    except:
        print("xbee coordinator transmit failed")


    # set pin d4 high to turn off TPS2042P
    #d4.value(1)


#    print("set d4 high and delay for 1 seconds")
#    utime.sleep(1)
#    d2.value(0)
#    d3.value(0)
    print("sleeping for 60 seconds")
    #sleep_ms = x.sleep_now(10000, True)
    sleep_ms = x.sleep_now(60000, True)
    print("woke up ")
