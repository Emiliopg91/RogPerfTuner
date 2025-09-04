#pragma once

#include <array>
#include <map>
#include <string>
#include <string_view>

#include "../../../models/hardware/usb_identifier.hpp"

const auto ASUS_AURA_ADDRESSABLE			   = UsbIdentifier{"0b05", "1867", "ASUS Aura Addressable"};
const auto ASUS_AURA_ADDRESSABLE_1			   = UsbIdentifier{"0b05", "1872", "ASUS Aura Addressable"};
const auto ASUS_AURA_ADDRESSABLE_2			   = UsbIdentifier{"0b05", "18a3", "ASUS Aura Addressable"};
const auto ASUS_AURA_ADDRESSABLE_3			   = UsbIdentifier{"0b05", "18a5", "ASUS Aura Addressable"};
const auto ASUS_AURA_CORE					   = UsbIdentifier{"0b05", "1854", "ASUS Aura Core"};
const auto ASUS_AURA_CORE_1					   = UsbIdentifier{"0b05", "1866", "ASUS Aura Core"};
const auto ASUS_AURA_CORE_2					   = UsbIdentifier{"0b05", "1869", "ASUS Aura Core"};
const auto ASUS_AURA_MOTHERBOARD			   = UsbIdentifier{"0b05", "18f3", "ASUS Aura Motherboard"};
const auto ASUS_AURA_MOTHERBOARD_1			   = UsbIdentifier{"0b05", "1939", "ASUS Aura Motherboard"};
const auto ASUS_AURA_MOTHERBOARD_2			   = UsbIdentifier{"0b05", "19af", "ASUS Aura Motherboard"};
const auto ASUS_AURA_MOTHERBOARD_3			   = UsbIdentifier{"0b05", "1aa6", "ASUS Aura Motherboard"};
const auto ASUS_ROG_AURA_TERMINAL			   = UsbIdentifier{"0b05", "1889", "ASUS ROG AURA Terminal"};
const auto ASUS_ROG_ALLY					   = UsbIdentifier{"0b05", "1abe", "ASUS ROG Ally"};
const auto ASUS_ROG_ALLY_X					   = UsbIdentifier{"0b05", "1b4c", "ASUS ROG Ally X"};
const auto ASUS_ROG_AZOTH_24GHZ				   = UsbIdentifier{"0b05", "1a85", "ASUS ROG Azoth 24GHz"};
const auto ASUS_ROG_AZOTH_USB				   = UsbIdentifier{"0b05", "1a83", "ASUS ROG Azoth USB"};
const auto ASUS_ROG_BALTEUS					   = UsbIdentifier{"0b05", "1891", "ASUS ROG Balteus"};
const auto ASUS_ROG_BALTEUS_QI				   = UsbIdentifier{"0b05", "1890", "ASUS ROG Balteus Qi"};
const auto ASUS_ROG_CHAKRAM_WIRELESS		   = UsbIdentifier{"0b05", "18e5", "ASUS ROG Chakram Wireless"};
const auto ASUS_ROG_CLAYMORE				   = UsbIdentifier{"0b05", "184d", "ASUS ROG Claymore"};
const auto ASUS_ROG_FALCHION_WIRED			   = UsbIdentifier{"0b05", "193c", "ASUS ROG Falchion Wired"};
const auto ASUS_ROG_FALCHION_WIRELESS		   = UsbIdentifier{"0b05", "193e", "ASUS ROG Falchion Wireless"};
const auto ASUS_ROG_GLADIUS_II				   = UsbIdentifier{"0b05", "1845", "ASUS ROG Gladius II"};
const auto ASUS_ROG_GLADIUS_II_CORE			   = UsbIdentifier{"0b05", "18dd", "ASUS ROG Gladius II Core"};
const auto ASUS_ROG_GLADIUS_II_ORIGIN		   = UsbIdentifier{"0b05", "1877", "ASUS ROG Gladius II Origin"};
const auto ASUS_ROG_GLADIUS_II_ORIGIN_COD	   = UsbIdentifier{"0b05", "18b1", "ASUS ROG Gladius II Origin COD"};
const auto ASUS_ROG_GLADIUS_II_ORIGIN_PNK_LTD  = UsbIdentifier{"0b05", "18cd", "ASUS ROG Gladius II Origin PNK LTD"};
const auto ASUS_ROG_GLADIUS_II_WIRELESS		   = UsbIdentifier{"0b05", "189e", "ASUS ROG Gladius II Wireless"};
const auto ASUS_ROG_GLADIUS_II_WIRELESS_1	   = UsbIdentifier{"0b05", "18a0", "ASUS ROG Gladius II Wireless"};
const auto ASUS_ROG_GLADIUS_III				   = UsbIdentifier{"0b05", "197b", "ASUS ROG Gladius III"};
const auto ASUS_ROG_GLADIUS_III_WIRELESS_24GHZ = UsbIdentifier{"0b05", "197f", "ASUS ROG Gladius III Wireless 24Ghz"};
const auto ASUS_ROG_GLADIUS_III_WIRELESS_AIMPOINT_24GHZ =
	UsbIdentifier{"0b05", "1a72", "ASUS ROG Gladius III Wireless AimPoint 24Ghz"};
const auto ASUS_ROG_GLADIUS_III_WIRELESS_AIMPOINT_USB =
	UsbIdentifier{"0b05", "1a70", "ASUS ROG Gladius III Wireless AimPoint USB"};
const auto ASUS_ROG_GLADIUS_III_WIRELESS_BLUETOOTH =
	UsbIdentifier{"0b05", "1981", "ASUS ROG Gladius III Wireless Bluetooth"};
const auto ASUS_ROG_GLADIUS_III_WIRELESS_USB = UsbIdentifier{"0b05", "197d", "ASUS ROG Gladius III Wireless USB"};
const auto ASUS_ROG_KERIS					 = UsbIdentifier{"0b05", "195c", "ASUS ROG Keris"};
const auto ASUS_ROG_KERIS_WIRELESS_24GHZ	 = UsbIdentifier{"0b05", "1960", "ASUS ROG Keris Wireless 24Ghz"};
const auto ASUS_ROG_KERIS_WIRELESS_AIMPOINT_24GHZ =
	UsbIdentifier{"0b05", "1a68", "ASUS ROG Keris Wireless AimPoint 24Ghz"};
