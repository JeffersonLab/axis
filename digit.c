/* figure out format for axis labels with linear scale.
   "format" is the format string for sprintf.
   "*scale" is the rescaling factor
   "*digflag" if a flag which means print at least one digit.

   comparisons must be done in single precision because arguments are
   floats.

   assumes xhi > xlo.
   */
#define	max(a,b)	((a)>(b)?(a):(b))
#include <math.h>
#include <stdio.h>
#include "axis.h"

void digit(char *format,float *scale,int *digflag,float xlo,float xhi,int nlab)
{
  double sig,delta,x,y;
  int p,q,width,digits;
    *digflag = 1;	/* except in very special cases */
    x=delta=((double)xhi-(double)xlo)/(nlab-1);
    sig = (1.0e-6)*(max( fabs(xlo),fabs(xhi) ) )/x;
    /* sig is approximate fractional roundoff error on delta */

    for(p= -1; x < 1.0 ; ){
	x *= 10.0; p--;
    }
    while( fabs(round(x) - x) > x*sig ){ 
	x *= 10.0; p--;
    }
    while( fabs(round(x) - x) < x*sig ){
	x /= 10.0; p++;
    }

    y=pow((double)10,(double)(-p));
    q=(int)(1.0+log10(max(
	fabs((double)xlo)*y,fabs((double)xhi)*y
	)+0.1*delta*y));
    /* p is the power of 10 for least significant digit, q is the
       number of significant figures */
    if( p>=4 	/* too many trailing zeroes, use sci. notation */
        || p+q < -3 ){	/* too many leading zeroes, use sci. notation */
	digits = q-1;
	width = q;
	if(q>1)width++;	/* space for decimal */
	if( xlo<0 || xhi < 0 ) (width) ++;	/* allow minus sign */
	*scale=pow((double)10,(double)(-p-q+1));
	if( p>=4 )
	    sprintf(format,"%%%d.%dfe+%2.2d",width,digits,p+q-1);
	else
	    sprintf(format,"%%%d.%dfe-%2.2d",width,digits,1-(p+q));
	/* some special cases don't require printing digit */
	if(nlab==3){
	    if((float)(xlo*y)== -1.0 && (float)(xhi*y)== 1.0)*digflag=0;
	}
	else if(nlab==2){
	    if( ((float)xlo==0.0 || (float)(xlo*y)== -1.0) &&
	        ((float)xhi==0.0 || (float)(xhi*y)==1.0))
		*digflag=0;
	}
    }
    else{	/* used fixed notation */
	if( p >=0 )  {digits =0;
			width =p+q;}
		else {digits= -p;
			width= 1+q+( (-p) >= q )*(1+(-p)-q);
		}
	if( xlo<0 || xhi < 0 ) (width) ++;	/* allow minus sign */
	*scale=1.0;
	sprintf(format,"%%%d.%df",width,digits);
    }
}
