/* 
 * File:   types.h
 * Author: torsten.roemer@luniks.net
 *
 * Created on 17. September 2023, 20:33
 */

#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>
#include <stdint.h>

/* Width, height and color space of bitmaps and glyphs */
typedef uint16_t    width_t;
typedef uint16_t    height_t;
typedef uint8_t     space_t;

/* Width * height * bytes per pixel */
typedef uint32_t    bytes_t;

/* X and Y coordinates of the display */
typedef uint16_t    x_t;
typedef uint16_t    y_t;

/* Char code (like UTF-8 code point) */
typedef uint8_t     code_t;

/* Number of glyphs of a font */
typedef uint8_t     length_t;

/**
 * A point with its x and y coordinates.
 */
typedef struct {
    int16_t x;
    int16_t y;
} Point;

/**
 * Flags for "payload ready" event.
 */
typedef struct {
    bool ready;
    bool crc;
} PayloadFlags;

/**
 * Temperature read from transmitter including
 * additional information.
 */
typedef struct {
    uint16_t raw;
    uint8_t power;
} Temperature;

/**
 * Pointer to a function that takes no argument and returns void.
 */
typedef void (*Function)(void);

/**
 * Pointer to a function that takes an array of bytes
 * and returns a boolean.
 */
typedef bool (*Consumer)(uint8_t*);

#endif /* TYPES_H */