const auto ASUS_ROG_KERIS_WIRELESS_AIMPOINT_USB = UsbIdentifier{"0b05", "1a66", "ASUS ROG Keris Wireless AimPoint USB"};
const auto ASUS_ROG_KERIS_WIRELESS_BLUETOOTH	= UsbIdentifier{"0b05", "1962", "ASUS ROG Keris Wireless Bluetooth"};
const auto ASUS_ROG_KERIS_WIRELESS_USB			= UsbIdentifier{"0b05", "195e", "ASUS ROG Keris Wireless USB"};
const auto ASUS_ROG_PG32UQ						= UsbIdentifier{"0b05", "19b9", "ASUS ROG PG32UQ"};
const auto ASUS_ROG_PUGIO						= UsbIdentifier{"0b05", "1846", "ASUS ROG Pugio"};
const auto ASUS_ROG_PUGIO_II_WIRED				= UsbIdentifier{"0b05", "1906", "ASUS ROG Pugio II Wired"};
const auto ASUS_ROG_PUGIO_II_WIRELESS			= UsbIdentifier{"0b05", "1908", "ASUS ROG Pugio II Wireless"};
const auto ASUS_ROG_RYUO_AIO					= UsbIdentifier{"0b05", "1887", "ASUS ROG Ryuo AIO"};
const auto ASUS_ROG_SPATHA_WIRED				= UsbIdentifier{"0b05", "181c", "ASUS ROG Spatha Wired"};
const auto ASUS_ROG_SPATHA_WIRELESS				= UsbIdentifier{"0b05", "1824", "ASUS ROG Spatha Wireless"};
const auto ASUS_ROG_STRIX_EVOLVE				= UsbIdentifier{"0b05", "185b", "ASUS ROG Strix Evolve"};
const auto ASUS_ROG_STRIX_FLARE					= UsbIdentifier{"0b05", "1875", "ASUS ROG Strix Flare"};
const auto ASUS_ROG_STRIX_FLARE_COD_BLACK_OPS_4_EDITION =
	UsbIdentifier{"0b05", "18af", "ASUS ROG Strix Flare CoD Black Ops 4 Edition"};
const auto ASUS_ROG_STRIX_FLARE_II		   = UsbIdentifier{"0b05", "19fe", "ASUS ROG Strix Flare II"};
const auto ASUS_ROG_STRIX_FLARE_II_ANIMATE = UsbIdentifier{"0b05", "19fc", "ASUS ROG Strix Flare II Animate"};
const auto ASUS_ROG_STRIX_FLARE_PNK_LTD	   = UsbIdentifier{"0b05", "18cf", "ASUS ROG Strix Flare PNK LTD"};
const auto ASUS_ROG_STRIX_IMPACT		   = UsbIdentifier{"0b05", "1847", "ASUS ROG Strix Impact"};
const auto ASUS_ROG_STRIX_IMPACT_II		   = UsbIdentifier{"0b05", "18e1", "ASUS ROG Strix Impact II"};
const auto ASUS_ROG_STRIX_IMPACT_II_ELECTRO_PUNK =
	UsbIdentifier{"0b05", "1956", "ASUS ROG Strix Impact II Electro Punk"};
const auto ASUS_ROG_STRIX_IMPACT_II_GUNDAM = UsbIdentifier{"0b05", "189e", "ASUS ROG Strix Impact II Gundam"};
const auto ASUS_ROG_STRIX_IMPACT_II_MOONLIGHT_WHITE =
	UsbIdentifier{"0b05", "19d2", "ASUS ROG Strix Impact II Moonlight White"};
const auto ASUS_ROG_STRIX_IMPACT_II_WIRELESS_24_GHZ =
	UsbIdentifier{"0b05", "1949", "ASUS ROG Strix Impact II Wireless 24 Ghz"};
const auto ASUS_ROG_STRIX_IMPACT_II_WIRELESS_USB =
	UsbIdentifier{"0b05", "1947", "ASUS ROG Strix Impact II Wireless USB"};
const auto ASUS_ROG_STRIX_IMPACT_III = UsbIdentifier{"0b05", "1a88", "ASUS ROG Strix Impact III"};
const auto ASUS_ROG_STRIX_LC		 = UsbIdentifier{"0b05", "879e", "ASUS ROG Strix LC"};
const auto ASUS_ROG_STRIX_SCAR_15	 = UsbIdentifier{"0b05", "19b6", "ASUS ROG Strix SCAR 15"};
const auto ASUS_ROG_STRIX_SCAR_17	 = UsbIdentifier{"0b05", "1866", "ASUS ROG Strix SCAR 17"};
const auto ASUS_ROG_STRIX_SCOPE		 = UsbIdentifier{"0b05", "18f8", "ASUS ROG Strix Scope"};
const auto ASUS_ROG_STRIX_SCOPE_II	 = UsbIdentifier{"0b05", "1ab3", "ASUS ROG Strix Scope II"};
const auto ASUS_ROG_STRIX_SCOPE_II_96_RX_WIRELESS_USB =
	UsbIdentifier{"0b05", "1b78", "ASUS ROG Strix Scope II 96 RX Wireless USB"};
const auto ASUS_ROG_STRIX_SCOPE_II_96_WIRELESS_USB =
	UsbIdentifier{"0b05", "1aae", "ASUS ROG Strix Scope II 96 Wireless USB"};
const auto ASUS_ROG_STRIX_SCOPE_II_RX = UsbIdentifier{"0b05", "1ab5", "ASUS ROG Strix Scope II RX"};
const auto ASUS_ROG_STRIX_SCOPE_NX_WIRELESS_DELUXE_24GHZ =
	UsbIdentifier{"0b05", "19f8", "ASUS ROG Strix Scope NX Wireless Deluxe 24GHz"};
const auto ASUS_ROG_STRIX_SCOPE_NX_WIRELESS_DELUXE_USB =
	UsbIdentifier{"0b05", "19f6", "ASUS ROG Strix Scope NX Wireless Deluxe USB"};
const auto ASUS_ROG_STRIX_SCOPE_RX = UsbIdentifier{"0b05", "1951", "ASUS ROG Strix Scope RX"};
const auto ASUS_ROG_STRIX_SCOPE_RX_EVA02_EDITION =
	UsbIdentifier{"0b05", "1b12", "ASUS ROG Strix Scope RX EVA02 Edition"};
