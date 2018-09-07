/*

  MASTER MIDI MASTER 
                   gcc -o control -lasound control.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <getopt.h>
#include <string.h>
#include <alsa/asoundlib.h>
#include <curses.h>
#define MIDI_CHANNEL 7
#define CONTROL 0x58

#define MIDI_PROGRAM 0
#define TICKS_PER_QUARTER 120
#define TIME_SIGNATURE_NUM 4
#define TIME_SIGNATURE_FIG 4
#define BPM 100
#define FALSE 0
#define TRUE  1

char *port_address;
snd_seq_t *seq_handle;
int queue_id, port_in_id, port_out_id;

int measure = 0;
int bpm = BPM;
int resolution = TICKS_PER_QUARTER;
int channel = MIDI_CHANNEL;

int num_parts = TIME_SIGNATURE_NUM;
int part_fig = TIME_SIGNATURE_FIG;
int first = TRUE;

FILE *songraw;
struct section {
	int bar;
	int tempo;
	int arp;
};
struct section song[256];
int songend=0;
int songlth=0;
int sect=1;

       	

void usage()
{
	printf(
	        "Usage: \n"
		"  control [-o|--output CLIENT:PORT] [-r|--resolution PPQ] [-s|--song file]\\n"
		"\n"
		"Options:\n"
		"  -h, --help         This message\n"
		"  -s, --song         song file for control program\n"
		"  -o, --output       Pair of CLIENT:PORT, as ALSA numbers or names\n"
		"  -r, --resolution   Tick resolution per quarter note (PPQ), default 120\n");
}

void open_sequencer()
{
	if (snd_seq_open(&seq_handle, "default", SND_SEQ_OPEN_DUPLEX, SND_SEQ_NONBLOCK) < 0) {
		fprintf(stderr, "Error opening ALSA sequencer\n");
		exit(EXIT_FAILURE);
	}
	snd_seq_set_client_name(seq_handle, "Metronome");
	if ((port_out_id = snd_seq_create_simple_port(seq_handle, "output",
			SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ,
			SND_SEQ_PORT_TYPE_APPLICATION | SND_SEQ_PORT_TYPE_MIDI_GENERIC )) < 0) {
		fprintf(stderr, "Error creating output port\n");
		snd_seq_close(seq_handle);
		exit(EXIT_FAILURE);
	}
	if ((port_in_id = snd_seq_create_simple_port(seq_handle, "input",
			SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE,
			SND_SEQ_PORT_TYPE_APPLICATION | SND_SEQ_PORT_TYPE_MIDI_GENERIC)) < 0) {
		fprintf(stderr, "Error creating input port\n");
		snd_seq_close(seq_handle);
		exit(EXIT_FAILURE);
	}
}

/* Subscribe to a dest port. It's name or address must allready in the
 * global variable port_address.
 */

void subscribe()
{
	snd_seq_addr_t dest, source;
	snd_seq_port_subscribe_t *subs;

	source.client = snd_seq_client_id(seq_handle);
	source.port = port_out_id;
	if (snd_seq_parse_address(seq_handle, &dest, port_address) < 0) {
		fprintf(stderr, "Invalid source address %s\n", port_address);
		snd_seq_close(seq_handle);
		exit(EXIT_FAILURE);
	}
	snd_seq_port_subscribe_alloca(&subs);
	snd_seq_port_subscribe_set_sender(subs, &source);
	snd_seq_port_subscribe_set_dest(subs, &dest);
	snd_seq_port_subscribe_set_queue(subs, queue_id);
	snd_seq_port_subscribe_set_time_update(subs, 1);
	if (snd_seq_get_port_subscription(seq_handle, subs) == 0) {
		fprintf(stderr, "Connection is already subscribed\n");
		snd_seq_close(seq_handle);
		exit(EXIT_FAILURE);
	}
	if (snd_seq_subscribe_port(seq_handle, subs) < 0) {
		fprintf(stderr, "Connection failed (%s)\n", snd_strerror(errno));
		snd_seq_close(seq_handle);
		exit(EXIT_FAILURE);
	}
}

void unsubscribe()
{ 
       	snd_seq_addr_t dest, source;
	snd_seq_port_subscribe_t *subs;

	source.client = snd_seq_client_id(seq_handle);
	source.port = port_out_id;
	snd_seq_port_subscribe_alloca(&subs);
	snd_seq_port_subscribe_set_sender(subs, &source);
	snd_seq_port_subscribe_set_dest(subs, &dest);
	snd_seq_port_subscribe_set_queue(subs, queue_id);
	snd_seq_port_subscribe_set_time_update(subs, 1);
	snd_seq_unsubscribe_port(seq_handle, subs);
}
/*
 * Queue commands
 *
 */

