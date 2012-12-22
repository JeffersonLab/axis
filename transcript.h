/* transcript.h
 *
 * Copyright (C) 1985 Adobe Systems Incorporated
 *
 * RCSID: $Header: /group/lattice/cvsroot/axis/transcript.h,v 1.2 2007/02/15 04:37:03 edwards Exp $
 *
 * some general global defines
 *
 * Edit History:
 * Andrew Shore: Sun Nov 17 15:40:47 1985
 * End Edit History.
 *
 * RCSLOG:
 * $Log: transcript.h,v $
 * Revision 1.2  2007/02/15 04:37:03  edwards
 * Cleanups on the type of malloc.
 *
 * Revision 1.1  2006/06/03 19:34:25  edwards
 * Initial version.
 *
 * Revision 2.1  85/11/24  11:51:21  shore
 * Product Release 2.0
 * 
 * Revision 1.3  85/11/20  00:58:05  shore
 * changes for Release 2 with System V support
 * 
 * Revision 1.2  85/05/14  11:31:52  shore
 * 
 * 
 *
 */

#define private static

#define TRUE	(1)
#define FALSE	(0)

#define PSVERSION 	23.0
#define COMMENTVERSION	"PS-Adobe-1.0"

#define VOID void
#define VOIDC (void)

#ifdef SYSV
#define INDEX strchr
#define RINDEX strrchr
#else
#define INDEX index
#define RINDEX rindex
#endif

/* external globals (from config.c) */

extern char LibDir[];
extern char TroffFontDir[];
extern char DitDir[];
extern char TempDir[];

/* definitions of basenames of various prologs, filters, etc */
/* all of these get concatenated as LibDir/filename */

#define BANNERPRO	"/banner.pro"
#define PLOTPRO		"/psplot.pro"
#define CATPRO		"/pscat.pro"
#define TEXTPRO		"/pstext.pro"
#define ENSCRIPTPRO	"/enscript.pro"
#define PS4014PRO	"/ps4014.pro"
#define PS630PRO	"/ps630.pro"
#define PSDITPRO	"/psdit.pro"
#define PSSUNPRO	"/pssun.pro"
#define FONTMAP		"/font.map"

#define ENSCRIPTTEMP	"/ESXXXXXX"
#define REVTEMP		"/RVXXXXXX"

#define PSTEXT		"pstext"
#define PSPLOT		"psplot"
#define PSCAT		"pscat"
#define PSDIT		"psdit"
#define PSIF		"psif"
#define PSSUN		"pssun"

/* psutil functions */
extern char *mstrcat();
extern char *envget();
extern VOID pexit();
extern VOID pexit2();

extern char *mapname();

#if 0
/* system functions */
extern VOID perror();
extern VOID exit();
extern char *ctime();
extern long time();
extern char *mktemp();
extern char *gets();
extern char *malloc();
#endif
