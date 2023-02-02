#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>     /* strtok */
#include <math.h>       /* sqrt */
#include <ncurses.h>
#include "room_acoustics.h"

double get_dist(Pt *src, Pt *lis)
{
    double dist = 1.0; //just so things work initially
    //YOUR CODE HERE
    return dist;
}

double get_dist_atten(double d)
{
    /* normalize full volume to distance of MIN_DIST m 
     * intensity decreases at 1/(distance^2) 
     * level decreases at 1/distance
     */
    return MIN_DIST/d;
}

void calc_images(Room *room)
{
    Image *p;

    /* default for maximum image delay */
    room->max_delay = 0;

    /* calculate position of direct path and store in images[0] */
    p = &room->image[0];
    p->distance = get_dist(&room->source, &room->listener);
    p->samp_delay = round(room->samplerate * (p->distance/SpeedOfSound));
    p->atten = get_dist_atten(p->distance);;
    if (room->delay_info) {
        PRINT("Path %d: delay %d, atten %f\n", 0, p->samp_delay, p->atten);
    }

    /* calculate position of phantom sound images */
    for (int i=1; i<=room->num_images; i++) {
        double d;
        /* point to image */
        p = &room->image[i];
        switch (i) {
            /* Construct 6 images for the 6 wall surfaces of room
             * each wall is perpendicular to a dimension: x, y, z
             * and located at -dx, +dx, -dy, +dy, -dx, +dz.
             * case 1: reflect across wall at -dx
             * case 2: reflect across wall at +dx
             * case 3: reflect across wall at -dy
             * case 4: reflect across wall at +dy
             * case 5: reflect across wall at -dz
             * case 6: reflect across wall at +dz
             */
            case 1: //wall at -dx, or -(room->dx)
                d = abs(-(room->dx) - room->source.x); //distance from source to wall
                p->loc.x = room->source.x - 2*d; //source image position in x dimension
                p->loc.y = room->source.y; //no change to other dimensions
                p->loc.z = room->source.z;
                break;
            //YOUR CODE HERE -- add remaining cases 
        }
        /* Calculate the image distance, in meters (default unit) 
         * and from this, the sample delay
         * finally, calculate the attenuation due to wall reflection and 
         * distance attenuation
         */
        //YOUR CODE HERE

        /* calculate max delay 
         * use signal[next_frame - max_delay] as input to parametric reverb
         */
        if (p->samp_delay > room->max_delay) {
            room->max_delay = p->samp_delay;
        }

        /* print information about image path */
        if (room->delay_info) {
            PRINT("Path %d: delay %d, atten %f\n", i, p->samp_delay, p->atten);
        }
    }
}
