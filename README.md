[![Release](https://github.com/Emiliopg91/RogControlCenter/actions/workflows/release.yml/badge.svg)](https://github.com/Emiliopg91/RogControlCenter/actions/workflows/release.yml)
[![Downloads](https://img.shields.io/github/downloads/Emiliopg91/RogControlCenter/total.svg)](https://github.com/Emiliopg91/RogControlCenter/releases)

# RogControlCenter
- [Features](#features)

- [Requirements](#requirements)

- [Compatibility](#compatibility)

- [Attribution](#attribution)

## Description
Are you tired of having to manually adjust operating system settings to get the most out of your Asus ROG?

Do you want breathtaking RGB effects on your laptop and peripherals? And with synchronized effects across all your RGBs?

What if I told you you can also integrate with Steam and assign which GPU to use from a graphical interface, and view performance metrics?

RogControlCenter is here to simplify your life and help you enjoy gaming on your Asus ROG laptop once and for all.

Do you want something similar for Asus Rog Ally/Ally X? Check [AllyDeckyCompanion](https://github.com/Emiliopg91/AllyDeckyCompanion/)

## Features

- [x] Selection of three performance profiles, which control:
  - CPU TDP
  - CPU Boost
  - CPU Governor
  - CPU and GPU Thermal throttling policy
  - GPU TGP
  - Fan curves
  - Power profile
  - SSD IO scheduler

- [x] RGB control for Asus ROG laptops and peripherics and support for custom AuraSync

- [x] Available RGB effects:
  - Breathing
  - Dance floor
  - Digital rain
  - Drops
  - Gaming
  - Rainbow wave
  - Spectrum cycle
  - Starry night
  - Static

- [x] Allow changing RGB brightness
  - Off
  - Low
  - Medium
  - High 

- [x] Limit battery charge threshold to improve its life-span
  - 50%
  - 75%
  - 100%

- [x] Steam integration through Decky plugin
  - Force use of dGPU or letting Linux select for you
  - Show performance metrics
  - Process CPU and IO priority

## Requirements
- asusctl 6.1.12 or later

## Compatibility
- Tested on Bazzite based on Fedora 41/42 with KDE environment
- Only tested on Laptops with Intel+Nvidia settings

## Attribution

Thanks to the following for making this plugin possible:

- [Bazzite](https://github.com/ublue-os/bazzite/) thank you because you made Linux gaming beautiful for me

- [Asusctl](https://gitlab.com/asus-linux/asusctl/) for giving the base for performance profiles.

- [OpenRGB](https://gitlab.com/CalcProgrammer1/OpenRGB/) base of the custom version for this project.

- [MangoHUD](https://github.com/flightlessmango/MangoHud/) used for showing performance metrics in-game.

- [Gamemode](https://github.com/FeralInteractive/gamemode/) to give inspiration about game processes optimizations.

- [Decky loader](https://github.com/SteamDeckHomebrew/decky-loader/) for allowing Steam integration.
