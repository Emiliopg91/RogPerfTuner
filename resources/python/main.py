from lib.utils.websocket_client import OpenRgbClient, WebSocketClient

import asyncio

async def initialize_all():
    try:
        # Crear el cliente OpenRGB
        orgb_client = OpenRgbClient()
        print("OpenRgbClient creado correctamente.")

        # Crear el cliente WebSocket
        await WebSocketClient(orgb_client).connect()
        print("WebSocketClient creado correctamente.")
    except Exception as e:
        print(f"Error durante la inicialización: {e}")

if __name__ == "__main__":
    loop = asyncio.new_event_loop()
    asyncio.set_event_loop(loop)
    loop.create_task(initialize_all())
    try:
        loop.run_forever()
    except KeyboardInterrupt:
        print("Client disconnected.")
        loop.close()