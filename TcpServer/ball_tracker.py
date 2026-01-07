import cv2
import numpy as np

class BallTracker:
    def __init__(self):
        # --- HSV range for white / light ball ---
        # Adjust if needed
        self.lower_hsv = np.array([31, 57, 48])
        self.upper_hsv = np.array([180, 255, 255])

        # --- Detection parameters ---
        self.min_area = 300
        self.min_circularity = 0.65
        self.min_radius = 6
        self.max_radius = 120

        # --- Motion filtering ---
        self.last_center = None
        self.min_motion_px = 2

    def process(self, frame):
        """
        Input: BGR frame
        Output: (frame_with_overlay, detection_dict or None)
        """

        hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)

        mask = cv2.inRange(hsv, self.lower_hsv, self.upper_hsv)

        # Clean mask
        kernel = np.ones((5, 5), np.uint8)
        mask = cv2.morphologyEx(mask, cv2.MORPH_OPEN, kernel)
        mask = cv2.morphologyEx(mask, cv2.MORPH_CLOSE, kernel)

        contours, _ = cv2.findContours(
            mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE
        )

        H, W = frame.shape[:2]

        best = None
        best_score = 0

        for c in contours:
            area = cv2.contourArea(c)
            if area < self.min_area:
                continue

            perimeter = cv2.arcLength(c, True)
            if perimeter == 0:
                continue

            circularity = 4 * np.pi * area / (perimeter * perimeter)
            if circularity < self.min_circularity:
                continue

            (x, y), radius = cv2.minEnclosingCircle(c)
            if radius < self.min_radius or radius > self.max_radius:
                continue

            # Reject contours touching frame edges (walls)
            bx, by, bw, bh = cv2.boundingRect(c)
            if bx <= 2 or by <= 2 or bx + bw >= W - 2 or by + bh >= H - 2:
                continue

            score = circularity * area
            if score > best_score:
                best_score = score
                best = (int(x), int(y), int(radius))

        detection = None

        if best:
            cx, cy, r = best

            # --- Motion gating ---
            if self.last_center:
                dx = cx - self.last_center[0]
                dy = cy - self.last_center[1]
                dist = np.sqrt(dx * dx + dy * dy)
                if dist < self.min_motion_px:
                    best = None

            self.last_center = (cx, cy)

        if best:
            cx, cy, r = best

            # Draw overlay
            cv2.circle(frame, (cx, cy), r, (0, 255, 0), 2)
            cv2.circle(frame, (cx, cy), 3, (0, 0, 255), -1)

            detection = {
                "x": cx,
                "y": cy,
                "radius": r
            }

        return frame, detection
