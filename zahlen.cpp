/************************************************

Zahlenspiel

Man mu· aus 6 Zahlen mit arithmetischen Operationen ein Ziel
berechnen. Die Zahlen und das Ziel werden auf dem Bildschirm
dargestellt, und der Benutzer kann den Rechenvorgang mit
der Maus eingeben. Der Computer berechnet auf Wunsch auch
eine Lîsung.

Das Programm benutzt eine rudimentÑre Klassenbibliothek (easy). Die 
Bibliothek wurde nur soweit getrieben, wie es zur Portierung notwendig
war. Ich entschuldige mich auch fÅr die spÑrlichen Kommentare.

**********************************************/

#include "easy.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zahlen.h"

Font font;

Textbox plus(font,25,4,3,1,"+");
Textbox minus(font,31,4,3,1,"-");
Textbox times(font,37,4,3,1,"*");
Textbox divi(font,43,4,3,1,"/");
Textbox *pop[4]={&plus,&minus,&times,&divi};
Textbox nbox(font,15,4,5,1);
Textbox solframe(font,12,7,20,5);
Textbox n1(font,2,1,5,1);
Textbox n2(font,10,1,5,1);
Textbox n3(font,18,1,5,1);
Textbox n4(font,26,1,5,1);
Textbox n5(font,34,1,5,1);
Textbox n6(font,42,1,5,1);
Textbox *pn[6]={&n1,&n2,&n3,&n4,&n5,&n6};

String swindow(IDS_Window);

class MyWindow : public Window
{	public :
	MyWindow (int id) :
		Window(id,swindow,
			FCF_NORMAL|FCF_MENU|FCF_ACCELTABLE)
	{	init(); }
	virtual void redraw (PS &ps);
	virtual int clicked (LONG x, LONG y, clicktype click);
	void setsize ();
};
MyWindow window(ID_Window);

String shelpname(IDS_Helpname);
String shelptitle(IDS_Helptitle);

Menu menu(window);
Help windowhelp(window,ID_Helptable,shelpname,shelptitle);

Time mytime;

class InputDialog : public Dialog
{   public :
	InputDialog (Window &w, int i) : Dialog(w,i) {}
	virtual void start ();
	virtual void stop ();
};

/* Ausgangszahlen, die gemischt werden und "numbers" ergeben */
#define N 20
int vnumbers [N]=
{2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,25,50,100};

int n; /* Ziel */

int numbers[6],valid[6];
/* Die fÅr den Benutzer wichtigen Zahlen und flags, die angeben,
welche Zahlen schon in Berechnungen verwendet wurden. */

char sol[5][64]={"","","","",""};
/* Text der Lîsungsschritte, z.B. "3 * 4 = 12" */

int solnumber[5], /* Ergebnis eines Lîsungsschrittes */
	solvalid[5], /* schon verwendet? */
	solop, /* aktuelle Opertation (+,-,*,/) */
	half=0, /* schon ein Operand eingegeben? */
	soln=0; /* aktuelle Lîsungszeile */
int solved=0; /* sol enthÑlt richtige Lîsung (computerberechnet) */

typedef int (*comparef) (const void *, const void *);
int compare (int *i, int *j)
/* Hilfsfunktion, um die 6 Zahlen zu sortieren */
{	if (*i<*j) return -1;
	if (*i>*j) return 1;
	return 0;
}

void mix (void)
/* Mische die N Zahlen und kopiere die ersten 6 */
{	int i,j,h;
	for (i=N; i>1; i--)
	{	j=rand()%i;
		if (i!=j)
		{	h=vnumbers[j];
			vnumbers[j]=vnumbers[i-1];
			vnumbers[i-1]=h;
		}
	}
	for (i=0; i<6; i++) numbers[i]=vnumbers[i];
	qsort(numbers,6,sizeof(int),(comparef)compare);
}

void problem (void)
/* Berechne neues Ziel */
{	n=rand()%899+100;
	mytime.set();
}

void dsol (void)
/* Lîsche die Lîsungstexte und markiere alle Zahlen als
unverbraucht */
{	int i;
	for (i=0; i<5; i++) { sol[i][0]=0; solvalid[i]=1; }
	soln=0; half=0;
	for (i=0; i<6; i++) valid[i]=1;
}