const auto ASUS_ROG_STRIX_SCOPE_RX_TKL_WIRELESS_DELUXE =
	UsbIdentifier{"0b05", "1a05", "ASUS ROG Strix Scope RX TKL Wireless Deluxe"};
const auto ASUS_ROG_STRIX_SCOPE_TKL			= UsbIdentifier{"0b05", "190c", "ASUS ROG Strix Scope TKL"};
const auto ASUS_ROG_STRIX_SCOPE_TKL_PNK_LTD = UsbIdentifier{"0b05", "1954", "ASUS ROG Strix Scope TKL PNK LTD"};
const auto ASUS_ROG_STRIX_XG279Q			= UsbIdentifier{"0b05", "1919", "ASUS ROG Strix XG279Q"};
const auto ASUS_ROG_STRIX_XG27AQ			= UsbIdentifier{"0b05", "198c", "ASUS ROG Strix XG27AQ"};
const auto ASUS_ROG_STRIX_XG27AQM			= UsbIdentifier{"0b05", "19bb", "ASUS ROG Strix XG27AQM"};
const auto ASUS_ROG_STRIX_XG27W				= UsbIdentifier{"0b05", "1933", "ASUS ROG Strix XG27W"};
const auto ASUS_ROG_STRIX_XG32VC			= UsbIdentifier{"0b05", "1968", "ASUS ROG Strix XG32VC"};
const auto ASUS_ROG_THRONE					= UsbIdentifier{"0b05", "18d9", "ASUS ROG Throne"};
const auto ASUS_ROG_THRONE_QI				= UsbIdentifier{"0b05", "18c5", "ASUS ROG Throne QI"};
const auto ASUS_ROG_THRONE_QI_GUNDAM		= UsbIdentifier{"0b05", "1994", "ASUS ROG Throne QI GUNDAM"};
const auto ASUS_SAGARIS_GK1100				= UsbIdentifier{"0b05", "1835", "ASUS Sagaris GK1100"};
const auto ASUS_TUF_GAMING_K1				= UsbIdentifier{"0b05", "1945", "ASUS TUF Gaming K1"};
const auto ASUS_TUF_GAMING_K3				= UsbIdentifier{"0b05", "194b", "ASUS TUF Gaming K3"};
const auto ASUS_TUF_GAMING_K5				= UsbIdentifier{"0b05", "1899", "ASUS TUF Gaming K5"};
const auto ASUS_TUF_GAMING_K7				= UsbIdentifier{"0b05", "18aa", "ASUS TUF Gaming K7"};
const auto ASUS_TUF_GAMING_M3				= UsbIdentifier{"0b05", "1910", "ASUS TUF Gaming M3"};
const auto ASUS_TUF_GAMING_M3_GEN_II		= UsbIdentifier{"0b05", "1a9b", "ASUS TUF Gaming M3 Gen II"};
const auto ASUS_TUF_GAMING_M5				= UsbIdentifier{"0b05", "1898", "ASUS TUF Gaming M5"};
const auto ASUS_ROG_CHAKRAM_CORE			= UsbIdentifier{"0b05", "1958", "Asus ROG Chakram Core"};
const auto ASUS_ROG_CHAKRAM_WIRED			= UsbIdentifier{"0b05", "18e3", "Asus ROG Chakram Wired"};
const auto ASUS_ROG_CHAKRAM_X_24GHZ			= UsbIdentifier{"0b05", "1a1a", "Asus ROG Chakram X 24GHz"};
const auto ASUS_ROG_CHAKRAM_X_USB			= UsbIdentifier{"0b05", "1a18", "Asus ROG Chakram X USB"};
const auto ASUS_ROG_SPATHA_X_24GHZ			= UsbIdentifier{"0b05", "1979", "Asus ROG Spatha X 24GHz"};
const auto ASUS_ROG_SPATHA_X_DOCK			= UsbIdentifier{"0b05", "1979", "Asus ROG Spatha X Dock"};
const auto ASUS_ROG_SPATHA_X_USB			= UsbIdentifier{"0b05", "1977", "Asus ROG Spatha X USB"};

