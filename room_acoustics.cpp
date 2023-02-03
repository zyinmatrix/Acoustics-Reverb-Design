/* Program to model room acoustics using acoustic image method */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>     /* sleep() */
#include <stdbool.h>    /* true, false */
#include <string.h>     /* memset */
#include <ctype.h>      /* loupper() */
#include <math.h>       /* round */
#include <sndfile.h>	/* libsndfile */
#include <portaudio.h>  /* portaudio */
#include "room_acoustics.h"
#include "paUtils.h"

/* data structure to pass to callback */
typedef struct {
    /* pointer to room data structure */
    Room *room;
    /* input signal */
    float *signal;
    int frames;
    int channels;
    int buf_frames;
    int next_frame;
} callBackData;

/* room data structure 
 * room must be a global to be accessible by display functions
 */
Room room_struct, *room = &room_struct;

/* globals for debug output signal */
SNDFILE *osfile = 0;
float *osignal = 0;
int ocount = 0;

/* PortAudio callback function protoype */
int paCallback( 
    const void *inputBuffer, void *outputBuffer,
    unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void *userData );

/* function prototypes for external functions */
float parametric_reverb(float x, float reflect);
void clear_reverb(void);
void display_ncurses( void );
void display_printf( void );
void display_openGL( void );

int main(int argc, char *argv[])
{
    char *ifile;
    int num_samples, buf_samples;
    float *signal;
    /* for debug output wav file */
    char *ofile = 0;
    SF_INFO osfinfo;

    /* libsndfile data structures */
    SNDFILE *sfile;
    SF_INFO sfinfo;
    /* Zero libsndfile structures */
    memset(&sfinfo, 0, sizeof(sfinfo));
    memset(&osfinfo, 0, sizeof(osfinfo));

    /* My callback data structure */
    callBackData my_data;
    /* PortAudio stream */
    PaStream *stream;

    /* parse command line 
     * print usage
     * open input WAV file and check that it is mono
     */
    //YOUR CODE HERE
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s signal/vega_mono.wav\n", argv[0]);
        return -1;
    }

    ifile = argv[1];
    if ((sfile = sf_open(ifile, SFM_READ, &sfinfo)) == nullptr)
    {
        fprintf(stderr, "Cannot read %s\n", ifile);
        return -1;
    }

    if (sfinfo.channels != 1)
    {
        fprintf(stderr, "%s should be mono\n", ifile);
        return -1;
    }

    /* allocate buffer to hold input signal, using calloc()
     * buffer must hold the entire signal plus reverb "ring-out" interval
     * that is, (sfinfo.frames + sfinfo.samplerate*MAX_REVERB) * sfinfo.channels) 
     */
    //YOUR CODE HERE
    signal = (float *)malloc((sfinfo.frames + sfinfo.samplerate*MAX_REVERB) * sfinfo.channels);
    if (signal == nullptr)
    {
        fprintf(stderr, "Error when malloc storage\n");
    }

    /* read input signal 
     * remainder of buffer will be zeros
     * close input WAV file
     */

    /* set up callback data structure */
    my_data.room = room;
    my_data.signal = signal;
    my_data.frames = sfinfo.frames;
    my_data.channels = sfinfo.channels;
    my_data.buf_frames = buf_samples; // works since input is mono
    my_data.next_frame = 0;

    /* open optional debug output wav file */
    if (ofile) {
        printf("Opening %s\n", ofile);
        osfinfo.samplerate = sfinfo.samplerate;
        osfinfo.channels = sfinfo.channels;
        osfinfo.format = sfinfo.format;
        if ( (osfile = sf_open (ofile, SFM_WRITE, &osfinfo)) == NULL ) {
            fprintf (stderr, "ERROR: could not open output wav file: %s\n", ofile);
            return -1;
        }
        if ( (osignal = (float*)calloc(buf_samples, sizeof(osignal))) == NULL) {
            fprintf(stderr, "ERROR: cannot calloc buffer\n");
            return -1;
        }
        ocount = 0;
    }
    else {
        osfile = 0;
        osignal = 0;
        ocount = 0;
    }

    /* room is centered at (0, 0, 0)
     * with initial dimensions XDIM, YDIM, ZDIM
     */
    room->dx = XDIM/2.0;
    room->dy = YDIM/2.0;
    room->dz = ZDIM/2.0;
    /* source and listener initial positions */
    room->source.x = -10;
    room->source.y = -5;
    room->source.z = -2;
    room->listener.x = 5;
    room->listener.y = 0;
    room->listener.z = 0;
    /* reflections */
    room->wall_reflection = W_REF;
    room->num_images = 6;   // room has 6 sides, one image per side
    /* switches */
    room->delay_info = true;
    room->images = true;
    room->parametric = PARA_STATE;
    /* sampling rate */
    room->samplerate = sfinfo.samplerate;

    /* start up Port Audio */
    stream = startupPa(1, 2, 
        sfinfo.samplerate, FRAMES_PER_BUFFER, paCallback, &my_data);

    /* Display room and take user input to modify 
     * room and listener position (and hence source image positions)
     */
    DISPLAY();

    /* shut down Port Audio */
    shutdownPa(stream);

    /* close debug file */
    if (osfile) {
        sf_close (osfile);        
    }
    return 0;
}

/* callback */
int paCallback( 
    const void *inputBuffer, void *outputBuffer,
    unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void *userData )
{
    /* Cast data passed to callback to our structure type */
    callBackData *pd = (callBackData *)userData; 
    float *output = (float *)outputBuffer;
    //float *input = (float *)inputBuffer; //not used in this program

    /* only consider direct sound if enable_images is false */
    int num_images = pd->room->images ? pd->room->num_images : 0;
    bool enable_parametric = pd->room->parametric;

    //YOUR CODE HERE
    /* input is mono, output is stereo 
     * for each output sample, sum direct and image valuess into output signal 
     * considering delay and attenuation
     */


    /* increment pointer to next sample */
    pd->next_frame += framesPerBuffer;
    /* if next output buf processing puts this past end of signal buffer
     * reset to start of signal buffer 
     */
    if (pd->next_frame > pd->buf_frames - framesPerBuffer) {
        pd->next_frame = 0; 
#ifdef PARA_REVERB
    /* If parametric reverb enabled, then clear parametric reverb delay lines */
        clear_reverb();
#endif
        ocount = 0;
    }
    return 0;
}
