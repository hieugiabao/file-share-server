#ifndef __TFTP_CHECKSUM__
#define __TFTP_CHECKSUM__

#include <string.h>
#include <stdint.h>

uint16_t checksum_(uint16_t len_udp, int padding, const uint16_t *tmp);

#endif
