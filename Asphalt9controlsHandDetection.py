import cv2
from cvzone.HandTrackingModule import HandDetector
import keyboard
import time
cap = cv2.VideoCapture(0)
detector = HandDetector(detectionCon=0.8, maxHands=2)
cap.set(cv2.CAP_PROP_FPS, 60) # newly added code
cfps = int(cap.get(cv2.CAP_PROP_FPS))

def compare(finger, check):
    if len(finger) == len(check):
        for i in range(len(finger)):
            if (finger[i] == check[i]) == False:
                return False
        return True
    
while True:
    success, img = cap.read()
    #hands, img = detector.findHands(img, draw=True)
    hands = detector.findHands(img, draw=False)
    if hands:
        hand1 = hands[0]
        lmList1 = hand1["lmList"]
        fingers1 = detector.fingersUp(hand1)
        if len(hands) == 2:
            hand2 = hands[1]
            lmList2 = hand2["lmList"]
            fingers2 = detector.fingersUp(hand2)

            if compare(fingers1, [0, 0, 0, 0, 0]):
                keyboard.press('right')
                time.sleep(0.1)
                keyboard.release('right')
            if compare(fingers1, [1, 0, 0, 0, 0]):
                keyboard.press('space')
                time.sleep(0.05)
                keyboard.release('space')
            if compare(fingers2, [1, 0, 0, 0, 0]):
                keyboard.press('down')
                time.sleep(0.05)
                keyboard.release('down')
            if compare(fingers2,[0,0,0,0,0]):
                keyboard.press('left')
                time.sleep(0.1)
                keyboard.release('left')
