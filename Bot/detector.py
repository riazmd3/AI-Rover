# from flask import Flask, request, jsonify
# from flask_cors import CORS
# import requests
# import cv2
# import numpy as np
# from ultralytics import YOLO
# import os

# app = Flask(__name__)
# CORS(app)
# model = YOLO(r"C:\Users\riazm\OneDrive\Desktop\Ai - Rover\Bot\yolov8_best.pt")

# @app.route('/detect', methods=['POST'])
# def detect():
#     if 'image' not in request.files:
#         return jsonify({'error': 'No image uploaded'}), 400
#     file = request.files['image']
#     img_bytes = file.read()
#     npimg = np.frombuffer(img_bytes, np.uint8)
#     image = cv2.imdecode(npimg, cv2.IMREAD_COLOR)
#     results = model(image)
#     detections = []
#     for r in results:
#         for box in r.boxes:
#             cls_id = int(box.cls[0])
#             class_name = r.names[cls_id]
#             confidence = float(box.conf[0])
#             detections.append({
#                 'disease': class_name,
#                 'confidence': round(confidence, 2)
#             })
#     return jsonify({'detections': detections})

# if __name__ == '__main__':
#     app.run(host='0.0.0.0', port=5000)

from flask import Flask, request, jsonify
from flask_cors import CORS
import requests
import cv2
import numpy as np
from ultralytics import YOLO
import os

app = Flask(__name__)
CORS(app)
model = YOLO(r"C:\Users\riazm\OneDrive\Desktop\Ai - Rover\Bot\yolov8_best.pt")

# Disease remedies mapped by class ID
disease_remedies = {
    0: "Apple Scab Leaf - Remedy: Use fungicides like Captan or apply resistant cultivars.",
    1: "Apple leaf - Remedy: Ensure proper pruning and apply neem oil spray.",
    2: "Apple rust leaf - Remedy: Remove nearby junipers and apply fungicide sprays.",
    3: "Bell_pepper leaf spot - Remedy: Apply copper-based fungicides and rotate crops.",
    4: "Bell_pepper leaf - Remedy: Improve air circulation and use resistant varieties.",
    5: "Blueberry leaf - Remedy: Use proper irrigation and apply fungicide if needed.",
    6: "Cherry leaf - Remedy: Prune affected leaves and use protective fungicides.",
    7: "Corn Gray leaf spot - Remedy: Apply foliar fungicides at VT-R1 stages.",
    8: "Corn leaf blight - Remedy: Plant resistant hybrids and rotate crops.",
    9: "Corn rust leaf - Remedy: Use resistant hybrids and apply fungicides.",
    10: "Peach leaf - Remedy: Apply fungicides during dormancy.",
    11: "Potato leaf early blight - Remedy: Spray chlorothalonil or mancozeb.",
    12: "Potato leaf late blight - Remedy: Use fungicides like metalaxyl and avoid overhead watering.",
    13: "Potato leaf - Remedy: Ensure proper spacing and remove infected leaves.",
    14: "Raspberry leaf - Remedy: Prune canes and apply sulfur-based sprays.",
    15: "Soyabean leaf - Remedy: Use resistant varieties and rotate crops.",
    16: "Soybean leaf - Remedy: Apply foliar fungicides when disease pressure is high.",
    17: "Squash Powdery mildew leaf - Remedy: Use sulfur sprays and resistant cultivars.",
    18: "Strawberry leaf - Remedy: Remove old leaves and apply fungicides.",
    19: "Tomato Early blight leaf - Remedy: Apply copper-based fungicides.",
    20: "Tomato Septoria leaf spot - Remedy: Use fungicides and avoid wetting leaves.",
    21: "Tomato leaf bacterial spot - Remedy: Use certified seeds and copper fungicides.",
    22: "Tomato leaf late blight - Remedy: Spray chlorothalonil or mancozeb.",
    23: "Tomato leaf mosaic virus - Remedy: Remove infected plants and disinfect tools.",
    24: "Tomato leaf yellow virus - Remedy: Control whiteflies and remove infected plants.",
    25: "Tomato leaf - Remedy: Maintain good airflow and avoid overwatering.",
    26: "Tomato mold leaf - Remedy: Apply fungicides and improve greenhouse ventilation.",
    27: "Tomato two spotted spider mites leaf - Remedy: Spray miticides or neem oil.",
    28: "Grape leaf black rot - Remedy: Use fungicides and remove mummified fruit.",
    29: "Grape leaf - Remedy: Prune properly and apply protective fungicides."
}


@app.route('/detect', methods=['POST'])
def detect():
    if 'image' not in request.files:
        return jsonify({'error': 'No image uploaded'}), 400
    file = request.files['image']
    img_bytes = file.read()
    npimg = np.frombuffer(img_bytes, np.uint8)
    image = cv2.imdecode(npimg, cv2.IMREAD_COLOR)
    results = model(image)
    detections = []
    for r in results:
        for box in r.boxes:
            cls_id = int(box.cls[0])
            class_name = r.names[cls_id]
            confidence = float(box.conf[0])
            
            # Get disease + remedy from dictionary
            disease_with_remedy = disease_remedies.get(cls_id, class_name + " - Remedy not available")
            
            detections.append({
                'disease': disease_with_remedy,
                'confidence': round(confidence, 2)
            })

        return jsonify({'detections': detections})

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
