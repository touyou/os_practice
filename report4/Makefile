all: udpechoserver udpechoclient tcpechoserver1 tcpechoserver2 tcpechoclient tcpechoclient2

udpechoserver: udpechoserver.c
	gcc udpechoserver.c -o udpechoserver

udpechoclient: udpechoclient.c
	gcc udpechoclient.c -o udpechoclient

tcpechoserver1: tcpechoserver1.c
	gcc -pthread tcpechoserver1.c -o tcpechoserver1

tcpechoserver2: tcpechoserver2.c
	gcc tcpechoserver2.c -o tcpechoserver2

tcpechoclient: tcpechoclient.c
	gcc tcpechoclient.c -o tcpechoclient

tcpechoclient2: tcpechoclient2.c
	gcc tcpechoclient2.c -o tcpechoclient2

clean:
	rm udpechoserver udpechoclient tcpechoserver1 tcpechoserver2 tcpechoclient
