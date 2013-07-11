#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/inotify.h>

#define BUFSZ   16384

int main(int arg ,char *argv[]) {
        int ifd, wd, i, n;
        char buf[BUFSZ];
		
		if (arg != 2) {
				printf("take exactly one argument %d given",arg - 1);
				exit (1);
		}
        ifd = inotify_init();
        if (ifd < 0) {
                perror("cannot obtain an inotify instance");
				exit (1);
		}

        wd = inotify_add_watch(ifd,argv[1] , IN_DELETE|IN_CREATE|IN_MODIFY);
        if (wd < 0) {
                perror("cannot add inotify watch");
				exit (1);
		}

        while (1) {
                n = read(ifd, buf, sizeof(buf));
                if (n <= 0) {
                        perror("read problem");
						exit (1);
				}

                i = 0;
                while (i < n) {
                        struct inotify_event *ev;

                        ev = (struct inotify_event *) &buf[i];
                        if (ev->len)
                                printf("file %s\n", ev->name);
                        else
                                printf("unexpected event - wd=%d mask=%d\n",
                                       ev->wd, ev->mask);

                        i += sizeof(struct inotify_event) + ev->len;
                }
                printf("---\n");
        }
}
