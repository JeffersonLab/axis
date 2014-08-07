/* Originally UNIX "graph" code.					*
*  Modified june 83 to include SLAC UGS whistles and bells.  John	*
*  Richardson and Bob Pearson						*
*  Modified June 84 - new string formats and change options from	*
*  the input data.   Doug Toussaint					*
*  Modified October 89 - to allow removal of x and y numerical labels	*
*   which is useful when wanting to place graphs right next to each 	*
*   other.  New options are 'nxn' and 'nyn' for "no x-numbers", etc.	*/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "axis.h"

struct xy {
  int	xlbf;		/*flag:explicit lower bound*/
  int 	xubf;		/*flag:explicit upper bound*/
  int	xqf;		/*flag:explicit quantum*/
  double (*xf)();		/*transform function, e.g. log*/
  float	xa,xb;		/*scaling coefficients*/
  float	xlb,xub;	/*lower and upper bound*/
  float	xquant;		/*quantum*/
  float	xoff;		/*screen offset fraction*/
  float	xsize;		/*screen fraction*/
  int	xbot,xtop;	/*screen coords of border*/	
} xd,yd;
struct datum {
  float xv;		/* x coordinate */
  float yv;		/* y coordinate */
  float yerr;		/* optional error on y */
  char line_mode;		/* current line mode */
  char err_flag;		/* does data have an error bar */
  char cflag;		/* conect to previous point ? */
  char *symptr;		/* optional plot symbol or label */
  float symsize;		/* size of plot symbol or label */
} *xx;
struct z {
  float lb,ub,quant;
} ;


#define LSIZ 1024		/* size of each label buffer */
char *plabs;			/* pointer to current label  */
int labsleft = 0;		/* space left in allocated buffer */

#define TICK  50
#define TOPX  4050
#define BOTX  800
#define LEFTX 300
#define TOPY  3900
#define BOTY  650
#define CENTERX 2425
#define CENTERY 2275
#define LABELSHIFT 80
#define XLD 120
#define YLL 50
#define YLL2 60
int tick = TICK;
int topx = TOPX;
int botx = BOTX;
int leftx= LEFTX;
int topy = TOPY;
int boty = BOTY;
int labelshift = LABELSHIFT;
int xld = XLD;
int yll = YLL;
int yll2 = YLL2;
float absx;	/* current abscissa if automatic ("a" option) */
float scale = 1.0;		/* overall rescaling factor */

float symsize = 1.5;		/* default symbol size */
float lblsize = 1.5;		/* default label character size */
float titlesize = 2.0;		/* default title character size */
float xtitlesize = 1.5;		/* default x title character size */
float ytitlesize = 1.5;		/* default y title character size */
float titlex = 0.0;
float titley = 0.0;
float xtitlex = 0.0;
float xtitley = 0.0;
float ytitlex = 0.0;
float ytitley = 0.0;
int titlexf = 0;
int titleyf = 0;
int xtitlexf = 0;
int xtitleyf = 0;
int ytitlexf = 0;
int ytitleyf = 0;
#define BSIZ 256
char	linebuf[BSIZ];
char	titlebuf[BSIZ];
char	xtitlebuf[BSIZ];
char	ytitlebuf[BSIZ];
char	*plotsymb;
int	n = 0;		/* no. of data points */
int	erasf = 1;	/* erase screen ? */
int	gridf = 1;	/* grid style */
int	symbf = 0;	/* default plot symbol in effect ? */
int	absf = 0;	/* supply abscissas ? */
int	errf = 0;	/* data has error bars ? */
int	transf = 0;	/* transpose ? */
int	lbflag = 1;	/* break after labels ? */
int	cflag = 1;	/* connect to previous ? */
int	xaxflag = 1;	/* want numerical x-axis labels ? */
int	yaxflag = 1;	/* want numerical y-axis labels ? */
float	dx = 1.0;

char *modes[] = {
  "disconnected",
  "solid",
  "dotted",
  "dotdashed",
  "shortdashed",
  "longdashed"
};
int mode = 1;
char *copystring();

void badarg(void)
{
  fprintf(stderr,"axis: error in arguments\n");
  exit(-1);
}

#if 1
int getline2(char *p)
{	/* get a line from stdin, put in string p */
  int i,c;	/* return length of string, -1 if EOF */
  i = -1;
  while ((c = getchar()) != EOF){
    if ( c == '\n'){p[++i] = '\0'; return(i);}
    else p[++i] = c;
  }
  if(i >= 0) p[++i] = '\0';
  return(i);
}
#endif

