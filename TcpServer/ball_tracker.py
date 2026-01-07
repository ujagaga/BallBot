# ball_tracker.py
import cv2
import numpy as np

# --- HSV color range for the ball (tweak for your ball color) ---
# Example: a standard orange ping-pong ball
LOWER_COLOR = np.array([5, 150, 150])    # lower HSV bound
UPPER_COLOR = np.array([15, 255, 255])   # upper HSV bound

# Minimum radius to consider a detection
MIN_RADIUS = 5

def detect_ball(frame):
    """
    Detect a ball in the frame and return the frame with a circle drawn.
    Returns:
        frame_out: frame with ball highlighted (if found)
        ball_coords: (x, y, radius) of detected ball or None if not found
    """
    if frame is None:
        return None, None

    # Blur to reduce noise
    blurred = cv2.GaussianBlur(frame, (11, 11), 0)

    # Convert to HSV
    hsv = cv2.cvtColor(blurred, cv2.COLOR_BGR2HSV)

    # Mask for the ball color
    mask = cv2.inRange(hsv, LOWER_COLOR, UPPER_COLOR)
    mask = cv2.erode(mask, None, iterations=2)
    mask = cv2.dilate(mask, None, iterations=2)

    # Find contours
    contours, _ = cv2.findContours(mask.copy(), cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

    ball_coords = None

    if contours:
        # Find largest contour (assume ball)
        c = max(contours, key=cv2.contourArea)
        ((x, y), radius) = cv2.minEnclosingCircle(c)

        if radius >= MIN_RADIUS:
            # Draw circle
            cv2.circle(frame, (int(x), int(y)), int(radius), (0, 255, 0), 2)
            ball_coords = (int(x), int(y), int(radius))

    return frame, ball_coords
