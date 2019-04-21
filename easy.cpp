#include "easy.h"

static int window_invalid=0;
static Window *actwindow;

Program program;

// ******************* Debug tools **********************

void dumplong (long n)
{	char s[256];
	sprintf(s,"%ld, %lx",n,n);
	WinMessageBox(HWND_DESKTOP,HWND_DESKTOP,s,"Dump",0,
		MB_OK);
}

void dump (char *s)
{	WinMessageBox(HWND_DESKTOP,HWND_DESKTOP,s,"Dump",0,
		MB_OK);
}

// *********** Messages etc. ***************************

void Message (char *s, char *title)
{	WinMessageBox(HWND_DESKTOP,HWND_DESKTOP,s,title,0,
		MB_OK|MB_ICONEXCLAMATION);
}

int Question (char *s, char *title)
{	return (WinMessageBox(HWND_DESKTOP,HWND_DESKTOP,s,title,0,
		MB_YESNO|MB_ICONQUESTION)==MBID_YES);
}

// **************** Time *************************

void Time::set ()
{	DosQuerySysInfo(QSV_TIME_LOW,QSV_TIME_LOW,
		&Seconds,sizeof(ULONG));
}

// ********************* Strings ************************

long defaultsize=256;

String::String (char *text)
{	P=new char[defaultsize];
	strcpy(P,text);
	Size=defaultsize;
}

String::String (int id)
{   char s[1024];
	WinLoadString(program.hab(),NULLHANDLE,id,1022,s);
	Size=strlen(s)+1;
	P=new char[Size];
	strcpy(P,s);
}


String::String ()
{   Size=defaultsize;
	P=new char[Size];
	*P=0;
}

String::~String ()
{	delete P;
}

void String::copy (char *text)
{	delete P;
	Size=strlen(text)+1;
	P=new char[Size];
	strcpy(text,P);
}

// ******************** Program *************************

MRESULT EXPENTRY MainWindowProc (HWND hwnd, ULONG msg,
	MPARAM mp1, MPARAM mp2)
{	Window *window;
	clicktype click;
	if (window_invalid) window=actwindow;
	else window=(Window *)WinQueryWindowPtr(hwnd,QWL_USER);
	POINTS *points;
	switch (msg)
	{	case WM_PAINT :
			{	RedrawPS ps(hwnd,*window);
				window->redraw(ps);
			}
			break;
		case WM_SIZE :
			window->Width=SHORT1FROMMP(mp2);
			window->Height=SHORT2FROMMP(mp2);
			window->sized();
			break;
		case WM_BUTTON1CLICK :
			click=button1; goto button;
		case WM_BUTTON2CLICK :
			click=button2; goto button;
		case WM_BUTTON3CLICK :
			click=button3; goto button;
		case WM_BUTTON1UP :
			click=button1up; goto button;
		case WM_BUTTON2UP :
			click=button2up; goto button;
		case WM_BUTTON3UP :
			click=button3up; goto button;
		case WM_BUTTON1DOWN :
			click=button1down; goto button;
		case WM_BUTTON2DOWN :
			click=button2down; goto button;
		case WM_BUTTON3DOWN :
			click=button3down; goto button;
		case WM_BUTTON1DBLCLK :
			click=button1double; goto button;
		case WM_BUTTON2DBLCLK :
			click=button2double; goto button;
		case WM_BUTTON3DBLCLK :
			click=button3double;
			button: points=(POINTS *)&mp1;
			if (window->clicked(points->x,points->y,click)) break;
			else goto def;
		case WM_COMMAND :
			if (window->Windowmenu)
				if (window->Windowmenu->call(SHORT1FROMMP(mp1)))
					break;
			goto def;
		default :
			def: return WinDefWindowProc(hwnd,msg,mp1,mp2);
	}
	return (MRESULT)FALSE;
}

Program::Program ()
{	Hab=WinInitialize(0);
	Hmq=WinCreateMsgQueue(Hab,0);
	WinRegisterClass(Hab,"MainWindow",MainWindowProc,
		CS_SIZEREDRAW,sizeof(Window *));
}

Program::~Program ()
{	WinDestroyMsgQueue(Hmq);
	WinTerminate(Hab);
}

inline int Program::getmessage ()
{	return WinGetMsg(Hab,&Qmsg,0L,0,0);
}

inline void Program::dispatch ()
{	WinDispatchMsg(Hab,&Qmsg);
}

