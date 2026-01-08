import cv2
import numpy as np

# --- CONFIGURATION ---
# Replace with your stream URL (e.g., 'http://192.168.1.100:8080/video')
# Use 0 for local webcam testing
VIDEO_URL = "http://192.168.0.199:9000/video_feed"

# Color Range for an Orange Ping Pong Ball (HSV)
# Note: For a White ball, use: lower = (0, 0, 168), upper = (172, 111, 255)
ORANGE_LOWER = np.array([5, 100, 100])
ORANGE_UPPER = np.array([15, 255, 255])


def detect_ball():
    cap = cv2.VideoCapture(VIDEO_URL)

    if not cap.isOpened():
        print("Error: Could not open video stream.")
        return

    while True:
        ret, frame = cap.read()
        if not ret:
            break

        # 1. Pre-processing: Blur to reduce noise
        blurred = cv2.GaussianBlur(frame, (11, 11), 0)
        hsv = cv2.cvtColor(blurred, cv2.COLOR_BGR2HSV)

        # 2. Create a mask for the ball color
        mask = cv2.inRange(hsv, ORANGE_LOWER, ORANGE_UPPER)

        # 3. Clean up the mask (remove small specks)
        mask = cv2.erode(mask, None, iterations=2)
        mask = cv2.dilate(mask, None, iterations=2)

        # 4. Find contours
        contours, _ = cv2.findContours(mask.copy(), cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

        if len(contours) > 0:
            # Assume the largest color-matching object is our ball
            c = max(contours, key=cv2.contourArea)
            ((x, y), radius) = cv2.minEnclosingCircle(c)

            # 5. Filter by minimum radius to avoid false positives
            if radius > 10:
                # Draw the circle and centroid on the frame
                cv2.circle(frame, (int(x), int(y)), int(radius), (0, 255, 255), 2)
                cv2.circle(frame, (int(x), int(y)), 5, (0, 0, 255), -1)
                cv2.putText(frame, "Ping Pong Ball", (int(x) - 10, int(y) - 10),
                            cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 255), 2)

        # Display results
        cv2.imshow("Ping Pong Detection", frame)
        cv2.imshow("Mask View (Debug)", mask)

        # Press 'q' to exit
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    cap.release()
    cv2.destroyAllWindows()


if __name__ == "__main__":
    detect_ball()