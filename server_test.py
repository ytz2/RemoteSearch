# Echo server program
import socket
import struct

HOST = '0.0.0.0'                 # Symbolic name meaning the local host
PORT = 50007              # Arbitrary non-privileged port
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.bind((HOST, PORT))
s.listen(1)
conn, addr = s.accept()
print 'Connected by', addr
while 1:
    data = conn.recv(4)
    if not data: break
    print struct.unpack("i",data)[0]
conn.close()
