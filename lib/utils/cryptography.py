from base64 import urlsafe_b64encode, urlsafe_b64decode

import os

# pylint: disable=E0611,E0401
from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes
from cryptography.hazmat.backends import default_backend

import keyring

from lib.utils.logger import Logger


class Cryptography:
    """Class for access cryptography methods"""

    def __init__(self):
        self._logger = Logger()
        self._service = "RogControlCenter"
        self._username = os.getlogin()

        if keyring.get_password(self._service, self._username) is None:
            keyring.set_password(self._service, self._username, urlsafe_b64encode(os.urandom(32)).decode())
            print("Encryption key generated and stored in keyring.")

    def retrieve_key_from_keyring(self) -> bytes:
        """Get key from keyring"""
        encoded_key = keyring.get_password(self._service, self._username)
        if not encoded_key:
            raise ValueError("No encryption key found for the specified service and username.")
        return urlsafe_b64decode(encoded_key.encode())

    def encrypt_string(self, plaintext: str) -> bytes:
        """Encrypt string"""
        nonce = os.urandom(12)

        cipher = Cipher(
            algorithms.AES(self.retrieve_key_from_keyring()),
            modes.GCM(nonce),
            backend=default_backend(),
        )
        encryptor = cipher.encryptor()

        ciphertext = encryptor.update(plaintext.encode()) + encryptor.finalize()

        return urlsafe_b64encode(nonce + encryptor.tag + ciphertext).decode()

    def decrypt_string(self, encrypted_text) -> str:
        """Decrypt string"""
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