void Program::loop ()
{	while(getmessage())
		dispatch();
}

Window::Window (int id, char *name, ULONG flags)
{	Id=id; Name=new String(name); Flags=flags;
}

void Window::init ()
{   window_invalid=1;
	actwindow=this;
	FrameHandle=WinCreateStdWindow(HWND_DESKTOP,
		WS_VISIBLE,&Flags,"MainWindow",
		Name->text(),WS_VISIBLE,(HMODULE)0,Id,&Handle);
	WinSetWindowPtr(Handle,QWL_USER,this);
	window_invalid=0;
	Windowmenu=NULL;
}

Window::~Window ()
{	WinDestroyWindow(FrameHandle);
}

void Window::redraw (PS &ps)
{	ps.erase();
}

int Window::clicked (LONG x, LONG y, clicktype click)
{	return 0;
}

void Window::sized ()
{
}

void Window:: size (LONG w, LONG h)
{   RECTL r;
	r.xLeft=r.yBottom=0; r.xRight=w; r.yTop=h;
	WinCalcFrameRect(Handle,&r,FALSE);
	WinSetWindowPos(FrameHandle,NULLHANDLE,0,0,
		r.xRight,r.yTop,SWP_SIZE);
}

// ************* Menus ************************************

Menu::~Menu ()
{	while (Mp)
	{	delete Mp;
		Mp=Mp->next();
	}
}

int Menu::call (int command)
{	Menuentry *m=Mp;
	while (m)
		if (m->command()==command)
		{	m->call(command);
			return 1;
		}
		else m=m->next();
	return 0;
}

// ************* Presentation Space class (PS) *************

void PS::move (LONG c, LONG r, ULONG col)
{   color(col);
	P.x=c; P.y=r; GpiMove(Handle,&P);
}

void PS::linerel (LONG w, LONG h, ULONG col)
{   color(col);
	P.x+=w; P.y+=h; GpiLine(Handle,&P);
}

void PS::lineto (LONG c, LONG r, ULONG col)
{   color(col);
	P.x=c; P.y=r; GpiLine(Handle,&P);
}

void PS::text (char *s, ULONG col, ULONG al)
{	color(col);
	alignment(al);
	GpiCharString(Handle,strlen(s),s);
}

void PS::frame (LONG w, LONG h, int r, ULONG col)
{	color(col);
	GpiSetBackMix(Handle,BM_OVERPAINT);
	GpiSetPattern(Handle,PATSYM_BLANK);
	P.x+=w; P.y+=h; GpiBox(Handle,DRO_OUTLINEFILL,&P,r,r);
}

void PS::area (LONG w, LONG h, int r, ULONG col)
{	color(col);
	GpiSetBackMix(Handle,BM_OVERPAINT);
	GpiSetPattern(Handle,PATSYM_SOLID);
	P.x+=w; P.y+=h; GpiBox(Handle,DRO_FILL,&P,r,r);
}

//******************* Fonts ********************************

Font::Font (PS &ps)
{   Ps=&ps;
	Fm=(FONTMETRICS *)malloc(sizeof(FONTMETRICS));
	GpiQueryFontMetrics(Ps->handle(),sizeof(FONTMETRICS),Fm);
}

Font::Font ()
{	Ps=NULL; Fm=NULL;
}

Font::~Font ()
{	if (Fm) free(Fm);
}

void Font::set (PS &ps)
{   Ps=&ps;
	if (Fm) free(Fm);
	Fm=(FONTMETRICS *)malloc(sizeof(FONTMETRICS));
	GpiQueryFontMetrics(Ps->handle(),sizeof(FONTMETRICS),Fm);
}


void Font::text (int c, int r, char *s, ULONG col)
// Draw a Text at char column and row
{	Ps->move(c*wbox(),Ps->height()-r*hbox()-above());
	Ps->text(s,col);
}

void Font::centertext (int c, int r, int w, char *s, ULONG col)
// Draw a Text at char column and row
{	Ps->move((2*c+w)*wbox()/2,Ps->height()-r*hbox()-above());
	Ps->text(s,col,TA_CENTER);
}

void Font::righttext (int c, int r, int w, char *s, ULONG col)
// Draw a Text at char column and row
{	Ps->move((c+w)*wbox(),Ps->height()-r*hbox()-above());
	Ps->text(s,col,TA_RIGHT);
}

void Font::textframe (int c, int r, int w, int h, int rad,
	ULONG col, int framed)
