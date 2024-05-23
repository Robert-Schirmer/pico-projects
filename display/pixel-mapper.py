#!/usr/bin/python3

import numpy as np
import cv2 as cv
import sys

if len(sys.argv) != 3:
    sys.exit("Usage: python3 pixel-mapper.py <image_path> <outoput_variable_name>")

img = cv.imread(sys.argv[1], cv.IMREAD_GRAYSCALE) # The image pixels have range [0, 255]
img //= 255  # Now the pixels have range [0, 1]
img_list = img.tolist() # We have a list of lists of pixels

width = len(img_list[0])
height = len(img_list)

variable_name = sys.argv[2]

result = f"""
#ifndef _IMAGE_{variable_name.upper()}_H
#define _IMAGE_{variable_name.upper()}_H

uint8_t {variable_name}_width = {width};
uint8_t {variable_name}_height = {height};

uint8_t {variable_name}[{height}][{width}] ="""

result += " {\n"

for row in img_list:
    row_str = [str(p) for p in row]
    result += "    {" + ", ".join(row_str) + "},\n"

result += """};

#endif
"""
f = open(f"./display/{variable_name}.h", "w")
f.write(result)
f.close()