void create_queue()
{
 	queue_id = snd_seq_alloc_queue(seq_handle);

}

void set_tempo(int tempo)
{
	snd_seq_queue_tempo_t *queue_tempo;
	int truetempo = (int) (6e7 / (tempo));

	snd_seq_queue_tempo_alloca(&queue_tempo);
	snd_seq_queue_tempo_set_tempo(queue_tempo, truetempo);
	snd_seq_queue_tempo_set_ppq(queue_tempo, resolution);
	snd_seq_set_queue_tempo(seq_handle, queue_id, queue_tempo);
}

void clear_queue()
{
	snd_seq_remove_events_t *remove_ev;

	snd_seq_remove_events_alloca(&remove_ev);
	snd_seq_remove_events_set_queue(remove_ev, queue_id);
	snd_seq_remove_events_set_condition(remove_ev,
					    SND_SEQ_REMOVE_OUTPUT |
					    SND_SEQ_REMOVE_IGNORE_OFF);
	snd_seq_remove_events(seq_handle, remove_ev);
}

void start_queue()
{
	snd_seq_start_queue(seq_handle, queue_id, NULL);
	snd_seq_drain_output(seq_handle);
}

void stop_queue()
{
	snd_seq_stop_queue(seq_handle, queue_id, NULL);
	snd_seq_drain_output(seq_handle);
}

void continue_queue()
{
	snd_seq_continue_queue(seq_handle, queue_id, NULL);
	snd_seq_drain_output(seq_handle);
}

/*
 * Event commands
 *
 */

void make_CC(unsigned char mess, unsigned char data, int tick)
{
	snd_seq_event_t ev;
	snd_seq_ev_clear(&ev);
	snd_seq_ev_set_controller(&ev, channel, mess, data);
	snd_seq_ev_schedule_tick(&ev, queue_id, 1, tick);
	snd_seq_ev_set_source(&ev, port_out_id);
	snd_seq_ev_set_subs(&ev);
	snd_seq_event_output_direct(seq_handle, &ev);
}
void make_echo(int tick)
{
	snd_seq_event_t ev;
	snd_seq_ev_clear(&ev);
	ev.type = SND_SEQ_EVENT_USR1;
	snd_seq_ev_schedule_tick(&ev, queue_id, 1, tick);
	snd_seq_ev_set_dest(&ev, snd_seq_client_id(seq_handle), port_in_id);
	snd_seq_event_output_direct(seq_handle, &ev);
}


void make_clock(int tick)
{
	snd_seq_event_t ev;
	snd_seq_ev_clear(&ev);
	ev.type = SND_SEQ_EVENT_CLOCK;
	snd_seq_ev_schedule_tick(&ev, queue_id, 1, tick);
	snd_seq_ev_set_source(&ev, port_out_id);
	snd_seq_ev_set_subs(&ev);
	snd_seq_event_output_direct(seq_handle, &ev);
}
 
void start_clock(int tick)
{
	snd_seq_event_t ev;
	snd_seq_ev_clear(&ev);
	ev.type = SND_SEQ_EVENT_START;
	snd_seq_ev_schedule_tick(&ev, queue_id, 1, tick);
	snd_seq_ev_set_source(&ev, port_out_id);
	snd_seq_ev_set_subs(&ev);
	snd_seq_event_output_direct(seq_handle, &ev);
}
void stop_clock(int tick)
{
	snd_seq_event_t ev;
	snd_seq_ev_clear(&ev);
	ev.type = SND_SEQ_EVENT_STOP;
	snd_seq_ev_schedule_tick(&ev, queue_id, 1, tick);
	snd_seq_ev_set_source(&ev, port_out_id);
	snd_seq_ev_set_subs(&ev);
	snd_seq_event_output_direct(seq_handle, &ev);
        unsubscribe();
}
void pattern()
{
	int j, tick, duration;
        if(first) {
	        start_clock(tick);
                first = FALSE;
         }
	duration = resolution / 24;
	for (tick = 0; tick < (resolution * 4); tick += duration) {
		make_clock(tick);
	}
	tick = 0;
	duration = resolution * 4 / part_fig;
	for(j = 0; j < num_parts; j++) {
      	   if(measure == song[sect].bar) {
	 	make_CC(CONTROL,song[sect].arp,tick);
		sect++;
	    }
	    tick += duration;
	}
	make_echo(tick);
	printw("measure: %5d\r", ++measure);
}

