#include <arpa/inet.h> // htonl() & htons()
#include <sys/socket.h> //socket(), sendto(), setsockopt()
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#define SIZE_MAGIC_PACKET 102
#define WOL_PORT 9

char pack[SIZE_MAGIC_PACKET];

int
main(int argc, char** argv)
{
	if(argc < 2 || argc > 3) {
		printf("%s [mac_address] [ip]\n", argv[0]);
		return 1;
	}

    in_addr_t dest;
    if(argc == 3) {
        dest = inet_addr(argv[2]);
        if(dest == INADDR_NONE) {
            printf("invalid ip address ip");
            return 1;
        }
    } else {
        dest = htonl(INADDR_BROADCAST);
    }

	int i,j; // avoid compilation with std=
    char mac[6];
    sscanf(argv[1], "%2x:%2x:%2x:%2x:%2x:%2x", (unsigned int*) &mac[0], (unsigned int*) &mac[1], (unsigned int*) &mac[2], (unsigned int*) &mac[3], (unsigned int*) &mac[4], (unsigned int*) &mac[5]);

    // magic packet begins with 0xFFFFFFFFFFFF
    for(i = 0; i < 6; i++) {
    	pack[i] = 255;
    }

    char* packtmp = &pack[6];
    for(i = 0; i < 16; i++) {
    	memcpy(packtmp, mac, 6);
    	packtmp += 6;
    }

#ifdef DEBUG
	printf("## magic packet ##\n");


	// display the magic packet
    packtmp = &pack[0];
	for(i = 0; i < 6; i++) {
		printf("%02hhX", *packtmp);
		packtmp++;
	}
	printf("\n");

    for(i = 0; i < 16; i++) {
    	for(j = 0; j < 5; j++) {
    		printf("%02hhx:", *packtmp);
    		packtmp++;
    	}
    	printf("%02hhx", *packtmp);
    	packtmp++;

    	printf("\n");
    }
    printf("###### end ######\n");
#endif

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock  == -1) {
        close(sock);
        fprintf(stderr, "socket error");
        return 1;
    }

	int iBroadcast = 1;
	int rc = setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &iBroadcast, sizeof(iBroadcast));
    if(rc  == -1) {
        close(sock);
        fprintf(stderr, "setsockopt error");
        return 1;
    }

    struct sockaddr_in dest_addr;
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(WOL_PORT);
    dest_addr.sin_addr.s_addr = dest;
    memset(dest_addr.sin_zero, '\0', sizeof(dest_addr.sin_zero));

	rc = sendto(sock, pack, sizeof(pack), 0, (struct sockaddr *) &dest_addr, sizeof(dest_addr));
    if(rc == -1) {
    	fprintf(stderr, "sendto error");
        close(sock);
        return 1;
    }

    close(sock);
    return 0;
}
