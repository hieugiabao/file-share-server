#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

uint16_t checksum_(uint16_t len_udp,
                   int padding, const uint16_t *temp)
{
  uint16_t padd = 0;
  uint16_t word16;
  uint64_t sum;
  static u_char buff[65536];
  int i;
  memset(buff, 0, 65536);
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
  printf("Sum: %d\n", sum);
  sum = ~sum;

  return ((uint16_t)sum);
}

int main()
{
  char *a = calloc(5, 1);
  strcpy(a, "abc");
  uint16_t b = checksum_(5, 1, (uint16_t *)a);
  // add check sum to a
  uint16_t c = htons(b);
  memcpy(a + 3, &b, 2);
  assert(checksum_(5, 1, (uint16_t *)a) == 0);
}