double ident(double x)
{
  return(x);
}

float modceil(float f, float t)
{
  t = fabs(t);
  return(ceil(f/t)*t);
}

float modfloor(float f,float t)
{
  t = fabs(t);
  return(floor(f/t)*t);
}


void init(struct xy *p)
{
  p->xlb = (float)(HUGE_VAL);
  p->xub = -(float)(HUGE_VAL);

  p->xf = ident;
}

void setopt(int argc,char *argv[])
{
  char c, *p1, *p2;
  float temp;
  int i,j;

  /* an options line sets a break! */
  cflag = 0;
  /* scan the options */
  for(i=1;i<argc;i++)
  {
    if(argv[i][0] == '-')argv[i]++;
    /* minus signs are optional */

    /* "e" and "e0" options toggle error flag */
    if(strcmp(argv[i],"e") == 0)errf = 1;
    else if(strcmp(argv[i],"e0") == 0) errf = 0;

    /* labels */
    else if(strcmp(argv[i],"lt")==0){    /* title */
      p1 = titlebuf;
      if (++i<argc) {
	p2 = argv[i];
	while ((*p1++ = *p2++));
      }
    }
    else if(strcmp(argv[i],"lx")==0){    /* x label */
      p1 = xtitlebuf;
      if (++i<argc) {
	p2 = argv[i];
	while ((*p1++ = *p2++));
      }
    }
    else if(strcmp(argv[i],"ly")==0){    /* y label */
      p1 = ytitlebuf;
      if (++i<argc) {
	p2 = argv[i];
	while ((*p1++ = *p2++));
      }
    }
    else if(strcmp(argv[i],"ltx")==0){    /* title x location */
      if(sscanf(argv[++i],"%e",&titlex) != 1)badarg();
      titlexf = 1;
    }
    else if(strcmp(argv[i],"lty")==0){    /* title y location */
      if(sscanf(argv[++i],"%e",&titley) != 1)badarg();
      titleyf = 1;
    }
    else if(strcmp(argv[i],"lxx")==0){    /* x label  x loc. */
      if(sscanf(argv[++i],"%e",&xtitlex) != 1)badarg();
      xtitlexf = 1;
    }
    else if(strcmp(argv[i],"lxy")==0){    /* x label  y loc. */
      if(sscanf(argv[++i],"%e",&xtitley) != 1)badarg();
      xtitleyf = 1;
    }
    else if(strcmp(argv[i],"lyx")==0){    /* y label  x loc. */
      if(sscanf(argv[++i],"%e",&ytitlex) != 1)badarg();
      ytitlexf = 1;
    }
    else if(strcmp(argv[i],"lyy")==0){    /* y label  y loc. */
      if(sscanf(argv[++i],"%e",&ytitley) != 1)badarg();
      ytitleyf = 1;
    }
    else if(strcmp(argv[i],"lts")==0){    /* title size */
      if(sscanf(argv[++i],"%e",&titlesize) != 1)badarg();
    }
    else if(strcmp(argv[i],"lxs")==0){    /* x label size */
      if(sscanf(argv[++i],"%e",&xtitlesize) != 1)badarg();
    }
    else if(strcmp(argv[i],"lys")==0){    /* y label size */
      if(sscanf(argv[++i],"%e",&ytitlesize) != 1)badarg();
    }

    else if(strcmp(argv[i],"ls") == 0){    /* scale char. size */
      if(sscanf(argv[++i],"%e",&lblsize) != 1)badarg();
    }

    else if(strcmp(argv[i],"m")==0){    /* line style */
      if(sscanf(argv[++i],"%d",&mode) != 1)badarg();
      if(mode>=sizeof(modes)/sizeof(*modes))
	mode = 1;
    }

    else if(strcmp(argv[i],"a") == 0){ /*automatic abscissas*/
      absf = 1;
      dx = 1.0;
      absx = 1.0;
      if(i+1<argc)
	if(sscanf(argv[i+1],"%e",&dx)!=1)continue;
      i++;
      if(i+1<argc)
	if(sscanf(argv[i+1],"%e",&absx)!=1)continue;
      i++;
      absf = 2;
    }
    else if(strcmp(argv[i],"a0") == 0) absf = 0;

    else if(strcmp(argv[i],"s") == 0)erasf = 0;
    /*save screen, overlay plot*/

    else if(strcmp(argv[i],"g") == 0){
      /*grid style 0 none, 1 ticks, 2 full*/
      if(sscanf(argv[++i],"%d",&gridf)!=1)badarg();
    }

    else if(strcmp(argv[i],"c0") == 0)symbf=0;
    /* turn off plot symbols */

    else if(strcmp(argv[i],"cs") == 0){    /* symbol size */
      if(sscanf(argv[++i],"%e",&symsize) != 1)badarg();
    }

    else if(strcmp(argv[i],"c") == 0){
      /*character(s) for plotting*/
      if(++i >= argc) badarg();
      symbf = 1;
      j = unix_to_ugs(argv[i]);
      plotsymb = copystring(argv[i],j);
    }

    else if(strcmp(argv[i],"t") == 0)transf=1;
    /*transpose*/

    else if(strcmp(argv[i],"lb1") == 0)lbflag=1;
    else if(strcmp(argv[i],"lb0") == 0)lbflag=0;
    /* set or unset breaks after labels */
    else if(strcmp(argv[i],"b") == 0)lbflag=0;
    /* archaic version of lb0, like "graph" */

    else if(strcmp(argv[i],"x") == 0){    /*x limits */
      if(strcmp(argv[i+1],"l")==0){
	i++; xd.xf = log10;
      }
      if(i+1<argc && sscanf(argv[i+1],"%e",&xd.xlb)==1){
	i++; xd.xlbf = 1;
      }
      if(i+1<argc && sscanf(argv[i+1],"%e",&xd.xub)==1){
	i++; xd.xubf = 1;
      }
      if(i+1<argc && sscanf(argv[i+1],"%e",&temp)==1){
	i++; xd.xqf = 1;
	if(xd.xub >= xd.xlb)xd.xquant = fabs(temp);
	else xd.xquant = -fabs(temp);
	/* sign of xquant flags reversed axis */
      }
    }
    else if(strcmp(argv[i],"y") == 0){    /*y limits */
      if(strcmp(argv[i+1],"l")==0){
	i++; yd.xf = log10;
      }
      if(i+1<argc && sscanf(argv[i+1],"%e",&yd.xlb)==1){
	i++; yd.xlbf = 1;
      }
      if(i+1<argc && sscanf(argv[i+1],"%e",&yd.xub)==1){
	i++; yd.xubf = 1;
      }
      if(i+1<argc && sscanf(argv[i+1],"%e",&yd.xquant)==1){
	i++; yd.xqf = 1;
      }
    }

    else if(strcmp(argv[i],"h")==0){    /* fraction of height */
      if(sscanf(argv[++i],"%e",&yd.xsize)!=1)badarg();
    }
    else if(strcmp(argv[i],"w")==0){    /* fraction of width */
      if(sscanf(argv[++i],"%e",&xd.xsize)!=1)badarg();
    }
    else if(strcmp(argv[i],"r")==0){    /* move to right */
      if(sscanf(argv[++i],"%e",&xd.xoff)!=1)badarg();
    }
    else if(strcmp(argv[i],"u")==0){    /* move up */
      if(sscanf(argv[++i],"%e",&yd.xoff)!=1)badarg();
    }
    else if(strcmp(argv[i],"sc")==0){    /* rescale whole graph*/
      if(sscanf(argv[++i],"%e",&scale)!=1)badarg();
      if(scale > 1.0)fprintf(stderr,
			     "Warning: scale > 1, graph may go off page\n");
      tick = TICK*scale;
      topx = CENTERX+(TOPX-CENTERX)*scale;
      botx = CENTERX+(BOTX-CENTERX)*scale;
      leftx= CENTERX+(LEFTX-CENTERX)*scale;
      topy = CENTERY+(TOPY-CENTERY)*scale;
      boty = CENTERY+(BOTY-CENTERY)*scale;
      labelshift = LABELSHIFT*scale;
      xld = XLD*scale;
      yll = YLL*scale;
      yll2 = YLL2*scale;
    }

    else if(strcmp(argv[i],"nxn") == 0)xaxflag=0;
    /* turn off numerical label on x-axis */

    else if(strcmp(argv[i],"nyn") == 0)yaxflag=0;
    /* turn off numerical label on y-axis */

    else badarg();
  }
}

