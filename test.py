import asyncio
import os
from threading import Thread

from rcc.models.message_type import MessageType

SOCKET_PATH = os.path.expanduser(os.path.join("~", "homebrew", "data", "RCCDeckyCompanion", "socket.sock"))


async def handle_client(reader, writer):
    addr = writer.get_extra_info("peername")
    print(f"Cliente conectado: {addr}")
    try:
        writer.write(MessageType(type="R", name="test", data=[]).to_json().encode())
        while True:
            data = await reader.readline()
            if not data:
                break
            message = data.decode().strip()
            print(f"-> {message}")
            response = MessageType.from_json(message)
            response.type = "RESPONSE"
            print(f"<- {response.to_json()}")
            writer.write(response.to_json().encode())
            await writer.drain()
    except Exception as e:
        print(f"Error: {e}")
    finally:
        print("Cliente desconectado")
        writer.close()
        await writer.wait_closed()


async def main():
    if os.path.exists(SOCKET_PATH):
        os.remove(SOCKET_PATH)
    server = await asyncio.start_unix_server(handle_client, path=SOCKET_PATH)
    async with server:
        print(f"Servidor escuchando en {SOCKET_PATH}")
        await server.serve_forever()


if __name__ == "__main__":
    asyncio.run(main())
