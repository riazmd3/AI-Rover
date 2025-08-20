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
    0: "Apple Scab Leaf - Remedy: Prune trees during the dormant season to improve air circulation and reduce humidity that favors fungal growth. Apply fungicides such as Captan, Mancozeb, or Myclobutanil during early spring before symptoms appear. Use resistant cultivars if possible. Remove and destroy fallen leaves to reduce overwintering spores.",
    
    1: "Apple Leaf Spot - Remedy: Maintain orchard sanitation by clearing fallen leaves and debris. Apply neem oil or copper-based sprays as preventive measures. Ensure balanced fertilization, as excess nitrogen promotes disease. Plant resistant apple varieties and avoid overhead irrigation to reduce leaf wetness.",
    
    2: "Apple Rust Leaf - Remedy: Remove or avoid planting juniper trees near apple orchards since they act as alternate hosts. Apply fungicides like Myclobutanil, Propiconazole, or Sulfur sprays at the pink bud stage. Prune infected leaves and improve airflow around the trees to minimize humidity.",
    
    3: "Bell Pepper Leaf Spot - Remedy: Rotate crops every 2–3 years to prevent pathogen buildup. Remove infected plant debris immediately. Apply copper-based fungicides regularly during the growing season. Avoid overhead watering and provide proper spacing between plants to reduce leaf wetness.",
    
    4: "Bell Pepper Healthy Leaf Maintenance - Remedy: Encourage strong plant growth by adding compost or well-balanced fertilizer. Use resistant pepper varieties to minimize disease risks. Provide proper sunlight and ensure well-drained soil. Apply preventive neem oil sprays every 10–14 days.",
    
    5: "Blueberry Leaf Spot - Remedy: Maintain good soil drainage and mulch plants to reduce splashing of fungal spores. Prune overcrowded branches to improve airflow. Apply fungicides like Captan or Switch if infections are severe. Use drip irrigation instead of overhead watering.",
    
    6: "Cherry Leaf Spot - Remedy: Remove and destroy fallen leaves after harvest to reduce overwintering fungi. Apply fungicides like Captan, Chlorothalonil, or Myclobutanil before symptoms appear. Ensure pruning is done in late winter to maintain open canopies for better air circulation.",
    
    7: "Corn Gray Leaf Spot - Remedy: Rotate with non-host crops such as soybean or alfalfa to reduce disease carryover. Apply foliar fungicides like Azoxystrobin or Pyraclostrobin between VT (tasseling) and R1 (silking) stages. Use resistant corn hybrids and maintain proper crop residue management.",
    
    8: "Corn Leaf Blight - Remedy: Select resistant hybrids to minimize disease risk. Rotate crops with legumes or cereals other than corn. Apply fungicides like Mancozeb or Propiconazole at early stages of infection. Avoid planting corn in the same field consecutively.",
    
    9: "Corn Rust Leaf - Remedy: Plant resistant corn varieties. Apply fungicides such as Tebuconazole or Azoxystrobin when pustules first appear. Remove volunteer corn plants that act as pathogen hosts. Monitor fields frequently during warm, humid conditions.",
    
    10: "Peach Leaf Curl - Remedy: Apply fungicides such as Copper Hydroxide or Chlorothalonil during the dormant season before buds swell. Remove and destroy infected leaves to prevent further spread. Avoid excessive nitrogen fertilizers, which make plants more susceptible.",
    
    11: "Potato Early Blight - Remedy: Plant certified disease-free seeds. Apply fungicides like Chlorothalonil, Mancozeb, or Azoxystrobin when the first symptoms appear. Remove infected leaves and ensure wide plant spacing for better airflow. Practice crop rotation with non-solanaceous crops.",
    
    12: "Potato Late Blight - Remedy: Use resistant potato varieties when available. Apply fungicides such as Metalaxyl, Cymoxanil, or Mancozeb regularly. Avoid overhead irrigation and ensure proper field drainage. Destroy infected plant material immediately to reduce spore spread.",
    
    13: "Potato Leaf General Care - Remedy: Maintain good soil fertility with adequate potassium to enhance resistance. Space plants properly and remove infected leaves at the first sign of disease. Use mulch to reduce soil splash that spreads pathogens.",
    
    14: "Raspberry Leaf Spot - Remedy: Prune canes after harvest to improve air circulation. Remove and burn infected leaves. Apply sulfur-based fungicides or copper sprays as a preventive measure. Ensure fields are weed-free to reduce humidity.",
    
    15: "Soybean Leaf Diseases - Remedy: Plant resistant soybean varieties and rotate crops with corn or wheat. Apply fungicides such as Tebuconazole, Azoxystrobin, or Trifloxystrobin when disease pressure is high. Ensure uniform row spacing to enhance airflow.",
    
    16: "Soybean Rust Leaf - Remedy: Scout fields frequently, especially during warm, wet conditions. Apply foliar fungicides like Triazoles (Propiconazole) or Strobilurins (Azoxystrobin) at early stages of infection. Rotate with non-legume crops to break disease cycles.",
    
    17: "Squash Powdery Mildew Leaf - Remedy: Plant resistant squash varieties whenever available. Apply sulfur or potassium bicarbonate sprays early in the infection. Remove severely infected leaves. Ensure good spacing and airflow to reduce humidity.",
    
    18: "Strawberry Leaf Spot - Remedy: Remove old, infected leaves after harvest. Use fungicides like Captan, Thiram, or Myclobutanil as preventive sprays. Ensure drip irrigation instead of overhead watering. Apply mulch to reduce soil splash.",
    
    19: "Tomato Early Blight - Remedy: Rotate with non-solanaceous crops for at least 2 years. Apply copper-based fungicides or Mancozeb when symptoms appear. Remove lower leaves to reduce infection risk. Mulch around plants to prevent soil-borne spores from splashing.",
    
    20: "Tomato Septoria Leaf Spot - Remedy: Apply fungicides such as Chlorothalonil or Copper Hydroxide every 7–10 days. Avoid wetting leaves during irrigation. Prune lower branches to prevent soil contact. Remove infected debris after harvest.",
    
    21: "Tomato Bacterial Spot - Remedy: Use certified, disease-free seeds. Apply copper fungicides at regular intervals. Avoid working in fields when plants are wet to prevent spread. Rotate crops and sterilize gardening tools frequently.",
    
    22: "Tomato Late Blight - Remedy: Apply fungicides like Chlorothalonil, Mancozeb, or Cymoxanil at first symptoms. Avoid overhead watering and ensure proper ventilation in greenhouses. Immediately remove and destroy infected plants to prevent rapid spread.",
    
    23: "Tomato Mosaic Virus - Remedy: Remove and destroy infected plants immediately. Disinfect tools, stakes, and hands with a bleach solution before handling healthy plants. Avoid tobacco users handling plants, as the virus spreads easily. Plant resistant varieties if available.",
    
    24: "Tomato Yellow Leaf Curl Virus - Remedy: Control whiteflies with yellow sticky traps or insecticidal soap. Remove and destroy infected plants promptly. Use resistant tomato cultivars. Apply reflective mulches to reduce vector activity.",
    
    25: "Tomato Leaf General Care - Remedy: Maintain consistent soil moisture and avoid overwatering. Ensure proper plant spacing and good airflow. Use mulch to suppress weeds and reduce soil splash. Apply preventive neem oil sprays biweekly.",
    
    26: "Tomato Mold (Gray Mold) Leaf - Remedy: Improve greenhouse or field ventilation to reduce humidity. Apply fungicides like Iprodione or Chlorothalonil. Remove and destroy infected plant material immediately. Avoid dense planting.",
    
    27: "Tomato Two-Spotted Spider Mite Infestation - Remedy: Regularly spray plants with water to dislodge mites. Apply neem oil, insecticidal soap, or miticides like Abamectin. Introduce natural predators such as ladybugs or predatory mites.",
    
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
