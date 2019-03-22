#!/usr/bin/env python3

import cv2
import numpy as np
import matplotlib.pyplot as plt
import py_grid_finder as gr
from math import cos, sin


def main():
    video = cv2.VideoCapture("../Video/easy.mp4")
    frame_width = int(video.get(3)) * 2
    frame_height = int(video.get(4))
    fps = video.get(cv2.CAP_PROP_FPS)
    out = cv2.VideoWriter('out.avi', cv2.VideoWriter_fourcc(
        'M', 'J', 'P', 'G'), fps, (frame_width, frame_height))

    framectr = 1
    result, image = video.read()
    while result and video.isOpened():
        try:
            image = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)
            mask = redmask(image)
            processed = processFrame(image, mask)
            mask = cv2.cvtColor(mask, cv2.COLOR_GRAY2RGB)
            font = cv2.FONT_HERSHEY_SIMPLEX
            cv2.putText(mask, str(framectr), (16, frame_height - 16),
                        font, 1, (0, 100, 255), 2, cv2.LINE_AA)
            outimg = np.concatenate((processed, mask), axis=1)
            out.write(cv2.cvtColor(outimg, cv2.COLOR_RGB2BGR))
        except Exception as e:
            print(e)
        result, image = video.read()
        framectr += 1

    video.release()
    out.release()


def showLine(line, color, image):
    # print(line.getLineCenter())
    # print(line.getAngle())
    # print(line.getWidth())
    p1 = (line.getLineCenter().x, line.getLineCenter().y)
    p2 = (line.getLineCenter().x + int(2 * 410 * cos(line.getAngle())),
          line.getLineCenter().y + int(2 * 410 * sin(line.getAngle())))
    cv2.line(image, p1, p2, color, 2)


def processFrame(image, mask):
    gf = gr.GridFinder(mask)
    lines, points = gf.findSquare()

    colors = [(0, 80, 255), (0, 200, 255), (0, 255, 0),
              (255, 255, 0), (255, 150, 0)]

    for i in range(len(lines)):
        if lines[i]:
            showLine(lines[i], colors[i], image)

    for point in points:
        point = (round(point.x), round(point.y))
        cv2.circle(image, point, 3, (255, 0, 0), -1)

    return image


def redmask(image):
    hsv = cv2.cvtColor(image, cv2.COLOR_RGB2HSV)

    lower_red1 = np.array([0, 30, 0])
    upper_red1 = np.array([10, 255, 255])
    mask1 = cv2.inRange(hsv, lower_red1, upper_red1)

    lower_red2 = np.array([170, 30, 0])
    upper_red2 = np.array([180, 255, 255])
    mask2 = cv2.inRange(hsv, lower_red2, upper_red2)

    mask = mask1 | mask2
    return mask


if __name__ == '__main__':
    main()
