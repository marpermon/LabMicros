import serial 

ser = serial.Serial(
    port='COM3',
    baudrate=9600,
    #parity=serial.PARITY_NONE,
    #stopbits=serial.STOPBITS_ONE,
    #bytesize=serial.EIGHTBITS,
    #timeout=0
)

print('Connected to: ' + ser.portstr)


# Abrir archivo CSV para guardar los datos
with open('voltages.csv', 'w') as f:
    f.write('V0, V1, V2, V3\n')  # Encabezado del archivo CSV

    try:
        while True:
            # Leer una línea completa del puerto serial
            v1 = ser.readline().decode('utf-8').strip()
            v2 = ser.readline().decode('utf-8').strip()
            v3 = ser.readline().decode('utf-8').strip()
            v4 = ser.readline().decode('utf-8').strip()
            
            # Si la línea tiene contenido
            if v1:
                print(f"Received: {v1, v2, v3, v4}")  # Imprimir en terminal
                f.write("{}, {}, {}, {}\n".format(v1,v2,v3,v4))  # Guardar en el archivo CSV

    except KeyboardInterrupt:
        # Finalizar al presionar Ctrl+C
        print("Finalizando la lectura.")
    finally:
        # Cerrar la conexión serie
        ser.close()
