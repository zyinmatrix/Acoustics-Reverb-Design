#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>     /* memset */
#include <ctype.h>      /* loupper() */
#include <math.h>       /* round */
#include <sndfile.h>    /* libsndfile, for my debugging output file */
#include "room_acoustics.h"

/* function prototypes for functions in this file */
void print_info(Room *room);
void print_room(Room *room);

/* Clears screen using ANSI escape sequences. */
void clear()
{
    printf("\033[2J");
    printf("\033[%d;%dH", 0, 0);
}

void display_printf( void )
{
    int ch;
    char line[LINE_LEN];
    bool done;
    extern Room *room;
    extern int samplerate;
    Pt *ps = &room->source;
    Pt *pl = &room->listener;

    ch = '\0'; /* Init ch to null character */
    done = false;
    while ( !done ) {
        clear();
        calc_images( room );
        print_info(room);
        print_room( room );

        fgets( line, LINE_LEN, stdin);
        ch = line[0];

        Pt t;
        /* temp values */
        t.x = pl->x;
        t.y = pl->y;
        t.z = pl->z;
        /* integer room dimensions */
        int idx = round(room->dx);
        int idy = round(room->dy);
        switch(ch) {
            /* move listener
             * but check that move is not closer than 1 m from room wall 
             * or closer than 2 m from source
             */
            case 'L':
                t.x -= 1;
                if ( t.x > -idx && get_dist(&t, ps) > MIN_DIST) {
                    pl->x = t.x;
                }
                break;
            case 'U':
                t.y -= 1;
                if ( t.y > -idy && get_dist(&t, ps) > MIN_DIST) {
                    pl->y = t.y;
                }
                break;
            case 'R':
                 t.x += 1;
                if ( t.x < idx && get_dist(&t, ps) > MIN_DIST) {
                    pl->x = t.x;
                }
                break;
            case 'D':
                t.y += 1;
                if ( t.y < idy && get_dist(&t, ps) > MIN_DIST) {
                    pl->y = t.y;
                }
                break;
            case '+':
            case '-':
                if (ch == '+') {
                    room->dx *= 1.1;
                    room->dy *= 1.1;
                    room->dz *= 1.1;
                }
                else {
                    room->dx /= 1.1;
                    room->dy /= 1.1;
                    room->dz /= 1.1;
                }
                /* re-calculate source and listener positions */
                    room->source.x = -5;
                    room->source.y = 0;
                    room->source.z = 0;
                    room->listener.x = +5;
                    room->listener.y = 0;
                    room->listener.z = 0;
                break;
            case 'W':
                room->wall_reflection *= R_FAC;
                if (room->wall_reflection > 1.0)
                    room->wall_reflection = 1.0; //cannot be greater than 1.0
                break;
            case 'w':
                room->wall_reflection /= R_FAC;
                break;
            case 'I':
                room->images = true;
                break;
            case 'i':
                room->images = false;;
                break;
            case 'P':
                room->parametric = true;
                break;
            case 'p':
                room->parametric = false;;
                break;
            // case 'D':
            //     room->delay_info = true;
            //     break;
            // case 'd':
            //     room->delay_info = false;;
            //     break;
            case 'Q':
            case 'q':
                done = true;
                break;
        } 
    }
}

void print_info(Room *room) {
    Pt *ps = &room->source;
    Pt *pl = &room->listener;
    /* print configuration */
    printf("Source Position: %6.2f %6.2f %6.2f\n", ps->x, ps->y, ps->z);
    printf("Listener Position: %6.2f %6.2f %6.2f (%6.2f)\n", 
        pl->x, pl->y, pl->z, get_dist(ps, pl));
    printf("Room dimensions: %6.2f %6.2f %6.2f\n",
        room->dx*2, room->dy*2, room->dz*2);
    printf("\n");
    printf("Use (U,D,L,R) keys to move listener\n");
    printf("Use +/- keys to make room larger or smaller\n");
    printf("Use W/w keys to make wall reflection larger or smaller (%4.2f)\n",
        room->wall_reflection);
    printf("Use I/i keys to enable/disable phantom images (%s)\n", 
        room->images ? "on " : "off");
// #ifdef  PARA_REVERB
//         printf("Use P/p keys to enable/disable parametric reverberation (%s)\n",
//             room->parametric ? "on " : "off");
// #endif
    printf("Q to quit\n");
    printf("\n");
}

void print_room(Room *room)
{
    int dxi = round(room->dx);
    int dyi = round(room->dy);
    int sx = round(room->source.x);
    int sy = round(room->source.y);
    int lx = round(room->listener.x);
    int ly = round(room->listener.y);

    for (int y=-dyi; y<=dyi; y++) { //rows
        for (int x=-dxi; x<=dxi; x++) { //columns
            if (sx == x && sy == y) {
                printf("X");
            }
            else if(lx == x && ly == y) {
                printf("O");
            }
            else if ( (x == -dxi && y == -dyi) || //LL
                (x == -dxi && y == dyi) || //LR
                (x == dxi && y == -dyi) || //UL
                (x == dxi && y == dyi) ) { //UR
                printf("+");
                if (x == dxi) {
                    printf("\n");
                }
            }
            else if (x == -dxi || x == dxi) {
                printf("|");
                if (x == dxi) {
                    printf("\n");
                }
            }
            else if (y == -dyi || y == dyi) {
                printf("-");
            }
            else {
                printf(" ");
            }
        }
    }
}