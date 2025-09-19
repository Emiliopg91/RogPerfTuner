
#include "../../include/translator/translator.hpp"

#include <nlohmann/json.hpp>
#include <optional>
#include <string>

using json = nlohmann::json;

Translator::Translator() : Loggable("Translator") {
	currentLang = []() -> Language {
		const char* lang = std::getenv("LC_MESSAGES");
		if (!lang || std::string(lang).empty()) {
			lang = std::getenv("LANG");
		}
		if (!lang || std::string(lang).empty()) {
			return FALLBACK_LANG;
		}
		std::string langStr(lang);

		auto pos = langStr.find('_');
		if (pos != std::string::npos) {
			langStr = langStr.substr(0, pos);
		}
		return Language::fromString(langStr);
	}();

	logger.debug("User language: {}", currentLang.toString());
}

std::string Translator::translate(const std::string& msg, const std::unordered_map<std::string, std::any>& replacement) {
	std::optional<std::string> translation = std::nullopt;

	for (auto entry : translations) {
		if (entry.key == msg) {
			translation = entry.getTranslation(currentLang);
			break;
		}
	}

	if (translation.has_value()) {
		auto result = std::string(translation.value());

		for (const auto& [key, value] : replacement) {
			std::string placeholder = "{" + key + "}";
			std::string valStr;

			try {
				valStr = std::any_cast<std::string>(value);
			} catch (const std::bad_any_cast&) {
				try {
					valStr = std::to_string(std::any_cast<int>(value));
				} catch (const std::bad_any_cast&) {
					valStr = "<invalid>";
				}
			}

			size_t pos = 0;
			while ((pos = result.find(placeholder, pos)) != std::string::npos) {
				result.replace(pos, placeholder.length(), valStr);
				pos += valStr.length();
			}
		}

		return result;
	} else {
		logger.warn("Missing translation for '{}'", msg);
		return msg;
	}
}