using CompatibleDeviceArray					  = std::array<UsbIdentifier, 100>;
const CompatibleDeviceArray compatibleDevices = {
	ASUS_AURA_ADDRESSABLE,
	ASUS_AURA_ADDRESSABLE_1,
	ASUS_AURA_ADDRESSABLE_2,
	ASUS_AURA_ADDRESSABLE_3,
	ASUS_AURA_CORE,
	ASUS_AURA_CORE_1,
	ASUS_AURA_CORE_2,
	ASUS_AURA_MOTHERBOARD,
	ASUS_AURA_MOTHERBOARD_1,
	ASUS_AURA_MOTHERBOARD_2,
	ASUS_AURA_MOTHERBOARD_3,
	ASUS_ROG_AURA_TERMINAL,
	ASUS_ROG_ALLY,
	ASUS_ROG_ALLY_X,
	ASUS_ROG_AZOTH_24GHZ,
	ASUS_ROG_AZOTH_USB,
	ASUS_ROG_BALTEUS,
	ASUS_ROG_BALTEUS_QI,
	ASUS_ROG_CHAKRAM_WIRELESS,
	ASUS_ROG_CLAYMORE,
	ASUS_ROG_FALCHION_WIRED,
	ASUS_ROG_FALCHION_WIRELESS,
	ASUS_ROG_GLADIUS_II,
	ASUS_ROG_GLADIUS_II_CORE,
	ASUS_ROG_GLADIUS_II_ORIGIN,
	ASUS_ROG_GLADIUS_II_ORIGIN_COD,
	ASUS_ROG_GLADIUS_II_ORIGIN_PNK_LTD,
	ASUS_ROG_GLADIUS_II_WIRELESS,
	ASUS_ROG_GLADIUS_II_WIRELESS_1,
	ASUS_ROG_GLADIUS_III,
	ASUS_ROG_GLADIUS_III_WIRELESS_24GHZ,
	ASUS_ROG_GLADIUS_III_WIRELESS_AIMPOINT_24GHZ,
	ASUS_ROG_GLADIUS_III_WIRELESS_AIMPOINT_USB,
	ASUS_ROG_GLADIUS_III_WIRELESS_BLUETOOTH,
	ASUS_ROG_GLADIUS_III_WIRELESS_USB,
	ASUS_ROG_KERIS,
	ASUS_ROG_KERIS_WIRELESS_24GHZ,
	ASUS_ROG_KERIS_WIRELESS_AIMPOINT_24GHZ,
	ASUS_ROG_KERIS_WIRELESS_AIMPOINT_USB,
	ASUS_ROG_KERIS_WIRELESS_BLUETOOTH,
	ASUS_ROG_KERIS_WIRELESS_USB,
	ASUS_ROG_PG32UQ,
	ASUS_ROG_PUGIO,
	ASUS_ROG_PUGIO_II_WIRED,
	ASUS_ROG_PUGIO_II_WIRELESS,
	ASUS_ROG_RYUO_AIO,
	ASUS_ROG_SPATHA_WIRED,
	ASUS_ROG_SPATHA_WIRELESS,
	ASUS_ROG_STRIX_EVOLVE,
	ASUS_ROG_STRIX_FLARE,
	ASUS_ROG_STRIX_FLARE_COD_BLACK_OPS_4_EDITION,
	ASUS_ROG_STRIX_FLARE_II,
	ASUS_ROG_STRIX_FLARE_II_ANIMATE,
	ASUS_ROG_STRIX_FLARE_PNK_LTD,
	ASUS_ROG_STRIX_IMPACT,
	ASUS_ROG_STRIX_IMPACT_II,
	ASUS_ROG_STRIX_IMPACT_II_ELECTRO_PUNK,
	ASUS_ROG_STRIX_IMPACT_II_GUNDAM,
	ASUS_ROG_STRIX_IMPACT_II_MOONLIGHT_WHITE,
	ASUS_ROG_STRIX_IMPACT_II_WIRELESS_24_GHZ,
	ASUS_ROG_STRIX_IMPACT_II_WIRELESS_USB,
	ASUS_ROG_STRIX_IMPACT_III,
	ASUS_ROG_STRIX_LC,
	ASUS_ROG_STRIX_SCAR_15,
	ASUS_ROG_STRIX_SCAR_17,
	ASUS_ROG_STRIX_SCOPE,
	ASUS_ROG_STRIX_SCOPE_II,
	ASUS_ROG_STRIX_SCOPE_II_96_RX_WIRELESS_USB,
	ASUS_ROG_STRIX_SCOPE_II_96_WIRELESS_USB,
	ASUS_ROG_STRIX_SCOPE_II_RX,
	ASUS_ROG_STRIX_SCOPE_NX_WIRELESS_DELUXE_24GHZ,
	ASUS_ROG_STRIX_SCOPE_NX_WIRELESS_DELUXE_USB,
	ASUS_ROG_STRIX_SCOPE_RX,
	ASUS_ROG_STRIX_SCOPE_RX_EVA02_EDITION,
	ASUS_ROG_STRIX_SCOPE_RX_TKL_WIRELESS_DELUXE,
	ASUS_ROG_STRIX_SCOPE_TKL,
	ASUS_ROG_STRIX_SCOPE_TKL_PNK_LTD,
	ASUS_ROG_STRIX_XG279Q,
	ASUS_ROG_STRIX_XG27AQ,
	ASUS_ROG_STRIX_XG27AQM,
	ASUS_ROG_STRIX_XG27W,
	ASUS_ROG_STRIX_XG32VC,
	ASUS_ROG_THRONE,
	ASUS_ROG_THRONE_QI,
	ASUS_ROG_THRONE_QI_GUNDAM,
	ASUS_SAGARIS_GK1100,
	ASUS_TUF_GAMING_K1,
	ASUS_TUF_GAMING_K3,
	ASUS_TUF_GAMING_K5,
	ASUS_TUF_GAMING_K7,
	ASUS_TUF_GAMING_M3,
	ASUS_TUF_GAMING_M3_GEN_II,
	ASUS_TUF_GAMING_M5,
	ASUS_ROG_CHAKRAM_CORE,
	ASUS_ROG_CHAKRAM_WIRED,
	ASUS_ROG_CHAKRAM_X_24GHZ,
	ASUS_ROG_CHAKRAM_X_USB,
	ASUS_ROG_SPATHA_X_24GHZ,
	ASUS_ROG_SPATHA_X_DOCK,
	ASUS_ROG_SPATHA_X_USB,
};

