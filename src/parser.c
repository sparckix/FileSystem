#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
int main(int argc, char **argv)
{
	int aflag = 0;
	int cflag = 0;
	int mflag = 0;
	char *cvalue = NULL;
	int index;
	int c;
	time_t ntime;
	opterr = 0;

	while ((c = getopt(argc, argv, "ad:cm")) != -1)
		switch (c) {
		case 'a':
			aflag = 1;
			break;
		case 'c':
			cflag = 1;
			break;
		case 'm':
			mflag = 1;
			break;
		case 'd':
			//cvalue = optarg;
			ntime = getdate(optarg, NULL);
			if (ntime == (time_t) - 1)
				printf("Formato de fecha invalido\n", optarg);
			struct tm *ts;
			char atime[80];
			ts = localtime(&ntime);
			strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S",
				 ts);
			printf("atime: %s", atime);
			break;
		case '?':
			if (optopt == 'd')
				fprintf(stderr,
					"La opción -%c requiere un argumento.\n",
					optopt);
			else if (isprint(optopt))
				fprintf(stderr, "Unknown option `-%c'.\n",
					optopt);
			else
				fprintf(stderr,
					"Unknown option character `\\x%x'.\n",
					optopt);
			return 1;
		default:
			abort();
		}

//       printf ("aflag = %d, cvalue = %s\n",
//             aflag, cvalue);

	for (index = optind; index < argc; index++)
		printf("Non-option argument %s\n", argv[index]);
	return 0;
}