void drawnum (PS &ps, Font &font)
/* Zeichne die Zahlen neu (Zahlen, Lîsungstexte) */
{	char text[32];
	int i;
	for (i=0; i<6; i++)
	{	font.textframe(i*8+2,1,5,1);
		sprintf(text,"%d",numbers[i]);
		if (valid[i]) font.centertext(i*8+2,1,5,text);
		else font.centertext(i*8+2,1,5,text,CLR_DARKGRAY);
	}
	solframe.redraw();
	for (i=0; i<5; i++)
	{   if (solvalid[i]) font.text(13,7+i,sol[i]);
		else font.text(13,7+i,sol[i],CLR_DARKGRAY);
	}
}

void MyWindow::setsize ()
{   WinPS ps(window);
	LONG x1,x2,y1,y2;
	font.set(ps);
	font.getframe(0,0,50,25,x1,y1,x2,y2);
	if (x2!=window.width() || y2!=window.height())
	{	window.size(x2,y2);
		window.update();
		return;
	}
}

void MyWindow::redraw (PS &ps)
/* Zeichne das Fenster neu (Zahlen, Ziel, Lîsungstexte) */
{	char ntext[32];
	font.set(ps);
	ps.erase();
	font.text(1,7,String(IDS_Solution));
	plus.redraw();
	minus.redraw();
	times.redraw();
	divi.redraw();
	font.text(1,4,String(IDS_Aim));
	nbox.settext(itoa(n,ntext,10));
	drawnum(ps,font);
}

int solve (int *z, int k)
/* Bilde Ziel n aus k Zahlen bei z. Rekursive Funktion.
Schreibe die korrekte Operation nach sol[6-k], falls
das Ziel gebildet werden konnte.
Da immer wieder z als Vektor verwendet wird, muû einiges
hin- und hergespeichert werden. */
{	int i,j,h,h1,h2;
	long hm;
	if (k==1) return (*z==n); /* Nur eine Zahl */
	h=z[0];
	/* Doppelschleife: */
	for (i=0; i<k-1; i++)
	{	h1=z[i];
		z[i]=h;
		if (h1==n) return 1;
		for (j=i+1; j<k; j++)
		{	h2=z[j];
			/* Probiere + */
			z[j]=h1+h2;
			if (z[j]==n || solve(z+1,k-1))
			{	sprintf(sol[6-k],"%d + %d = %d",h1,h2,h1+h2);
				goto found;
			}
			if (h2>h1)
			{	/* Probiere - */
				z[j]=h2-h1;
				if (z[j]==n || solve(z+1,k-1))
				{	sprintf(sol[6-k],"%d - %d = %d",h2,h1,h2-h1);
					goto found;
				}
				/* Probiere / */
				if (h2%h1==0)
				{	z[j]=h2/h1;
					if (z[j]==n || solve(z+1,k-1))
					{	sprintf(sol[6-k],"%d / %d = %d",h2,h1,h2/h1);
						goto found;
					}
				}
			}
			else if (h2<h1) /* wie oben */
			{	z[j]=h1-h2;
				if (z[j]==n || solve(z+1,k-1))
				{	sprintf(sol[6-k],"%d - %d = %d",h1,h2,h1-h2);
					goto found;
				}
				if (h1%h2==0)
				{	z[j]=h1/h2;
					if (z[j]==n || solve(z+1,k-1))
					{	sprintf(sol[6-k],"%d / %d = %d",h1,h2,h1/h2);
						goto found;
					}
				}
			}
			/* probiere * */
			hm=(long)h1*h2;
			if (hm<30000l)
			{	z[j]=(int)hm;
				if (z[j]==n || solve(z+1,k-1))
				{	sprintf(sol[6-k],"%d * %d = %d",h1,h2,h1*h2);
					goto found;
				}
			}
			z[j]=h2;
		}
		z[i]=h1;
	}
	return 0;
	found:
	z[i]=h1; z[j]=h2; return 1;
}

void help (int command)
// Berechne mit Nachfrage die Lîsung
{   if (!Question(String(IDS_QSolve),String(IDS_TQSolve)))
		return;
	dsol();
	if (!solve(numbers,6))
		Message(String(IDS_Notfound),"");
	window.update();
	solved=1;
}

void new_numbers (int command)
{	mix(); problem(); dsol(); window.update();
}

void new_aim (int command)
{	problem(); dsol(); window.update();
}

void quit (int command)
{   exit(0);
}

void info (int command)
{	Message(String(IDS_Message),String(IDS_TMessage));
}

void helpindex (int command)
{	windowhelp.index();
}

void showhelp (int command)
{	windowhelp.content();
}

void showgeneral (int command)
{	windowhelp.general();
}

