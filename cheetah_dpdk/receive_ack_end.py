import socket
import sys
import os

from control_flow_constants import *

print("HELLO")
from timeit import default_timer as timer

start = timer()

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

server_address = (CHEETAH_MASTER_IP, CHEETAH_MASTER_PORT)
sock.bind(server_address)

for i in range(CHEETAH_WORKER_NODES):
    sock.recvfrom(4096)

end = timer()
print("End to end delay:", end - start, "s")
