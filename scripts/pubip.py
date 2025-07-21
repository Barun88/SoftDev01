import requests
ip = requests.get("https://api.ipify.org").text
print("Public IP Address:", ip)


