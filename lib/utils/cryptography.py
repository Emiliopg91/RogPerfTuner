from .logger import Logger

from base64 import urlsafe_b64encode, urlsafe_b64decode
from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes
from cryptography.hazmat.backends import default_backend
from cryptography.hazmat.primitives.kdf.pbkdf2 import PBKDF2HMAC
from cryptography.hazmat.primitives import hashes

import keyring
import os


class Cryptography:
    def __init__(self):
        self.logger = Logger("Cryptography")
        self.service = "RogControlCenter"
        self.username = os.getlogin()

        if keyring.get_password(self.service, self.username) is None:
            keyring.set_password(
                self.service, self.username, urlsafe_b64encode(os.urandom(32)).decode()
            )
            print("Encryption key generated and stored in keyring.")

    def retrieve_key_from_keyring(self):
        encoded_key = keyring.get_password(self.service, self.username)
        if not encoded_key:
            raise ValueError(
                "No encryption key found for the specified service and username."
            )
        return urlsafe_b64decode(encoded_key.encode())

    def encrypt_string(self, plaintext: str):
        nonce = os.urandom(12)

        cipher = Cipher(
            algorithms.AES(self.retrieve_key_from_keyring()),
            modes.GCM(nonce),
            backend=default_backend(),
        )
        encryptor = cipher.encryptor()

        ciphertext = encryptor.update(plaintext.encode()) + encryptor.finalize()

        return urlsafe_b64encode(nonce + encryptor.tag + ciphertext).decode()

    def decrypt_string(self, encrypted_text):
        encrypted_data = urlsafe_b64decode(encrypted_text)

        nonce = encrypted_data[:12]
        tag = encrypted_data[12:28]
        ciphertext = encrypted_data[28:]

        cipher = Cipher(
            algorithms.AES(self.retrieve_key_from_keyring()),
            modes.GCM(nonce, tag),
            backend=default_backend(),
        )
        decryptor = cipher.decryptor()

        return (decryptor.update(ciphertext) + decryptor.finalize()).decode()


cryptography = Cryptography()
