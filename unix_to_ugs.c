/* convert a character string containing troff-like escapes to		*
*  the extended UGS character set.  Escapes are backslashes followed	*
*  by two letters. "\\" is a real backslash.  With few exceptions	*
*  the escape sequences resemble troff escapes without the '('.		*
*  Return length of the result string.					*
*  Conversion takes place in situ. (Input string is always longer than	*
*  output.)								*/
#include <stdio.h>

#define TSIZE 103 
short unix_tbl[TSIZE] = {
				/* Plotting symbols */
  0x706c,0x6372,0x6469,	/* \pl = plus, \cr = cross, \di = diamond */
  0x7371,0x6664,0x6673,	/* \sq=square,\fd=fancy dia.,\fs=fancy square*/
  0x6670,0x6678,0x6275,	/* \fp=fancy plus,\fx=fancy cross,\bu=burst */
  0x6f63,			/* \oc = octagon */
				/* Greek letters (both cases) */
  0x6761,0x6762,0x6767,	/* \ga = alpha, \gb = beta, \gg = gamma */
  0x6764,0x6765,0x677a,	/* \gd = delta, \ge = epsilon, \gz = zeta */
  0x6779,0x6768,0x6769,	/* \gy = eta, \gh = theta, \gi = iota	*/
  0x676b,0x676c,0x676d,	/* \gk = kappa, \gl = lambda, \gm = mu	*/
  0x676e,0x6763,0x676f,	/* \gn = nu, \gc = xi, \go = omicron	*/
  0x6770,0x6772,0x6773,	/* \gp = pi, \gr = rho, \gs = sigma	*/
  0x6774,0x6775,0x6766,	/* \gt = tau, \gu = upsilon, \gf = phi	*/
  0x6778,0x6771,0x6777,	/* \gx = chi, \gq = psi, \gw = omega	*/
				/* Control characters */
  0x7370,0x6570,		/* \sp = start superscript, \ep = end it */
  0x7362,0x6562,		/* \sb = start subscript, \eb = end it	*/
  0x6d6b,0x7274,		/* \mk = mark location, \rt = return */
				/* Unknown control characters */
  0x5833,0x5834,		/* \x3 = ugs 96, \x4 = ugs 97	*/
  0x5835,0x5836,		/* \x5 = ugs 98, \x6 = ugs 99	*/
  0x5837,0x5838,		/* \x6 = ugs 9a, \x8 = ugs 9b	*/
  0x5839,0x5861,		/* \x9 = ugs 9c, \xa = ugs 9d	*/
  0x5862,0x5863,		/* \xb = ugs 9e, \xc = ugs 9f	*/
  0x5864,0x5865,		/* \xd = ugs aa, \xe = ugs ab	*/
  0x5866,0x5867,		/* \xf = ugs ac, \xg = ugs ad	*/
  0x5868,0x5869,		/* \xh = ugs ae, \xi = ugs af	*/
  0x5870,0x5871,		/* \xj = ugs f9, \xk = ugs fa	*/
  0x5872,0x5873,		/* \xl = ugs fb, \xm = ugs fc	*/
  0x5874,0x5875,		/* \xn = ugs fd, \xo = ugs fe	*/
  0x5876,			/* \xp = ugs e0,		*/
				/* Special symbols */
  0x7074,0x6973,		/* \pt=proportional to, \is=integral sign */
  0x746d,0x6476,		/* \tm="times", \dv = "divide"	*/
  0x7e3d,0x7064,		/* \~= = "approx", \pd = "partial" */
  0x6361,0x6962,		/* \ca = "cap"(intersection),\ib=interabang*/
  0x6c61,0x7261,		/* \la = "left angle", \ra = "right angle" */
  0x6464,0x2b2d,		/* \dd="double dagger", \+- = "plus-minus" */
  0x3c3d,0x7363,		/* \<= = "<=", \sc = "section"		*/
  0x6862,0x6c62,		/* \hb = "h-bar", \lb = "lambda-bar"	*/
  0x6375,0x6465,		/* \cu="cup"(union), \de = "degree" */
  0x646c,0x6461,		/* \dl="del"(gradient), \da = "down arrow" */
  0x3c2d,0x2d3e,		/* \<- = "left arrow", \-> = "right arrow" */
  0x3c3e,0x6c69,        /* \<> = "double arrow", \li = "line integral" */
  0x6d6f,0x6e6d,		/* \mo = "member of", \nm = "not member" */
  0x7372,0x756c,		/* \sr = "square root", \ul = "underline" */
  0x726e,0x6578,		/* \rn = "overbar", \ex = "exists" */
  0x3e3d,0x616c,		/* \>= = ">=", \al = "for all" */
  0x7362,0x6473,		/* \sb = "subset", \ds = "direct sum"(xor) */
  0x6470,0x6467,		/* \dp = "direct product", \dg = "dagger" */
  0x6e3d,0x7370,		/* \n= = "not equal", \sp = "superset" */
  0x696e,0x7561,		/* \in = "infinity", \ua = "up arrow" */
};
char ugs_tbl[TSIZE] = {
  0xb0,0xb1,0xb2,		/* Plotting symbols */
  0xb3,0xb4,0xb5,
  0xb6,0xb7,0xb8,
  0xb9,
  0xe1,0xe2,0xe3,		/* Greek */
  0xe4,0xe5,0xe6,
  0xe7,0xe8,0xe9,
  0xea,0xeb,0xec,
  0xed,0xee,0xef,
  0xf0,0xf1,0xf2,
  0xf3,0xf4,0xf5,
  0xf6,0xf7,0xf8,
  0x90,0x91,	/* Control characters */
  0x92,0x93,
  0x94,0x95,
  /* Unknown controls */
  0x96,0x97,	/* 6C, 7C */
  0x98,0x99,	/* 8C, 9C */
  0x9a,0x9b,	/* ?, ? */
  0x9c,0x9d,	/* ?, ? */
  0x9e,0x9f,	/* EC, FC */
  0xaa,0xab,	/* ?, ? */
  0xac,0xad,	/* ?, ? */
  0xae,0xaf,	/* ?, ? */
  0xf9,0xfa,	/* 1V, 2V */
  0xfb,0xfc,	/* 3V, 4V */
  0xfd,0xfe,	/* 5V, 6V */
  0xe0,		/* ? */
  0x80,0x81,	/* Special symbols */
  0x82,0x83,
  0x84,0x85,
  0x86,0x87,
  0x88,0x89,
  0x8a,0x8b,
  0x8c,0x8d,
  0x8e,0x8f,
  0xa0,0xa1,
  0xa2,0xa3,
  0xa4,0xa5,
  0xa6,0xa7,
  0xa8,0xa9,
  0xaa,0xab,
  0xac,0xad,
  0xae,0xaf,
  0xbb,0xbb,
  0xbc,0xbd,
  0xbe,0xbf,
  0xff,0x60,
};

int unix_to_ugs(char *src)
{
  char *dest;
  register int i,j,c,d;
  int uc_flag;	/* flag upper case greek */
  j=0;
  dest = src;
  while((c = *(src++)) != '\0'){
    uc_flag=0;
    if(c != '\\') dest[j++] = c;	/* Ascii character, don't mess */
    else{
      c = *(src++);
      if(c == '\\'){			/* A real backslash */
	dest[j++] = '\\';
	continue;
      }
      if(c == 'g' || c == 'G' || c == '*'){	/* Greek character */
	/* only put greek alphabet in table once */
	c = 'g';
	d = *(src++);
	if(d < 'a'){uc_flag=1;d += 'a'-'A';}	/* upper case */
	c = (c<<8)|d;
      }
      else c = (c<<8)|(*(src++));
      for(i=0;i<TSIZE;i++)if(unix_tbl[i] == c)break;
      if(i == TSIZE){
	fprintf(stderr,"Unknown escape sequence\n");
	continue;
      }
      dest[j++] = uc_flag ? ugs_tbl[i] - ('a'-'A') : ugs_tbl[i];
    }
  }			/* close while loop */
  dest[j] = '\0';	/* terminate string */
  return(j);
}
