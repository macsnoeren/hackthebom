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
    if re.search(".*\.md$", file):
        # Read the file!
        f = open(os.path.dirname(__file__) + "/" + file, 'r')
        plain = f.read().encode('utf8')
        f.close()

        # Create the cipher
        aes = AES.new(aes_key, AES.MODE_CBC)
        ct_bytes = aes.encrypt(pad(plain, AES.block_size))

        iv = b64encode(aes.iv).decode('utf-8')
        cipher = b64encode(ct_bytes).decode('utf-8')

        # Create the encrypted file
        result = json.dumps({'iv':iv, 'ciphertext':cipher})

        with open(os.path.dirname(__file__) + "/" + file + ".enc", "wb") as f:
            f.write(result.encode('utf8'))
