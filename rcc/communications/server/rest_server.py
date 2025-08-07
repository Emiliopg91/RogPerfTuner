# pylint: disable=missing-function-docstring, duplicate-code

import logging
from threading import Thread
from flask import Flask, jsonify
from rcc.services.hardware_service import HARDWARE_SERVICE
from rcc.services.steam_service import STEAM_SERVICE
from rcc.services.rgb_service import RGB_SERVICE
from rcc.services.profile_service import PROFILE_SERVICE
from rcc.utils.constants import REST_SERVER_PORT
from framework.logger import Logger
from framework.singleton import singleton

app = Flask(__name__)
logger = Logger()


@singleton
class HelloService:
    """REST service logic"""

    def next_profile(self):
        if len(STEAM_SERVICE.running_games) == 0:
            next_t = PROFILE_SERVICE.performance_profile.next_performance_profile
            PROFILE_SERVICE.set_performance_profile(next_t)
            return {"result": next_t.name}
        return {"error": "Not available on game session"}

    def next_effect(self):
        next_t = RGB_SERVICE.get_next_effect()
        RGB_SERVICE.apply_effect(next_t)
        return {"result": next_t}

    def increase_brightness(self):
        next_t = RGB_SERVICE.brightness.get_next_brightness()
        RGB_SERVICE.apply_brightness(next_t)
        return {"result": next_t.name}

    def decrease_brightness(self):
        next_t = RGB_SERVICE.brightness.get_previous_brightness()
        RGB_SERVICE.apply_brightness(next_t)
        return {"result": next_t.name}

    def get_param_running_app(self, app_id):
        env, wrap, params = STEAM_SERVICE.get_run_configuration(app_id)
        return {"environment": env, "wrappers": wrap, "parameters": params}

    def renice(self, app_id):
        HARDWARE_SERVICE.renice(app_id)


hello_service = HelloService()


@app.route("/nextProfile", methods=["GET"])
def next_profile():
    return jsonify(hello_service.next_profile())


@app.route("/nextEffect", methods=["GET"])
def next_effect():
    return jsonify(hello_service.next_effect())


@app.route("/increaseBrightness", methods=["GET"])
def increase_brightness():
    return jsonify(hello_service.increase_brightness())


@app.route("/decreaseBrightness", methods=["GET"])
def decrease_brightness():
    return jsonify(hello_service.decrease_brightness())


@app.route("/getParamRunningApp/<int:appid>", methods=["GET"])
def get_param_running_app(appid: int):
    result = hello_service.get_param_running_app(appid)
    return jsonify(result)


@app.route("/renice/<int:appid>", methods=["GET"])
def renice(appid: int):
    hello_service.renice(appid)
    return jsonify({})


@singleton
class RESTServer:
    """REST server initializer and script generator"""

    def __init__(self):
        self._port = REST_SERVER_PORT

    def run_flask(self):
        logger.info(f"Starting REST server on http://127.0.0.1:{self._port}")
        logging.getLogger("werkzeug").disabled = True
        logging.getLogger("flask.app").disabled = True
        app.run(host="127.0.0.1", port=self._port, threaded=True, use_reloader=False)

    def start(self):
        Thread(name="Rest Server", target=self.run_flask).start()


REST_SERVER = RESTServer()
