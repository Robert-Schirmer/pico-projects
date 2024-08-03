from socketserver import ThreadingTCPServer, StreamRequestHandler
import socket
import time
import logging
import uuid

# Set server adress to machines IP
SERVER_ADDR = "192.168.1.220"
SERVER_PORT = 4242

logging.basicConfig(
    format="%(asctime)s %(levelname)-8s %(message)s", level=logging.INFO
)


def time_now_ms():
    return int(time.time() * 1000)


class MyTCPServer(ThreadingTCPServer):
    def server_bind(self):
        self.socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.socket.bind(self.server_address)


class Counter:
    count_n = 0

    @staticmethod
    def count():
        Counter.count_n += 1
        return Counter.count_n


class echohandler(StreamRequestHandler):
    def handle(self):
        self.request_number = Counter.count()
        self.request_id = uuid.uuid4()

        logging.info(
            f"{self.request_id} Connected: {self.client_address[0]}:{self.client_address[1]}, Session number: {self.request_number}"
        )
        while True:
            # get message
            msg_bytes = self.rfile.readline()
            if not msg_bytes:
                logging.info(
                    f"{self.request_id} Disconnected: {self.client_address[0]}:{self.client_address[1]}"
                )
                break  # exits handler, framework closes socket

            msg_string = msg_bytes.decode("utf-8").strip()
            logging.info(f"{self.request_id} Received: {msg_string}")

            if msg_string == "PING":
                self.handle_ping()
            else:
                self.handle_stats(msg_string)

    def handle_ping(self):
        self.write("Server says hi!")
        self.write("END")
        self.write_flush()

    def handle_stats(self, msg_string):
        parsed_msg = self.parse_msg(msg_string)

        plant_id = parsed_msg.get("plant_id")

        file_name = plant_id if plant_id else "unknown"

        f = open(f"./plant_logs/log_{file_name}.txt", "a+")
        f.write(f"received={time_now_ms()},{msg_string}\n")
        f.close()

        self.write("END")
        self.write_flush()

    def parse_msg(self, msg_string):
        fields = {}
        fields_list = msg_string.strip().split(",")

        for field in fields_list:
            key, value = field.split("=")
            fields[key] = value

        return fields

    def write(self, msg):
        logging.info(f"{self.request_id} Sending: {msg}")
        self.wfile.write(str.encode(msg))

    def write_flush(self):
        logging.info(f"{self.request_id} Sending Flushed")
        self.wfile.flush()


server = MyTCPServer((SERVER_ADDR, SERVER_PORT), echohandler)
logging.info(f"Server listening on {SERVER_ADDR}:{SERVER_PORT}")
server.serve_forever()
