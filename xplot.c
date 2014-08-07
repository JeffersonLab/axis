/*
**++
**  FACILITY:  XPLOT
**
**  MODULE DESCRIPTION:
**
**      X Window filter for AXIS.
**
**  AUTHORS:
**
**      Weiqiang Liu
**
**  CREATION DATE:   4-MAY-1989
**
**  DESIGN ISSUES:
**
**	The graphics window will disappear if one hits any key while in this
**      window.
**
**
**  MODIFICATION HISTORY:
**
**      Doug Toussaint 29-MAY-1989
**	Tony Kennedy  3-JUN-1991 17:11:30.38 (VMS version)
**	Tony Kennedy  9-MAR-1998 17:57:56.35 Support color.
**      Tony Kennedy 11-MAR-1998 09:59:43.76 Do resource database properly.
**      Tony Kennedy 11-MAR-1998 12:25:35.31 Add -c option to toggle color.
**--
*/

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xresource.h>

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
//#include "unixcrtl.h"

/* For non-ANSI C compilers (thank you, SUN) */

#ifndef EXIT_FAILURE
#define EXIT_FAILURE 1
#endif

#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#endif

/* On Ultrix, Decstation 3100, this icon is in 
   /usr/include/mit/X11/bitmaps/icon, and can just be included
   #include <mit/X11/bitmaps/icon> */

#define icon_width 16
#define icon_height 16
static char icon_bits[] = {
   0xff, 0xff, 0xab, 0xaa, 0x55, 0xd5, 0xab, 0xaa, 0x05, 0xd0, 0x0b, 0xa0,
   0x05, 0xd0, 0x0b, 0xa0, 0x05, 0xd0, 0x0b, 0xa0, 0x05, 0xd0, 0x0b, 0xa0,
   0x55, 0xd5, 0xab, 0xaa, 0x55, 0xd5, 0xff, 0xff};

#define BITMAPDEPTH 1
#define SMALL 1
#define OK 0

void get_opt(int, char**);
void load_font(XFontStruct **);
void getGC(Window, GC*, XFontStruct*);
void TooSmall(Window, GC, XFontStruct*);
void draw_graphics(Window, GC*, XFontStruct*, unsigned, unsigned, char*);

/*	
**  These are used as arguments to nearly every Xlib routine, so it saves
**  routine arguments to declare them global. If there were additional source
**  files, they would be declared extern there.
*/	

Display *display;
int screen;
char *display_name = NULL;
char display_name_space[128];	/* space for constructing display name */
char *colorname, *progname;
int depth, button_press_to_quit = 1, key_press_to_quit = 0, use_color = 0,
  quit = 0, toggle_color = 1;
Colormap cmap;
XColor color;
unsigned long pixel;
XrmDatabase db;

char *getenv();
char* app_class = "XPlot";

char* get_resource_string(char *res_name, char*res_class) {
  XrmValue value;
  char	*type, *p;
  char full_name [1024], full_class [1024];

  strcpy (full_name, progname);
  strcat (full_name, ".");
  strcat (full_name, res_name);
  strcpy (full_class, app_class);
  strcat (full_class, ".");
  strcat (full_class, res_class);
  if (XrmGetResource (db, full_name, full_class, &type, &value)) {
    char* str = (char*) malloc(value.size + 1);
    strncpy (str, (char*) value.addr, value.size);
    str[value.size] = 0;
    return str;
    }
  return 0;
  }

int get_resource_boolean (char *res_name, char *res_class, int def) {
  char *tmp, buf[100];
  char *s = get_resource_string(res_name, res_class);
  if (!s) return def;
  for (tmp = buf; *s; s++)
    *tmp++ = isupper (*s) ? tolower (*s) : *s;
  *tmp = 0;

  if (!strcmp (buf, "on") || !strcmp (buf, "true") || !strcmp (buf, "yes"))
    return 1;
  if (!strcmp (buf,"off") || !strcmp (buf, "false") || !strcmp (buf,"no"))
    return 0;
  fprintf (stderr, "%s: %s must be boolean, not %s\n", progname, res_name, buf);
  return def;
}

