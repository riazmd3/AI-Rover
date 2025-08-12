from ultralytics import YOLO

# Load the trained model
model = YOLO("yolov8_best.pt")

# Print class names
print("Class names in the model:")
print(model.names)
