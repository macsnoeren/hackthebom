#!/usr/bin/python

import os
import re
import json

from base64 import b64encode, b64decode
from Crypto.Protocol.KDF import PBKDF2
from Crypto.Cipher import AES
from Crypto.Util.Padding import pad, unpad
from Crypto.Hash import HMAC, SHA256, SHA512
from Crypto.Random import get_random_bytes
from getpass import getpass

password = getpass("Type your password: ")
salt = b'!\xe0\xec\xf1%2\xe3\xebX\x04\x92\x92\x1a\x11\xe4\xe5' #get_random_bytes(16)
keys = PBKDF2(password, salt, 64, count=1000000, hmac_hash_module=SHA512)
aes_iv = keys[:32]
aes_key = keys[32:]

dir_list = os.listdir( os.path.dirname(__file__) )
for file in dir_list:
    if re.search(".*\.md\.enc$", file):
        # Read the file!
        f = open(os.path.dirname(__file__) + "/" + file, 'r')
        cipher = f.read().encode('utf8')
        f.close()

        try:
            b64 = json.loads(cipher)
            iv = b64decode(b64['iv'])
            ct_bytes = b64decode(b64['ciphertext'])
            aes = AES.new(aes_key, AES.MODE_CBC, iv)
            plain = unpad(aes.decrypt(ct_bytes), AES.block_size)
            
            # Create the encrypted file
            with open(os.path.dirname(__file__) + "/" + file[0:-4], "wb") as f:
                f.write(plain)

        except (ValueError, KeyError):
            print("Incorrect decryption")

