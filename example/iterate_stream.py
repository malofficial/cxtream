#!/usr/bin/env python3
import stream
import cv2

for img in stream.get_epoch_iterator() :
    cv2.imshow("Image in Python", img)
    cv2.waitKey(500);