void midi_action()
{
	snd_seq_event_t *ev;

	do {
		snd_seq_event_input(seq_handle, &ev);
		switch (ev->type) {
		case SND_SEQ_EVENT_USR1:
			pattern();
			break;
		case SND_SEQ_EVENT_START:
			start_queue();
			pattern();
			break;
		case SND_SEQ_EVENT_CONTINUE:
			continue_queue();
			break;
		case SND_SEQ_EVENT_STOP:
			stop_queue();
			break;
		}
	} while (snd_seq_event_input_pending(seq_handle, 0) > 0);
}


void sigterm_exit(int sig)
{
	clear_queue();
	sleep(1);
	snd_seq_stop_queue(seq_handle, queue_id, NULL);
	snd_seq_free_queue(seq_handle, queue_id);
	snd_seq_close(seq_handle);
	exit(0);
}

int check_range(int val, int min, int max, char *msg)
{
	if ((val < min) | (val > max)) {
		fprintf(stderr, "Invalid %s, range is %d to %d\n", msg, min, max);
		return 1;
	}
	return 0;
}

int parse_options(int argc, char *argv[])
{
	int c;
	long x;
	char *sep;
	int option_index = 0;
	struct option long_options[] = {
		{"help",        0, 0, 'h'},
		{"song",        1, 0, 's'},
		{"output",      1, 0, 'o'},
		{"resolution",  1, 0, 'r'},
		{0, 0, 0, 0}
	};

	while (1) {
		c = getopt_long(argc, argv, "ho:r:s:t:",
				long_options, &option_index);
		if (c == -1)
			break;

		switch (c) {
		case 's':
			songraw=fopen(optarg,"r");
			break;
		case 'o':
			port_address = optarg;
			break;
		case 'r':
			resolution = atoi(optarg);
			if (check_range(resolution, 48, 480, "resolution"))
				return 1;
			break;
		case 0:
		case 'h':
		default:
			return 1;
		}
	}
	return 0;
}

int main(int argc, char *argv[])
{
	int npfd, j, i;
	struct pollfd *pfd;
        char key=0x00;
	if (parse_options(argc, argv) != 0) {
		usage();
		return EXIT_FAILURE;
	}
	if (port_address == NULL) {
		port_address = getenv("ALSA_OUTPUT_PORTS");
		/* Try the old name for the environment variable */
		if (port_address == NULL)
			port_address = getenv("ALSA_OUT_PORT");
		if (port_address == NULL) {
			fprintf(stderr, "No client/port specified. "
				"You must supply one as argument or with the\n"
				"environment variable ALSA_OUTPUT_PORTS.\n");
			usage();
			return EXIT_FAILURE;
		}
	}

	for(i=0; i<256; i++) {
	   if(fscanf(songraw,"%d %d %d", &song[i].bar, &song[i].tempo, &song[i].arp) != 3) {
	      songend = song[i].bar;
	      break;
	   }
	}
	
	fclose(songraw);

	printf("amaster - MIDI ALSA Master CLock Via Sequencer\n");
	printf("press s to start,then q to stop\n");
        while(getchar() != 's');
	initscr();
	cbreak();
	noecho();
	scrollok(stdscr, TRUE);
	nodelay(stdscr,TRUE);

	signal(SIGINT, sigterm_exit);
	signal(SIGTERM, sigterm_exit);

	open_sequencer();
	create_queue();
	subscribe();

	npfd = snd_seq_poll_descriptors_count(seq_handle, POLLIN);
	pfd = (struct pollfd *) alloca(npfd * sizeof(struct pollfd));
	snd_seq_poll_descriptors(seq_handle, pfd, npfd, POLLIN);

	bpm = song[0].tempo;
	set_tempo(bpm);
	start_queue();
	pattern();

	while ((key != 'q') && (measure <= songend)) {
		if (poll(pfd, npfd, 1000) > 0) {
			for (j = 0; j < npfd; j++) {
				if (pfd[j].revents > 0)
					midi_action();
			}
		}
        	key = getch();
	}
	endwin();
        stop_clock(0);
	return 0;
}
