all: udpechoserver udpechoclient tcpechoserver1 tcpechoserver2 tcpechoclient iperfc iperfs

udpechoserver: udpechoserver.c
	gcc -Wall -std=gnu99 udpechoserver.c -o udpechoserver

udpechoclient: udpechoclient.c
	gcc -Wall -std=gnu99 udpechoclient.c -o udpechoclient

tcpechoserver1: tcpechoserver1.c
	gcc -pthread -Wall -std=gnu99 tcpechoserver1.c -o tcpechoserver1

tcpechoserver2: tcpechoserver2.c
	gcc -Wall -std=gnu99 tcpechoserver2.c -o tcpechoserver2

tcpechoclient: tcpechoclient.c
	gcc -Wall -std=gnu99 tcpechoclient.c -o tcpechoclient

iperfc: iperfc.c
	gcc -Wall -std=gnu99 -g iperfc.c -o iperfc

iperfs: iperfs.c
	gcc -pthread -Wall -std=gnu99 iperfs.c -o iperfs

clean:
	rm udpechoserver udpechoclient tcpechoserver1 tcpechoserver2 tcpechoclient iperfs iperfc
