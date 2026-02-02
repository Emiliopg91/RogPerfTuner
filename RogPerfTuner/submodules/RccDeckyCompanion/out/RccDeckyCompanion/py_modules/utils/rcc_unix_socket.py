# pylint: disable=R0801
import decky
import asyncio
import queue
import os
import yaml
import socket

from abc import ABC
from threading import Thread, Lock
from dataclasses import dataclass, field, asdict
import uuid


@dataclass
class MessageType:
    """Data class for YAML message"""

    type: str
    name: str
    data: list[any] = field(default_factory=list)
    error: str | None = None
    id: str = field(default_factory=lambda: str(uuid.uuid4()))

    def to_yaml(self) -> str:
        """Serialize the dataclass to a YAML string"""
        return yaml.safe_dump(asdict(self), sort_keys=False)

    @staticmethod
    def from_yaml(yaml_str: str) -> "MessageType":
        """Deserialize from YAML string"""
        data = yaml.safe_load(yaml_str)
        return MessageType(**data)


class UnixSocketServer(ABC):
    """Base class for WebSocket servers"""

    SOCKET = os.path.join(decky.DECKY_PLUGIN_RUNTIME_DIR, "socket")
    INVOCATION_TIMEOUT = 3

    def __init__(self):
        super().__init__()

        self._server = None
        self._client = None

        self._message_queue = queue.Queue()
        self.lock = Lock()

        self.loop = asyncio.new_event_loop()

        Thread(
            daemon=True,
            target=lambda: self.loop.run_forever(),
        ).start()

        asyncio.run_coroutine_threadsafe(self.__start_server(), self.loop)

        Thread(
            daemon=True,
            target=self._message_sender,
        ).start()

    async def __start_server(self):
        if os.path.exists(UnixSocketServer.SOCKET):
            os.remove(UnixSocketServer.SOCKET)

        self._server = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        self._server.bind(UnixSocketServer.SOCKET)
        os.chmod(UnixSocketServer.SOCKET, 0o666)
        self._server.listen(1)
        self.running = True

        decky.logger.info(f"[SERVER] Listening on {UnixSocketServer.SOCKET}...")

        Thread(target=self._accept_loop, daemon=True).start()

    def _accept_loop(self):
        while self.running:
            client_sock, _ = self._server.accept()
            with self.lock:
                if self._client is not None:
                    decky.logger.warning("[SERVER] Closing previous client connection")
                    try:
                        self._client.close()
                    except Exception as e:
                        decky.logger.error(f"[SERVER] Error closing old client: {e}")
                    self._client = None

                self._client = client_sock
                decky.logger.info("[SERVER] Client connected")
                Thread(
                    target=self._handle_client, args=(client_sock,), daemon=True
                ).start()

    def _message_sender(self):
        while True:
            decky.logger.info("Waiting for outgoing messages...")
            message = self._message_queue.get(block=True)
            decky.logger.info(f"Preparing to send {message}")

            if self._server and self._client:
                try:
                    data = message.to_yaml().encode("utf-8")
                    header = len(data).to_bytes(4, byteorder="big")
                    self._client.sendall(header + data)
                    decky.logger.info("Sent to client")
                except Exception as e:
                    decky.logger.error(f"Failed to send message to client: {e}")
            else:
                decky.logger.info("No server/client running, message not sent.")

    def _add_message_to_queue(self, message):
        self._message_queue.put(message)

    def send_message(self, message):
        """Método público para enviar un mensaje."""
        self._add_message_to_queue(message)

    def _handle_client(self, sock: socket.socket):
        try:
            while self.running:
                header = sock.recv(4)
                if not header:
                    break
                msg_len = int.from_bytes(header, byteorder="big")
                data = sock.recv(msg_len).decode("utf-8")

                msg = MessageType.from_yaml(data)
                decky.logger.info(f"[SERVER] Received: {msg}")

                # Responder si es request
                if msg.type == "REQUEST":
                    asyncio.run_coroutine_threadsafe(
                        self._handle_message(data), self.loop
                    )
        except Exception as e:
            decky.logger.error("[SERVER] Error:", e)
        finally:
            decky.logger.info("[SERVER] Client disconnected")
            with self.lock:
                self._client = None
            sock.close()

    async def _handle_message(self, input_msg):
        decky.logger.info(f"Received message: '{input_msg}'")
        message: MessageType = None
        try:
            message = MessageType.from_yaml(input_msg)
        except Exception as e:
            message = None
            decky.logger.error(f"Error on message parsing: {e}")

        if message is not None and message.type == "REQUEST":
            if message.name == "ping":
                response = MessageType("RESPONSE", "ping", ["pong"], None, message.id)
                self.send_message(response)
            else:
                if message.name not in (
                    "get_running_games",
                    "get_apps_details",
                    "set_launch_options",
                    "get_icon",
                ):
                    message.type = "RESPONSE"
                    message.error = f"No such method '{message.name}'"
                    self.send_message(message)  # pylint: disable=E1101
                else:
                    await decky.emit(message.name, message.id, *message.data)

    def emit(self, event: str, *data: any):
        """Emit event with specified data"""
        msg = MessageType("EVENT", event, data)
        self.send_message(msg)  # pylint: disable=E1101

    def send_response(self, msg_id: str, method: str, *data: any):
        msg = MessageType("RESPONSE", method, list(data), None, msg_id)
        self.send_message(msg)  # pylint: disable=E1101

    def shutdown(self):
        self.running = False
        with self.lock:
            if self._client is not None:
                try:
                    self._client.close()
                except Exception as e:
                    decky.logger.error(f"Error closing client: {e}")
                self._client = None

            if self._server is not None:
                try:
                    self._server.close()
                except Exception as e:
                    decky.logger.error(f"Error closing server: {e}")
                self._server = None

        if os.path.exists(self.SOCKET):
            try:
                os.remove(self.SOCKET)
            except Exception as e:
                decky.logger.error(f"Error removing socket file: {e}")

        decky.logger.info("[SERVER] Stopped")
