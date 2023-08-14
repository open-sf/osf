import cv2
import imutils
import socket
import numpy as np
import time
import base64
import struct
import logging
import argparse

# Constants
WIDTH = 280
QUALITY = 60
FRAME_RATE = 10
BUFF_SIZE = 65536
CHUNK_SIZE = 1000 - struct.calcsize('hhh')
PORT = 9999

# Initialize logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

# Create the parser
arg_parser = argparse.ArgumentParser(description='Video Streaming Server')
# Add the arguments
arg_parser.add_argument('HostIP',
                       metavar='hostip',
                       type=str,
                       help='the host IP address')
# Execute the parse_args() method
args = arg_parser.parse_args()
# Now use args.HostIP in the place of 'fd02::1'
host_ip = args.HostIP


def calculate_fps(st, cnt, frames_to_count):
    """Calculates the frames per second."""
    try:
        fps = round(frames_to_count / (time.time() - st))
        st = time.time()
        cnt = 0
    except ZeroDivisionError:
        logger.warning("ZeroDivisionError when calculating fps")
        fps = 0

    return fps, st, cnt

def process_frame(frame):
    """Process a single frame."""
    frame = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    frame = imutils.resize(frame, width=WIDTH)
    _, buffer = cv2.imencode('.webp', frame, [cv2.IMWRITE_WEBP_QUALITY, QUALITY])
    message = base64.b64encode(buffer)

    return message, frame

def main():
    global CHUNK_SIZE
    
    # Set up server
    af = socket.AF_INET6 if ':' in host_ip else socket.AF_INET
    server_socket = socket.socket(af, socket.SOCK_DGRAM)
    server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, BUFF_SIZE)
    socket_address = (host_ip, PORT)
    server_socket.bind(socket_address)
    logger.info('Listening at: %s', socket_address)

    vid = cv2.VideoCapture(0) # replace with 0 for webcam
    fps, st, frames_to_count, cnt = (0, time.time(), 20, 0)
    prev = 0

    while True:
        _, client_addr = server_socket.recvfrom(BUFF_SIZE)
        logger.info('Got connection from %s', client_addr)
        while vid.isOpened():
            _, frame = vid.read()
            time_elapsed = time.time() - prev
            if time_elapsed < 1.0 / FRAME_RATE:
                continue
            prev = time.time()

            message, frame = process_frame(frame)
            
            if cnt == frames_to_count:
                fps, st, cnt = calculate_fps(st, cnt, frames_to_count)
            cnt += 1

            frame = cv2.putText(frame, f'FPS: {fps}', (10, 40), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 0, 255), 2)
            cv2.imshow('TRANSMITTING VIDEO', frame)
            cv2.waitKey(1)

            # Ensure each chunk is a complete base64 string
            while CHUNK_SIZE % 4 != 0:
                CHUNK_SIZE += 1
                logger.warning('Incorrect chunk size. Increasing by 1 to %d', CHUNK_SIZE)

            chunks = [message[i:i+CHUNK_SIZE] for i in range(0, len(message), CHUNK_SIZE)]
            for index, chunk in enumerate(chunks):
                header = struct.pack("hhh", index, len(chunks), CHUNK_SIZE)
                packet = bytearray(header)
                packet.extend(chunk)
                server_socket.sendto(packet, client_addr)
                time.sleep(0.020)
            
            logger.info('UDP packet size: %d, resulting in %d chunks of len %d', len(message), len(chunks), CHUNK_SIZE)
            prev = time.time()

if __name__ == "__main__":
    main()
