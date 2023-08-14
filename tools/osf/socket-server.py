import sys
import socket

def get_address_family(ip):
    try:
        socket.inet_pton(socket.AF_INET6, ip)
        return socket.AF_INET6
    except socket.error:
        return socket.AF_INET

def tcp_server(ip, port, scope_id=None):
    af = get_address_family(ip)
    with socket.socket(af, socket.SOCK_STREAM) as s:
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        if af == socket.AF_INET6 and scope_id:
            s.bind((ip, port, 0, scope_id))
        else:
            s.bind((ip, port))
        s.listen(1)
        conn, addr = s.accept()
        with conn:
            print(f"Connected by {addr}")
            data = conn.recv(1024)
            if data:
                print(f"Received data: {data.decode()}")

def udp_server(ip, port, scope_id=None):
    af = get_address_family(ip)
    with socket.socket(af, socket.SOCK_DGRAM) as s:
        if af == socket.AF_INET6 and scope_id:
            s.bind((ip, port, 0, scope_id))
        else:
            s.bind((ip, port))
        data, addr = s.recvfrom(1024)
        if data:
            print(f"Received data from {addr}: {data.decode()}")

if len(sys.argv) < 4:
    print("Usage: server.py <tcp|udp> <ip> <port> [<scope_id>]")
    sys.exit(1)

protocol = sys.argv[1].lower()
ip = sys.argv[2]
port = int(sys.argv[3])
scope_id = int(sys.argv[4]) if len(sys.argv) == 5 else None

if protocol == "tcp":
    tcp_server(ip, port, scope_id)
elif protocol == "udp":
    udp_server(ip, port, scope_id)
else:
    print("Invalid protocol. Choose 'tcp' or 'udp'.")
    sys.exit(1)

