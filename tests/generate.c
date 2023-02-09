#include <stdlib.h>
#include <stdio.h>

char *generate_random_string(int size)
{
  char *str = malloc(size + 1);
  for (int i = 0; i < size; i++)
  {
    str[i] = 'a' + (rand() % 26);
  }
  str[size] = '\0';
  return str;
}

int main()
{
  for (int i = 0; i < 10; i++)
  {
    char *a = generate_random_string(5);
    printf("%s\n", a);
  }
}
