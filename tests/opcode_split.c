#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
int compare_opcode(const uint8_t *op1, const uint8_t *op2, size_t len) {
    for (size_t i = 0; i < len; i++)
    {
        if ((int)op1[i] != (int)op2[i])
            return 0;
    }
    return 1;
}

// int __bytes_to_int(const uint8_t *bytes, size_t len)
// {
// //   printf("len: %d\n", len);
//   int result = 0;
//   for (int i = len - 1; i >= 0; i--)
//   {
//     result += ((int)bytes[i]) * (int)pow(16, (len-i-1) * 2);
//     printf("Id: %d\n", result);
//   }
//   return result;
// }

uint8_t *__int_to_bytes(int num, size_t len)
{
  uint8_t *result = malloc(len);
  for (int i = 0; i < len; i++)
  {
    result[i] = (uint8_t)(num % 256);
    num /= 256;
  }
  return result;
}

int main() {
    int a = 1025;
    uint8_t *b = __int_to_bytes(a, 2);
    printf("%hhX%hhx\n", b[1], b[0]);
}