// Begin translations
std::vector<TranslationEntry> Translator::translations = {
	{
		"performance",
		"Performance",
		"Rendimiento",
	},
	{
		"profile",
		"Profile",
		"Perfil",
	},
	{
		"label.profile.PERFORMANCE",
		"Performance",
		"Rendimiento",
	},
	{
		"label.profile.BALANCED",
		"Balanced",
		"Equilibrado",
	},
	{
		"label.profile.QUIET",
		"Quiet",
		"Silencioso",
	},
	{
		"effect",
		"Effect",
		"Efecto",
	},
	{
		"brightness",
		"Brightness",
		"Brillo",
	},
	{
		"label.brightness.MAX",
		"Maximum",
		"Máximo",
	},
	{
		"label.brightness.HIGH",
		"High",
		"Alto",
	},
	{
		"label.brightness.MEDIUM",
		"Medium",
		"Medio",
	},
	{
		"label.brightness.LOW",
		"Low",
		"Bajo",
	},
	{
		"label.brightness.OFF",
		"Off",
		"Apagado",
	},
	{
		"color",
		"Color",
		"Color",
	},
	{
		"color.select",
		"Select color",
		"Seleccione color",
	},
	{
		"battery",
		"Battery",
		"Bateria",
	},
	{
		"charge.threshold",
		"Charge limit",
		"Límite de carga",
	},
	{
		"profile.applied",
		"Profile {profile} applied succesfully",
		"Perfil {profile} aplicado con éxito",
	},
	{
		"applied.battery.threshold",
		"Battery charge limit setted to {value}%",
		"Límite de carga de bateria establecido al {value}%",
	},
	{
		"open.ui",
		"Show interface",
		"Mostrar interfaz",
	},
	{
		"open.logs",
		"Open log file",
		"Abrir registro",
	},
	{
		"close",
		"Quit",
		"Salir",
	},
	{
		"select.color",
		"Pick color",
		"Elegir color",
	},
	{
		"authentication.required",
		"Authentication required",
		"Autenticación requerida",
	},
	{
		"enter.sudo.password",
		"Enter sudo password",
		"Introduzca contraseña sudo",
	},
	{
		"accept",
		"Accept",
		"Aceptar",
	},
	{
		"cancel",
		"Cancel",
		"Cancelar",
	},
	{
		"canceled",
		"Canceled",
		"Cancelado",
	},
	{
		"user.canceled.operation",
		"User canceled operation",
		"El usuario canceló la operación",
	},
	{
		"authentication.failed",
		"Authentication failed",
		"Fallo de autenticación",
	},
	{
		"applying.update",
		"Applying update, please wait",
		"Aplicando actualizacion, espere",
	},
	{
		"update.available",
		"Update availabe",
		"Actualización disponible",
	},
	{
		"apply.now",
		"Apply now",
		"Aplicar ahora",
	},
	{
		"initializing",
		"Initializing application",
		"Inicializando aplicación",
	},
	{
		"boost",
		"CPU Boost",
		"CPU Boost",
	},
	{
		"label.boost.AUTO",
		"Auto",
		"Automatico",
	},
	{
		"label.boost.ON",
		"Enabled",
		"Activado",
	},
	{
		"label.boost.OFF",
		"Disabled",
		"Desactivado",
	},
	{
		"games",
		"Games",
		"Juegos",
	},
	{
		"label.game.configure",
		"Setup profiles",
		"Configurar perfiles",
	},
	{
		"profile.applied.for.game",
		"Profile {profile} applied succesfully for {game}",
		"Perfil {profile} aplicado con éxito para {game}",
	},
	{
		"game.title",
		"Game",
		"Juego",
	},
	{
		"game.performance.configuration",
		"Game performance configuration",
		"Configuracion de rendimiento de juegos",
	},
	{
		"label.dgpu.auto",
		"Automatic",
		"Automatica",
	},
	{
		"label.dgpu.discrete",
		"Discrete",
		"Dedicada",
	},
	{
		"used.gpu",
		"GPU",
		"GPU",
	},
	{
		"metrics",
		"Metrics",
		"Métricas",
	},
	{
		"label.level",
		"Level",
		"Nivel",
	},
	{
		"label.level.no_display",
		"Disabled",
		"Desactivado",
	},
	{
		"label.level.fps_only",
		"Only FPS",
		"Solo FPS",
	},
	{
		"label.level.horizontal",
		"Horizontal",
		"Horizontal",
	},
	{
		"label.level.extended",
		"Extended",
		"Extendido",
	},
	{
		"label.level.detailed",
		"Detailed",
		"Detallado",
	},
	{
		"application",
		"Application",
		"Aplicación",
	},
	{
		"autostart",
		"Start on boot",
		"Iniciar automaticamente",
	},
	{
		"winesync",
		"Synchronization",
		"Sincronización",
	},
	{
		"label.winesync.auto",
		"Auto",
		"Auto",
	},
	{
		"label.winesync.ntsync",
		"NTSync",
		"NTSync",
	},
	{
		"label.winesync.esync",
		"ESync",
		"ESync",
	},
	{
		"label.winesync.fsync",
		"FSync",
		"FSync",
	},
	{
		"label.winesync.none",
		"None",
		"No",
	},
	{
		"environment",
		"Environment",
		"Entorno",
	},
	{
		"params",
		"Parameters",
		"Parámetros",
	},
	{
		"used.steamdeck",
		"Mode",
		"Modo",
	},
	{
		"label.steamdeck.no",
		"Computer",
		"Ordenador",
	},
	{
		"label.steamdeck.yes",
		"SteamDeck",
		"SteamDeck",
	},
	{
		"running",
		"Running",
		"Ejecutando",
	},
	{
		"settings",
		"Settings",
		"Configuración",
	},
	{
		"config.for.game",
		"Game configuration",
		"Configuración de juego",
	},
	{
		"save.and.run",
		"Save and launch",
		"Guardar y ejecutar",
	},
	{
		"confirmation.required",
		"Confirmation required",
		"Confirmación necesaria",
	},
	{
		"run.with.default.config",
		"Launch game with default configuration?",
		"¿Lanzar juego con la configuración por defecto?",
	},
	{
		"configuration",
		"Configuration",
		"Configuración",
	},
	{
		"edit",
		"Edit",
		"Editar",
	},
	{
		"save",
		"Save",
		"Guardar",
	},
	{
		"wrappers",
		"Wrappers",
		"Wrappers",
	},
	{
		"pick.color",
		"Pick color",
		"Selección de color",
	},
	{
		"select",
		"Select",
		"Seleccionar",
	},
};

// End translations