/* fake set of low level routines for axis, to make it independent	*
*  of plot library.							*/

#include <stdio.h>
#include <string.h>
#include "axis.h"

/* put a coordinate in the file. Coordinates are short integers */
#define putcoord(i) { putchar(i & 255); putchar((i>>8)&255); }

/* move to a point */
void move(int ix,int iy) { putchar('m'); putcoord(ix); putcoord(iy); }

/* plot a point */
void point(int ix,int iy) { putchar('p'); putcoord(ix); putcoord(iy); }

/* line from here to there */
void line(int ix1,int iy1,int ix2,int iy2) 
{
  putchar('l');putcoord(ix1);putcoord(iy1);
  putcoord(ix2);putcoord(iy2);
}

/* continue to a point */
void cont(int ix,int iy) {putchar('n');putcoord(ix);putcoord(iy);}

/* define plotting space */
void space(int ix1,int iy1,int ix2,int iy2) 
{
  putchar('s');putcoord(ix1);putcoord(iy1);
  putcoord(ix2);putcoord(iy2);
}

/* make a label */
void label(char *s) {putchar('t');puts(s);}

/* change linestyle */
void linemod(char *s) {putchar('f');puts(s);}

/* erase the screen */
void erase() {putchar('e');}

#if 0
/* provide the string index function */
char *index(s,c) char *s,c;{
	while(*s != '\0'){if(*s == c)return(s);s++;}
	return(NULL);}
#endif

