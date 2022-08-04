#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "type.h"
#include "error_func.h"
#include "sockf.h"
#include "send.h"
#include "ip.h"
#include "icmp.h"
#include "tcp.h"
#include "udp.h"

#define BUFF_SIZE 4096

unsigned int ip_src, ip_dst;
unsigned char ttl,  protocol, type, tcp_flag;
unsigned short port_src, port_dst;

void inject()
{
	int sockfd;
	char buffer[BUFF_SIZE];
	struct sockaddr_in sock_dst;
	memset(&sock_dst, 0, sizeof(struct sockaddr_in));

	sockfd = init_socket();
	sock_dst.sin_family = AF_INET;
	sock_dst.sin_addr.s_addr = ip_dst;
	if (protocol == IPPROTO_TCP || protocol == IPPROTO_UDP)
		sock_dst.sin_port = port_dst;

	struct ip_hdr *iph = (struct ip_hdr *)buffer;
	set_ip(iph, ip_src, ip_dst, ttl, protocol);

	if (protocol == IPPROTO_ICMP) {
		struct icmp_hdr *icmph = (struct icmp_hdr *)
			(buffer + sizeof(struct ip_hdr));
		set_icmp(icmph, type, 0, 1);
	}
	if (protocol ==  IPPROTO_TCP) {
		struct tcp_hdr *tcph = (struct tcp_hdr *)
			(buffer + sizeof(struct ip_hdr));
		set_tcp(tcph, iph, port_src, port_dst, tcp_flag, 1, 1);
	}
	if (protocol == IPPROTO_UDP) {
               struct udp_hdr *udph = (struct udp_hdr *)
                       (buffer + sizeof(struct ip_hdr));
               set_udp(udph, iph, port_src, port_dst);
	}

	send_data(sockfd, buffer, iph->length, &sock_dst);
}


void parser(int argc, char *argv[])
{
	int opt;
	char *usage="\n usage : ./inject [protocol] [options]\n\n\
 protocol :\n\
 \tip : ip packets\n\
 \ticmp : icmp packets\n\
 \ttcp : tcp packets\n\
 \tudp : udp packets\n\n\
 options :\n\
 \t-s : source ip address\n\
 \t-d : destination ip address\n\
 \t-l : ttl\n\n\
 \t-t : icmp type\n\n\
 \t-o : source port\n\
 \t-p : destination port\n\
 \t-f : tcp flag\n\n\
 \t-h : this help message\n\n\
 examples :\n\
 \t sending echo request :\n\
 \t./inject icmp -s 192.168.1.50 -d 192.168.1.1 -t 8\n\n\
 \t sending syn packet :\n\
 \t./inject tcp -s 192.168.1.50 -d 192.168.1.1 -o 5000 -p 80 -f 2\n\n\
 \t sending udp packet :\n\
 \t./inject udp -s 192.168.1.50 -d 192.168.1.1 -o 5000 -p 4000\n\n";

	if (argc < 2) {
		printf("%s", usage);
		exit(EXIT_FAILURE);
	}

	if (!strcmp(argv[1], "ip"))
		protocol = IPPROTO_RAW;
	if (!strcmp(argv[1], "icmp"))
		protocol = IPPROTO_ICMP;
	if (!strcmp(argv[1], "tcp"))
		protocol = IPPROTO_TCP;
	if (!strcmp(argv[1], "udp"))
		protocol = IPPROTO_UDP;

	while ((opt = getopt(argc, argv, "s:d:l:t:o:p:f:h")) != -1) {
		switch (opt) {
		case 's':
			ip_src = inet_addr(optarg);
			break;
		case 'd':
			ip_dst = inet_addr(optarg);
			break;
		case 'l':
			ttl = atoi(optarg);
			break;
		case 't':
			type = atoi(optarg);
			break;
		case 'o':
			port_src = atoi(optarg);
			break;
		case 'p':
			port_dst = atoi(optarg);
			break;
		case 'f':
			tcp_flag = atoi(optarg);
			break;
		case 'h':
			printf("%s", usage);
			exit(EXIT_SUCCESS);
		case '?':
			break;
		}
	}
}

int main(int argc, char *argv[])
{
	parser(argc, argv);
	inject();

	return 0;
}
