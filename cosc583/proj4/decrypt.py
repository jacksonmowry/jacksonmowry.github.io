# Diffie Part of the lab
from hashlib import sha256

# Hex encoded shared key from my C code
shared_key = "013e9852988262ee51d5352f93fc896baaa586d4f23ff71b268e4a8fc80c79369e568151d42d6235c63889267598beb66bea8bd67dcd87eea34a8e8b8dbe810e1d094f1b8faa502bf231d349a1982bfe45dc419a1958cfaf9a5e43309c34cecc995a46a9881acea55c73466f46b96eee929cc0bb0bb56eaad90f78f9d53c2f6dba"
print(sha256(bytes.fromhex(shared_key)).hexdigest())

# RSA Part of the lab
from Crypto.Cipher import AES
from Crypto.Util.Padding import unpad
import binascii

key = bytes.fromhex("2ae03a439c16b8f9d59a33eb2d9c595b")
iv = bytes.fromhex("2035fee926711d218b503564fbc85fc4")
cipher = bytes.fromhex(
    "de34a4841448fc7856974e253eb12c1d77dee628a5a373f09467221bb21f2f4557f771e56418d9d673a94c6e7b2e2472ee3b7a7650b2fc36b03177cd3d83400a717c13307a0459d80c287172f868253b8c8c17bb4cfc01dfd713651c058009d5"
)

deciper = AES.new(key, AES.MODE_CBC, iv)
print(unpad(deciper.decrypt(cipher), AES.block_size))
