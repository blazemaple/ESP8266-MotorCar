#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import socket, time

HOST = '192.168.100.175'
PORT = 8888
server_addr = (HOST, PORT)

s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

while True:
    outdata = input('cmd:')
    print('sendto ' + str(server_addr) + ': ' + outdata)
    s.sendto(outdata.encode(), server_addr)
    time.sleep(1)
    indata, addr = s.recvfrom(1024)
    
    print('recvfrom ' + str(addr) + ': ' + indata.decode())
