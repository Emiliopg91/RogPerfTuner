import asyncio
from rcc.clients.websocket.base.abstract_websocket_server import steam_server  # pylint: disable=W0611,C0413

asyncio.new_event_loop().run_forever()