int main(int argc, char** argv) {
  Window win;
  unsigned int width, height, x = 0, y = 0;     /* window size and position */
  unsigned int border_width = 4;    /* four pixels */
  unsigned int display_width, display_height;
  unsigned int icon_width_t, icon_height_t;
  char *window_name = "XPlot Window";
  char *icon_name = "";
  Pixmap icon_pixmap;
  XSizeHints size_hints;
  XIconSize *size_list;
  int count,i,j;
  XEvent report;
  KeySym ks;
  char buf[1];
  GC gc[5];
  XFontStruct *font_info;
  int window_size = 0;    /* OK, or too SMALL to display contents */
  XRectangle rectangle;    /* for exposure event processing */
  char *ps, *temp;
  char *xPlotDefaults = "\
    *background:		black\n\
    *solidColor:		white\n\
    *dottedColor:		red\n\
    *shortdashedColor:	blue\n\
    *longdashedColor:		green\n\
    *dotdashedColor:		cyan\n";

/*	
**  Set default display name. This may be changed by get_opt.
**
**  If the environment variable (logical name) DECW$DISPLAY is set then use it,
**  otherwise try DISPLAY.
*/	

  display_name = getenv("DECW$DISPLAY");
  if (display_name == NULL) display_name = getenv("DISPLAY");

/*
**  Get UNIX-style command line options
*/
  progname = argv[0];
#ifdef VMS
  while ((temp = rindex (progname, ']'))) progname = temp + 1;
  if (temp = rindex (progname, '.')) *temp = '\0';
#else
  if ((temp = rindex (progname, '/'))) progname = temp + 1;
#endif
  get_opt (argc, argv);

/*
**  Read in the graphic content
*/
  ps = (char*) malloc(10000 * sizeof(char));
  j = 0;
  while((i = getchar()) != EOF) {
    ps[j++] = (unsigned char) i;
    if(j % 10000 == 0) {
      temp = (char*) realloc(ps,(10000+j)*sizeof(char));
      ps = temp;
      }
    }
  ps[j++] = 'Q';

  /* connect to X server */

  if ((display = XOpenDisplay(display_name)) == NULL) {
    (void) fprintf(stderr, "%s: cannot connect to X server %s\n",
      progname, XDisplayName(display_name));
    exit(EXIT_FAILURE);
    }

/*
**  Find resource data base
*/
  db = XrmGetStringDatabase(xPlotDefaults);

  if ((temp = XResourceManagerString(display)))
  XrmMergeDatabases(XrmGetStringDatabase(temp), &db);

  if (getenv("XENVIRONMENT") &&
    access((char*) getenv("XENVIRONMENT"), R_OK) == 0)
      XrmMergeDatabases(XrmGetFileDatabase((char*) getenv("XENVIRONMENT")),
	&db);
  else {
#ifdef VMS
  char *fn;
  fn = "DECW$SYSTEM_DEFAULTS:XPLOT.DAT";

  if (access(fn, R_OK) == 0) {
#ifdef DEBUG
    fprintf(stderr, "%s: using resource file %s\n", progname, fn);
#endif
    XrmMergeDatabases(XrmGetFileDatabase(fn), &db);
    }
  fn = "DECW$USER_DEFAULTS:XPLOT.DAT";

  if (access (fn, R_OK) == 0) {
#ifdef DEBUG
    fprintf(stderr, "%s: using resource file %s\n", progname, fn);
#endif
    XrmMergeDatabases(XrmGetFileDatabase(fn), &db);
    }

  fn = "DECW$USER_DEFAULTS:DECW$XDEFAULTS.DAT";
  if (access(fn, R_OK) == 0) {
#ifdef DEBUG
    fprintf(stderr, "%s: using resource file %s\n", progname, fn);
#endif
    XrmMergeDatabases(XrmGetFileDatabase(fn), &db);
    }
#else
  char fn[1000];
  sprintf(fn, "%s/.Xdefaults", getenv("HOME"));

  if (access(fn, R_OK) == 0) {
#ifdef DEBUG
    fprintf(stderr, "%s: using resource file %s\n", progname, fn);
#endif
    XrmMergeDatabases(XrmGetFileDatabase(fn), &db);
    }
  {
    char* xapplresdir = getenv("XAPPLRESDIR");
    if (xapplresdir) {
      sprintf(fn, "%s/XPlot", xapplresdir);
      if (access(fn, R_OK) != 0)
	sprintf(fn, "%s/app-defaults/XPlot", getenv("HOME"));
      }
    else sprintf(fn, "%s/app-defaults/XPlot", getenv("HOME"));

    if (access(fn, R_OK) == 0) {
#ifdef DEBUG
      fprintf(stderr, "%s: using resource file %s\n", progname, fn);
#endif
      XrmMergeDatabases(XrmGetFileDatabase(fn), &db);
      }
    }
#endif
  }

/* Get resource information */

  use_color = toggle_color ^ get_resource_boolean("useColor", "Boolean", 0);
  key_press_to_quit = !get_resource_boolean("ignoreKeypress", "Boolean", 1);
  button_press_to_quit =
    !get_resource_boolean("ignoreButtonpress", "Boolean", 0);

/*	
**  Get the screen size from display structure macro
*/	
  screen = DefaultScreen(display);
  display_width = DisplayWidth(display, screen);
  display_height = DisplayHeight(display, screen);
  cmap = DefaultColormap(display, screen);
  depth = DisplayPlanes(display, screen);

/*	
**  Note that x and y are 0, since the default position of the window is the top
**  left corner of the root. This is fine since the window manager often allows
**  the user to position the window before mapping it.
**
**  Size the window with enough room for text.
*/

  width = 2*display_width/3, height = 2*display_height/3;

/*	
**  Create an opaque window.
*/
  win = XCreateSimpleWindow(display, RootWindow(display,screen),
    x, y, width, height, border_width,
    WhitePixel(display, screen), BlackPixel(display, screen));

/*	
**  Get available icon sizes from Window manager.
*/
  if (XGetIconSizes(display, RootWindow(display, screen), &size_list, &count)
      == 0) {
#ifdef VMS
    (void) fprintf(stderr,
	"%s: Window manager didn't set icon sizes - using default.\n",
	progname);
#endif
    icon_width_t = 16;
    icon_height_t = 16;
    }
  else {
    while (icon_width_t < size_list->min_width) {
      icon_width_t = size_list->max_width;
      icon_height_t = size_list->max_height;
      }
    }

/*	
**  Create pixmap of depth 1 (bitmap) for icon.
*/
  icon_pixmap = XCreateBitmapFromData( display, win, icon_bits,
    icon_width, icon_height);

/*	
**  Set resize hints.
*/
  size_hints.flags = PPosition | PSize | PMinSize;
  size_hints.x = x;
  size_hints.y = y;
  size_hints.width = width;
  size_hints.height = height;
  size_hints.min_width = 300;
  size_hints.min_height = 200;

/*	
**  Set properties for window manager (always before mapping).
*/

  XSetStandardProperties(display, win, window_name, icon_name,
    icon_pixmap, argv, argc, &size_hints);

  /* Select event types wanted */
  if (button_press_to_quit) XSelectInput(display, win, 
    ExposureMask | ButtonPressMask | KeyPressMask | StructureNotifyMask);
  else XSelectInput(display, win, 
    ExposureMask | KeyPressMask | StructureNotifyMask);

  load_font(&font_info);

  /* create GC for text and drawing */
  getGC(win, gc, font_info);

  /* Display window */
  XMapWindow(display, win);

  /* printf("%d\n", XDoesBackingStore(XScreenOfDisplay(display,screen))); */

  /* get events, use first to display text and graphics */
  while (1) {
    XNextEvent(display, &report);
#ifdef DEBUG
    printf("processing event...");
#endif
    switch (report.type) {

    case Expose:
#ifdef DEBUG
      printf("Expose\n");
#endif
      /* if this is the last contiguous expose
       * in a group, draw the window */
      if (report.xexpose.count == 0) {
	/* if window too small to use */
	while (XCheckTypedEvent(display, Expose, &report));
	if (window_size == SMALL) TooSmall(win, gc[0], font_info);
	else draw_graphics(win, gc, font_info, width, height, ps);
	}
      break;

    case ConfigureNotify:
#ifdef DEBUG
      printf("ConfigureNotify\n");
#endif
      /* window has been resized, change width and height to send to 
	 place_text and place_graphics in next Expose	*/

      width = report.xconfigure.width;
      height = report.xconfigure.height;
      if ((width < size_hints.min_width) || (height < size_hints.min_height))
	window_size = SMALL;
      else window_size = OK;
      break;

    case ButtonPress:
#ifdef DEBUG
      printf("ButtonPress\n");
#endif
      if (button_press_to_quit) quit = 1;
      /* trickle down into KeyPress (no break) */

    case KeyPress:
#ifdef DEBUG
      printf("KeyPress\n");
#endif
      if (key_press_to_quit) {

	/* Figure out what key was pressed, quit on 'q' or 'Q' */

	count = XLookupString((XKeyEvent*)(&report), buf, 100, &ks, NULL);
	if (count == 1 && (buf[0]=='q' || buf[0]=='Q')) quit = 1;
	else fprintf(stderr, "Use 'q' to quit %s\n", progname);
	}

      if (quit) {
	XUnloadFont(display, font_info->fid);
	for(i = 0; i < 5; i++) XFreeGC(display, gc[i]);
	XCloseDisplay(display);
	exit(EXIT_SUCCESS);
	}

      break;

    default:
#ifdef DEBUG
      printf("Other\n");
#endif
      /* all events selected by StructureNotifyMask except ConfigureNotify are 
	 thrown away here, since nothing is done with them */

      break;
      } /* end switch */
    } /* end while */
  }

