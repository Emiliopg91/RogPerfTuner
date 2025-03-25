import asyncio
import time
from rcc.communications.client.dbus.linux.power_management_keyboard_brightness_control import (
    KEYBOARD_BRIGHTNESS_CONTROL,
)


async def keep_running():
    while True:
        await asyncio.sleep(3600)


async def main():
    KEYBOARD_BRIGHTNESS_CONTROL.on_brightness_change(lambda v: print(f"{v}!!!"))
    await keep_running()


asyncio.run(main())
