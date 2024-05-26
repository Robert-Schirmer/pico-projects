#!/usr/bin/python3

from socketserver import ThreadingTCPServer,StreamRequestHandler
import socket
import time
import os
from groq import Groq

client = Groq(
    # This is the default and can be omitted
    api_key=os.environ.get("PLANTINGTOSH_AI_API_KEY"),
)

# Set server adress to machines IP
SERVER_ADDR = "192.168.1.220"
SERVER_PORT = 4242

# These constants should match the client
BUF_SIZE = 2048

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

            start_time = time.time()
            chat_completion = client.chat.completions.create(
                messages=[
                    {
                        "role": "user",
                        "content": f"You are a plant, with stats {msg_string}, tell me how you are feeling in a no more than 10 words. Don't quote the reply. Start the reply with 'I'm feeling'.",
                    }
                ],
                model="llama3-8b-8192",
            )

            end_time = time.time()
            duration = end_time - start_time
            message_content = chat_completion.choices[0].message.content
            print(f"Request duration: {duration} seconds, response: {message_content}")
            
            self.wfile.write(str.encode(message_content + "END"))
            self.wfile.flush()

server = MyTCPServer((SERVER_ADDR,SERVER_PORT),echohandler)
print(f'Server listening on {SERVER_ADDR}:{SERVER_PORT}')
server.serve_forever()
