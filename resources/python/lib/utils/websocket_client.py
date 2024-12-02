import asyncio
import websockets
import json
import sys
import traceback
from lib.models.backend_message import BackendMessage
from .openrgb_client import OpenRgbClient


class WebSocketClient:
    def __init__(self, orgb_client: OpenRgbClient):
        self.port = int(sys.argv[1])
        self.uri = f"ws://localhost:{self.port}"
        self.token = sys.argv[2]
        self.websocket = None
        self.orgb_client = orgb_client

    async def handle_message(self): 
        async for message in self.websocket:
            msg = BackendMessage.parse(message)
            if msg.type == 'request':
                resp = BackendMessage('response', None, None, None, msg.id)
                try:
                    result = None

                    if msg.method == "available_modes":
                        result = [list(self.orgb_client.available_modes)]
                    elif msg.method == "set_mode":
                        result = [await self.orgb_client.set_mode(msg.params[0], msg.params[1], msg.params[2] if len(msg.params) == 3 else None)]
                    else:
                        print(f"Unknown method {msg.method}")
                        resp.error = f"Unknown method {msg.method}"

                    resp.params = result                        
                except Exception as e:
                    print(f"Error processing request: {traceback.format_exc()}")
                    resp.error = str(e)
                await self.websocket.send(resp.to_json())
            else:
                print(f"Invalid incomming message type {msg.type}", file=sys.stderr)

    async def connect(self):
        try:
            self.websocket = await websockets.connect(self.uri)
            await self.emit({"token": self.token})
            asyncio.create_task(self.handle_message())
        except websockets.exceptions.ConnectionClosed as e:
            print(f"Connection closed: {e}")
        except Exception as e:
            print(f"An error occurred: {e}")

    async def emit(self, message):
        if not self.websocket:
            print("Client is not connected.")
            return

        try:
            await self.websocket.send(json.dumps(message))
        except Exception as e:
            print(f"Error sending message: {e}")