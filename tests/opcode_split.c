#include <stdio.h>
#include <string.h>
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

int main() {
    char *c = "\000\003";
    char d[4];

    memcpy(d, c, 2);
    sprintf(d + 2, "%c%c", 0, 2);
    printf("%hhX\n", d[1]);
}