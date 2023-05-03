#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import socket, time

HOST = '0.0.0.0'
PORT = 8888
server_addr = (HOST, PORT)


s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.bind((HOST, PORT))
print('wait for connection...')

while True:
    indata, addr = s.recvfrom(1024)
    print('recvfrom ' + str(addr) + ': ' + indata.decode())
