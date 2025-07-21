import requests
import cv2
import numpy as np
from ultralytics import YOLO

model = YOLO("yolov8_best.pt")

esp32_url = "http://10.184.195.84/capture"
response = requests.get(esp32_url)
if response.status_code == 200:
    with open("image.jpg", "wb") as f:
        f.write(response.content)

    image = cv2.imread("image.jpg")
    results = model(image)

    for r in results:
        for box in r.boxes:
            cls_id = int(box.cls[0])
            class_name = r.names[cls_id]
            confidence = float(box.conf[0])
            print(f"Disease: {class_name}, Confidence: {confidence:.2f}")
            # You can map class_name to remedies here and return
else:
    print("Failed to get image from ESP32-CAM")
