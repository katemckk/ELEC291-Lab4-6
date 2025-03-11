import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import sys, time, math
import serial
import csv
import os
import pygame

# Serial Port Configuration 
ser = serial.Serial(
    port='COM3',  # Change this to your actual COM port
    baudrate=115200,
    parity=serial.PARITY_NONE,
    stopbits=serial.STOPBITS_ONE, 
    bytesize=serial.EIGHTBITS
)


