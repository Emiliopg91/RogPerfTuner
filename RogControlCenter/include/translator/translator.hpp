#pragma once

#include <any>

#include "../logger/logger.hpp"
#include "../models/translator/translation_entry.hpp"

class Translator {
  public:
	static Translator& getInstance() {
		static Translator instance;
		return instance;
	}

	std::string translate(const std::string& msg, const std::unordered_map<std::string, std::any>& replacement = {});

  private:
	Translator();
	Logger logger{"Translator"};
	Language currentLang = []() -> Language {
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

	inline static Language FALLBACK_LANG = Language::Enum::EN;
	// Begin translations
	inline static const constexpr std::array<TranslationEntry, 72> initialTranslations = {{
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
			"label.profile.TURBO",
			"Turbo",
			"Turbo",
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
			"label.level.0",
			"Disabled",
			"Desactivado",
		},
		{
			"label.level.1",
			"Only FPS",
			"Solo FPS",
		},
		{
			"label.level.2",
			"Horizontal",
			"Horizontal",
		},
		{
			"label.level.3",
			"Extended",
			"Extendido",
		},
		{
			"label.level.4",
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
	}};

	// End translations
};