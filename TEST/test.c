#include <stdio.h>
#include<string.h> 
int main()
{
   // printf() 中字符串需要引号
   printf("Hello, World!\n");
   char str[10];
   sprintf(str, "%2d-%02d", 2012, 13);
   printf("%s\n", str);

   char scoreStr[12];
   sprintf(scoreStr, "%s%d", "Score:", 10);
   printf("%d\n", strlen(scoreStr));
   return 0;
}