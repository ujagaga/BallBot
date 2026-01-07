import cv2
import numpy as np

# --- Internal state (kept across frames) ---
_prev_gray = None


def detect_ball(frame):
    """
    Detect a moving ball using:
    - motion gating (frame differencing)
    - Hough circle detection
    Returns:
        frame (with drawing)
        info dict or None
    """
    global _prev_gray

    # --- Preprocess ---
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    gray = cv2.GaussianBlur(gray, (9, 9), 1.5)

    # --- First frame init ---
    if _prev_gray is None:
        _prev_gray = gray
        return frame, None

    # --- Motion detection ---
    diff = cv2.absdiff(_prev_gray, gray)
    _prev_gray = gray

    _, motion_mask = cv2.threshold(diff, 25, 255, cv2.THRESH_BINARY)
    motion_mask = cv2.dilate(motion_mask, None, iterations=2)

    # --- Apply motion mask ---
    masked_gray = cv2.bitwise_and(gray, gray, mask=motion_mask)

    # --- Circle detection ---
    circles = cv2.HoughCircles(
        masked_gray,
        cv2.HOUGH_GRADIENT,
        dp=1.2,
        minDist=80,
        param1=120,
        param2=25,
        minRadius=15,
        maxRadius=80
    )

    if circles is None:
        return frame, None

    circles = np.uint16(np.around(circles))
    x, y, r = circles[0][0]

    # --- Draw ---
    cv2.circle(frame, (x, y), r, (0, 255, 0), 2)
    cv2.circle(frame, (x, y), 2, (0, 0, 255), 3)
    cv2.putText(
        frame,
        f"Ball r={r}",
        (x - 30, y - r - 10),
        cv2.FONT_HERSHEY_SIMPLEX,
        0.5,
        (0, 255, 0),
        1,
        cv2.LINE_AA
    )

    info = {
        "x": int(x),
        "y": int(y),
        "radius": int(r)
    }

    return frame, info
