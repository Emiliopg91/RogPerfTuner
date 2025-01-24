# pylint: disable=C0116
from __future__ import annotations
from rcc.clients.tcp.openrgb.client.utils import Plugin
from rcc.clients.tcp.openrgb.client.network import NetworkClient
from rcc.clients.tcp.openrgb.client.plugins.common import ORGBPlugin

from rcc.clients.tcp.openrgb.client.plugins.effects import EffectsPlugin

PLUGIN_NAMES = {"OpenRGB Effects Plugin": EffectsPlugin}


def create_plugin(plugin: Plugin, comms: NetworkClient) -> ORGBPlugin:
    return PLUGIN_NAMES[plugin.name](plugin, comms)
