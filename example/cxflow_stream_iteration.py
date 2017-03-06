#!/usr/bin/env python3
import cxflow_stream
import cv2

ds = cxflow_stream.Dataset("/tmp/CIFAR-10")

for batch in ds.create_train_stream():
    for img, path, label, str_label in zip(batch['images'], batch['fpaths'], batch['labels'], batch['str_labels']):
        print(path, label, str_label, img.shape)
        cv2.imshow(str_label, img)
        cv2.waitKey()
