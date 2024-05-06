#!/bin/python3

# TODO: time scale
# TODO: wartosci
# TODO: ??? problem z gubieniem klatek
# TODO: niespójne czasy 
# np. [21172, 2, 51]
    # [21172, 3, 50]
    # [3, 2, 53]
    # [33244, 3, 52]
    # [33253, 0, 55]
    # [33253, 1, 54]

import serial
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
import time
import math

thresholds = [200, 600, 200, 600]
delay = 10
servos = [19, -25, 17, -15]
timestart = 0


port = "/dev/ttyACM0"
baudRate = 115200
serialCom = serial.Serial(port, baudRate, timeout=1, write_timeout=10)


n = 360_000
iters = [0, 0, 0, 0]

rows = 20
fig, ax = plt.subplots()
ax.tick_params(
    axis='both',         
    which='both',      
    bottom=False,      
    top=False,         
    left = False,
    right = False,
    labelbottom=False,
    labeltop=False,
    labelleft=False,
    labelright = False)
# fig = plt.figure()
# ax.grid(axis='x',which="major")
# ax.grid(axis='x')
fig.tight_layout()


# data = np.random.choice([0,122,255],size=(n*rows,4))
# data = np.random.randint(1024,size=(n*rows,4),dtype=np.uint16)
data = np.full((n*rows,4),1024,dtype=np.uint16)
data_slice = np.full((rows,4),0,dtype=np.uint16)

def get_slice(i):
    i = max(rows,i)
    part = data[i-rows:i, :]
    newpart = np.where(part == 1024, 122, np.where(part < thresholds, 0, 255))
    return np.repeat(newpart[:,:, np.newaxis], 3, axis=2)
    
im = plt.imshow(get_slice(0), aspect="auto", interpolation="nearest")

def init():
    im.set_data(np.full((rows,4,3),122))

def animate(line):
    if line == None:
        return
    a = line[1]
    data[iters[a], a] = line[2]
    iters[a] += 1
    i = max(iters)
    im.set_data(get_slice(i))
   
def read_line_serial():
    line = get_photoresistor_measurement(read_bytes_string())
    # print(line)
    if line[1] == -1:
        yield None
    yield line


def get_photoresistor_measurement(input):
    if input != "":
        try:
            values = [int(x) for x in input.split()]
            if len(values) == 3:
                return values
        except ValueError:
            pass
    return [-1, -1, 1024]

def get_thresholds(input="", thresholds=thresholds):
    if input != "":
        try:
            values = [int(x) for x in input.split()]
            if len(values) == 4:
                return values
        except ValueError:
            pass
    return thresholds

def get_time(input="", time1=delay):
    if input != "":
        try:
            return int(input)
        except ValueError:
            pass
    return time1

def get_servos(input="", servos=servos):
    if input != "":
        try:
            values = [int(x) for x in input.split()]
            if len(values) == 4:
                return values
        except ValueError:
            pass
    return servos


def read_bytes_string():
    # try:
    s_bytes = serialCom.readline()
    return s_bytes.decode("ascii").strip()
    # except serial.SerialException:
        # return ""


def main(thresholds=thresholds, delay=delay, servos=servos):

    # print("Opened port ", port)
    time.sleep(2)
    serialCom.flushInput()
    serialCom.setDTR(True)

    to_arduino = (
        ". "
        + ",".join(map(str, thresholds))
        + " ; "
        + str(delay)
        + " ; "
        + ",".join(map(str, servos))
        + "\n"
    )
    # print(to_arduino, end="")
    serialCom.write(to_arduino.encode("ascii"))
    serialCom.flushInput()

    thresholds = get_thresholds(read_bytes_string())
    delay = get_time(read_bytes_string())
    servos = get_servos(read_bytes_string())
    timestart = get_time(read_bytes_string(), 0)


if __name__ == "__main__":
    try:
        main()
        # anim = FuncAnimation(fig,animate,init_func=init,interval=math.ceil(delay/4),cache_frame_data=False)
        anim = FuncAnimation(fig,animate,init_func=init,frames=read_line_serial, interval=0, cache_frame_data=False)
        plt.show()
    finally:
        serialCom.close()
        