void readin(void) 
{
  char *t,*tt;
  char *index(),*argv[32];
  struct datum *temp;
  int i,qf,argc; 

  for(;;) 
  {
    if(getline2(linebuf) == -1) return;	/* End of file or garbage */

    t = linebuf; while( *t == ' ' || *t == '\t')t++;
    /* find first nonwhite character */
    if(*t == '#'){		/* Control line */
      /* make fake argc and argv, call setopt() again */
      i = 0; qf = 0; argc = 1;
      for(t++; *t != '\0';t++){	/* scan line */
	switch(*t){
	case ' ':	/* replace white space by nulls except */
	case '\t':  /* in quotes. i=0 means next nonwhite is new*/
	  if( !qf ){ *t = '\0'; i=0; }
	  break;
	case '"':	/* start or finish a quotation */
	  if( !qf ) qf = 1;
	  else{
	    *t = '\0';
	    qf = 0;
	  }
	  break;
	default:
	  if(i == 0)argv[argc++] = t;
	  if(argc > 32)break;
	  i = 1;	/* i=1 means in middle of arg. */
	  break;
	}
      }
      setopt(argc,argv);
    }
    else
    {
      /* make a new structure for next value */
      temp = (struct datum *)realloc((char*)xx,
				     (unsigned)(n+1)*sizeof(struct datum));
      if(temp==NULL) return;
      xx = temp;

      /* read in the value */
      if(absf){
	xx[n].xv = absx; absx += dx;
	if(errf){
	  if(sscanf(linebuf,"%f%f",&xx[n].yv,&xx[n].yerr)
	     != 2)continue;
	}
	else{
	  if(sscanf(linebuf,"%f",&xx[n].yv) != 1) continue;
	}
      }
      else{
	if(errf){
	  if(sscanf(linebuf,"%f%f%f",&xx[n].xv,&xx[n].yv,
		    &xx[n].yerr) != 3)continue;
	}
	else{
	  if(sscanf(linebuf,"%f%f",&xx[n].xv,&xx[n].yv) != 2)continue;
	}
      }
      xx[n].symptr = NULL;
      if( (t = index(linebuf,'"')) != NULL){
	if( (tt = index(++t,'"')) != NULL) *tt = '\0';
	i = unix_to_ugs(t);
	xx[n].symptr = copystring(t,i);
	xx[n].symsize = symsize;
      }
      else if(symbf){
	xx[n].symptr = plotsymb;
	xx[n].symsize = symsize;
      }

      /* decide whether point should be connected to previous */
      xx[n].cflag = cflag;
      cflag = 1;
      /* if a label break is set and there is a symbol or
	 an error bar, set break for next point */
      if(lbflag  && (xx[n].symptr != NULL || errf))cflag = 0;

      xx[n].line_mode = mode;
      xx[n].err_flag = errf;
      n++;
    }
  }
}

