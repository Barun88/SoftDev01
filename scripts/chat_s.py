import socket
import threading

clients = []

def handle_client(client_socket):
    while True:
        try:
            message = client_socket.recv(1024)
            if not message:
                break
            print(f"Received: {message.decode().strip()}")
            broadcast(message, client_socket)
        except:
            break

    if client_socket in clients:
        clients.remove(client_socket)
    client_socket.close()

def broadcast(message, sender_socket):
    print("Broadcasting:", message.decode().strip())
    for client in clients:
        if client != sender_socket:
            try:
                client.send(message)
            except:
                client.close()
                if client in clients:
                    clients.remove(client)

def admin_send():
    while True:
        try:
            message = input()
            full_message = f"Admin: {message}".encode()
            broadcast(full_message, None)
        except:
            break

server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server.bind(("127.0.0.1", 12345)) 
server.listen(5)
print("Server started. Waiting for connections...")

admin_thread = threading.Thread(target=admin_send)
admin_thread.daemon = True
admin_thread.start()

while True:
    client_socket, addr = server.accept()
    print(f"Connection from {addr}")
    clients.append(client_socket)
    thread = threading.Thread(target=handle_client, args=(client_socket,))
    thread.start()
