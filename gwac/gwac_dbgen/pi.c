#include<stdio.h>
void main()
{
int i,j=1;
double pi=0;
for(i=1;i<1e8;i=i+2) /*这里的精度自己取*/
 {
pi=pi+j*(1.0/i);
j=-j;
 }
pi=pi*4;
printf("%lf\n",pi);
}