void transpose(void)
{
  int i;
  float f;
  struct xy t;
  if(!transf) return;
  t = xd; xd = yd; yd = t;
  for(i= 0;i<n;i++) 
  {
    if(xx[i].err_flag)
    {
      fprintf(stderr, "Don't transpose data with errors!\n");
      exit(1);
    }
    f = xx[i].xv; xx[i].xv = xx[i].yv; xx[i].yv = f;
  }
}

char *copystring(char *p, int length)	/* copy a string into the label list */
{
  char *temp;
  int i;

  length++;		/* length of string plus null char */
  if(length > labsleft)
  {		/* need more space */
    plabs = (char *)malloc(LSIZ);
    labsleft = LSIZ;
    if(plabs == NULL)exit(-1);
  }
  temp = plabs;
  for(i=0;i<length;i++) plabs[i] = p[i];
  plabs += length;
  labsleft -= length;
  return(temp);
}

void getxlim(struct xy *p, struct datum *v)
{
  int i;

  if(n==0)
  {
    if(p->xf == log10)
    {
      if(!p->xlbf )p->xlb = 1.0;
      if(!p->xubf )p->xub = 10.0;
    }
    else
    {
      if(!p->xlbf )p->xlb = 0.0;
      if(!p->xubf )p->xub = 1.0;
    }
  }
  else 
    for(i=0;i<n;i++)
    {
      if(!p->xlbf && p->xlb>v[i].xv)
	p->xlb = v[i].xv;
      if(!p->xubf && p->xub<v[i].xv)
	p->xub = v[i].xv;
    }
}

