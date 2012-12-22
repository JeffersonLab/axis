#
# Boiler Plate stuff: Autoconf Substitutions etc
#
#INCFLAGS = -I@top_srcdir@/include -I@top_builddir@/include 
#AM_CCFLAGS = $(INCFLAGS) @CCFLAGS@
#AM_LDFLAGS = -L@top_builddir@/lib @LDFLAGS@

AM_CCFLAGS = @CCFLAGS@
AM_LDFLAGS = -L/usr/X11R6/lib @LDFLAGS@
LDADD = -lX11 @LIBS@

#
# The programs to build
# 
## NOTE: if you want the programs to be build by default when 'make' is run
##   add them to bin_PROGRAMS.  But, if you don't want them to be installed
##   in @prefix@/bin by 'make install' then add them to nodist_PROGRAMS.
##   Another option is to add it to check_PROGRAMS, which means it will only
##   be built by 'make check' and is not installed.
bin_PROGRAMS = axis xplot psplot

#
# The program and its dependencies
#
axis_SOURCES = axis.c lowlevel.c g_to_ugs.c unix_to_ugs.c ascii.c digit.c ugs_strokes.c

xplot_SOURCES = xplot.c

psplot_SOURCES = psplot.c psutil.c config.c

# $(CC) $(CFLAGS) -o xplot xplot.c $(DUMMY) -lX11 -ldl -lpthread

