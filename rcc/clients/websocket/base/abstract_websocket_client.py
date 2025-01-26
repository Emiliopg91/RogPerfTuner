from abc import ABC
import queue
from threading import Thread
from typing import Callable
import asyncio
import time
import websockets
from websockets.exceptions import ConnectionClosedError

from rcc.models.message_type import MessageType
from rcc.utils.event_bus import event_bus
from rcc.utils.logger import Logger


class WebSocketInvocationError(Exception):
    """Excepcion"""


class WebSocketTimeoutError(Exception):
    """Excepcion"""


class AbstractWebsocketClient(ABC):
    """Base class for websocket clients"""

    HOST = "127.0.0.1"
    INVOCATION_TIMEOUT = 3

    def __init__(self, port):
        super().__init__()
        self._logger = Logger()
        self._port = port
        self._current_client = None
        self._message_queue = queue.Queue()
        self.__connected = False
        self.__responses: dict[str, queue.Queue] = {}

        # Inicia el hilo para conectar al servidor
        Thread(
            daemon=True,
            name=f"{self.__class__.__name__}-main",
            target=lambda: asyncio.new_event_loop().run_until_complete(self.__connect()),
        ).start()
        Thread(
            daemon=True,
            name=f"{self.__class__.__name__}-writer",
            target=lambda: asyncio.new_event_loop().run_until_complete(self._message_sender()),
        ).start()

    @property
    def connected(self) -> bool:
        """Flag to indicate if is connected to server"""
        return self._current_client is not None

    async def __connect(self):
        """Conectar al servidor WebSocket."""
        uri = f"ws://{self.HOST}:{self._port}"
        while True:  # Intentamos reconectar indefinidamente
            try:
                async with websockets.connect(uri) as websocket:
                    self._current_client = websocket
                    self._logger.debug(f"Connected to WebSocket server at {uri}")
                    event_bus.emit(f"{self.__class__.__name__}.connected")
                    self.__connected = True
                    await self._handler(websocket)
            except Exception as e:
                self._current_client = None
                if self.__connected:
                    event_bus.emit(f"{self.__class__.__name__}.disconnected")
                self.__connected = False
                self._logger.debug(f"Error connecting to WebSocket server: {e.__class__} - {e}")
                self._logger.debug("Reconnecting in 5 seconds...")
                await asyncio.sleep(5)  # Esperar 5 segundos antes de reintentar

    async def _message_sender(self):
        while True:
            message = self._message_queue.get(block=True)  # Espera hasta que haya un mensaje

            if self._current_client:
                try:
                    await self._current_client.send(message)
                    self._logger.debug(f"Sent message: {message}")
                except ConnectionClosedError:
                    self._logger.debug(
                        f"Failed to send message to {self._current_client.remote_address}, client disconnected."
                    )
            else:
                self._logger.debug("No client connected, message not sent.")

    def _add_message_to_queue(self, message):
        self._message_queue.put(message)

    def send_message(self, message):
        """Método público para enviar un mensaje."""
        self._add_message_to_queue(message)

    async def _handler(self, websocket):
        """Manejador de WebSocket: Gestiona la recepción de mensajes."""
        self._logger.debug("Listening websocket for messages")
        try:
            async for message in websocket:
                await self._handle_message(message)
        except ConnectionClosedError:
            pass
        finally:
            self._current_client = None

    async def _handle_message(self, input_msg):
        self._logger.debug(f"Received message: '{input_msg}'")
        message: MessageType = None
        try:
            message = MessageType.from_json(input_msg)  # pylint: disable=E1101
        except Exception as e:
            message = None
            self._logger.debug(f"Error on message parsing: {e}")

        if message is not None:
            if message.type == "EVENT":
                event_bus.emit(f"{self.__class__.__name__}.{message.name}", *message.data)
            elif message.type == "RESPONSE":
                if message.id in self.__responses:
                    self._logger.debug(f"Received response for {message.id}")
                    self.__responses[message.id].put(message)
                    del self.__responses[message.id]
                else:
                    self._logger.debug(f"No request waiting for response with id {message.id}")

    def _invoke(self, method: str, *args: any, timeout: int | float = None):
        if self.connected:
            if timeout is None:
                timeout = self.INVOCATION_TIMEOUT
            message = MessageType("REQUEST", method, args)
            msg_id = message.id
            self.__responses[msg_id] = queue.Queue()

            self._logger.debug("Sending request")
            self.send_message(message.to_json())  # pylint: disable=E1101
            self._logger.debug(f"Waiting {int(timeout*1000)} ms for response")

            response = None
            t0 = time.time()
            try:
                response = self.__responses[msg_id].get(True, timeout)
                self._logger.debug(f"Response received after {int(1000*(time.time()-t0))} ms: {response}")
                if response.error is not None:
                    self._logger.debug(f"Error on method invocation: {response.error}")
                    raise WebSocketInvocationError(f"Error on method invocation: {response.error}")
                return response.data
            except queue.Empty:
                del self.__responses[msg_id]
                self._logger.debug(f"No response after {int(1000*(time.time()-t0))} ms")
                raise WebSocketTimeoutError(  # pylint: disable=W0707
                    f"No response after {int(1000*(time.time()-t0))} ms"
                )
        else:
            raise ConnectionError("No connection to server")  # pylint: disable=W0707

    def on(self, event: str, callback: Callable[..., None]):
        """Defines handler for event"""
        event_bus.on(f"{self.__class__.__name__}.{event}", callback)

    def on_connected(self, callback: Callable[..., None]):
        """Defines handler for event"""
        event_bus.on(f"{self.__class__.__name__}.connected", callback)

    def on_disconnected(self, callback: Callable[..., None]):
        """Defines handler for event"""
        event_bus.on(f"{self.__class__.__name__}.disconnected", callback)
