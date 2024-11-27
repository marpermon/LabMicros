import pandas as pd
import numpy as np
import tensorflow as tf
from sklearn.preprocessing import StandardScaler
from sklearn.model_selection import train_test_split
import matplotlib.pyplot as plt

# Función auxiliar para filtrar filas numéricas
def load_csv_numeric_only(filename):
    with open(filename, 'r') as file:
        lines = file.readlines()
        
    # Filtrar filas que contienen solo datos numéricos
    numeric_lines = [line for line in lines if all(c.isdigit() or c in "-.,\n" for c in line.strip())]
    
    # Crear DataFrame a partir de las filas filtradas
    df = pd.DataFrame([line.strip().split(',') for line in numeric_lines]).astype(float)
    return df

# Cargar los datos y omitir las líneas no numéricas
df1 = load_csv_numeric_only('labels.csv')
x = df1.to_numpy()

df2 = pd.read_csv('features.csv',header = None)
y = df2.to_numpy()

print(x.shape)
print(y.shape)

# Normalizar los datos
scaler_x = StandardScaler()
scaler_y = StandardScaler()
x = scaler_x.fit_transform(x)
y = scaler_y.fit_transform(y)

# Dividir los datos en entrenamiento y validación
x_train, x_val, y_train, y_val = train_test_split(x, y, test_size=0.2, random_state=42)

# Definir el modelo
model = tf.keras.Sequential([
    tf.keras.layers.Input(shape=(x.shape[1],)),
    tf.keras.layers.Dense(units=95, activation='relu',kernel_initializer=tf.keras.initializers.HeUniform(),bias_initializer=tf.keras.initializers.Zeros()),
    #tf.keras.layers.PReLU(),
    tf.keras.layers.BatchNormalization(),
    tf.keras.layers.Dropout(0.2),
    tf.keras.layers.PReLU(),
    tf.keras.layers.Dense(units=75, activation='relu',kernel_initializer=tf.keras.initializers.HeUniform(),bias_initializer=tf.keras.initializers.Zeros()),
    #tf.keras.layers.PReLU(),
    tf.keras.layers.BatchNormalization(),
    tf.keras.layers.Dropout(0.2),
    tf.keras.layers.Dense(units=95, activation='relu',kernel_initializer=tf.keras.initializers.HeUniform(),bias_initializer=tf.keras.initializers.Zeros()),
    tf.keras.layers.BatchNormalization(),
    tf.keras.layers.Dropout(0.1),
    #tf.keras.layers.Dense(units=25, activation='relu'),
    #tf.keras.layers.Dropout(0.1),
    tf.keras.layers.Dense(units=y.shape[1], activation='linear')  
])

# Compilar el modelo
model.compile(
    optimizer=tf.keras.optimizers.Adam(0.0005),  
    loss='mean_squared_error',  
    metrics=['mae']  
)


early_stopping = tf.keras.callbacks.EarlyStopping(monitor='val_loss', patience=5, restore_best_weights=True)

history = model.fit(x_train, y_train, epochs=1500, batch_size=40, validation_data=(x_val, y_val), verbose=1)
callbacks=[early_stopping]


val_loss, val_mae = model.evaluate(x_val, y_val)
print(f'Validation Loss: {val_loss}, Validation MAE: {val_mae}')


entrada = np.array([[0.000282,0.011414,0.000282,-0.001221,-0.000050,-0.002153]])
entrada = scaler_x.transform(entrada)
resultado = model.predict(entrada)
resultado = scaler_y.inverse_transform(resultado)
print(resultado)


plt.figure(figsize=(12, 6))

# Pérdida
plt.subplot(1, 2, 1)
plt.plot(history.history['loss'], label='Pérdida de entrenamiento')
plt.plot(history.history['val_loss'], label='Pérdida de validación')
plt.title('Pérdida durante el entrenamiento y validación')
plt.xlabel('Épocas')
plt.ylabel('Pérdida')
plt.legend()

# MAE
plt.subplot(1, 2, 2)
plt.plot(history.history['mae'], label='MAE de entrenamiento')
plt.plot(history.history['val_mae'], label='MAE de validación')
plt.title('MAE durante el entrenamiento y validación')
plt.xlabel('Épocas')
plt.ylabel('MAE')
plt.legend()

plt.tight_layout()
plt.show()

model.save('ModeloUltimo.h5')
np.save('scaler_x.npy', scaler_x.mean_)
np.save('scaler_y.npy', scaler_y.mean_)
np.save('scaler_x_scale.npy', scaler_x.scale_)
np.save('scaler_y_scale.npy', scaler_y.scale_)


