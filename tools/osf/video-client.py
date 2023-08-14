import socket
import base64
import numpy as np
import cv2
import imutils
import struct
import logging
import time
import argparse

# Global constants
BUFF_SIZE = 65536
PORT = 9999
WIDTH = 1080

# Set up logging
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

def receive_frame(client_socket):
    """Receive a video frame."""
    chunks = []
    while True:
        packet,_ = client_socket.recvfrom(BUFF_SIZE)
        header = packet[:struct.calcsize('hhh')]
        index,total,chunk_size = struct.unpack('hhh',header)
        message = packet[struct.calcsize('hhh'):]
        chunks.append(message)
        logger.info('Received chunk %d/%d, len %d', index, total, len(message))
        if index == total-1:
            break

    frame_data = b''.join(chunks)
    frame = base64.b64decode(frame_data)
    frame = np.fromstring(frame, dtype=np.uint8)
    frame = cv2.imdecode(frame, 1)
    return frame, len(frame_data)

def main():
    af = socket.AF_INET6 if ':' in host_ip else socket.AF_INET
    client_socket = socket.socket(af, socket.SOCK_DGRAM)
    client_socket.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, BUFF_SIZE)
    client_socket.sendto(b'Hello', (host_ip, PORT))
    accumulator = 0
    start_time = time.time()

    fps, st, frames_to_count, cnt = (0, time.time(), 20, 0)
    prev = 0

    while True:
        frame, frame_size = receive_frame(client_socket)
        accumulator += frame_size
        time_elapsed = time.time() - start_time
        if time_elapsed > 0:  # To avoid division by zero at the start
            logger.info("UDP received an average of %0.3fkbps"%((accumulator/time_elapsed)*8.0/1000.0))
        if frame is not None:
            frame = imutils.resize(frame,width=WIDTH)  # Resize the window while keeping the aspect ratio

            if cnt == frames_to_count:
                fps, st, cnt = calculate_fps(st, cnt, frames_to_count)
            cnt += 1

            frame = cv2.putText(frame, 'FPS: ' + str(fps), (10, 40), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 0, 255), 2)
            cv2.imshow('RECEIVING VIDEO', frame)
            if cv2.waitKey(1) & 0xFF == ord('q'):
                break

    client_socket.close()
    cv2.destroyAllWindows()

if __name__ == "__main__":
    main()
