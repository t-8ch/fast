#include <stdlib.h>
#include <stddef.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <curses.h>

static const char INFO[] = "fare v1.0";
static const char USAGE[] = "[SPACE] play/pause  [g] |<   [h] <   [j] --   [k] ++   [l] >   [q] quit";

static const char ACCEPT[] = "\n\t ";

char *inputString(FILE* fp, size_t size){
	//The size is extended by the input with the value of the provisional
	char *str;
	int ch;
	size_t len = 0;
	str = realloc(NULL, sizeof(char)*size);//size is start size
	if(!str)return str;
	while(EOF!=(ch=fgetc(fp))){
		str[len++]=ch;
		if(len==size){
			str = realloc(str, sizeof(char)*(size+=16));
			if(!str)return str;
		}
	}
	str[len++]='\0';

	return realloc(str, sizeof(char)*len);
}

int pivotLetter(int l)
{
	if(l == 1)
		return 0;
	else if(l >=2 && l<= 5)
		return 1;
	else if(l >= 6 && l <= 9)
		return 2;
	else if(l >= 10 && l <= 13)
		return 3;
	else 
		return 4;
}

char *backwards(char *data, char *index, char *accept)
{
	if((int)(index-data)<0)
		return (char *) data;


	while((int)(index-data)>0)
	{
	    const char *a = accept;
        while (*a != '\0')
            if(*a++ == *index)
                return (char *) index;
        index--;
	}
	return (char *) index;
}

void fastread(char *data, int x, int y, int speed)
{
	int index = 0;
	bool run = false;
	bool pause = false;
	bool last = false;
	bool back = false;
	char mid[2];
	int ch;
	char *curr = data;
	int length = 0;
	int pivot = 0;
	while(true)
	{

		mvprintw(y,0," ");
		clrtoeol();
        
        char *next = curr;

        next = strpbrk(next,"\n\t ");
        if(!next){
            run = false;
            last = true;
            next = strchr(curr,'\0');
        }

		length = (int)(next-curr);
		pivot = pivotLetter(length);
		
		if(pivot > 0)
        {
            char pre[pivot+1];
            strncpy(pre, curr, pivot);
            pre[pivot+1] = '\0';
            mvprintw(y,x-pivot,pre);
        }

		strncpy(mid, curr+pivot, 1);
		mid[1] = '\0';
		attron(COLOR_PAIR(1));
		mvprintw(y,x,mid);
		attroff(COLOR_PAIR(1));
		
		if(length>pivot+1)
		{
			char end[length-pivot];
			strncpy(end, curr+pivot+1, length-pivot-1);
			end[length-pivot-1] = '\0';
			mvprintw(y,x+1,end);
		}
	
		refresh();

		if(pause)
		{
			pause = false;
			run = false;
		}

		do
		{
			if(run && !last)
				timeout(0);
			ch = getch();

			switch(ch)
			{
				case ' ':
					if(run)
						run = false;
					else if(!last)
						run = true;
					break;
				case 'q':
					return;
					break;
				case 'k':
					speed+=50;
					mvprintw(0,40,"words / minute: %i",speed);
					break;
				case 'j':
					speed-=50;
					if(speed < 50)
						speed = 50;
					mvprintw(0,40,"words / minute: %i",speed);
					clrtoeol();
					break;
				case 'h':
					run = true;
					pause = true;
					last = false;
					next = backwards(data, next-2, "\n\t ");
					do
					    next = backwards(data, --next, "\n\t ");
					while(strchr("\n\t ",*(--next)) && (int)(next-data)>0);
					break;
				case 'l':
					run = true;
					pause = true;
					break;
				case 'g':
					curr = data;
					last = false;
					pause = true;
					run = true;
					break;
				default:
					break;
			}
			if(run)
				usleep(1000*1000*60/speed);
		}
		while(last || !run);
        
        while(strchr("\t\n ",*(++next)))
            next = strpbrk(next,"\n\t ");
        curr = next;
	}
}

int main(void){
	int x,y,height,width;
	char *data = inputString(stdin, 10);

	freopen("/dev/tty", "rw", stdin);

	// enabling ncurses mode
	WINDOW *win = initscr();
	keypad(win, TRUE);
	noecho();
	cbreak();
	curs_set(0);
	
	start_color();
	use_default_colors();
	short fg,bg;
	pair_content(0,&fg,&bg);
	init_pair(1,COLOR_RED,-1);


	getmaxyx(win,height,width);
	y=height/2;
	x=width/2;

	mvaddstr(0,0,INFO);
	mvaddstr(height-1,0,USAGE);
	
	attron(COLOR_PAIR(1));
	mvaddstr(y-1,x,".");
	mvaddstr(y+1,x,"'");
	attroff(COLOR_PAIR(1));
	
	refresh();
	int ch;

	int speed = 350;
	
	mvprintw(0,40,"words / minute: %i",speed);

	fastread(data,x,y,speed);

	free(data);
	endwin();
	return 0;
}
