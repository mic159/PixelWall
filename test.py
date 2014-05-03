#!/usr/bin/env python

import argparse
import socket
import Image

DISPLAY_WIDTH = 16
DISPLAY_HEIGHT = 8
SYNC = chr(0xA0)
#DAT = 'FF000000FF000000FFFFFFFF000000FFFF0000FFFFFF00FF'.decode('hex')

def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('image'
    , help='The image to send'
    )
  parser.add_argument('--ip'
    , help='IP address of the pixel wall'
    , default="10.0.0.27"
    )
  parser.add_argument('--port'
    , help='Port on the PixelWall to send the data'
    , default=7000
    , type=int
    )
  parser.add_argument('--offset-x'
    , help='pixel offset in the image'
    , default=0
    , type=int
    )
  parser.add_argument('--offset-y'
    , help='pixel offset in the image'
    , default=0
    , type=int
    )
  args = parser.parse_args()

  img = Image.open(args.image)
  pixels = img.load()
  data = SYNC
  for y in range(args.offset_y, min(img.size[1], args.offset_y + DISPLAY_HEIGHT)):
    row = []
    for x in range(args.offset_x, min(img.size[0], args.offset_x + DISPLAY_WIDTH)):
      px = pixels[x,y]
      if isinstance(px, tuple):
        row.append(chr(px[0]) + chr(px[1]) + chr(px[2]))
      else:
        print 'image not supported'
        return
    if y % 2 == 1:
      row.reverse()
    data += ''.join(row)
  #print data

  # UDP socket
  sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

  sock.sendto(data, (args.ip, args.port))

if __name__ == '__main__':
  main()
