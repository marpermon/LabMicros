#!/bin/env python3

import serial
import time
import csv

import serial.serialutil

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
            data_captured = serial_port.readline().decode('utf-8').replace('\r', "").replace("\n", "").split('\t')
            if not data_captured or len(data_captured) < 3:
                continue

            if len(data_captured) >= 3:
                x_value = int(data_captured[0].split()[1])
                y_value = int(data_captured[1].split()[1])
                z_value = int(data_captured[2].split()[1])
                print([x_value, y_value, z_value])
                csv_writer.writerow([x_value, y_value, z_value])
            time.sleep(0.5)  # Controla la frecuencia de envío
        except serial.serialutil.SerialException:
            continue