void getylim(struct xy *p, struct datum *v)
{
  int i;
  float t;

  if(n==0)
  {
    if(p->xf == log10)
    {
      if(!p->xlbf )p->xlb = 1.0;
      if(!p->xubf )p->xub = 10.0;
    }
    else
    {
      if(!p->xlbf )p->xlb = 0.0;
      if(!p->xubf )p->xub = 1.0;
    }
  }
  else 
    for(i=0;i<n;i++)
    {
      if(v[i].err_flag && p->xf == log10)
      {
	/***OLD CODE****
	    if(!p->xlbf &&
	    p->xlb>(v[i].yv*(t=exp(-v[i].yerr/v[i].yv))))
	    p->xlb = v[i].yv*t;
	    if(!p->xubf && p->xub<v[i].yv/t)
	    p->xub = v[i].yv/t;
	    ***END OLD CODE***/
	/***NEW CODE *****/
	if(!p->xlbf)
	{
	  t = (v[i].yv>v[i].yerr ? v[i].yv-v[i].yerr : v[i].yv);
	  if( p->xlb > t) p->xlb = t;
	}
	if(!p->xubf && p->xub < v[i].yv+v[i].yerr)
	  p->xub = v[i].yv+v[i].yerr;
	/***END NEW CODE****/
      }
      else if(v[i].err_flag)
      {
	if(!p->xlbf && p->xlb>(v[i].yv - (t=v[i].yerr)))
	  p->xlb = v[i].yv - t;
	if(!p->xubf && p->xub<(v[i].yv + t))
	  p->xub = v[i].yv + t;
      }
      else 
      {
	if(!p->xlbf && p->xlb>v[i].yv)
	  p->xlb = v[i].yv;
	if(!p->xubf && p->xub<v[i].yv)
	  p->xub = v[i].yv;
      }
    }
}

int submark(int *xmark,int *pxn,float x,struct xy *p)
{
  if(1.001*p->xlb < x && .999*p->xub > x){
    xmark[(*pxn)++] = log10(x)*p->xa + p->xb;
    return(1);
  }
  return(0);
}

int setmark(int *xmark,int *lmark,struct xy *p)
{
  int i,decades,xn = 0;
  float x,xl,xu;
  float q;
  decades = (int)(float)log10(fabs(p->xub/p->xquant));
  if(p->xf==log10&&!p->xqf) {
    for(x=p->xquant; x<p->xub; x*=10) {
      lmark[xn]=3*tick;
      submark(xmark,&xn,x,p);
      if(decades < 12){
	for(i=2;i<10;i++){
	  lmark[xn]=tick;
	  submark(xmark,&xn,i*x,p);
	}
      }
    }
  } else {
    xn = 0;
    q = p->xquant;
    if(q>0) {
      xl = modfloor(p->xlb,q);
      xu = modfloor(p->xub-q/10,q/5)+q/10;
    } else {
      xl = modfloor(p->xub,q);
      xu = modfloor(p->xlb+q/10,q/5)-q/10;
    }
    for(i=0,x=xl; x<=xu; i++,x+=fabs(p->xquant)/5.0){
      if(q>0)if(x<=p->xlb||x>=p->xub)continue;
      if(q<0)if(x>=p->xlb||x<=p->xub)continue;
      xmark[xn] = (*p->xf)(x)*p->xa + p->xb;
      if(i%5){lmark[xn++] = tick;}
      else lmark[xn++] = 3*tick;
    }
  }
  return(xn);
}

void setloglim(struct z *zpt,int lbf,int ubf,float lb,float ub)
{
  float r,s,t;

  for(s=1; lb*s<1; s*=10) ;
  for(r=1/s; 10*r<=lb; r*=10) ;
  for(t=1/s; t<ub; t*=10) ;
  zpt->lb = !lbf ? r : lb;
  zpt->ub = !ubf ? t : ub;
  if(ub/lb<100) {
    if(!lbf) {
      if(lb >= 5*zpt->lb)
	zpt->lb *= 5;
      else if(lb >= 2*zpt->lb)
	zpt->lb *= 2;
    }
    if(!ubf) {
      if(ub*5 <= zpt->ub)
	zpt->ub /= 5;
      else if(ub*2 <= zpt->ub)
	zpt->ub /= 2;
    }
  }
  zpt->quant = r;
}

void setlinlim(struct z *zpt,int lbf,int ubf,float xlb,float xub)
{
  float r,s,delta;
  float ub,lb;

 loop:
  ub = xub;
  lb = xlb;
  delta = ub - lb;
  /*scale up by s, a power of 10, so range (delta) exceeds 1*/
  /*find power of 10 quantum, r, such that delta/10<=r<delta*/

  s = 1;
  while(delta*s < 10)s *= 10;
  r=1/s;
  while(10*r < delta)r *= 10;

  /*set r=(1,2,5)*10**n so that 3-5 quanta cover range*/

  if(r>=delta/2)r /= 2;
  else if(r<delta/5)r *= 2;

  zpt->ub = ubf? ub: modceil(ub,r);
  zpt->lb = lbf? lb: modfloor(lb,r);
  if(!lbf && zpt->lb<=r && zpt->lb>0) {
    xlb = 0;
    goto loop;
  }
  else if(!ubf && zpt->ub>=-r && zpt->ub<0) {
    xub = 0;
    goto loop;
  }
  zpt->quant = r;
}

