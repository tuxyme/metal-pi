#!/bin/bash

stty -F /dev/ttyUSB0 115200
sx -vv kernel.img < /dev/ttyUSB0 > /dev/ttyUSB0
#minicom