using CompatibleDeviceNameMap						  = std::map<std::string, std::string>;
const CompatibleDeviceNameMap compatibleDeviceNameMap = {
	{std::string(ASUS_AURA_ADDRESSABLE.id_vendor) + ":" + std::string(ASUS_AURA_ADDRESSABLE.id_product),
	 std::string(ASUS_AURA_ADDRESSABLE.name)},
	{std::string(ASUS_AURA_ADDRESSABLE_1.id_vendor) + ":" + std::string(ASUS_AURA_ADDRESSABLE_1.id_product),
	 std::string(ASUS_AURA_ADDRESSABLE_1.name)},
	{std::string(ASUS_AURA_ADDRESSABLE_2.id_vendor) + ":" + std::string(ASUS_AURA_ADDRESSABLE_2.id_product),
	 std::string(ASUS_AURA_ADDRESSABLE_2.name)},
	{std::string(ASUS_AURA_ADDRESSABLE_3.id_vendor) + ":" + std::string(ASUS_AURA_ADDRESSABLE_3.id_product),
	 std::string(ASUS_AURA_ADDRESSABLE_3.name)},
	{std::string(ASUS_AURA_CORE.id_vendor) + ":" + std::string(ASUS_AURA_CORE.id_product),
	 std::string(ASUS_AURA_CORE.name)},
	{std::string(ASUS_AURA_CORE_1.id_vendor) + ":" + std::string(ASUS_AURA_CORE_1.id_product),
	 std::string(ASUS_AURA_CORE_1.name)},
	{std::string(ASUS_AURA_CORE_2.id_vendor) + ":" + std::string(ASUS_AURA_CORE_2.id_product),
	 std::string(ASUS_AURA_CORE_2.name)},
	{std::string(ASUS_AURA_MOTHERBOARD.id_vendor) + ":" + std::string(ASUS_AURA_MOTHERBOARD.id_product),
	 std::string(ASUS_AURA_MOTHERBOARD.name)},
	{std::string(ASUS_AURA_MOTHERBOARD_1.id_vendor) + ":" + std::string(ASUS_AURA_MOTHERBOARD_1.id_product),
	 std::string(ASUS_AURA_MOTHERBOARD_1.name)},
	{std::string(ASUS_AURA_MOTHERBOARD_2.id_vendor) + ":" + std::string(ASUS_AURA_MOTHERBOARD_2.id_product),
	 std::string(ASUS_AURA_MOTHERBOARD_2.name)},
	{std::string(ASUS_AURA_MOTHERBOARD_3.id_vendor) + ":" + std::string(ASUS_AURA_MOTHERBOARD_3.id_product),
	 std::string(ASUS_AURA_MOTHERBOARD_3.name)},
	{std::string(ASUS_ROG_AURA_TERMINAL.id_vendor) + ":" + std::string(ASUS_ROG_AURA_TERMINAL.id_product),
	 std::string(ASUS_ROG_AURA_TERMINAL.name)},
	{std::string(ASUS_ROG_ALLY.id_vendor) + ":" + std::string(ASUS_ROG_ALLY.id_product),
	 std::string(ASUS_ROG_ALLY.name)},
	{std::string(ASUS_ROG_ALLY_X.id_vendor) + ":" + std::string(ASUS_ROG_ALLY_X.id_product),
	 std::string(ASUS_ROG_ALLY_X.name)},
	{std::string(ASUS_ROG_AZOTH_24GHZ.id_vendor) + ":" + std::string(ASUS_ROG_AZOTH_24GHZ.id_product),
	 std::string(ASUS_ROG_AZOTH_24GHZ.name)},
	{std::string(ASUS_ROG_AZOTH_USB.id_vendor) + ":" + std::string(ASUS_ROG_AZOTH_USB.id_product),
	 std::string(ASUS_ROG_AZOTH_USB.name)},
	{std::string(ASUS_ROG_BALTEUS.id_vendor) + ":" + std::string(ASUS_ROG_BALTEUS.id_product),
	 std::string(ASUS_ROG_BALTEUS.name)},
	{std::string(ASUS_ROG_BALTEUS_QI.id_vendor) + ":" + std::string(ASUS_ROG_BALTEUS_QI.id_product),
	 std::string(ASUS_ROG_BALTEUS_QI.name)},
	{std::string(ASUS_ROG_CHAKRAM_WIRELESS.id_vendor) + ":" + std::string(ASUS_ROG_CHAKRAM_WIRELESS.id_product),
	 std::string(ASUS_ROG_CHAKRAM_WIRELESS.name)},
	{std::string(ASUS_ROG_CLAYMORE.id_vendor) + ":" + std::string(ASUS_ROG_CLAYMORE.id_product),
	 std::string(ASUS_ROG_CLAYMORE.name)},
	{std::string(ASUS_ROG_FALCHION_WIRED.id_vendor) + ":" + std::string(ASUS_ROG_FALCHION_WIRED.id_product),
	 std::string(ASUS_ROG_FALCHION_WIRED.name)},
	{std::string(ASUS_ROG_FALCHION_WIRELESS.id_vendor) + ":" + std::string(ASUS_ROG_FALCHION_WIRELESS.id_product),
	 std::string(ASUS_ROG_FALCHION_WIRELESS.name)},
	{std::string(ASUS_ROG_GLADIUS_II.id_vendor) + ":" + std::string(ASUS_ROG_GLADIUS_II.id_product),
	 std::string(ASUS_ROG_GLADIUS_II.name)},
	{std::string(ASUS_ROG_GLADIUS_II_CORE.id_vendor) + ":" + std::string(ASUS_ROG_GLADIUS_II_CORE.id_product),
	 std::string(ASUS_ROG_GLADIUS_II_CORE.name)},
	{std::string(ASUS_ROG_GLADIUS_II_ORIGIN.id_vendor) + ":" + std::string(ASUS_ROG_GLADIUS_II_ORIGIN.id_product),
	 std::string(ASUS_ROG_GLADIUS_II_ORIGIN.name)},
	{std::string(ASUS_ROG_GLADIUS_II_ORIGIN_COD.id_vendor) + ":" +
		 std::string(ASUS_ROG_GLADIUS_II_ORIGIN_COD.id_product),
	 std::string(ASUS_ROG_GLADIUS_II_ORIGIN_COD.name)},
	{std::string(ASUS_ROG_GLADIUS_II_ORIGIN_PNK_LTD.id_vendor) + ":" +
		 std::string(ASUS_ROG_GLADIUS_II_ORIGIN_PNK_LTD.id_product),
	 std::string(ASUS_ROG_GLADIUS_II_ORIGIN_PNK_LTD.name)},
	{std::string(ASUS_ROG_GLADIUS_II_WIRELESS.id_vendor) + ":" + std::string(ASUS_ROG_GLADIUS_II_WIRELESS.id_product),
	 std::string(ASUS_ROG_GLADIUS_II_WIRELESS.name)},
	{std::string(ASUS_ROG_GLADIUS_II_WIRELESS_1.id_vendor) + ":" +
		 std::string(ASUS_ROG_GLADIUS_II_WIRELESS_1.id_product),
	 std::string(ASUS_ROG_GLADIUS_II_WIRELESS_1.name)},
	{std::string(ASUS_ROG_GLADIUS_III.id_vendor) + ":" + std::string(ASUS_ROG_GLADIUS_III.id_product),
	 std::string(ASUS_ROG_GLADIUS_III.name)},
	{std::string(ASUS_ROG_GLADIUS_III_WIRELESS_24GHZ.id_vendor) + ":" +
		 std::string(ASUS_ROG_GLADIUS_III_WIRELESS_24GHZ.id_product),
	 std::string(ASUS_ROG_GLADIUS_III_WIRELESS_24GHZ.name)},
	{std::string(ASUS_ROG_GLADIUS_III_WIRELESS_AIMPOINT_24GHZ.id_vendor) + ":" +
		 std::string(ASUS_ROG_GLADIUS_III_WIRELESS_AIMPOINT_24GHZ.id_product),
	 std::string(ASUS_ROG_GLADIUS_III_WIRELESS_AIMPOINT_24GHZ.name)},
	{std::string(ASUS_ROG_GLADIUS_III_WIRELESS_AIMPOINT_USB.id_vendor) + ":" +
		 std::string(ASUS_ROG_GLADIUS_III_WIRELESS_AIMPOINT_USB.id_product),
	 std::string(ASUS_ROG_GLADIUS_III_WIRELESS_AIMPOINT_USB.name)},
	{std::string(ASUS_ROG_GLADIUS_III_WIRELESS_BLUETOOTH.id_vendor) + ":" +
		 std::string(ASUS_ROG_GLADIUS_III_WIRELESS_BLUETOOTH.id_product),
	 std::string(ASUS_ROG_GLADIUS_III_WIRELESS_BLUETOOTH.name)},
	{std::string(ASUS_ROG_GLADIUS_III_WIRELESS_USB.id_vendor) + ":" +
		 std::string(ASUS_ROG_GLADIUS_III_WIRELESS_USB.id_product),
	 std::string(ASUS_ROG_GLADIUS_III_WIRELESS_USB.name)},
	{std::string(ASUS_ROG_KERIS.id_vendor) + ":" + std::string(ASUS_ROG_KERIS.id_product),
	 std::string(ASUS_ROG_KERIS.name)},
	{std::string(ASUS_ROG_KERIS_WIRELESS_24GHZ.id_vendor) + ":" + std::string(ASUS_ROG_KERIS_WIRELESS_24GHZ.id_product),
	 std::string(ASUS_ROG_KERIS_WIRELESS_24GHZ.name)},
	{std::string(ASUS_ROG_KERIS_WIRELESS_AIMPOINT_24GHZ.id_vendor) + ":" +
		 std::string(ASUS_ROG_KERIS_WIRELESS_AIMPOINT_24GHZ.id_product),
	 std::string(ASUS_ROG_KERIS_WIRELESS_AIMPOINT_24GHZ.name)},
	{std::string(ASUS_ROG_KERIS_WIRELESS_AIMPOINT_USB.id_vendor) + ":" +
		 std::string(ASUS_ROG_KERIS_WIRELESS_AIMPOINT_USB.id_product),
	 std::string(ASUS_ROG_KERIS_WIRELESS_AIMPOINT_USB.name)},
	{std::string(ASUS_ROG_KERIS_WIRELESS_BLUETOOTH.id_vendor) + ":" +
		 std::string(ASUS_ROG_KERIS_WIRELESS_BLUETOOTH.id_product),
	 std::string(ASUS_ROG_KERIS_WIRELESS_BLUETOOTH.name)},
	{std::string(ASUS_ROG_KERIS_WIRELESS_USB.id_vendor) + ":" + std::string(ASUS_ROG_KERIS_WIRELESS_USB.id_product),
	 std::string(ASUS_ROG_KERIS_WIRELESS_USB.name)},
	{std::string(ASUS_ROG_PG32UQ.id_vendor) + ":" + std::string(ASUS_ROG_PG32UQ.id_product),
	 std::string(ASUS_ROG_PG32UQ.name)},
	{std::string(ASUS_ROG_PUGIO.id_vendor) + ":" + std::string(ASUS_ROG_PUGIO.id_product),
	 std::string(ASUS_ROG_PUGIO.name)},
	{std::string(ASUS_ROG_PUGIO_II_WIRED.id_vendor) + ":" + std::string(ASUS_ROG_PUGIO_II_WIRED.id_product),
	 std::string(ASUS_ROG_PUGIO_II_WIRED.name)},
	{std::string(ASUS_ROG_PUGIO_II_WIRELESS.id_vendor) + ":" + std::string(ASUS_ROG_PUGIO_II_WIRELESS.id_product),
	 std::string(ASUS_ROG_PUGIO_II_WIRELESS.name)},
	{std::string(ASUS_ROG_RYUO_AIO.id_vendor) + ":" + std::string(ASUS_ROG_RYUO_AIO.id_product),
	 std::string(ASUS_ROG_RYUO_AIO.name)},
	{std::string(ASUS_ROG_SPATHA_WIRED.id_vendor) + ":" + std::string(ASUS_ROG_SPATHA_WIRED.id_product),
	 std::string(ASUS_ROG_SPATHA_WIRED.name)},
	{std::string(ASUS_ROG_SPATHA_WIRELESS.id_vendor) + ":" + std::string(ASUS_ROG_SPATHA_WIRELESS.id_product),
	 std::string(ASUS_ROG_SPATHA_WIRELESS.name)},
	{std::string(ASUS_ROG_STRIX_EVOLVE.id_vendor) + ":" + std::string(ASUS_ROG_STRIX_EVOLVE.id_product),
	 std::string(ASUS_ROG_STRIX_EVOLVE.name)},
	{std::string(ASUS_ROG_STRIX_FLARE.id_vendor) + ":" + std::string(ASUS_ROG_STRIX_FLARE.id_product),
	 std::string(ASUS_ROG_STRIX_FLARE.name)},
	{std::string(ASUS_ROG_STRIX_FLARE_COD_BLACK_OPS_4_EDITION.id_vendor) + ":" +
		 std::string(ASUS_ROG_STRIX_FLARE_COD_BLACK_OPS_4_EDITION.id_product),
	 std::string(ASUS_ROG_STRIX_FLARE_COD_BLACK_OPS_4_EDITION.name)},
	{std::string(ASUS_ROG_STRIX_FLARE_II.id_vendor) + ":" + std::string(ASUS_ROG_STRIX_FLARE_II.id_product),
	 std::string(ASUS_ROG_STRIX_FLARE_II.name)},
	{std::string(ASUS_ROG_STRIX_FLARE_II_ANIMATE.id_vendor) + ":" +
		 std::string(ASUS_ROG_STRIX_FLARE_II_ANIMATE.id_product),
	 std::string(ASUS_ROG_STRIX_FLARE_II_ANIMATE.name)},
	{std::string(ASUS_ROG_STRIX_FLARE_PNK_LTD.id_vendor) + ":" + std::string(ASUS_ROG_STRIX_FLARE_PNK_LTD.id_product),
	 std::string(ASUS_ROG_STRIX_FLARE_PNK_LTD.name)},
	{std::string(ASUS_ROG_STRIX_IMPACT.id_vendor) + ":" + std::string(ASUS_ROG_STRIX_IMPACT.id_product),
	 std::string(ASUS_ROG_STRIX_IMPACT.name)},
	{std::string(ASUS_ROG_STRIX_IMPACT_II.id_vendor) + ":" + std::string(ASUS_ROG_STRIX_IMPACT_II.id_product),
	 std::string(ASUS_ROG_STRIX_IMPACT_II.name)},
	{std::string(ASUS_ROG_STRIX_IMPACT_II_ELECTRO_PUNK.id_vendor) + ":" +
		 std::string(ASUS_ROG_STRIX_IMPACT_II_ELECTRO_PUNK.id_product),
	 std::string(ASUS_ROG_STRIX_IMPACT_II_ELECTRO_PUNK.name)},
	{std::string(ASUS_ROG_STRIX_IMPACT_II_GUNDAM.id_vendor) + ":" +
		 std::string(ASUS_ROG_STRIX_IMPACT_II_GUNDAM.id_product),
	 std::string(ASUS_ROG_STRIX_IMPACT_II_GUNDAM.name)},
	{std::string(ASUS_ROG_STRIX_IMPACT_II_MOONLIGHT_WHITE.id_vendor) + ":" +
		 std::string(ASUS_ROG_STRIX_IMPACT_II_MOONLIGHT_WHITE.id_product),
	 std::string(ASUS_ROG_STRIX_IMPACT_II_MOONLIGHT_WHITE.name)},
	{std::string(ASUS_ROG_STRIX_IMPACT_II_WIRELESS_24_GHZ.id_vendor) + ":" +
		 std::string(ASUS_ROG_STRIX_IMPACT_II_WIRELESS_24_GHZ.id_product),
	 std::string(ASUS_ROG_STRIX_IMPACT_II_WIRELESS_24_GHZ.name)},
	{std::string(ASUS_ROG_STRIX_IMPACT_II_WIRELESS_USB.id_vendor) + ":" +
		 std::string(ASUS_ROG_STRIX_IMPACT_II_WIRELESS_USB.id_product),
	 std::string(ASUS_ROG_STRIX_IMPACT_II_WIRELESS_USB.name)},
	{std::string(ASUS_ROG_STRIX_IMPACT_III.id_vendor) + ":" + std::string(ASUS_ROG_STRIX_IMPACT_III.id_product),
	 std::string(ASUS_ROG_STRIX_IMPACT_III.name)},
	{std::string(ASUS_ROG_STRIX_LC.id_vendor) + ":" + std::string(ASUS_ROG_STRIX_LC.id_product),
	 std::string(ASUS_ROG_STRIX_LC.name)},
	{std::string(ASUS_ROG_STRIX_SCAR_15.id_vendor) + ":" + std::string(ASUS_ROG_STRIX_SCAR_15.id_product),
	 std::string(ASUS_ROG_STRIX_SCAR_15.name)},
	{std::string(ASUS_ROG_STRIX_SCAR_17.id_vendor) + ":" + std::string(ASUS_ROG_STRIX_SCAR_17.id_product),
	 std::string(ASUS_ROG_STRIX_SCAR_17.name)},
	{std::string(ASUS_ROG_STRIX_SCOPE.id_vendor) + ":" + std::string(ASUS_ROG_STRIX_SCOPE.id_product),
	 std::string(ASUS_ROG_STRIX_SCOPE.name)},
	{std::string(ASUS_ROG_STRIX_SCOPE_II.id_vendor) + ":" + std::string(ASUS_ROG_STRIX_SCOPE_II.id_product),
	 std::string(ASUS_ROG_STRIX_SCOPE_II.name)},
	{std::string(ASUS_ROG_STRIX_SCOPE_II_96_RX_WIRELESS_USB.id_vendor) + ":" +
		 std::string(ASUS_ROG_STRIX_SCOPE_II_96_RX_WIRELESS_USB.id_product),
	 std::string(ASUS_ROG_STRIX_SCOPE_II_96_RX_WIRELESS_USB.name)},
	{std::string(ASUS_ROG_STRIX_SCOPE_II_96_WIRELESS_USB.id_vendor) + ":" +
		 std::string(ASUS_ROG_STRIX_SCOPE_II_96_WIRELESS_USB.id_product),
	 std::string(ASUS_ROG_STRIX_SCOPE_II_96_WIRELESS_USB.name)},
	{std::string(ASUS_ROG_STRIX_SCOPE_II_RX.id_vendor) + ":" + std::string(ASUS_ROG_STRIX_SCOPE_II_RX.id_product),
	 std::string(ASUS_ROG_STRIX_SCOPE_II_RX.name)},
	{std::string(ASUS_ROG_STRIX_SCOPE_NX_WIRELESS_DELUXE_24GHZ.id_vendor) + ":" +
		 std::string(ASUS_ROG_STRIX_SCOPE_NX_WIRELESS_DELUXE_24GHZ.id_product),
	 std::string(ASUS_ROG_STRIX_SCOPE_NX_WIRELESS_DELUXE_24GHZ.name)},
	{std::string(ASUS_ROG_STRIX_SCOPE_NX_WIRELESS_DELUXE_USB.id_vendor) + ":" +
		 std::string(ASUS_ROG_STRIX_SCOPE_NX_WIRELESS_DELUXE_USB.id_product),
	 std::string(ASUS_ROG_STRIX_SCOPE_NX_WIRELESS_DELUXE_USB.name)},
	{std::string(ASUS_ROG_STRIX_SCOPE_RX.id_vendor) + ":" + std::string(ASUS_ROG_STRIX_SCOPE_RX.id_product),
	 std::string(ASUS_ROG_STRIX_SCOPE_RX.name)},
	{std::string(ASUS_ROG_STRIX_SCOPE_RX_EVA02_EDITION.id_vendor) + ":" +
		 std::string(ASUS_ROG_STRIX_SCOPE_RX_EVA02_EDITION.id_product),
	 std::string(ASUS_ROG_STRIX_SCOPE_RX_EVA02_EDITION.name)},
	{std::string(ASUS_ROG_STRIX_SCOPE_RX_TKL_WIRELESS_DELUXE.id_vendor) + ":" +
		 std::string(ASUS_ROG_STRIX_SCOPE_RX_TKL_WIRELESS_DELUXE.id_product),
	 std::string(ASUS_ROG_STRIX_SCOPE_RX_TKL_WIRELESS_DELUXE.name)},
	{std::string(ASUS_ROG_STRIX_SCOPE_TKL.id_vendor) + ":" + std::string(ASUS_ROG_STRIX_SCOPE_TKL.id_product),
	 std::string(ASUS_ROG_STRIX_SCOPE_TKL.name)},
	{std::string(ASUS_ROG_STRIX_SCOPE_TKL_PNK_LTD.id_vendor) + ":" +
		 std::string(ASUS_ROG_STRIX_SCOPE_TKL_PNK_LTD.id_product),
	 std::string(ASUS_ROG_STRIX_SCOPE_TKL_PNK_LTD.name)},
	{std::string(ASUS_ROG_STRIX_XG279Q.id_vendor) + ":" + std::string(ASUS_ROG_STRIX_XG279Q.id_product),
	 std::string(ASUS_ROG_STRIX_XG279Q.name)},
	{std::string(ASUS_ROG_STRIX_XG27AQ.id_vendor) + ":" + std::string(ASUS_ROG_STRIX_XG27AQ.id_product),
	 std::string(ASUS_ROG_STRIX_XG27AQ.name)},
	{std::string(ASUS_ROG_STRIX_XG27AQM.id_vendor) + ":" + std::string(ASUS_ROG_STRIX_XG27AQM.id_product),
	 std::string(ASUS_ROG_STRIX_XG27AQM.name)},
	{std::string(ASUS_ROG_STRIX_XG27W.id_vendor) + ":" + std::string(ASUS_ROG_STRIX_XG27W.id_product),
	 std::string(ASUS_ROG_STRIX_XG27W.name)},
	{std::string(ASUS_ROG_STRIX_XG32VC.id_vendor) + ":" + std::string(ASUS_ROG_STRIX_XG32VC.id_product),
	 std::string(ASUS_ROG_STRIX_XG32VC.name)},
	{std::string(ASUS_ROG_THRONE.id_vendor) + ":" + std::string(ASUS_ROG_THRONE.id_product),
	 std::string(ASUS_ROG_THRONE.name)},
	{std::string(ASUS_ROG_THRONE_QI.id_vendor) + ":" + std::string(ASUS_ROG_THRONE_QI.id_product),
	 std::string(ASUS_ROG_THRONE_QI.name)},
	{std::string(ASUS_ROG_THRONE_QI_GUNDAM.id_vendor) + ":" + std::string(ASUS_ROG_THRONE_QI_GUNDAM.id_product),
	 std::string(ASUS_ROG_THRONE_QI_GUNDAM.name)},
	{std::string(ASUS_SAGARIS_GK1100.id_vendor) + ":" + std::string(ASUS_SAGARIS_GK1100.id_product),
	 std::string(ASUS_SAGARIS_GK1100.name)},
	{std::string(ASUS_TUF_GAMING_K1.id_vendor) + ":" + std::string(ASUS_TUF_GAMING_K1.id_product),
	 std::string(ASUS_TUF_GAMING_K1.name)},
	{std::string(ASUS_TUF_GAMING_K3.id_vendor) + ":" + std::string(ASUS_TUF_GAMING_K3.id_product),
	 std::string(ASUS_TUF_GAMING_K3.name)},
	{std::string(ASUS_TUF_GAMING_K5.id_vendor) + ":" + std::string(ASUS_TUF_GAMING_K5.id_product),
	 std::string(ASUS_TUF_GAMING_K5.name)},
	{std::string(ASUS_TUF_GAMING_K7.id_vendor) + ":" + std::string(ASUS_TUF_GAMING_K7.id_product),
	 std::string(ASUS_TUF_GAMING_K7.name)},
	{std::string(ASUS_TUF_GAMING_M3.id_vendor) + ":" + std::string(ASUS_TUF_GAMING_M3.id_product),
	 std::string(ASUS_TUF_GAMING_M3.name)},
	{std::string(ASUS_TUF_GAMING_M3_GEN_II.id_vendor) + ":" + std::string(ASUS_TUF_GAMING_M3_GEN_II.id_product),
	 std::string(ASUS_TUF_GAMING_M3_GEN_II.name)},
	{std::string(ASUS_TUF_GAMING_M5.id_vendor) + ":" + std::string(ASUS_TUF_GAMING_M5.id_product),
	 std::string(ASUS_TUF_GAMING_M5.name)},
	{std::string(ASUS_ROG_CHAKRAM_CORE.id_vendor) + ":" + std::string(ASUS_ROG_CHAKRAM_CORE.id_product),
	 std::string(ASUS_ROG_CHAKRAM_CORE.name)},
	{std::string(ASUS_ROG_CHAKRAM_WIRED.id_vendor) + ":" + std::string(ASUS_ROG_CHAKRAM_WIRED.id_product),
	 std::string(ASUS_ROG_CHAKRAM_WIRED.name)},
	{std::string(ASUS_ROG_CHAKRAM_X_24GHZ.id_vendor) + ":" + std::string(ASUS_ROG_CHAKRAM_X_24GHZ.id_product),
	 std::string(ASUS_ROG_CHAKRAM_X_24GHZ.name)},
	{std::string(ASUS_ROG_CHAKRAM_X_USB.id_vendor) + ":" + std::string(ASUS_ROG_CHAKRAM_X_USB.id_product),
	 std::string(ASUS_ROG_CHAKRAM_X_USB.name)},
	{std::string(ASUS_ROG_SPATHA_X_24GHZ.id_vendor) + ":" + std::string(ASUS_ROG_SPATHA_X_24GHZ.id_product),
	 std::string(ASUS_ROG_SPATHA_X_24GHZ.name)},
	{std::string(ASUS_ROG_SPATHA_X_DOCK.id_vendor) + ":" + std::string(ASUS_ROG_SPATHA_X_DOCK.id_product),
	 std::string(ASUS_ROG_SPATHA_X_DOCK.name)},
	{std::string(ASUS_ROG_SPATHA_X_USB.id_vendor) + ":" + std::string(ASUS_ROG_SPATHA_X_USB.id_product),
	 std::string(ASUS_ROG_SPATHA_X_USB.name)},
};