void setlim(struct xy *p)
{
  float t,delta,sign;
  struct z z;
  int nmark,lmark[512],mark[512];
  float lb,ub;
  int lbf,ubf;

  lb = p->xlb;
  ub = p->xub;
  delta = ub-lb;
  if(p->xqf) 
  {    /* user supplied quant */
    if(delta*p->xquant <=0 )
      badarg();
    return;
  }
  sign = 1;
  lbf = p->xlbf;    /* is user supplied lower bound */
  ubf = p->xubf;    /* is user supplied upper bound */
  if(delta < 0) 
  {
    sign = -1;
    t = lb;
    lb = ub;
    ub = t;
    t = lbf;
    lbf = ubf;
    ubf = t;
  }
  else
  { 
    if(delta == 0) 
    {
      if(ub > 0) 
      {
	ub = 2*ub;
	lb = 0;
      } 
      else if(lb < 0) 
      {
	lb = 2*lb;
	ub = 0;
      } 
      else 
      {
	ub = 1;
	lb = -1;
      }
    }
  }
  if(p->xf==log10 && lb>0 && ub>lb) 
  {
    setloglim(&z,lbf,ubf,lb,ub);
    p->xlb = z.lb;
    p->xub = z.ub;
    p->xquant = z.quant;
    if(setmark(mark,lmark,p)<7)
    {
      p->xqf = lbf = ubf = 1;
      lb = z.lb; ub = z.ub;
    }
    else return;
  }
  setlinlim(&z,lbf,ubf,lb,ub);
  if(sign > 0) {
    p->xlb = z.lb;
    p->xub = z.ub;
  } else {
    p->xlb = z.ub;
    p->xub = z.lb;
  }
  p->xquant = sign*z.quant;
}

void xscale(struct xy *p)
{
  float edge;

  setlim(p);
  edge = topx-botx;
  p->xa = p->xsize*edge/((*p->xf)(p->xub) - (*p->xf)(p->xlb));
  p->xbot = botx + edge*p->xoff;
  p->xtop = p->xbot + edge*p->xsize;
  p->xb = p->xbot - (*p->xf)(p->xlb)*p->xa + .5;
}

void yscale(struct xy *p)
{
  float edge;

  setlim(p);
  edge = topy-boty;
  p->xa = p->xsize*edge/((*p->xf)(p->xub) - (*p->xf)(p->xlb));
  p->xbot = boty + edge*p->xoff;
  p->xtop = p->xbot + edge*p->xsize;
  p->xb = p->xbot - (*p->xf)(p->xlb)*p->xa + .5;
}

void axes(void )
{
  int i;
  int lmark[512],mark[512];
  int xn, yn;
  if(gridf==0)
    return;

  if(mode!=1)
  {linemod(modes[1]); mode = 1;}
  /* draw a box */
  line(xd.xbot,yd.xbot,xd.xtop,yd.xbot);
  cont(xd.xtop,yd.xtop);
  cont(xd.xbot,yd.xtop);
  cont(xd.xbot,yd.xbot);

  xn = setmark(mark,lmark,&xd);
  for(i=0; i<xn; i++) {
    if(lmark[i]==3*tick && gridf==2)
      line(mark[i],yd.xbot,mark[i],yd.xtop);
    else {
      line(mark[i],yd.xbot,mark[i],yd.xbot+lmark[i]);
      line(mark[i],yd.xtop-lmark[i],mark[i],yd.xtop);
    }
  }
  yn = setmark(mark,lmark,&yd);
  for(i=0; i<yn; i++) {
    if(lmark[i]==3*tick && gridf==2)
      line(xd.xbot,mark[i],xd.xtop,mark[i]);
    else {
      line(xd.xbot,mark[i],xd.xbot+lmark[i],mark[i]);
      line(xd.xtop-lmark[i],mark[i],xd.xtop,mark[i]);
    }
  }
}

int conv(float xv,struct xy *p,int *ip)
{
  long ix;
  ix = p->xa*(*p->xf)(xv) + p->xb;
  if(ix<p->xbot || ix>p->xtop)
    return(0);
  *ip = ix;
  return(1);
}

