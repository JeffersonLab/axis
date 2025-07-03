#include <math.h>
#include <stdio.h>
#include "axis.h"

extern float scale;
float	x[20000],y[20000];
int ntext,nsize=20000,npts,bbits[20000];


void ascii(char *ptext,int xorg,int yorg,float size)
{
  int i;
  char c;
  float z;

  z=size*scale;
  i=0;
  while( ptext[i]!='\n' && ptext[i]!='\0')i++;
  ptext[i] = '\0';
  npts = strokes(ptext,x,y,bbits);

  for(i=0;i<npts;i++) {
    if( bbits[i]==0) move(xorg+(int)(z*x[i]),yorg+(int)(z*y[i]));
    else cont(xorg+(int)(z*x[i]),yorg+(int)(z*y[i]));
  }
}

/* right justified string */
void rascii(char *ptext,int xorg,int yorg,float size)
{
  int i;
  char c;
  float z;
  float xmax = -(float)(HUGE_VAL);

  z=size*scale;
  i=0;
  while( ptext[i]!='\n' && ptext[i]!='\0')i++;
  ptext[i] = '\0';
  npts = strokes(ptext,x,y,bbits);
  for(i=0;i<npts;i++) xmax=(xmax>x[i])?xmax:x[i];
  xorg -= z*xmax;

  for(i=0;i<npts;i++) {
    if( bbits[i]==0) move(xorg+(int)(z*x[i]),yorg+(int)(z*y[i]));
    else cont(xorg+(int)(z*x[i]),yorg+(int)(z*y[i]));
  }
}

/* centered text */
void cascii(char *ptext,int xorg,int yorg,float size)
{
  int i;
  char c;
  float z;
  float xmin = (float)(HUGE_VAL);
  float xmax = -(float)(HUGE_VAL);

  z=size*scale;
  i=0;
  while( ptext[i]!='\n' && ptext[i]!='\0')i++;
  ptext[i] = '\0';
  npts = strokes(ptext,x,y,bbits);
  for(i=0;i<npts;i++){
    xmin=(xmin<x[i])?xmin:x[i];
    xmax=(xmax>x[i])?xmax:x[i];
  }
  xorg -= z*(xmax+xmin)/2.;

  for(i=0;i<npts;i++) {
    if( bbits[i]==0) move(xorg+(int)(z*x[i]),yorg+(int)(z*y[i]));
    else cont(xorg+(int)(z*x[i]),yorg+(int)(z*y[i]));
  }
}

/* centered vertical text */
void cvascii(char *ptext,int xorg,int yorg,float size)
{
  int i;
  char c;
  float z,temp;
  float ymin = (float)(HUGE_VAL);
  float ymax = -(float)(HUGE_VAL);

  z=size*scale;
  i=0;
  while( ptext[i]!='\n' && ptext[i]!='\0')i++;
  ptext[i] = '\0';
  npts = strokes(ptext,x,y,bbits);
  for(i=0;i<npts;i++){
    temp = y[i];
    y[i] = x[i];
    x[i] = -temp;
    ymin=(ymin<y[i])?ymin:y[i];
    ymax=(ymax>y[i])?ymax:y[i];
  }
  yorg -= z*(ymax+ymin)/2.;

  for(i=0;i<npts;i++) {
    if( bbits[i]==0) move(xorg+(int)(z*x[i]),yorg+(int)(z*y[i]));
    else cont(xorg+(int)(z*x[i]),yorg+(int)(z*y[i]));
  }
}