void getGC(Window win, GC gc[], XFontStruct *font_info) 
{
  unsigned long valuemask = 0; /* ignore XGCvalues and use defaults */
  XGCValues values;
  unsigned int line_width = 1;
  int line_style;
  int cap_style = CapRound;
  int join_style = JoinRound;
  int dash_offset = 0;
  static char dotted[] = {1, 3};
  static char shortdashed[] = {3, 4};
  static char longdashed[] = {9, 4};
  static char dotdashed[] = {4, 3, 1, 3};

  /* Set window background color */

  if (depth == 1 || !use_color)
    pixel = BlackPixel(display, screen);
  else {
    colorname = get_resource_string("background", "Background");
    colorname = colorname ? colorname : "black";
#ifdef DEBUG
    fprintf(stderr, "%s: background is %s\n", progname, colorname);
#endif
    if (!XParseColor(display, cmap, colorname, &color)) {
      fprintf(stderr, "%s: cannot find color %s\n", progname, colorname);
      exit(EXIT_FAILURE);
      }
    if (!XAllocColor(display, cmap, &color)) {
      fprintf(stderr, "%s: cannot allocate color %s\n", progname, colorname);
      exit(EXIT_FAILURE);
      }
    pixel = color.pixel;
    }
  XSetWindowBackground(display, win, pixel);

  /* solid */

  gc[0] = XCreateGC(display, win, valuemask, &values);
  if (depth == 1 || !use_color)
    pixel = WhitePixel(display, screen);
  else {
    colorname = get_resource_string("solidColor", "Foreground");
    colorname = colorname ? colorname : "white";
#ifdef DEBUG
    fprintf(stderr, "%s: solidColor is %s\n", progname, colorname);
#endif
    if (!XParseColor(display, cmap, colorname, &color)) {
      fprintf(stderr, "%s: cannot find color %s\n", progname, colorname);
      exit(EXIT_FAILURE);
      }
    if (!XAllocColor(display, cmap, &color)) {
      fprintf(stderr, "%s: cannot allocate color %s\n", progname, colorname);
      exit(EXIT_FAILURE);
      }
    pixel = color.pixel;
    }
      
  line_style = LineSolid;
  XSetFont(display, gc[0], font_info->fid);
  XSetForeground(display, gc[0], pixel);
  XSetLineAttributes(display, gc[0], line_width, line_style, cap_style,
      join_style);

  /* dotted */

  gc[1] = XCreateGC(display, win, valuemask, &values);
  if (depth == 1 || !use_color) {
    pixel = WhitePixel(display, screen);
    line_style = LineOnOffDash;
    XSetDashes(display, gc[1], dash_offset, dotted, 2);
    }
  else {
    colorname = get_resource_string("dottedColor", "Foreground");
    colorname = colorname ? colorname : "red";
#ifdef DEBUG
    fprintf(stderr, "%s: dottedColor is %s\n", progname, colorname);
#endif
    if (!XParseColor(display, cmap, colorname, &color)) {
      fprintf(stderr, "%s: cannot find color %s\n", progname, colorname);
      exit(EXIT_FAILURE);
      }
    if (!XAllocColor(display, cmap, &color)) {
      fprintf(stderr, "%s: cannot allocate color %s\n", progname, colorname);
      exit(EXIT_FAILURE);
      }
    pixel = color.pixel;
    line_style = LineSolid;
    }
  XSetFont(display, gc[1], font_info->fid);
  XSetForeground(display, gc[1], pixel);
  XSetLineAttributes(display, gc[1], line_width, line_style, cap_style,
    join_style);

  /* shortdashed */

  gc[2] = XCreateGC(display, win, valuemask, &values);
  if (depth == 1 || !use_color) {
    pixel = WhitePixel(display, screen);
    line_style = LineOnOffDash;
    XSetDashes(display, gc[2], dash_offset, shortdashed, 2);
    }
  else {
    colorname = get_resource_string("shortdashedColor", "Foreground");
    colorname = colorname ? colorname : "blue";
#ifdef DEBUG
    fprintf(stderr, "%s: shortdashedColor is %s\n", progname, colorname);
#endif
    if (!XParseColor(display, cmap, colorname, &color)) {
      fprintf(stderr, "%s: cannot find color %s\n", progname, colorname);
      exit(EXIT_FAILURE);
      }
    if (!XAllocColor(display, cmap, &color)) {
      fprintf(stderr, "%s: cannot allocate color %s\n", progname, colorname);
      exit(EXIT_FAILURE);
      }
    pixel = color.pixel;
    line_style = LineSolid;
    }
  XSetFont(display, gc[2], font_info->fid);
  XSetForeground(display, gc[2], pixel);
  XSetLineAttributes(display, gc[2], line_width, line_style, cap_style,
    join_style);

  /* longdashed */

  gc[3] = XCreateGC(display, win, valuemask, &values);
  if (depth == 1 || !use_color) {
    pixel = WhitePixel(display, screen);
    line_style = LineOnOffDash;
    XSetDashes(display, gc[3], dash_offset, longdashed, 2);
    }
  else {
    colorname = get_resource_string("longdashedColor", "Foreground");
    colorname = colorname ? colorname : "green";
#ifdef DEBUG
    fprintf(stderr, "%s: longdashedColor is %s\n", progname, colorname);
#endif
    if (!XParseColor(display, cmap, colorname, &color)) {
      fprintf(stderr, "%s: cannot find color %s\n", progname, colorname);
      exit(EXIT_FAILURE);
      }
    if (!XAllocColor(display, cmap, &color)) {
      fprintf(stderr, "%s: cannot allocate color %s\n", progname, colorname);
      exit(EXIT_FAILURE);
      }
    pixel = color.pixel;
    line_style = LineSolid;
    }
  XSetFont(display, gc[3], font_info->fid);
  XSetForeground(display, gc[3], pixel);
  XSetLineAttributes(display, gc[3], line_width, line_style, cap_style,
    join_style);

  /* dotdashed */

  gc[4] = XCreateGC(display, win, valuemask, &values);
  if (depth == 1 || !use_color) {
    pixel = WhitePixel(display, screen);
    line_style = LineOnOffDash;
    XSetDashes(display, gc[4], dash_offset, dotdashed, 4);
    }
  else {
    colorname = get_resource_string("dotdashedColor", "Foreground");
    colorname = colorname ? colorname : "cyan";
#ifdef DEBUG
    fprintf(stderr, "%s: dotdashedColor is %s\n", progname, colorname);
#endif
    if (!XParseColor(display, cmap, colorname, &color)) {
      fprintf(stderr, "%s: cannot find color %s\n", progname, colorname);
      exit(EXIT_FAILURE);
      }
    if (!XAllocColor(display, cmap, &color)) {
      fprintf(stderr, "%s: cannot allocate color %s\n", progname, colorname);
      exit(EXIT_FAILURE);
      }
    pixel = color.pixel;
    line_style = LineSolid;
    }
  XSetFont(display, gc[4], font_info->fid);
  XSetForeground(display, gc[4], pixel);
  XSetLineAttributes(display, gc[4], line_width, line_style, cap_style,
    join_style);
  }

