import paho.mqtt.client as mqtt
import serial
import json
import time

# Configuración del puerto serial
serial_port = serial.Serial(port="/dev/ttyACM0", baudrate=115200, timeout=1)
print("Conectado al microcontrolador")

# Datos del cliente MQTT
broker = "iot.eie.ucr.ac.cr" 
port = 1883
device_token = "66dd4RwHoqM4bvSqsRKu" 
topic = "v1/devices/me/telemetry"

# Configuración del cliente MQTT
client = mqtt.Client()
client.username_pw_set(device_token)
client.connect(broker, port, keepalive=60)

# Función de callback para confirmar publicación
# def on_publish(client, userdata, result):
#    print("Datos publicados correctamente \n")

#client.on_publish = on_publish

while True:
    data_captured = serial_port.readline().decode('utf-8').replace('\r', "").replace("\n", "").split('\t')
    if len(data_captured) >= 3:
        data = {
            "x": int(data_captured[0].split()[1]),
            "y": int(data_captured[1].split()[1]),
            "z": int(data_captured[2].split()[1])
            #,
            #"Battery Low": "Si" if data_captured[3] == '1' else "No",
            #"Battery Level": data_captured[4]
        }
        output = json.dumps(data)
        print(output)
        client.publish(topic, output)
    time.sleep(1)  # Controla la frecuencia de envío