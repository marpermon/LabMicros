#!/bin/env python3

import serial
import time
import csv

# Configuración del puerto serial
serial_port = serial.Serial(port="/dev/ttyACM0", baudrate=115200, timeout=1)
print("Conectado al microcontrolador")

# El nombre del archivo CSV
csv_filename = "sensor_data.csv"

# Abrir el archivo CSV para escritura (se creará si no existe)
with open(csv_filename, mode='w', newline='') as csv_file:
    csv_writer = csv.writer(csv_file)
    csv_writer.writerow(["x", "y", "z"])

    while True:
        try:
            data_captured = serial_port.readline().decode('utf-8').replace('\n','').split(' ')
            if not data_captured or len(data_captured) != 3:
                continue
            if len(data_captured) == 3:
                for i in range(3):
                    data_captured[i] = int(data_captured[i])
                print(data_captured)
                csv_writer.writerow(data_captured)
            time.sleep(0.5)  # Controla la frecuencia de envío
        except serial.serialutil.SerialException:
            continue