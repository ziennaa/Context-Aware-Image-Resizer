stb_image.h
stb_image_write.h

stb_image.h
reads image files and converts them into plain array
of numbers that we can work with in cpp

stb_image_write.h
takes my array of numbers and saves it back as an image file

Why you need them:
A JPG or PNG file on disk is not just raw pixel values. It's compressed, encoded data. You can't just open a JPG and read pixels directly. These libraries handle all that decoding/encoding for you.
Without them you'd have to write your own image decoder from scratch which is a completely separate and massive problem.

stb_image     →    YOUR CODE                           →    stb_image_write
loads image        energy function                          saves output image
                   DP seam finding
                   seam removal
                   loop


Pure red pixel   → [255, 0,   0  ]
Pure blue pixel  → [0,   0,   255]
White pixel      → [255, 255, 255]
Black pixel      → [0,   0,   0  ]
Yellow pixel     → [255, 255, 0  ]


A flat 1D array of all these values, left to right, top to bottom:

Image (2x2 pixels):
[RED_pixel] [BLUE_pixel]
[WHITE_pixel] [BLACK_pixel]

stb gives you:
[255, 0, 0,    0, 0, 255,    255, 255, 255,    0, 0, 0]
 ^red pixel    ^blue pixel   ^white pixel       ^black pixel