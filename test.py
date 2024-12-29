#!/bin/env python3 
import pyudev

def monitor_devices():
    # Crear el contexto para interactuar con udev
    context = pyudev.Context()

    # Crear un monitor para escuchar eventos del sistema
    monitor = pyudev.Monitor.from_netlink(context)
    monitor.filter_by('usb')  # Filtrar eventos de dispositivos USB

    print("Monitoreando eventos de conexión y desconexión de dispositivos USB...")

    for action, device in monitor:
        if action == 'add':
            print(f"Dispositivo conectado: {device.device_path}")
            print(f"  Vendor ID: {device.get('ID_VENDOR_ID')}")
            print(f"  Product ID: {device.get('ID_MODEL_ID')}")
        elif action == 'remove':
            # Intentar obtener Vendor ID y Product ID antes de que el dispositivo sea completamente removido
            vendor_id = device.get('ID_VENDOR_ID', 'Desconocido')
            product_id = device.get('ID_MODEL_ID', 'Desconocido')
            print(f"Dispositivo desconectado: {device.device_path}")
            print(f"  Vendor ID: {vendor_id}")
            print(f"  Product ID: {product_id}")

# Llamar a la función
if __name__ == "__main__":
    monitor_devices()
