#!/bin/python3

# export QT_QPA_PLATFORM=xcb 

# TODO: time scale
# TODO: wartosci
# TODO: zmniejszenie jitter / implementacja smooth scrolling za pomocą shift (klatki przejściowe pomiędzy skokami dużych pikseli / bloczków)

# Required dependencies
# pip install pyserial opencv-python numpy
import serial
import numpy as np
import time
import cv2

thresholds = [600, 600, 600, 600]
delay = 25
servos = [5, -16, 16, -8]
timestart = 0


port = "/dev/ttyACM0"
baudRate = 115200
serialCom = serial.Serial(port, baudRate, timeout=10, write_timeout=10) # open a serial connection
n = 360_000 # rows of the history
rows = 40 #  rows of the preview
title = "Wizualizacja danych z robota" # title of the window
bilateral_filtering = False



# data = np.random.randint(1024,size=(n*rows,4),dtype=np.uint16)
iters = [0, 0, 0, 0] # pointers to the next row to fill for each column, could be simplified
data = np.full((n*rows,4),1024,dtype=np.uint16) # full history of measurements

def get_slice(i): 
    # Get a slice of data expressed as rbg triplets ready for rendering

    i = max(rows,i)
    part = data[i-rows:i, :] # get a slice of data ending at location i
    newpart = np.where(part == 1024, 122, np.where(part < thresholds, 0, 255)) # convert measurement values to single channel rgb
    return np.repeat(newpart[:,:, np.newaxis], 3, axis=2).astype(np.uint8) # convert single channel rgb to tripple and change type to match opencv
 

def animate(line):
    # Convert an incoming line from serial to a frame 

    # line[0] - time
    # line[1] - column
    # line[2] - measurement value

    if line is None: # line from serial has corrupted data, just re-render the same frame 
        return get_slice(max(iters))
    a = line[1] # which photoresistor (column)
    i = iters[a]
    data[i, a] = line[2] # fill the correct column at next available row with value from the measurement
    iters[a] += 1 # increase column pointer
    return get_slice(i) # render a frame with updated data with i+1 row downmost
   
def read_line_serial():
    # read a line from serial, none if corrupted (lazy check)
    line = get_photoresistor_measurement(read_bytes_string())
    if line[1] < 0 or line[1] > 3: # column bounds check and exclude c
        return None
    elif line[2] < 0 or line[2] > 1023: # check if measurements make sense
        return None
    # note no time check
    print(string_photores(line))
    return line


def get_photoresistor_measurement(input):
    # Parse string to get 3 ints or return [-1, -1, 1024]
    if input != "":
        try:
            values = [int(x) for x in input.split()]
            if len(values) == 3:
                return values
        except ValueError:
            pass
    return [-1, -1, 1024]

def get_thresholds(input="", thresholds=thresholds):
    # Parse string to get 4 ints or return default value
    if input != "":
        try:
            values = [int(x) for x in input.split()]
            if len(values) == 4:
                return values
        except ValueError:
            pass
    return thresholds

def get_time(input="", time1=delay):
    # Parse string to get an int or return default value
    if input != "":
        try:
            return int(input)
        except ValueError:
            pass
    return time1

def get_servos(input="", servos=servos):
    # Parse string to get 4 ints or return default value
    if input != "":
        try:
            values = [int(x) for x in input.split()]
            if len(values) == 4:
                return values
        except ValueError:
            pass
    return servos


def read_bytes_string():
    # Read incoming bytes from serial as string
    try:
        s_bytes = serialCom.readline()
        return s_bytes.decode("ascii").strip()
    except serial.SerialException:
        return ""


