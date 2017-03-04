#!/usr/bin/env python3
import column_stream
import cv2

for batch in column_stream.get_epoch_iterator("/home/floop/Workspace/20170120_cpp_stream_prototype/pictures/"):
    for img, path in zip(batch['rimage'], batch['fpath']):
        cv2.imshow(path, img)
