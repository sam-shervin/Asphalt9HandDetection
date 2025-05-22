import platform
import ctypes
import time

import cv2
import mediapipe as mp
import pyautogui  # pip install pyautogui

# === CONFIG ===
TURN_COOLDOWN = 0.6
FRAME_W, FRAME_H, FPS = 512, 512, 30
EXIT_THRESHOLD = 5  # pixels from (0,0)

# Key bytes
KEY_BYTES = {k: v for k,v in {
    'space': b'space', 'left': b'left', 'right': b'right',
    'up': b'up', 'down': b'down'
}.items()}

# — load keyhandler lib
lib_path = './keyhandler.dll' if platform.system()=='Windows' else './libkeyhandler.dylib'
keylib = ctypes.CDLL(lib_path)
keylib.pressKeyDown.argtypes = [ctypes.c_char_p]
keylib.pressKeyUp  .argtypes = [ctypes.c_char_p]

def press(key):
    kb = KEY_BYTES[key]
    keylib.pressKeyDown(kb)
    time.sleep(0.05)
    keylib.pressKeyUp(kb)

# — load gesture_utils lib
gj_path = './gesture_utils.dll' if platform.system()=='Windows' else './libgesture_utils.dylib'
gj = ctypes.CDLL(gj_path)
gj.fingers_up.argtypes = [ctypes.POINTER(ctypes.c_float),
                          ctypes.c_uint8,
                          ctypes.POINTER(ctypes.c_uint8)]
gj.fingers_up.restype  = None
gj.calc_slope.argtypes = [ctypes.c_float, ctypes.c_float,
                          ctypes.c_float, ctypes.c_float]
gj.calc_slope.restype  = ctypes.c_double

# Mediapipe setup
mp_hands = mp.solutions.hands
hands = mp_hands.Hands(max_num_hands=2,
                       model_complexity=0,
                       min_detection_confidence=0.7,
                       min_tracking_confidence=0.5)

cap = cv2.VideoCapture(0)
cap.set(cv2.CAP_PROP_FRAME_WIDTH, FRAME_W)
cap.set(cv2.CAP_PROP_FRAME_HEIGHT, FRAME_H)
cap.set(cv2.CAP_PROP_FPS, FPS)

try:
    while True:
        # --- exit check ---
        x, y = pyautogui.position()
        if x <= EXIT_THRESHOLD and y <= EXIT_THRESHOLD:
            print("Cursor touched Top-left corner — exiting.")
            break

        ret, frame = cap.read()
        if not ret:
            continue

        img = cv2.flip(frame, 1)
        rgb = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
        res = hands.process(rgb)

        if res.multi_hand_landmarks and res.multi_handedness:
            # map handedness to landmarks
            data = list(zip(res.multi_hand_landmarks, res.multi_handedness))
            M = {h.classification[0].label: lm for lm, h in data}

            buf = (ctypes.c_uint8 * 5)()

            # Right-hand closed?
            if 'Right' in M:
                arr = (ctypes.c_float * 42)(
                    *sum([[lm.x, lm.y] for lm in M['Right'].landmark], [])
                )
                gj.fingers_up(arr, ctypes.c_uint8(1), buf)
                if all(flag == 0 for flag in buf):
                    press('space')

            # Left-hand closed?
            if 'Left' in M:
                arr = (ctypes.c_float * 42)(
                    *sum([[lm.x, lm.y] for lm in M['Left'].landmark], [])
                )
                gj.fingers_up(arr, ctypes.c_uint8(0), buf)
                if all(flag == 0 for flag in buf):
                    press('down')

            # Two-hand slope gesture
            if 'Left' in M and 'Right' in M:
                p1 = M['Left'].landmark[0]
                p2 = M['Right'].landmark[0]
                slope = gj.calc_slope(p1.x, p1.y, p2.x, p2.y)
                direction = 'right' if slope > 0 else 'left'
                keylib.pressKeyDown(KEY_BYTES[direction])
                time.sleep(abs(slope * TURN_COOLDOWN))
                keylib.pressKeyUp(KEY_BYTES[direction])

        if cv2.waitKey(1) & 0xFF == 27:  # Esc key
            break

finally:
    cap.release()
    cv2.destroyAllWindows()
