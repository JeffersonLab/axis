/* subroutine accepts a string as produced by sprintf possibly
 * using g format including e formats and returns a string
 * in the UGS extended character set.
 */

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include "axis.h"

void g_to_ugs(char* c0, char* c1,int digflag)
/* digflag=1 means provide at least one digit */
{
  int i,n;
  char *p;
  *c1 = '\0';
  n = strlen(c0);
  p = index(c0,'e');
  if(p==(char*)0){    /* easy case */
    strcpy(c1,c0);
    return;
  }
  *p = '\0';
  if( !digflag && atof(c0)==0.0){
    /* 0eXX  ->  0 */
    strcpy(c1,"0");
    return;
  }
  if(!digflag && atof(c0)==1.0){
    /*  1eXX  -> 10**XX */
  }
  else if(!digflag && atof(c0)== -1.0){
    /*  -1eXX  -> -10**XX */
    c1[0]='-';
    c1[1]='\0';
  }
  else {
    strcpy(c1,c0);
    n = strlen(c1);
    c1[n] = 0x82;    /* UGS "times" sign */
    c1[n+1] = '\0';
  }

  p++;
  strcat(c1,"10");
  n = strlen(c1);
  c1[n] = 0x90;    /* UGS begin superscript */
  c1[n+1] = '\0';
  sscanf(p,"%d",&i);
  sprintf(p,"%d",i);
  strcat(c1,p);
  return;
}
