#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#ifdef HAVE_SYS_SOCKIO_H
#include <sys/sockio.h>
#endif

#ifdef HAVE_NETINET_IN6_VAR_H
#include <netinet/in6_var.h>
#endif

#ifndef max
#define max(a, b) ((a)>(b)?(a):(b))
#endif

#include "ifaddrss.h"

static int getifaddrss2(struct ifaddrs **ifap, int af, int siocgifconf, int siocgifflags, size_t ifreq_sz){
    int ret;
    int fd;
    size_t buf_size;
    char *buf;
    struct ifconf ifconf;
    //int num;//, j = 0;
    char *p;
    size_t sz;
    struct sockaddr sa_zero;
    struct ifreq *ifr;

    struct ifaddrs *start, **end = &start;
    buf = NULL;
    memset(&sa_zero, 0, sizeof(sa_zero));
    fd = socket(af, SOCK_DGRAM, 0);
    if(fd < 0) return -1;

    buf_size = 8192;
    for(;;){
        buf = calloc(1, buf_size);
        if(buf == NULL){
            ret = ENOMEM;
            goto error_out;
        }
        ifconf.ifc_len = buf_size;
        ifconf.ifc_buf = buf;
        if(ioctl(fd, siocgifconf, &ifconf) < 0 && errno != EINVAL){
            ret = errno;
            goto error_out;
        }
        if(ifconf.ifc_len < (int)buf_size) break;
        free(buf);
        buf_size *= 2;
    }

    //num = ifconf.ifc_len / ifreq_sz;
    //j = 0;
    for(p = ifconf.ifc_buf; p < ifconf.ifc_buf + ifconf.ifc_len; p += sz){
        struct ifreq ifreq;
        struct sockaddr *sa;
        size_t salen;
        ifr = (struct ifreq *)p;
        sa = &ifr->ifr_addr;

        sz = ifreq_sz;
        salen = sizeof(struct sockaddr);
#ifdef HAVE_STRUCT_SOCKADDR_SA_LEN
        salen = sa->sa_len;
        sz = max(sz, sizeof(ifr->ifr_name) + sa->sa_len);
#endif
#ifdef SA_LEN
        salen = SA_LEN(sa);
        sz = max(sz, sizeof(ifr->ifr_name) + SA_LEN(sa));
#endif
        memset(&ifreq, 0, sizeof(ifreq));
        memcpy(ifreq.ifr_name, ifr->ifr_name, sizeof(ifr->ifr_name));
        if(ioctl(fd, siocgifflags, &ifreq) < 0){
            ret = errno;
            goto error_out;
        }

        *end = malloc(sizeof(**end));

        (*end)->ifa_next = NULL;
        (*end)->ifa_name = strdup(ifr->ifr_name);
        (*end)->ifa_flags = ifreq.ifr_flags;
        (*end)->ifa_addr = malloc(salen);
        memcpy((*end)->ifa_addr, sa, salen);
        (*end)->ifa_netmask = NULL;
#if 0
        if(ifreq.ifr_flags & IFF_BROADCAST){
            (*end)->ifa_broadaddr = malloc(sizeof(ifr->ifr_broadaddr));
            memcpy((*end)->ifa_broadaddr, &ifr->ifr_broadaddr, sizeof(ifr->ifr_broadaddr));
        } else if(ifreq.ifr_flags & IFF_POINTOPOINT){
            (*end)->ifa_dstaddr = malloc(sizeof(ifr->ifr_dstaddr));
            memcpy((*end)->ifa_dstaddr, &ifr->ifr_dstaddr, sizeof(ifr->ifr_dstaddr));
        }else{
            (*end)->ifa_dstaddr = NULL;
        }
#else
        (*end)->ifa_dstaddr = NULL;
#endif
        (*end)->ifa_data = NULL;
        end = &(*end)->ifa_next;
    }
    *ifap = start;
    close(fd);
    free(buf);
    return 0;
error_out:
    close(fd);
    free(buf);
    errno = ret;
    return - 1;
}

int getifaddrs(struct ifaddrs **ifap){
    int ret = -1;
    errno = ENXIO;

#if defined(AF_INET6) && defined(SIOCGIF6CONF) && defined(SIOCGIF6FLAGS)
    if(ret) ret = getifaddrss2(ifap, AF_INET6, SIOCGIF6CONF, SIOCGIF6FLAGS, sizeof(struct in6_ifreq));
#endif

#if defined(AF_INET) && defined(SIOCGIFCONF) && defined(SIOCGIFFLAGS)
    if(ret) ret = getifaddrss2(ifap, AF_INET, SIOCGIFCONF, SIOCGIFFLAGS, sizeof(struct ifreq));
#endif
    return ret;
}

void freeifaddrs(struct ifaddrs *ifp){
    struct ifaddrs *p, *q;

    for(p = ifp; p; ){
        free(p->ifa_name);
        if(p->ifa_addr) free(p->ifa_addr);
        if(p->ifa_dstaddr) free(p->ifa_dstaddr);
        if(p->ifa_netmask) free(p->ifa_netmask);
        if(p->ifa_data) free(p->ifa_data);
        q = p;
        p = p->ifa_next;
        free(q);
    }
}
