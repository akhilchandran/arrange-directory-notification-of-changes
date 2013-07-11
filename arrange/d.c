#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<errno.h>
#include<sys/types.h>
#include <sys/stat.h>
#include<fcntl.h>
#include<dirent.h>

void correct_dict(char *p);
DIR *open_dir(char *p);
int  formatcmp(int *format,int *buff);
void print_buff(int p[]);
void write_file(char *arg1,char *arg2);
int openfile(char *file);
int cmp(char a[],char b[]);
char error[500];
extern int errno;

int main(int arg,char *argv[])

{
    	DIR *dirp;
    	char pdf[] = {0x25,0x50,0x44,0x46,0x2d,'\0'};
    	char jpg[] = {0xff,0xd8,0xff,0xe0,0x00,'\0'};
    	char png[] = {0x89,0x50,0x4e,0x47,0xd,'\0'};
    	char mp4[] = {0x0,0x0,0x0,0x20,0x66,'\0'};
    	char fileloc[100];
    	char fileloc1[100];
    	char arg1[500] = "";
    	char arg2[500] = "";
	char buff[5];
	int fd,a;
    	ssize_t n;
    	struct dirent *file;

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
	//printf("%s",arg1);
	//printf("%s",arg2);
	dirp = open_dir(arg1);
	while ((file = readdir(dirp)) != 0){
		if (file->d_type != DT_REG)
			continue;
		fileloc[0] = '\0';
		strcat(fileloc,arg1);
		strcat(fileloc,file->d_name);
		fd = openfile(fileloc);
		n = read(fd,buff,5);
		if (n < 0){
			error[0]='\0';
			strcat(error,"\nerror in reading ");
			strcat(error,fileloc);
			perror(error);
			exit(1);
	        }
	        close(fd);       
	        fileloc1[0] ='\0';
	        strcat(fileloc1,arg2);
	        if (cmp(png,buff))
	        	strcat(fileloc1,"png/");
		else if (cmp(jpg,buff))
			strcat(fileloc1,"jpg/");
		else if (cmp(pdf,buff))
			strcat(fileloc1,"pdf/");
        	else if (cmp(mp4,buff))
        		strcat(fileloc1,"mp4/");
        	else
        		continue;        
        	a = mkdir(fileloc1,0777);
		if ((a < 0)&(errno != EEXIST)){
			error[0] = '/';
			strcat(error,"\nerror in creating directory ");
			strcat(error,fileloc1);
			perror(error);
			exit(1);
		}
        	strcat(fileloc1,file->d_name);
        	write_file(fileloc,fileloc1);
        	unlink(fileloc);    
    	}
    	closedir(dirp);
	return 0;
}
void correct_dict(char *p)
{
	int l;
	l = strlen(p);
	if (p[l-1]!='/')
		strcat(p,"/");
}
DIR *open_dir(char *p)
{
	DIR *dir;
	dir = opendir(p);
	if (dir == 0){
		error[0] = '\0';
		strcat(error,"\nerror in opening ");
		strcat(error,p);
		perror(error);
		exit(1);
	}
	return(dir);
}
void write_file(char *arg1,char *arg2)
{
	int fdr,fdw,n;
	char buf[512];
	fdr = open(arg1,O_RDONLY);
	if (fdr <= 0){
		error[0] = '\0';
		strcat(error,"\nerror in opening ");
		strcat(error,arg1);
		perror(error);
	exit(1);
	}
	fdw = open(arg2,O_WRONLY|O_CREAT|O_TRUNC, 0644);    
	if (fdw <= 0){ 
		error[0] = '\0';
		strcat(error,"\nerror in opening ");
		strcat(error,arg2);
		perror(error);
		exit(1);
	}
	while ((n = read(fdr,buf,sizeof(buf)))>0)
		if (write(fdw, buf, n) != n) {
        		error[0] = '\0';
        		strcat(error,"\nerror in writing ");
        		strcat(error,arg2);
        		perror(error);
        		exit(1);
        	}
		if (n < 0) {
			error[0] = '\0';
			strcat(error,"\nerror in reading ");
			strcat(error,arg1);
			perror(error);
			exit(1);
		}
}
int openfile(char *file)
{
	int fd;
	fd = open(file,O_RDONLY);
	if (fd < 0){ 
		error[0]='\0';
		strcat(error,"\nerror in opening ");
		strcat(error,file);
		perror(error);
		exit(1);
	}
	return fd;
}
int cmp(char a[],char b[])
{
	int i,l;
	l = strlen(a);
	for (i = 0; i<l; i++){
		if (a[i]!=b[i]){
		//printf(",%d,%x,%x\n",i,a[i],b[i]);
			return 0;
		}
	}
	return 1;
}