void symbol(int ix,int iy,char *ptr,float size)
{
  if(ptr == NULL)
  { 
    if(mode == 0)
      point(ix,iy); 
  }
  else
  {
    ascii( ptr, ix,iy,size);
    move(ix,iy);
  }
}

void num_lbl(char c,char *buf,int mark,int digflag)
{
  char pri[BSIZ];
  int i,nchar;

  g_to_ugs(buf,pri,digflag);

  switch(c){
  case 'x':
    cascii(pri,mark,yd.xbot - (int)(xld*lblsize),
	   lblsize);
    break;
  case 'y':
    i = xd.xbot - yll*lblsize - yll2*strlen(pri)*lblsize;
    leftx = (leftx>i)?i:leftx;
    rascii(pri,xd.xbot-(int)(yll*lblsize),
	   mark,lblsize);
    break;
  }
}

void axlab(char c, struct xy *p)
{
  int i,mark,decades,xn = 0;
  int width,digits,digflag;
  char buf[128],format[10];
  float x,xl,xu;
  float q;

  if(p->xf==log10&&!p->xqf) 
  {
    /* if too large or too small, use scientific notation.
       use p->xquant in lower limit because it will have same
       exponent as lowest label.  Not true for p->xlb. */
    decades = (int)(float)log10(fabs(p->xub/p->xquant));
    decades = decades/8 + 1;	/* decades between labels */
    if(p->xub/p->xlb > 100.0){
      if(p->xub > 10000 || p->xquant < 0.001)
	strcpy(format,"%5.0e");
      else strcpy(format,"%g");
      digflag=0;
    }
    else {
      if(p->xub > 100000 || p->xquant < 0.0001)
	strcpy(format,"%5.0e");
      else strcpy(format,"%g");
      digflag=1;
    }
    i = (int)(float)log10(p->xquant);
    for(x=p->xquant; x<=p->xub; x*=10,i++) {
      if(p->xlb <= x && p->xub >= x && i%decades == 0){
	sprintf(buf,format,x);
	mark = log10(x)*p->xa + p->xb;
	num_lbl(c,buf,mark,digflag);
      }
      if(p->xub/p->xlb<=100) {
	if(p->xlb <= 2*x && p->xub >= 2*x){
	  sprintf(buf,format,2*x);
	  mark = log10(2*x)*p->xa + p->xb;
	  num_lbl(c,buf,mark,digflag);
	}
	if(p->xlb <= 5*x && p->xub >= 5*x){
	  sprintf(buf,format,5*x);
	  mark = log10(5*x)*p->xa + p->xb;
	  num_lbl(c,buf,mark,digflag);
	}
      }
    }
  }
  else { /* its linear */
    float scale;
    xn = 0;
    q = p->xquant;
    if(q>0) {
      xl = modceil(p->xlb-q/6,q);
      xu = modfloor(p->xub+q/6,q);
    } else {
      xl = modceil(p->xub+q/6,q);
      xu = modfloor(p->xlb-q/6,q);
    }
    digit(format,&scale,&digflag,
	  xl,xu,(int)((xu-xl)/fabs(q) + 1.5));

    for(x=xl; x <= xu+0.5*fabs(p->xquant); x+=fabs(q)){
      /* WE NEED TO WORRY ABOUT ROUNDOFF HERE. */
      if(q>0)if( x < p->xlb - 1e-5*q || x > p->xub + 1e-5*q )continue;
      if(q<0)if( x > p->xlb - 1e-5*q || x < p->xub - 1e-5*q )continue;
      if(x==0.0)x=0.0;	/* get rid of "-0" on sun 4 */
      sprintf(buf,format,scale*x);
      mark = (*p->xf)(x)*p->xa + p->xb;
      num_lbl(c,buf,mark,digflag);
    }
  }
}

