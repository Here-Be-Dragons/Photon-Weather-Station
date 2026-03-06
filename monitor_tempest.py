import socket
s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.bind(('', 50222))
while True:
    data, addr = s.recvfrom(1024)
    print(f"Received from {addr}: {data}")