def live_view(bilateral_filtering):
    while True:
            line = read_line_serial()
            # line = [0,random.randint(0,3),random.randint(0,1023)]
            image = cv2.resize(animate(line), dsize=(1000, 1200), interpolation=cv2.INTER_NEAREST)
            if bilateral_filtering:
                # a convolution would be much better honestly
                image = cv2.GaussianBlur(image, (15,15),0)
                # image = cv2.bilateralFilter(image,5,75,75) # apply a blur in one direction basically
            cv2.imshow(title, cv2.flip(image,0))
            k = cv2.waitKey(1)
            if k == 27: # esc
                scrollable_view()
            elif k == ord('b'):
                print("bilateral_filtering: On")
                bilateral_filtering = True 
            elif k == ord('n'):
                print("bilateral_filtering: Off")
                bilateral_filtering = False 



def scrollable_view():
    serialCom.close()
    current_row = max(iters)
    max_row = current_row+1
    print("Esc to exit")
    print("w - scroll up, s - scroll down")
    while True:
        image = cv2.resize(get_slice(current_row), dsize=(1000, 1200), interpolation=cv2.INTER_NEAREST)
        cv2.imshow(title, cv2.flip(image,0))
        k = cv2.waitKey(1)
        if k == 27: # esc
            try:
                pass
            finally:
                cv2.destroyAllWindows()
                serialCom.close()
            exit()
        elif k==ord('w'): # up
            current_row = max(current_row-1,rows)
        elif k==ord('s'): # down
            current_row = min(current_row+1, max_row)


def string_list(input_list):
    s = ""
    for i in input_list:
        s += f"{i:>6}|"
    return s

def string_photores(input_list):
    s = f"|{(input_list[0]):>6}|"
    for i in range(input_list[1]):
        s+= "      |"
    if (input_list[2] < thresholds[input_list[1]]):
        s += f"{input_list[2]:>5}*|"
    else:
        s += f"{input_list[2]:>6}|"
    for i in range(3-input_list[1]):
        s+= "      |"
    return s


def main(thresholds=thresholds, delay=delay, servos=servos):

    print("Esc to stop & scroll")
    # Send signal to restart arduino over serial
    # serialCom.setDTR(False)
    time.sleep(2)
    serialCom.flushInput()
    serialCom.setDTR(True)
    print("Opened port ", port)

    #      Thresholds    Delay     Servos
    # . 200,200,200,200 ; 100 ; 25,25,25,25\n
    to_arduino = (
        ". "
        + ",".join(map(str, thresholds))
        + " ; "
        + str(delay)
        + " ; "
        + ",".join(map(str, servos))
        + "\n"
    )
    # Send a command to overwrite program parameters (thresholds, delay, servos) using the values defined on the top of the current file
    serialCom.write(to_arduino.encode("ascii"))
    serialCom.flushInput()

    # Read updated values (or not if something went wrong) from serial
    thresholds = get_thresholds(read_bytes_string(), thresholds)
    delay = get_time(read_bytes_string(), delay)
    servos = get_servos(read_bytes_string(), servos)
    released_servos = get_servos(read_bytes_string(), [0,0,0,0])
    timestart = get_time(read_bytes_string(), 0)

    print("Thresholds: {"+(",".join(map(str, thresholds)))+"}")
    print("Start delay in miliseconds: ",delay)
    print("Pressed degrees delta: {"+(",".join(map(str, servos)))+"}")
    print("| Type |  S0  |  S1  |  S2  |  S3  |")
    print("+------+------+------+------+------+")
    print("Default|"+string_list(released_servos))
    print("Pressed|"+string_list(servos))
    print("Starting time: ",timestart)

    print("| Time |  P0  |  P1  |  P2  |  P3  |")
    print("+------+------+------+------+------+")

if __name__ == "__main__":
    try:
        main()
        cv2.namedWindow(title, cv2.WINDOW_GUI_NORMAL | cv2.WINDOW_AUTOSIZE)
        live_view(bilateral_filtering)

    except KeyboardInterrupt:
        scrollable_view()

    finally: # ALWAYS executes at the end of the program, no matter what
        cv2.destroyAllWindows()
        serialCom.close()
        
