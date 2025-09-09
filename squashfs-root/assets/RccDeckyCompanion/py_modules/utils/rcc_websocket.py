# pylint: disable=R0801
import decky
import asyncio
import queue
from abc import ABC
from threading import Thread
import websockets
from websockets.exceptions import ConnectionClosedError
from dataclasses import dataclass, field
import uuid
from dataclasses_json import dataclass_json


@dataclass_json
@dataclass
class MessageType:
    """Data class for message"""

    type: str
    name: str
    data: list[any] | None = field(default_factory=list)
    error: str | None = None
    id: str = field(default_factory=lambda: str(uuid.uuid4()))


class WebsocketServer(ABC):
    """Base class for WebSocket servers"""

    HOST = "127.0.0.1"
    INVOCATION_TIMEOUT = 3

    def __init__(self, port):
        super().__init__()
        self._port = port
        self._server = None
        self._message_queue = queue.Queue()
        self._active_websockets = set()

        # Inicia el hilo para escuchar conexiones del servidor
        Thread(
            daemon=True,
            target=lambda: asyncio.new_event_loop().run_until_complete(self.__start_server()),
        ).start()
        Thread(
            daemon=True,
            target=lambda: asyncio.new_event_loop().run_until_complete(self._message_sender()),
        ).start()

    async def __start_server(self):
        """Iniciar el servidor WebSocket."""
        uri = f"ws://{self.HOST}:{self._port}"

        async def handle_client(websocket):
            self._active_websockets.add(websocket)
            decky.logger.info(f"New client connected: {websocket.remote_address}")
            try:
                await self._handler(websocket)
            finally:
                self._active_websockets.remove(websocket)
                decky.logger.info(f"Client disconnected: {websocket.remote_address}")

        try:
            self._server = await websockets.serve(handle_client, self.HOST, self._port)
            decky.logger.info(f"WebSocket server started at {uri}")
            await self._server.wait_closed()
        except Exception as e:
            # Manejo de error al iniciar el servidor
            decky.logger.error(f"Error starting WebSocket server: {e.__class__} - {e}")

    async def _message_sender(self):
        while True:
            decky.logger.info("Waiting for outgoing messages...")
            message = self._message_queue.get(block=True)  # Espera hasta que haya un mensaje
            decky.logger.info(f"Preparing to send {message}")

            if self._server:
                if self._active_websockets:
                    for websocket in list(self._active_websockets):
                        try:
                            await websocket.send(message)
                            decky.logger.info(f"Sent message: {message}")
                        except ConnectionClosedError:
                            decky.logger.info(
                                f"Failed to send message to {websocket.remote_address}, client disconnected."
                            )
                decky.logger.info(f"Sent to {len(self._active_websockets)} clients")
            else:
                decky.logger.info("No server running, message not sent.")

    def _add_message_to_queue(self, message):
        self._message_queue.put(message)

    def send_message(self, message):
        """Método público para enviar un mensaje."""
        self._add_message_to_queue(message)

    async def _handler(self, websocket):
        """Manejador de WebSocket: Gestiona la recepción de mensajes."""
        try:
            async for message in websocket:
                await self._handle_message(message, websocket)
        except ConnectionClosedError:
            pass
        finally:
            decky.logger.info(f"Client disconnected: {websocket.remote_address}")

    async def _handle_message(self, input_msg, websocket):
        decky.logger.info(f"Received message from {websocket.remote_address}: '{input_msg}'")
        message: MessageType = None
        try:
            message = MessageType.from_json(input_msg)  # pylint: disable=E1101
        except Exception as e:
            message = None
            decky.logger.error(f"Error on message parsing: {e}")

        if message is not None and message.type == "REQUEST":
            if message.name not in (
                "get_running_games",
                "get_apps_details",
                "set_launch_options",
                "get_icon",
            ):
                message.type = "RESPONSE"
                message.error = f"No such method '{message.name}'"
                self.send_message(message.to_json())  # pylint: disable=E1101
            else:
                await decky.emit(message.name, message.id, *message.data)

    def emit(self, event: str, *data: any):
        """Emit event with specified data"""
        msg = MessageType("EVENT", event, data)
        self.send_message(msg.to_json())  # pylint: disable=E1101

    def send_response(self, msg_id: str, method: str, *data: any):
        msg = MessageType("RESPONSE", method, list(data), None, msg_id)
        self.send_message(msg.to_json())  # pylint: disable=E1101

    def shutdown(self):
        self._server.close()
