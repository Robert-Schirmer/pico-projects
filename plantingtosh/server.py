#!/usr/bin/python3

from socketserver import ThreadingTCPServer,StreamRequestHandler
import socket
import time

# Set server adress to machines IP
SERVER_ADDR = "192.168.1.220"
SERVER_PORT = 4242

# These constants should match the client
BUF_SIZE = 2048

def time_now_ms():
    return int(time.time() * 1000)

class MyTCPServer(ThreadingTCPServer):
    def server_bind(self):
        self.socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.socket.bind(self.server_address)

class Counter():
  count_n = 0

  @staticmethod
  def count():
    Counter.count_n += 1
    return Counter.count_n

class echohandler(StreamRequestHandler):
    def handle(self):
        print(f'Connected: {self.client_address[0]}:{self.client_address[1]}')
        while True:
            # get message
            msg_bytes = self.rfile.readline()
            if not msg_bytes:
                print(f'Disconnected: {self.client_address[0]}:{self.client_address[1]}')
                break # exits handler, framework closes socket

            msg_string = msg_bytes.decode('utf-8').strip()
            print(f'Received: {msg_string}, Session number: {Counter.count()}')
            
            if msg_string == "PING":
                self.handle_ping()
            else:
                self.handle_stats(msg_string)

    def handle_ping(self):
        self.wfile.write(str.encode("Server says hi!"))
        self.wfile.write(str.encode("END"))
        self.wfile.flush()

    def handle_stats(self, msg_string):
        f = open("log.txt", "a")
        f.write(f"received={time_now_ms()},{msg_string}\n")
        f.close()

        self.wfile.write(str.encode("END"))
        self.wfile.flush()

server = MyTCPServer((SERVER_ADDR,SERVER_PORT),echohandler)
print(f'Server listening on {SERVER_ADDR}:{SERVER_PORT}')
server.serve_forever()