void load_font(XFontStruct **font_info) {
//  char *fontname = "*Terminal*18*";
  char *fontname = "9x15";

  /* Access font */
  if ((*font_info = XLoadQueryFont(display,fontname)) == NULL) {
    (void) fprintf(stderr, "%s: Cannot open font\n", progname);
    exit(EXIT_FAILURE);
    }
  }

void TooSmall(Window win, GC tgc, XFontStruct *font_info) {
  char *string1 = "Too Small";
  int y_offset, x_offset;

  y_offset = font_info->max_bounds.ascent + 2;
  x_offset = 2;

  /* output text, centered on each line */
  XDrawString(display, win, tgc, x_offset, y_offset, string1, strlen(string1));
  }

void draw_graphics(Window win, GC gc[], XFontStruct *font_info, 
  unsigned window_width, unsigned window_height, char *pp) 
{
    float getcoord();
    int i, code, slen, swidth, dots, xplot, yplot, xlast, ylast;
    float x, y, xmin, xmax, ymin, ymax, xdel, ydel, xcurrent, ycurrent,
      xscale, yscale;
    char label[128];
    unsigned int line_width = 1;
    int line_style;
    int cap_style = CapRound;
    int j, join_style = JoinRound;
    GC tgc = gc[0];
    char *sgets();

    dots = window_width > window_height ? window_height : window_width;
    xmin = 0.0;
    ymin = 0.0;
    xmax = 1000.0;
    ymax = 1000.0;
    xdel = (xmax - xmin) / dots;
    ydel = (ymax - ymin) / dots;
    xscale = 1.0 / xdel;
    yscale = 1.0 / ydel;

#ifdef DEBUG
    printf("draw_graphics: drawing\n");
#endif

    /* now begin translating input file to Xwindow calls */
    /* 'Q' is fake EOF code */

    j = 0;
    while((code = pp[j++]) != 'Q') {
      switch(code){

      case 'e':
	XClearWindow(display, win);    /* erase screen */
	break;

      case 's':
	xmin = getcoord(pp, &j);    /* set up plotting window */
	ymin = getcoord(pp, &j);
	xmax = getcoord(pp, &j);
	ymax = getcoord(pp, &j);
	xdel = (xmax - xmin) / dots;
	ydel = (ymax - ymin) / dots;
	xscale = 1.0 / xdel;
	yscale = 1.0 / ydel;
	break;

      case 'm':                       /* move current point */
	xcurrent = getcoord(pp, &j);
	ycurrent = getcoord(pp, &j);
	xplot = (xcurrent - xmin) * xscale;
	yplot = (ymax - ycurrent) * yscale;
	break;

      case 'p':                       /* draw current point */
	xcurrent = getcoord(pp, &j);
	ycurrent = getcoord(pp, &j);
	xplot = (xcurrent - xmin) * xscale;
	yplot = (ymax - ycurrent) * yscale;
	XDrawPoint(display, win, tgc, xplot, yplot);
	break;

      case 'l':                       /* plot a line  */
	xcurrent = getcoord(pp, &j);
	ycurrent = getcoord(pp, &j);
	xlast = (xcurrent - xmin) * xscale;
	ylast = (ymax - ycurrent) * yscale;
	xcurrent = getcoord(pp, &j);
	ycurrent = getcoord(pp, &j);
	xplot = (xcurrent - xmin) * xscale;
	yplot = (ymax - ycurrent) * yscale;
	XDrawLine(display, win, tgc, xlast, ylast, xplot, yplot);
	break;

      case 'n':                       /* line from current to next*/
	xcurrent = getcoord(pp, &j);
	ycurrent = getcoord(pp, &j);
	xplot = (xcurrent - xmin) * xscale;
	yplot = (ymax - ycurrent) * yscale;
	XDrawLine(display, win, tgc, xlast, ylast, xplot, yplot);
	break;

      case 't':       /* put a label in the graph */
	sgets(label, pp, &j);
	slen = strlen(label);
	swidth = XTextWidth(font_info, label, slen);
	xplot = (xcurrent - xmin) * xscale;
	yplot = (ymax - ycurrent) * yscale;
	XDrawString(display, win, tgc, xplot, yplot, label, slen);
	xcurrent += swidth / xscale;
	break;

      case 'a':       /* draw an arc */
	fprintf(stderr, "%s: don't know how to draw arcs\n", progname);
	for(i = 0; i < 6; i++) (void)getcoord(pp, &j);	/* flush it */
	break;

      case 'c':       /* draw a circle */
	fprintf(stderr, "%s: don't know how to draw circles\n", progname);
	for(i = 0; i < 3; i++) (void)getcoord(pp,&j); /* flush it */
	break;

      case 'f':       /* change linestyle */
	sgets(label, pp, &j);
	if (strcmp(label, "solid") == 0) tgc = gc[0];
	else if (strcmp(label, "dotted") == 0) tgc = gc[1];
	else if (strcmp(label, "shortdashed") == 0) tgc = gc[2];
	else if (strcmp(label, "longdashed") == 0) tgc = gc[3];
	else if (strcmp(label, "dotdashed") == 0) tgc = gc[4];
	break;

      default:
	fprintf(stderr, "%s: unknown code = %c\n", progname, code);
      }
    xlast = xplot;
    ylast = yplot;
    }
  }

