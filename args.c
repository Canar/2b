#include <getopt.h>

#define RATE 44100
#define CHAN 1

void usage(const char *prog_name) {
	fprintf(stderr, "Usage: %s [options]\n", prog_name);
	fprintf(stderr, "Reads raw PCM audio data from stdin and plays it via %s.\n",API);
	fprintf(stderr, "\nOptions:\n");
	fprintf(stderr, "  -r, --rate <rate>       Set the sample rate in Hz (default: %d)\n",RATE);
	fprintf(stderr, "  -c, --channels <chans>  Set the number of channels (default: %d)\n",CHAN);
	fprintf(stderr, "  -h, --help              Display this help message\n");
}
void handle_args(int argc,char**argv,int*chan,int*rate){
	static struct option long_options[] = {
		{"rate",     required_argument, 0, 'r'},
		{"channels", required_argument, 0, 'c'},
		{"help",     no_argument,       0, 'h'},
	{0,0,0,0}};

	int opt;
	int index = 0;

	*rate=RATE;
	*chan=CHAN;

	while ((opt = getopt_long(argc, argv, "r:c:h", long_options, &index)) != -1)
	switch (opt) {
	case 'r':
		(*rate) = (int)strtol(optarg,0,10);
		break;
	case 'c':
		(*chan) = (int)strtol(optarg,0,10);
		break;
	case 'h':
	case '?':
	default:
		usage(argv[0]);
		exit(0);
	}
}
