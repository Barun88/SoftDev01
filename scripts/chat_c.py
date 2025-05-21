import socket
import threading

def receive_messages(sock):
    while True:
        try:
            message = sock.recv(1024).decode()
            print(message)
        except:
            print("Disconnected from server.")
            sock.close()
            break

client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client.connect(("127.0.0.1", 12345))  # Connect to localhost server

thread = threading.Thread(target=receive_messages, args=(client,))
thread.start()

while True:
    message = input()
    client.send(message.encode())
