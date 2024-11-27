import tensorflow as tf

# Specify the .h5 file name (in the same directory)
model_filename = "ModeloUltimo.h5"  # Replace with your actual file name

# Load the Keras model
model = tf.keras.models.load_model(model_filename)

# Convert to TensorFlow Lite with optimization
converter = tf.lite.TFLiteConverter.from_keras_model(model)
converter.optimizations = [tf.lite.Optimize.DEFAULT]  # Enable default optimization (dynamic range quantization)
tflite_model = converter.convert()

# Save the optimized TFLite model
with open("model_optimized.tflite", "wb") as f:
    f.write(tflite_model)

print("Optimized model converted and saved as model_optimized.tflite.")
