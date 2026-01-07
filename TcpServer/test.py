# hsv_picker.py
import cv2
import numpy as np

def nothing(x):
    pass

cap = cv2.VideoCapture("http://192.168.0.199:9000/video_feed")  # replace with your feed if needed

cv2.namedWindow("Trackbars")
cv2.createTrackbar("H Low","Trackbars",0,179,nothing)
cv2.createTrackbar("H High","Trackbars",179,179,nothing)
cv2.createTrackbar("S Low","Trackbars",0,255,nothing)
cv2.createTrackbar("S High","Trackbars",255,255,nothing)
cv2.createTrackbar("V Low","Trackbars",0,255,nothing)
cv2.createTrackbar("V High","Trackbars",255,255,nothing)

while True:
    ret, frame = cap.read()
    if not ret:
        continue

    hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)

    h_low = cv2.getTrackbarPos("H Low","Trackbars")
    h_high = cv2.getTrackbarPos("H High","Trackbars")
    s_low = cv2.getTrackbarPos("S Low","Trackbars")
    s_high = cv2.getTrackbarPos("S High","Trackbars")
    v_low = cv2.getTrackbarPos("V Low","Trackbars")
    v_high = cv2.getTrackbarPos("V High","Trackbars")

    lower = np.array([h_low, s_low, v_low])
    upper = np.array([h_high, s_high, v_high])

    mask = cv2.inRange(hsv, lower, upper)
    result = cv2.bitwise_and(frame, frame, mask=mask)

    cv2.imshow("Original", frame)
    cv2.imshow("Mask", mask)
    cv2.imshow("Result", result)

    if cv2.waitKey(1) & 0xFF == 27:
        break

cap.release()
cv2.destroyAllWindows()
