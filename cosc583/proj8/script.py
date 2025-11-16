#!/usr/bin/env python3
from hashlib import sha256

def hexstr_to_int(h):
    return int.from_bytes(bytes.fromhex(h), byteorder='big')

def int_to_be(i):
    return i.to_bytes((i.bit_length()+7)//8, byteorder='big')

a = 47
g = 5
p = 233000556327543348946447470779219175150430130236907257523476085501968599658761371268535640963004707302492862642690597042148035540759198167263992070601617519279204228564031769469422146187139698860509698350226540759311033166697559129871348428777658832731699421786638279199926610332604408923157248859637890960407

public_key = (g**a) % p

print(f"public_key: {public_key}")

username = 'jmowry4'
password = 'chiccories'
salt_hex = "c50bf0f9"
salt_bytes = bytes.fromhex(salt_hex)
password_hashing_iterations = 1000
b_bar = 79133821056842278836378108672064282299325357902251842348769465485666601604961271649022029188101291303597860646321108627331311219283215517796328907443757841185787283022617728717021427212137254490598644602789258383863778194998101152409952640187822874467755819581692425917293405197152346682100160481245166758662
g_a = 710542735760100185871124267578125
k = hexstr_to_int(sha256(int_to_be(p) + int_to_be(g)).hexdigest())
print(f"k is {k}")

x = sha256(salt_bytes + password.encode('ascii')).digest()

for iterations in range(password_hashing_iterations-1):
    x = sha256(x).digest()

password_integer = int.from_bytes(x, byteorder='big')

print(f"Password hash as an integer {password_integer}")

g_b = (b_bar - (k * pow(g, password_integer, p) % p) % p)
if g_b < 0:
    g_b += p

print(f"g**b is {g_b}")

u = hexstr_to_int(sha256(int_to_be(g_a) + int_to_be(g_b)).hexdigest())

print(f"u is {u}")

shared_key = pow(g_b, a+u*password_integer, p)

print(f"shared key is {shared_key}")

m_1 = hexstr_to_int(
    sha256(
        (
            int_to_be((int.from_bytes(sha256(int_to_be(p)).digest(), byteorder='big')
             ^
             int.from_bytes(sha256(int_to_be(g)).digest(), byteorder='big')))
            +
            sha256(username.encode('ascii')).digest()
            +
            salt_bytes
            +
            int_to_be(g_a)
            +
            int_to_be(g_b)
            +
            int_to_be(shared_key)
        )
    ).hexdigest()
)

m_1_hex = sha256(
        (
            int_to_be((int.from_bytes(sha256(int_to_be(p)).digest(), byteorder='big')
             ^
             int.from_bytes(sha256(int_to_be(g)).digest(), byteorder='big')))
            +
            sha256(username.encode('ascii')).digest()
            +
            salt_bytes
            +
            int_to_be(g_a)
            +
            int_to_be(g_b)
            +
            int_to_be(shared_key)
        )
    ).hexdigest()

print(f"m_1 is {m_1_hex}")

m_2 = sha256(
        (
            int_to_be(g_a)
            +
            int_to_be(m_1)
            +
            int_to_be(shared_key)
        )
    ).hexdigest()

print(f"m_2 is {m_2}")
