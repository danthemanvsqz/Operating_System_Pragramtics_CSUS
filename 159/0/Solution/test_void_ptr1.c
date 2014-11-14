// test_void_ptr1.c

#include <stdio.h>

void Zero(void *p, int size)
{
   int i;

   for(i=0;i<size;i++) // *p[i]=0;
   printf("   %i  %u\n", i, p++);

//   while(size--)*p++==0;
}

main()
{
   typedef struct { char a, b, c; } x_t;

   int i;
   x_t x;
   int j;

   i = 33;
   j = 44;

   Zero((void *)&x, sizeof(x_t));

   printf("i is %d, j is %d.\n", i, j);
}


