void arc(int x0, int y0, int x1 , int y1,
	 int x2, int y2 );

void box(int x0, int y0, int x1 , int y1);

void circle(int x, int y , int r);

void closepl();

void closevt();

void cont(int x, int y);

void erase();

void label(char *s);

void line(int x0, int y0, int x1 , int y1);

void linmod(char *s);

void move(int x, int y);

void openpl();

void openvt();

void point(int x, int y);

void space(int x0, int y0, int x1 , int y1);

int unix_to_ugs(char *src);

void ascii(char *ptext,int xorg,int yorg,float size);
void rascii(char *ptext,int xorg,int yorg,float size);
void cascii(char *ptext,int xorg,int yorg,float size);
void cvascii(char *ptext,int xorg,int yorg,float size);
