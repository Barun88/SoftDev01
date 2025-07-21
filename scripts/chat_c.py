import socket
import threading

def receive_messages(sock):
    while True:
        try:
            message = sock.recv(1024).decode()
            if not message:
                break
            print(message)
        except:
            print("Disconnected from server.")
            sock.close()
            break

client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client.connect(("127.0.0.1", 12345))  # Connect to server

name = input("Enter your name: ")

thread = threading.Thread(target=receive_messages, args=(client,))
thread.start()

while True:
    try:
        message = input()
        if message.lower() == "exit":
            break
        full_message = f"{name}: {message}"
        client.send(full_message.encode())
    except:
        print("Error sending message.")
        client.close()
        break
