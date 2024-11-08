import asyncio
import subprocess
import sys
import traceback
import websockets
from models.messages import BackendMessageType, BackendMessage
from clients.aura_dbus import AuraClient
from clients.fan_curves_dbus import FanCurvesClient
from clients.platform_dbus import PlatformClient
from clients.power_dbus import PowerProfilesClient

aura = AuraClient()
power = PowerProfilesClient()
fans = FanCurvesClient()
platform = PlatformClient()

async def connect_to_server():
    uri = "ws://localhost:18158"  # Dirección del servidor WebSocket

    try:
        async with websockets.connect(uri) as websocket:
            print("Conectado al servidor WebSocket")

            # Escuchar continuamente los mensajes del servidor
            while True:
                message = await websocket.recv()
                print(f"Mensaje recibido del servidor: {message}")
                msg=BackendMessage.parse(message)

                result = []
                error = None
                if msg.type==BackendMessageType.REQUEST :
                    try:
                        if msg.option == "getBrightness":
                            result.append(aura.brightness)
                        elif msg.option == "setBrightness":
                            aura.brightness = msg.data[0]
                        elif msg.option == "getLedMode":
                            result.append(aura.led_mode)
                        elif msg.option == "setLedMode":
                            aura.led_mode = msg.data[0]   
                        elif msg.option == "getActivePowerProfile":
                            result.append(power.active_profile)
                        elif msg.option == "setActivePowerProfile":
                            power.active_profile = msg.data[0]        
                        elif msg.option == "resetProfileCurves":
                            fans.reset_profile_curves(msg.data[0])
                        elif msg.option == "setCurvesToDefaults":
                            fans.reset_profile_curves(msg.data[0])  
                        elif msg.option == "setFanCurvesEnabled":
                            fans.set_fan_curves_enabled(msg.data[0], msg.data[1])    
                        elif msg.option == "getChargeControlEndThresold":
                            result.append(platform.charge_control_end_threshold)
                        elif msg.option == "setChargeControlEndThresold":
                            platform.charge_control_end_threshold = msg.data[0] 
                        elif msg.option == "getThrottleThermalProfile":
                            result.append(platform.throttle_thermal_policy)
                        elif msg.option == "setThrottleThermalProfile":
                            platform.throttle_thermal_policy = msg.data[0]
                        elif msg.option == "runBoostCommand":
                            command = f"echo \"{msg.data[0]}\" | sudo -S bash -c \"echo '{msg.data[1]}' | tee {msg.data[2]} > /dev/null\""
                            process = subprocess.run(command, capture_output=True, text=True, shell=True)
                            result.append(process.returncode)
                        else:
                            error = f"Unknown option {msg.option}"
                            print(error)
                    except Exception as e:
                        traceback.print_exc()
                        error = str(e)
                else:
                    error = f"Invalid message type {msg.type}"
                    print(error)

                msg.type = BackendMessageType.RESPONSE
                msg.data = result
                msg.error = error
                
                await websocket.send(str(msg))

    except (websockets.ConnectionClosed, ConnectionRefusedError) as e:
        print(f"Desconectado del servidor: {e}")
        print("Terminando el script debido a la desconexión.")
        sys.exit(1)  # Salida con error

# Ejecuta el cliente de forma indefinida
asyncio.run(connect_to_server())
