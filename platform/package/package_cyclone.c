/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org>
*/
#include "package_private.h"

/* Reverse Engineering of Cyclone's ART packages
 * usages:
 *  Requiem Avenging Angel
 *
 * Seems to be made up of a combination of both
 * an ART package and DAT package? One assumes
 * that ART contains the actual texture data and
 * the other, DAT, contains indexes?
 *
 * The textures in the game are referred to by name
 * which means there's potentially some compression
 * going on here :(
 */

typedef struct DATIndex {

} DATIndex;

/* First two bytes are different depending upon
 * the type of ART package it is, followed by two
 * blank bytes. Can be assumed this
 */
typedef struct ARTHeader {
    uint32_t unknown0;
    uint32_t unknown2;
} ARTHeader;

PLPackage *LoadARTPackage(const char *filename, bool precache) {
#if 0
    char art_path[PL_SYSTEM_MAX_PATH] = { '\0' };
    char dat_path[PL_SYSTEM_MAX_PATH] = { '\0' };
#endif

    const char *extension = plGetFileExtension(filename);
    if(!strcmp(extension, "art")) {

    }

    return NULL;
}