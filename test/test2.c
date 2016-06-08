#include <stdio.h>


int set_ipv4_config_bin(const short *ip, const short *netmask, const short *gateway) {
	char buf[512];
	printf("aaa\n");
	sprintf(buf, "ifconfig eth1 %hd.%hd.%hd.%hd "
			"netmask %hd.%hd.%hd.%hd;"
			"route add default gw %hd.%hd.%hd.%hd"
			,*ip, *(ip + 1), *(ip + 2), *(ip + 3)
			,*(netmask), *(netmask + 1), *(netmask + 2), *(netmask + 3)
			,*(gateway), *(gateway + 1), *(gateway + 2), *(gateway + 3));
//	printf("%s\n", buf);
	int rc = -1;
	rc = system(buf);
	return rc;
}

int set_ipv6_config_bin(const short *ip, const short *netmask, const short *gateway) {
	char buf[512];
	sprintf(buf, "ifconfig eth1 down;ifconfig eth1 up;ip -f inet6 address add %hd:%hd:%hd:%hd:%hd:%hd:%hd:%hd/%hd dev eth1;"
			"route -A inet6 add default gw %hd:%hd:%hd:%hd:%hd:%hd:%hd:%hd"
			,*ip, *(ip + 1), *(ip + 2), *(ip + 3) , *(ip + 4), *(ip + 5), *(ip + 6), *(ip + 7)
			,*(netmask)
			,*(gateway), *(gateway + 1), *(gateway + 2), *(gateway + 3), *(gateway + 4), *(gateway + 5), *(gateway + 6), *(gateway + 7));
//	printf("%s\n", buf);
	int rc = -1;
	rc = system(buf);
	return rc;
}


int main() {
//    short a[] = {192, 168, 31, 131, 255, 255, 255, 255, 192, 168, 31, 1};
//    set_ipv4_config_bin(a, a + 4, a + 8);

    short a0[] = {2001, 0, 0, 0, 0, 0, 0, 4, 64, 2001, 0, 0, 0, 0, 0, 0, 9};
    set_ipv6_config_bin(a0, a0 + 8, a0 + 9);
    printf("main\n");
    return 0;
}
