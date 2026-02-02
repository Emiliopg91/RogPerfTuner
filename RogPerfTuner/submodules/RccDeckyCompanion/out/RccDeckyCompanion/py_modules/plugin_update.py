import os
import stat
import decky
import urllib.request
import json
import ssl
import shutil


class PluginUpdate:
    @staticmethod
    def recursive_chmod(path, perms):
        for dirpath, dirnames, filenames in os.walk(path):
            current_perms = os.stat(dirpath).st_mode
            os.chmod(dirpath, current_perms | perms)
            for filename in filenames:
                os.chmod(os.path.join(dirpath, filename), current_perms | perms)

    @staticmethod
    def download_latest_build():
        # ssl._create_default_https_context = ssl._create_unverified_context
        url = "http://api.github.com/repos/Emiliopg91/AllyDeckyCompanion/releases/latest"
        decky.logger.info("Downloading plugin update")
        gcontext = ssl.SSLContext()

        response = urllib.request.urlopen(url, context=gcontext)
        json_data = json.load(response)

        download_url = json_data.get("name")

        file_path = "/tmp/AllyDeckyCompanion.tar.gz"

        with urllib.request.urlopen(download_url, context=gcontext) as response, open(file_path, "wb") as output_file:
            output_file.write(response.read())
            output_file.close()
        decky.logger.info("Downloaded!")

        return file_path

    @staticmethod
    def ota_update():
        downloaded_filepath = PluginUpdate.download_latest_build()

        if os.path.exists(downloaded_filepath):
            plugin_dir = f"{decky.DECKY_USER_HOME}/homebrew/plugins/AllyDeckyCompanion"

            try:
                PluginUpdate.recursive_chmod(plugin_dir, stat.S_IWUSR)
                shutil.rmtree(plugin_dir)
            except Exception as e:
                decky.logger.error(f"OTA error during removal of old plugin {e}")

            try:
                shutil.unpack_archive(downloaded_filepath, f"{decky.DECKY_USER_HOME}/homebrew/plugins")
                os.remove(downloaded_filepath)
            except Exception as e:
                decky.logger.error(f"Error during OTA install {e}")

            return True
