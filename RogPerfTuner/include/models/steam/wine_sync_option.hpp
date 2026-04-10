#pragma once

enum class WineSyncOption {
	AUTO,
#ifdef NTSYNC_MOD
	NTSYNC,
#endif
	FSYNC,
	ESYNC,
	NONE,
};