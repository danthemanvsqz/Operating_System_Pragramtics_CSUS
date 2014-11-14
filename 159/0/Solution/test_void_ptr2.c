// test_void_ptr2.c

#include <stdio.h>

void Zero(void *p[], int size)
{
   int i, *q;

   for(i=0;i<size;i++)
   {
      printf("p[%d] is %d.\n", i, p[i]);
      q = (int *)p[i];
      printf("q is %d.\n", q);
//      *q = 65; // would segfault
   }
}

main()
{
   int i, j;
   char us[5][4] = { 'a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a'};

   Zero((void *)us[4], 4);

   for(i=0;i<5;i++)
      for(j=0;j<4;j++)
         printf("us[%d][%d] is %c\n", i, j, us[i][j]);
}


