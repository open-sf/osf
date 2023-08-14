import sys
import socket

def get_address_family(ip):
    try:
        socket.inet_pton(socket.AF_INET6, ip)
        return socket.AF_INET6
    except socket.error:
        return socket.AF_INET

def tcp_client(ip, port):
    af = get_address_family(ip)
    with socket.socket(af, socket.SOCK_STREAM) as s:
        s.connect((ip, port))
        s.sendall(b"HELLO WORLD")

def udp_client(ip, port):
    af = get_address_family(ip)
    with socket.socket(af, socket.SOCK_DGRAM) as s:
        s.sendto(b"HELLO WORLD", (ip, port))

if len(sys.argv) != 4:
    print("Usage: client.py <tcp|udp> <ip> <port>")
    sys.exit(1)

protocol = sys.argv[1].lower()
ip = sys.argv[2]
port = int(sys.argv[3])

if protocol == "tcp":
    tcp_client(ip, port)
elif protocol == "udp":
    udp_client(ip, port)
else:
    print("Invalid protocol. Choose 'tcp' or 'udp'.")
    sys.exit(1)

