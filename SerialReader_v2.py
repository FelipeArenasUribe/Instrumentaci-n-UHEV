import serial
import time
import struct
from datetime import datetime

#Estructura de los mensajes:

#   VI,timestamp,voltaje,corriente
#   Llanta,timestamp,Periodo
#   Motor,timestamp,Periodo

Data_Identifier = {
    'V': Voltaje_Corriente,
    'M': RPM_Motor,
    'L': RPM_Llanta,
}

def Voltaje_Corriente(line):
    voltaje = line.split(',')[1]
    voltaje = voltaje*(5.0/1024)
    corriente = line.split(',')[2]
    corriente = ((corriente*(5.0/1024))-2.5)/0.1
    t_stamp = line.split(',')[3]
    t_stamp = t_stamp/1000
    print(f"{t_stamp},{voltaje:.2f},{corriente:.2f}")
    f.write(f"{t_stamp},{voltaje:.2f},{corriente:.2f}")
    f.write("\n")


def RPM_Motor(line):
    Periodo_Motor = line.split(',')[1]
    RPM_Motor = (1/Periodo_Motor)*60000
    t_stamp = line.split(',')[2]
    t_stamp = t_stamp/1000
    print(f"{t_stamp},{voltaje:.2f},{corriente:.2f}")
    f.write(f"{t_stamp},{voltaje:.2f},{corriente:.2f}")
    f.write("\n")


def RPM_Llanta(line):
    Periodo_Llanta = line.split(',')[1]
    RPM_Llanta = (1/Periodo_Llanta)*60000
    t_stamp = line.split(',')[2]
    t_stamp = t_stamp/1000
    print(f"{t_stamp},{voltaje:.2f},{corriente:.2f}")
    f.write(f"{t_stamp},{voltaje:.2f},{corriente:.2f}")
    f.write("\n")

with open(f'Prueba-UHEV_{datetime.now().strftime("%Y_%m_%d-%H_%M_%S")}.csv', 'w') as f:
    with serial.Serial("COM3", 9600) as s:
        print("Estableciendo Conexion...")
        s.timeout = 1
        total_processed = 0
        last_timestamp = 0
        # time.sleep(0.5)
        #Sincronizar dispositivos
        while True:
            sync = s.read(1)
            s.write(sync)
            print(f"Byte recibido: {sync.hex()}")
            sync = s.read(1)
            print(f"Byte recibido de respuesta: {sync.hex()}")
            if sync == bytes([0xFF]):
                break
        s.timeout = None
        print("Dispositivos sincronizados.")
        while True:
            line = s.readline()
            line2 = line.decode()
            line = line2.rstrip()
            ID=line.split(',')[0]
            Data_Identifier.get(line, lambda: "Invalid input")
            last_timestamp = t_stamp
            total_processed += 1
            if total_processed % 10 == 0:
                f.flush()
                print(
                    f"Procesado {total_processed} datos. Ultimo timestamp: {last_timestamp} Bytes en el buffer: {s.in_waiting}")