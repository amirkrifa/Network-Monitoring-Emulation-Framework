#include <stdio.h>

int main()
{
FILE * f;
f = fopen("imed.txt", "w");
fprintf(f, "lol");
fclose(f);
system("echo hhg > /root/flowd/ka.txt");
return 0;
}