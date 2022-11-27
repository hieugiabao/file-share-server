#include "checksum.h"

#define BUFF_SIZE 65536

/* Add up checksum, return value will be filled in checksum filed in header */
uint16_t checksum_(uint16_t len_udp,
                   int padding, const uint16_t *temp)
{
  uint16_t padd = 0;
  uint16_t word16;
  uint64_t sum;
  static uint8_t buff[BUFF_SIZE];
  int i;
  memset(buff, 0, BUFF_SIZE);
  memcpy(buff, temp, len_udp);

  // take padding into account
  if ((padding & 1) == 1)
  {
    padd = 1;
    buff[len_udp] = 0;
  }

  // initialize sum to zero
  sum = 0;

  // make 16 bit words out of every two adjacent 8 bit words and
  // calculate the sum of all 16 bit words
  for (i = 0; i < len_udp + padd; i = i + 2)
  {
    word16 = ((buff[i] << 8) & 0xFF00) + (buff[i + 1] & 0xFF);
    // cout << hex << (int) buff[i] << " " << (int) buff[i + 1] << " ";
    sum = sum + (unsigned long)word16;
  }
  while (sum >> 16)
    sum = (sum & 0xFFFF) + (sum >> 16);
  sum = ~sum;

  return ((uint16_t)sum);
}
