#!/usr/bin/env python3
import cxflow_stream
import cv2

ds = cxflow_stream.Dataset("/tmp/CIFAR-10")

for batch in ds.create_train_stream():
    for img, path in zip(batch['images'], batch['fpaths']):
        print(path, img.shape)
        # cv2.imshow(path, img)