float getcoord(unsigned char *pp, int *pj) {
  int c, d;

  c = pp[(*pj)++];
  d = pp[(*pj)++];
  return((float)((d << 8) | c));
  }

char* sgets(char *s, unsigned char *pp, int *pj) {
  int c;
  char *cs;

  cs = s;
  while((c = pp[(*pj)++]) != EOF) if((*(cs++) = c) == '\n') break;
  *(--cs) = '\0';
  return((c == EOF && cs == s) ? NULL : s);
  }

void usage(void) {
  fprintf(stderr, "%s [-d <display>] [-c]\nVersion: %s\n", progname, VERSION);
  }

/* read command line options */

void get_opt(int argc, char **argv) {
  int i;
  float linewidth, scale, x;
  char *tp;

  for (i = 1; i < argc; i++) {
    if(strcmp(argv[i], "-w") == 0) {
      /* THIS IS NOT YET IMPLEMENTED */
      if (i == argc - 1)
      fprintf(stderr, "%s: missing xplot linewidth option\n", progname);
      else if (sscanf(argv[++i], "%e", &x) == 1) linewidth=x;
      else fprintf(stderr, "%s: bad xplot linewidth option %s\n",
	progname, argv[i]);
      }
    else if (strcmp(argv[i], "-d") == 0) {
      if (i == argc - 1)
	fprintf(stderr, "%s: missing xplot display_name option\n", progname);
      else {
	strncpy(display_name_space, argv[++i], 128);
	display_name_space[127] = '\0';
	display_name = display_name_space;
	}
      }
    else if (strcmp(argv[i], "-h") == 0) usage();
    else if (strcmp(argv[i], "-c") == 0) toggle_color = 1;
    else {
      fprintf(stderr, "%s: Illegal xplot option %s\n", progname, argv[i]);
      usage();
      }
    }
  }
