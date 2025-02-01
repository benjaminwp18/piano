#include <ao/ao.h>
#include <assert.h>
#include <ctype.h>
#include <err.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#define TWRT2 1.05946 /* 12th root of 2 */
#define TWOPI (3.14159 * 2.0)
#define MAXBEATS 20
#define CHORD_SIZE 20

static double trumpet(double);

static struct termios oldattr;
static size_t samps_per_beat;
static ao_sample_format snd_format;
static ao_device *snd_device;
static int16_t *sndbuf;

static double (*wave_func)(double) = trumpet;
static bool piano_layout	   = false;
static const char *chromatic_chars = "qwertyuiopasdfghjklzxcvbnm";
static const char *piano_chars	   = "awsedftgyhujkolp;']zbxncv";
static unsigned int beat_duration  = 100;
static unsigned int amplitude	   = INT16_MAX / 2;
// frequency of the lowest note
static float base_freq		   = 185.0;

static void
initsound(void)
{
	ao_initialize();
	atexit(ao_shutdown);
	int default_driver = ao_default_driver_id();

	snd_format.bits = 16;
	snd_format.channels = 1;
	snd_format.rate = 44100;
	snd_format.byte_format = AO_FMT_NATIVE;

	snd_device = ao_open_live(default_driver, &snd_format, NULL);
	if (snd_device == NULL)
		errx(1, "Can't open ao device");

	samps_per_beat = (snd_format.rate * beat_duration) / 1000;
	sndbuf = malloc(sizeof(*sndbuf) * samps_per_beat * MAXBEATS);
}

static void
playsnd(unsigned int beats)
{
	assert(beats <= MAXBEATS);
	if (beats == 0)
		beats = 1;
	size_t nbytes = samps_per_beat * beats * sizeof(*sndbuf);
	ao_play(snd_device, (char *)sndbuf, nbytes);
}

static double
trumpet(double theta)
{
	return powf(sinf(theta), 18.7);
}

static double
sawtooth(double theta)
{
	return 2.0 * fmod(theta, TWOPI) / TWOPI - 1.0;
}

static double
triangle(double theta)
{
	return fabs(fmod(theta, 4) - 2) - 1;
}

static void
beep(int freq, unsigned int beats, float amplitude_divisor)
{
	assert(beats <= MAXBEATS);
	if (beats == 0) beats = 1;
	size_t nsamps = samps_per_beat * beats;
	float ease_width = nsamps / 8;
	float theta = 0;

	for (size_t i = 0; i < nsamps; i++) {
		float amp = amplitude / amplitude_divisor;
		if (i < ease_width)
			amp *= (float)i / ease_width;
		else if (i > nsamps - ease_width)
			amp *= (float)(nsamps - i) / ease_width;

		sndbuf[i] += wave_func(theta) * amp;
		theta += TWOPI * ((float)freq) / ((float)snd_format.rate);
		theta = fmod(theta, TWOPI);
	}
}

static void
rest(unsigned int beats)
{
	assert(beats <= MAXBEATS);
	if (beats == 0) beats = 1;
	memset(sndbuf, 0, sizeof(*sndbuf) * samps_per_beat * beats);
	playsnd(beats);
}

static void
resetterm(void)
{
	tcsetattr(STDIN_FILENO, TCSANOW, &oldattr);
}

static void
setterm(void)
{
	tcgetattr(STDIN_FILENO, &oldattr);

	struct termios attr;
	memcpy(&attr, &oldattr, sizeof(attr));
	attr.c_lflag &= ~ICANON;
	attr.c_cc[VMIN] = 1;
	attr.c_cc[VTIME] = 0;

	tcsetattr(STDIN_FILENO, TCSAFLUSH, &attr);
	atexit(resetterm);
}

static int
getfreq(char ch)
{
	const char *chars = piano_layout ? piano_chars : chromatic_chars;
	const char *cptr = strchr(chars, ch);
	if (cptr == NULL)
		return -1;
	size_t idx = cptr - chars;

	return base_freq * powf(TWRT2, idx);
}

static void
parseargs(int argc, char **argv)
{
	int ch;
	char *endptr;
	unsigned int a;
	float b;
	while ((ch = getopt(argc, argv, "a:b:d:pt:")) != -1) {
		switch (ch) {
		case 'a':
			a = strtoul(optarg, &endptr, 10);
			if (*optarg == '\0' || *endptr != '\0' || a > 100)
				errx(1, "invalid amplitude");
			amplitude = (float)INT16_MAX * ((float)a / 100.0);
			break;
		case 'b':
			b = strtof(optarg, &endptr);
			if (*optarg == '\0' || *endptr != '\0' || b < 20 || b > 20000)
				errx(1, "invalid base frequency");
			base_freq = b;
			break;
		case 'd':
			beat_duration = strtoul(optarg, &endptr, 10);
			if (*optarg == '\0' || *endptr != '\0')
				errx(1, "invalid duration");
			break;
		case 'p':
			piano_layout = true;
			break;
		case 't':
			if (strcmp(optarg, "sawtooth") == 0)
				wave_func = sawtooth;
			else if (strcmp(optarg, "sine") == 0)
				wave_func = sin;
			else if (strcmp(optarg, "triangle") == 0)
				wave_func = triangle;
			else
				errx(1, "invalid timbre");
			break;
		default:
			exit(1);
		}
	}
	argc -= optind;
	argv += optind;
}

int
main(int argc, char **argv)
{
	parseargs(argc, argv);
	initsound();

	int is_tty = isatty(STDIN_FILENO);
	if (is_tty)
		setterm();

	char ch;
	unsigned int nbeats = 0;
	ssize_t nread;
	bool building_chord = false;
	int chord[CHORD_SIZE] = {0};
	size_t chordpos = 0;
	int octave = 0;
	while ((nread = read(STDIN_FILENO, &ch, sizeof(ch))) != 0) {
		if (nread == -1)
			err(1, "read");
		if (is_tty && ch == oldattr.c_cc[VEOF])
			break;

		int freq;
		if (ch == '-') {
			octave -= 1;
		} else if (ch == '+') {
			octave += 1;
		} else if (ch == '`') {
			if (building_chord) {
				memset(sndbuf, 0, sizeof(*sndbuf) * samps_per_beat * MAXBEATS);
				for (size_t i = 0; i < chordpos; i++) {
					if (chord[i] != 0) {
						beep(chord[i], nbeats, chordpos);
					}
					chord[i] = 0;
				}
				playsnd(nbeats);
				nbeats = 0;
				chordpos = 0;
			}

			building_chord = !building_chord;
		} else if (building_chord && (freq = getfreq(ch)) != -1) {
			freq *= powf(2, octave);
			chord[chordpos] = freq;
			if (chordpos + 1 < CHORD_SIZE) {
				chordpos++;
			}
		} else {
			if (isdigit(ch)) {
				nbeats = nbeats * 10 + (ch - '0');
				if (nbeats > MAXBEATS)
					nbeats = MAXBEATS;
			} else if (ch == ' ') {
				rest(nbeats);
				nbeats = 0;
			} else if ((freq = getfreq(ch)) != -1) {
				freq *= powf(2, octave);
				memset(sndbuf, 0, sizeof(*sndbuf) * samps_per_beat * MAXBEATS);
				beep(freq, nbeats, 1);
				playsnd(nbeats);
				nbeats = 0;
				octave = 0;
			}

		}
	}
}
