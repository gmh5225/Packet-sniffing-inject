#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "type.h"
#include "udp.h"
#include "random.h"
#include "checksum.h"

static unsigned short udp_check(struct udp_hdr *udph, struct ip_hdr *iph)
{
	struct psd_hdr psh;
	char *psd;
	unsigned short check;

	psh.src = iph->src;
	psh.dst = iph->dst;
	psh.placeholder = 0;
	psh.protocol = IPPROTO_UDP;
	psh.length = htons(sizeof(struct udp_hdr));

	psd = malloc(sizeof(struct udp_hdr) + sizeof(struct psd_hdr));
	memcpy(psd, (char *)&psh, sizeof(struct psd_hdr));
	memcpy(psd + sizeof(struct psd_hdr), udph, sizeof(struct udp_hdr));

	check = checksum((unsigned short *)psd,
			 sizeof(struct udp_hdr) + sizeof(struct psd_hdr));
	free(psd);

	return check;
}

void set_udp(char *buffer, unsigned short src, unsigned short dst)
{
	struct ip_hdr *iph = (struct ip_hdr *)buffer;
	struct udp_hdr *udph = (struct udp_hdr *)
		(buffer + sizeof(struct ip_hdr));

	udph->src = (src) ? htons(src) : htons(rand_port());
	udph->dst = (dst) ? htons(dst) : htons(rand_port());
	udph->length = htons(sizeof(struct udp_hdr));
	udph->check = 0;
	udph->check = udp_check(udph, iph);
}