void title(void )
{
  int i,nchar;
  int ix,iy;

  if(*titlebuf){
    nchar = unix_to_ugs(titlebuf);
    ix=titlexf?
      titlex*(xd.xtop-xd.xbot)+xd.xbot:
      (xd.xtop+xd.xbot)/2.;
    iy=titleyf?
      titley*(yd.xtop-yd.xbot)+yd.xbot:
      yd.xtop-4*tick-(int)(titlesize*40);
    cascii(titlebuf,ix,iy,titlesize);
    move(xd.xbot,yd.xbot-100);
  }
  leftx = xd.xbot;
  if(gridf) {
    if(xaxflag) axlab('x',&xd);
    if(yaxflag) axlab('y',&yd);
  }
  if(*xtitlebuf){
    nchar = unix_to_ugs(xtitlebuf);
    ix=xtitlexf?
      xtitlex*(xd.xtop-xd.xbot)+xd.xbot:
      (xd.xtop+xd.xbot)/2.;
    iy=xtitleyf?
      xtitley*(yd.xtop-yd.xbot)+yd.xbot:
      yd.xbot - (int)((lblsize*2 + xtitlesize)*labelshift);
    cascii(xtitlebuf,ix,iy,xtitlesize);
  }
  if(*ytitlebuf){
    nchar = unix_to_ugs(ytitlebuf);
    ix=ytitlexf?
      ytitlex*(xd.xtop-xd.xbot)+xd.xbot:
      leftx - (int)(labelshift*ytitlesize);
    iy=ytitleyf?
      ytitley*(yd.xtop-yd.xbot)+yd.xbot:
      (yd.xtop+yd.xbot)/2;
    cvascii(ytitlebuf,ix,iy,ytitlesize);
  }
}


void plot(void )
{
  int ix,iy,iy1,iy2;
  int jx,jy,jy1,jy2;
  int ix1,ix2;
  int bar,dbar,delta;
  int i;
  int conn;
  float t;

  bar = 3*tick*(n+10)/(3*n+10);
  conn = 0;
  if(mode!=0)
    linemod(modes[mode]);
  for(i=0;i<n;i++){
    conn &= xx[i].cflag;    /* check for break */
    if( xx[i].line_mode != mode){
      mode = xx[i].line_mode;
      linemod(modes[mode]);
    }
    if(xx[i].err_flag){
      jx = conv(xx[i].xv,&xd,&ix);
      jy = conv(xx[i].yv,&yd,&iy);
      if(yd.xf == log10){
	/** original code:
	    jy1 = conv(xx[i].yv*
	    (t=exp(xx[i].yerr/xx[i].yv)),
	    &yd,&iy1);
	    jy2 = conv(xx[i].yv/t,&yd,&iy2);
	**/
	/* Asymmetric error bars on log scale */
	jy1 = conv(xx[i].yv+xx[i].yerr,&yd,&iy1);
	jy2 = (xx[i].yv>xx[i].yerr)?
	  conv(xx[i].yv-xx[i].yerr,&yd,&iy2):0.0;
      }
      else {
	jy1 = conv(xx[i].yv+xx[i].yerr,&yd,&iy1);
	jy2 = conv(xx[i].yv-xx[i].yerr,&yd,&iy2);
      }
      if(jx==0)continue;
      if(jy==0)continue;
      if(conn)cont(ix,iy);    /* connect center of bar */
      if(jy1==0)iy1 = yd.xtop;
      if(jy2==0)iy2 = yd.xbot;
      delta = iy1-iy2;
      delta = delta/5;
      dbar = (bar<delta)?bar:delta;
      ix1=(xd.xbot<ix-dbar)?ix-dbar:xd.xbot;
      ix2=(xd.xtop>ix+dbar)?ix+dbar:xd.xtop;
      if(iy1){move(ix1,iy1);cont(ix2,iy1);}
      if(iy2){move(ix1,iy2);cont(ix2,iy2);}
      move(ix,iy1);
      cont(ix,iy2);
      symbol(ix,iy,xx[i].symptr,xx[i].symsize);
      move(ix,iy);	/* move back to center of bar */
      conn = 1;
      continue;
    }
    else {
      if(!conv(xx[i].xv,&xd,&ix) ||
	 !conv(xx[i].yv,&yd,&iy)) {
	/* point out of bounds */
	conn = 0;
	continue;
      }
      if(mode!=0) {
	if(conn) cont(ix,iy);
	else move(ix,iy);
	conn = 1;
      }
      symbol(ix,iy,xx[i].symptr,xx[i].symsize);
    }
  }
  linemod(modes[1]);
}

int main(int argc,char *argv[])
{
  init(&xd);
  init(&yd);
  xd.xsize = yd.xsize = 1.;
  xx = (struct datum *)malloc((size_t)sizeof(struct datum));
  setopt(argc,argv);
  readin();
  setopt(argc,argv);
  /* do setopt on command line again at end so command line 
     options override.  Must also do it first to get error
     bar flags, etc. */
  transpose();
  getxlim(&xd,xx);
  getylim(&yd,xx);
  xscale(&xd);
  yscale(&yd);
  space(0,0,4096,4096);
  if(erasf)erase();
  plot();
  axes();
  title();
  move(1,1);
  return(0);
}
