#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>

#define NTP_SERVER "pool.ntp.org"
#define NTP_PORT 123
#define NTP_TIMESTAMP_DELTA 2208988800ULL

typedef struct {
    uint8_t li_vn_mode;
    uint8_t stratum;
    uint8_t poll;
    uint8_t precision;
    uint32_t root_delay;
    uint32_t root_dispersion;
    uint32_t ref_id;
    uint32_t ref_ts_sec;
    uint32_t ref_ts_frac;
    uint32_t orig_ts_sec;
    uint32_t orig_ts_frac;
    uint32_t recv_ts_sec;
    uint32_t recv_ts_frac;
    uint32_t trans_ts_sec;
    uint32_t trans_ts_frac;
} ntp_packet;

int main() {
    int sockfd;
    struct sockaddr_in server_addr;
    ntp_packet packet = {0};
    ssize_t n;

    // Create UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd < 0) {
        perror("socket");
        return 1;
    }

    // Resolve server address
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;
    if (getaddrinfo(NTP_SERVER, "123", &hints, &res) != 0) {
        perror("getaddrinfo");
        close(sockfd);
        return 1;
    }
    server_addr = *(struct sockaddr_in *)res->ai_addr;
    freeaddrinfo(res);

    // Initialize NTP packet
    packet.li_vn_mode = 0x1b;  // LI=0, VN=3, Mode=3 (client)

    // Send packet
    n = sendto(sockfd, &packet, sizeof(packet), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (n < 0) {
        perror("sendto");
        close(sockfd);
        return 1;
    }

    // Receive response
    n = recvfrom(sockfd, &packet, sizeof(packet), 0, NULL, NULL);
    if (n < 0) {
        perror("recvfrom");
        close(sockfd);
        return 1;
    }

    close(sockfd);

    // Convert NTP time to Unix time
    uint32_t ntp_sec = ntohl(packet.trans_ts_sec);
    time_t unix_time = (time_t)(ntp_sec - NTP_TIMESTAMP_DELTA);

    // Set system time
    struct timeval tv;
    tv.tv_sec = unix_time;
    tv.tv_usec = 0;
    if (settimeofday(&tv, NULL) < 0) {
        perror("settimeofday");
        return 1;
    }

    printf("System time updated successfully.\n");
    return 0;
}