int nid[6]={ID_N1,ID_N2,ID_N3,ID_N4,ID_N5,ID_N6};

void InputDialog::start ()
{   char s[256];
	int i;
	for (i=0; i<6; i++)
	{	itoa(numbers[i],s,10);
		settext(nid[i],s);
	}
	itoa(n,s,10); settext(ID_Aim,s);
}

void InputDialog::stop ()
{	int i;
	for (i=0; i<6; i++)
	{	numbers[i]=atoi(gettext(nid[i]));
	}
	n=atoi(gettext(ID_Aim));
}

void input (int command)
// Erlaube Eingabe eines eigenen Problemes
{   int i;
	InputDialog d(window,ID_Input);
	restart: d.init();
	if (d.result()!=DID_OK) return;
	for (i=0; i<6; i++)
	{	if (numbers[i]<=0 || numbers[i]>100) goto error;
	}
	if (n<=0 || n>1000) goto error;
	dsol(); window.update();
	mytime.set();
	return;
	error: Message(String(IDS_EInput),String(IDS_TEInput));
	goto restart;
}

int solupdate (int k)
/* Mache Lîsungszeile neu, wenn der Benutzer k als Zahl
gewÑhlt hat. Gib zurÅck, ob diese Zahl zulÑssig ist. */
{	char s[256];
	ULONG t;
	if (half && solop<0) return 0; /* HÑlfte ohne Operator */
	if (!half) /* Noch nichts eingegeben */
	{	solnumber[soln]=k;
		half=1;
		solop=-1;
		sprintf(sol[soln],"%d",k);
	}
	else /* HÑlfte mit Operator */
	{	if (solop<0) return 0;
		switch (solop)
		{	case 0 :
				solnumber[soln]+=k; break;
			case 1 :
				if (k>=solnumber[soln]) return 0;
				solnumber[soln]-=k; break;
			case 2 :
				if ((long)k*solnumber[soln]>30000l) return 0;
				solnumber[soln]*=k; break;
			case 3 :
				if (solnumber[soln]%k!=0) return 0;
				solnumber[soln]/=k; break;
		}
		sprintf(s,"%d = %d",k,solnumber[soln]);
		strcat(sol[soln],s);
		if (solnumber[soln]==n)
		{	t=mytime.seconds();
			sprintf(s,String(IDS_Seconds),mytime-t);
			Message(s,String(IDS_TSeconds));
		}
		soln++; half=0;
	}
	return 1;
}

int MyWindow::clicked (LONG x, LONG y, clicktype click)
// Behandle Mausereignisse im Fenster.
{   int k;
	if (click!=button1) return 0;
	WinPS ps(window);
	font.set(ps);
	if (solved) dsol();
	solved=0;
	if (soln>=5) return 1;
	for (k=0; k<6; k++)
		if (valid[k] && pn[k]->contains(x,y))
		{	if (solupdate(numbers[k])) valid[k]=0;
			else return 1;
			goto ok;
		}
	if (half && solop<0)
		for (k=0; k<4; k++)
			if (pop[k]->contains(x,y))
			{	switch (k)
				{	case 0 : strcat(sol[soln]," + "); break;
					case 1 : strcat(sol[soln]," - "); break;
					case 2 : strcat(sol[soln]," * "); break;
					case 3 : strcat(sol[soln]," / "); break;
					default : return 1;
				}
				solop=k;
				goto ok;
			}
	if (solframe.contains(x,y))
	{	k=solframe.line(y);
		if (k<0 || k>=soln || !solvalid[k]) return 1;
		if (solupdate(solnumber[k])) solvalid[k]=0;
		else return 1;
		goto ok;
	}
	return 0;
	ok: window.update();
	return 0;
}

void undo (int command)
// Lîsche Lîsungsversuch
{	dsol();
	window.update();
}

void main (void)
/* Hauptschleife, bis Fenster geschlossen wurde. */
{	srand((int)mytime);
	mix(); problem(); dsol();
	menu.add(IDM_Solution,help);
	menu.add(IDM_Exit,quit);
	menu.add(IDM_NewNumbers,new_numbers);
	menu.add(IDM_NewAim,new_aim);
	menu.add(IDM_Info,info);
	menu.add(IDM_Help,showhelp);
	menu.add(IDM_General,showgeneral);
	menu.add(IDM_Index,helpindex);
	menu.add(IDM_ClearSolution,undo);
	menu.add(IDM_Input,input);
	window.setsize();
	window.loop();
	exit(0);
}

