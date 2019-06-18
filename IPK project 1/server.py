#!/usr/bin/python3

import sys, re, platform, subprocess
from socket import *
from time import sleep

class Server:
	def __init__(self, port):
		self.port = port

	__hostname = str(platform.node())
	__cpuname = subprocess.check_output("cat /proc/cpuinfo | grep 'model name' | head -n 1", shell=True).strip().decode('UTF-8').replace("model name\t: ", '')

	def getcpuLoad(self):
		prevcpuInfo = subprocess.check_output("cat /proc/stat | head -n 1", shell=True).strip().decode('UTF-8').split()
		sleep(1)
		cpuInfo = subprocess.check_output("cat /proc/stat | head -n 1", shell=True).strip().decode('UTF-8').split()

		prevIdle = int(prevcpuInfo[4]) + int(prevcpuInfo[5])
		idle = int(cpuInfo[4]) + int(cpuInfo[5])

		prevNonIdle = int(prevcpuInfo[1]) + int(prevcpuInfo[2]) + int(prevcpuInfo[3]) + int(prevcpuInfo[6]) + int(prevcpuInfo[7]) + int(prevcpuInfo[8])
		nonIdle = int(cpuInfo[1]) + int(cpuInfo[2]) + int(cpuInfo[3]) + int(cpuInfo[6]) + int(cpuInfo[7]) + int(cpuInfo[8])

		prevTotal = prevIdle + prevNonIdle
		total = idle + nonIdle

		totald = total - prevTotal
		idled = idle - prevIdle

		return (totald - idled)/totald * 100
		
	def requestHandler(self, clientsocket):
		rd = clientsocket.recv(5000).decode()
		msg = rd.split("\n")
			
		print(msg[0])

		if (msg[0].strip() == "GET /hostname HTTP/1.1"):
			data = "HTTP/1.1 200 OK\r\n"
			data += "Content-Type: text/plain; charset=utf-8\r\n"
			data += "\r\n"
			data += self.__hostname+"\r\n\r\n"
			clientsocket.sendall(data.encode())
			clientsocket.close()

		elif (msg[0].strip() == "GET /cpu-name HTTP/1.1"):
			data = "HTTP/1.1 200 OK\r\n"
			data += "Content-Type: text/plain; charset=utf-8\r\n"
			data += "\r\n"
			data += self.__cpuname+"\r\n\r\n"
			clientsocket.sendall(data.encode())
			clientsocket.close()

		elif (msg[0].strip() == "GET /load HTTP/1.1"):
			data = "HTTP/1.1 200 OK\r\n"
			data += "Content-Type: text/plain; charset=utf-8\r\n"
			data += "\r\n"
			data += str(self.getcpuLoad())+"%"+"\r\n\r\n"
			clientsocket.sendall(data.encode())
			clientsocket.close()

		elif (re.match(r"GET /load\?refresh=[0-9]* HTTP/1.1", msg[0].strip())):
			refresh = msg[0].strip()[18:].split()[0]
			data = "HTTP/1.1 200 OK\r\n"
			data += "Content-Type: text/html; charset=utf-8\r\n"
			data += "\r\n"
			data += "<html><meta http-equiv=\"refresh\" content=\""+refresh+"\"/><body>"+str(self.getcpuLoad())+"%"+"</body></html>\r\n\r\n"
			clientsocket.sendall(data.encode())
			clientsocket.close()
				
		else:
			data = "HTTP/1.1 400 Bad Request\r\n"
			data += "Content-Type: text/plain; charset=utf-8\r\n"
			data += "\r\n"
			data += "400 Bad Request\r\n\r\n"
			clientsocket.sendall(data.encode())
			clientsocket.close()

	def startServer(self):
		serversocket = socket(AF_INET, SOCK_STREAM)
		serversocket.setsockopt(SOL_SOCKET, SO_REUSEADDR, 1)	

		try :
			serversocket.bind((platform.node(), self.port))
			
			print("\nRunning on port "+str(self.port)+"...\n")

			serversocket.listen(5)
			
			while True:
				(clientsocket, address) = serversocket.accept()
				
				self.requestHandler(clientsocket)

		except KeyboardInterrupt:
			print("\nShutting down...\n")
			serversocket.close()

def main(argv):
	port = 8080
	
	if (argv[0] == "-p"):
		if re.match(r"[0-9]*", argv[1]):
			port=int(argv[1])
	else:
		print("server.py -p <port>")
		sys.exit()

	server = Server(port)
	server.startServer()

if __name__ == "__main__":
	main(sys.argv[1:])
   
