import sys
from PIL import Image

im = Image.open(sys.argv[1])
pix = im.load()
print("Image Size = " + ' x '.join([str(int) for int in im.size]))  # Get the width and hight of the image for iterating over

chars = ''
char = 0
count = 8

for x in range(0,im.size[0]):
    for y in range(0,im.size[0]):
        # pix[x,y] = the RGBA Value of the a pixel of an image
        # either 1 or 0 as this a one-bit image to be expected
        if pix[x,y] == 0:
            char = (1 << count-1) | char
        count -= 1
        if count == 0:
            chars += '\\x' + hex(char)[2:]
            count = 8
            char = 0

print("C++ char array:")
print('"' + chars + '\\0"') 