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

    start = time.time()

    while time.time() - start < 20.0:
        try:
            data_captured = serial_port.readline().decode('utf-8').replace('\n','')
            if len(data_captured.split()) == 3:
                print(data_captured)
                csv_writer.writerow(data_captured.split())
        except serial.serialutil.SerialException:
            continue
        except UnicodeDecodeError:
            continue