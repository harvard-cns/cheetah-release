import socket
import sys

from control_flow_constants import *

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
server_address = (CHEETAH_MASTER_IP, CHEETAH_MASTER_PORT)
msg = b'ACK'

try:
    sent = sock.sendto(msg, server_address)

finally:
    sock.close()