// Draw a frame around a text area
{	Ps->move(c*wbox()-wbox()/2,Ps->height()-(r+h+1)*hbox()+hbox()/2);
	if (framed) Ps->frame((w+1)*wbox(),(h+1)*hbox(),rad,col);
	else Ps->area((w+1)*wbox(),(h+1)*hbox(),rad,CLR_WHITE);
}

int Font::inframe (int c, int r, int w, int h, LONG x, LONG y)
{	LONG c1,c2,r1,r2;
	c1=c*wbox()-wbox()/2; c2=c1+(w+1)*wbox();
	r1=Ps->height()-(r+h+1)*hbox()+hbox()/2; r2=r1+(h+1)*hbox();
	return (x>c1 && x<c2 && y>r1 && y<r2);
}

void Font::getframe (int c, int r, int w, int h,
	LONG &x1, LONG &y1, LONG &x2, LONG &y2)
{	x1=c*wbox()-wbox()/2; x2=x1+(w+1)*wbox();
	y1=Ps->height()-(r+h+1)*hbox()+hbox()/2; y2=y1+(h+1)*hbox();
}

int Font::line (int r, int h, LONG y)
{	return (Ps->height()-r*hbox()-y)/hbox();
}

// *********************** Text Box **************************

void Textbox::redraw ()
{   char *p=Text,*q,c;
	int line=0;
	if (!*F) return;
	F->textframe(X,Y,W,H,Rad,Color,Framed);
	while (1)
	{   q=p; while (*q && *q!='\n') q++;
		c=*q; *q=0;
		switch (Justify)
		{   case text_left : F->text(X,Y+line,p,Color); break;
			case text_right : F->righttext(X,Y+line,W,p,Color); break;
			default : F->centertext(X,Y+line,W,p,Color); break;
		}
		if (!c) break;
		*q=c;
		p=q+1;
		line++;
	}
}

//******************* Help ******************************

Help::Help (Window &window, int id, char *filename, char *title)
{	HELPINIT hini;
	hini.cb=sizeof(HELPINIT);
	hini.ulReturnCode=0L;
	hini.pszTutorialName=(PSZ)NULL;
	hini.phtHelpTable=
		(PHELPTABLE)MAKELONG(id,0xFFFF);
	hini.hmodHelpTableModule=(HMODULE)0;
	hini.hmodAccelActionBarModule=(HMODULE)0;
	hini.idAccelTable=0;
	hini.idActionBar=0;
	hini.pszHelpWindowTitle=title;
	hini.fShowPanelId = CMIC_HIDE_PANEL_ID;
	hini.pszHelpLibraryName=filename;
	Handle=WinCreateHelpInstance(program.hab(),&hini);
	if (!Handle) { Valid=0; return; }
	Valid=1;
	WinAssociateHelpInstance(Handle,window.framehandle());
}

// ****************** Dialogs *************************

Dialog *activedlg;

static MRESULT EXPENTRY dialogproc (HWND hwnd, ULONG msg,
	MPARAM mp1, MPARAM mp2)
{	switch (msg)
	{   case WM_INITDLG :
			activedlg->Handle=hwnd;
			activedlg->start();
			break;
		case WM_COMMAND :
			activedlg->Result=(SHORT1FROMMP(mp1));
			if (activedlg->handler(SHORT1FROMMP(mp1)))
			{   activedlg->stop();
				WinDismissDlg(hwnd,SHORT1FROMMP(mp1));
			}
			break;
		default :
			return WinDefDlgProc(hwnd,msg,mp1,mp2);
	}
	return (MRESULT)FALSE;
}

Dialog::Dialog (Window &window, int id) : S()
{	W=&window;
	Id=id;
}

void Dialog::init ()
{   activedlg=this;
	Handle=WinDlgBox(HWND_DESKTOP,W->handle(),dialogproc,
		(HMODULE)0,Id,NULL);
}

int Dialog::handler (int command)
{	return 1;
}

char *Dialog::gettext (int id, char *text, long size)
{   HWND handle=WinWindowFromID(Handle,id);
	WinQueryWindowText(handle,size,text);
	return text;
}

char *Dialog::gettext (int id)
{   HWND handle=WinWindowFromID(Handle,id);
	WinQueryWindowText(handle,S.size(),(PSZ)S.text());
	return S;
}

void Dialog::settext (int id, char *text)
{	HWND handle=WinWindowFromID(Handle,id);
	WinSetWindowText(handle,text);
}



