#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/inotify.h>

#define buff_size  16384

extern int errno;

void correct_dict(char *p)
{
        int l;
        l = strlen(p);
        if (p[l-1]!='/')
                strcat(p,"/");
}

void callerror(char *string, char *file)
{
	char error[512] = "";
	strcat(error, string);
	strcat(error, file);
	perror(string);
	exit(1);
}

void write_file(char *arg1,char *arg2)
{
        int fdr,fdw,n;
        char buf[512];
        fdr = open(arg1,O_RDONLY);
        if (fdr <= 0)
		callerror("error in opening ", arg1);
        fdw = open(arg2,O_WRONLY|O_CREAT|O_TRUNC, 0644);
        if (fdw <= 0)
		callerror("error in opening ",arg2);
        while ((n = read(fdr,buf,sizeof(buf)))>0)
                if (write(fdw, buf, n) != n) 
			callerror("error in writing", arg2);                
        if (n < 0)
                callerror("error in reading ",arg1);
                
}

int openfile(char *file)
{
	int fd;
	fd = open(file,O_RDONLY);
	if (fd < 0){
		callerror("error in opening ",file);
	}
	return fd;	
}
int cmp(char a[],char b[])
{
        int i,l;
        l = 4;
        for (i = 0; i<l; i++)
                if (a[i]!=b[i])
                        return 0;
        return 1;
}

void get_format(char *loc, char *format)
{	
	int fd, l;
	char buff[10];
        char pdf[] = {0x25,0x50,0x44,0x46,0x2d,'\0'};
        char jpg[] = {0xff,0xd8,0xff,0xe0,0x00,'\0'};
        char png[] = {0x89,0x50,0x4e,0x47,0x0d,'\0'};
        char mp4[] = {0x00,0x00,0x00,0x20,0x66,'\0'};
	fd = openfile(loc);
	l  = read(fd,buff,10);
	if (l < 0)
		callerror("error in reading",loc);
	close(fd);
	format[0] = '\0';
	if (cmp(pdf,buff))
		strcat(format,"pdf/");
	else if (cmp(jpg,buff))
		strcat(format,"jpg/");
        else if (cmp(png,buff))
		strcat(format,"png/");
	else if (cmp(mp4,buff))
		strcat(format,"mp4/");
	else
		format[0] = '\0';
}
void correct_ext(char *file, char *format)
{
	int i;
	i = strlen(format);
	format[i - 1] ='\0';
	strcat(file,".");
	strcat(file,format);
}

int main(int arg, char *argv[])
{
	int ifd, wd, i, n, a;
	char buf[buff_size];
	char fileloc[1024];
	char fileloc1[1024];
	char format[15];
        char arg1[512] = "";
        char arg2[512] = "";
	struct inotify_event *ev;
	struct stat sb;

        if (arg == 3){
        	strcat(arg1,argv[1]);
        	strcat(arg2,argv[2]);
        }
        else if (arg == 2){
        	strcat(arg1,argv[1]);
        	strcat(arg2,argv[1]);
        }
        else {
                printf("take one or two arguments,%d given\n",arg -1);
                exit(1);
        }
        correct_dict(arg1);
        correct_dict(arg2);
	printf("copy from %s to  %s..................\n",arg1,arg2);

	ifd = inotify_init();
	if (ifd < 0)
		callerror("cannot obtain an inotify instance","");
	wd = inotify_add_watch(ifd, argv[1], IN_CLOSE_WRITE|IN_MOVED_TO);
	if (wd < 0)
		callerror("can not add inotify watch","");

	while (1){
		n = read(ifd, buf, sizeof(buf));
		if (n<= 0)
			callerror("Problem in reading watch instance","");		
		i = 0;
		while (i < n){
			ev = (struct inotify_event *) &buf[i];
			if (!ev->len){
				i += sizeof(struct inotify_event) + ev->len;
				continue;
			}
			fileloc[0] = '\0';
                	strcat(fileloc,arg1);
                	strcat(fileloc,ev->name);
			printf("found a new %s \n", fileloc);
			if (stat(fileloc, &sb) == -1){
				printf("can not get status of %s \n",fileloc);
                                i += sizeof(struct inotify_event) + ev->len;
                                continue;
			}

			if (!(sb.st_mode & S_IFREG)){
				printf("\t but not a file");
				i += sizeof(struct inotify_event) + ev->len;
				continue;
			}
			get_format(fileloc,format);
			if (format[0] == '\0'){
				printf("unknown file signature file skiped\n");
				i += sizeof(struct inotify_event) + ev->len;
				continue;
			}
                        fileloc1[0] = '\0';
                        strcat(fileloc1,arg2);
			strcat(fileloc1,format);
			//for (a = 0; a < 5; a++)
				//printf("%x\n",buff[a]);

                        a = mkdir(fileloc1,0777);
                        if ((a < 0)&(errno != EEXIST))
                                callerror("error in creating directory",fileloc1);
                        strcat(fileloc1,ev->name);
			correct_ext(fileloc1,format);
                        write_file(fileloc,fileloc1);
			printf("just copyed from %s to  %s \n",fileloc,fileloc1);
			i += sizeof(struct inotify_event) + ev->len;
		}
	}
}

