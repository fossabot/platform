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

#include "platform_image.h"
#include "platform_filesystem.h"

PLresult plLoadImage(const PLchar *path, PLImage *out) {
    plFunctionStart();

    if (!plIsValidString(path)) {
        return PL_RESULT_FILEPATH;
    }

    PLresult result = PL_RESULT_FILETYPE;

    // Xenon uses a lot of long extensions, as do some other modern
    // applications, so that's why we're using a size 16.

    FILE *fin = fopen(path, "rb");
    if(!fin) {
        return PL_RESULT_FILEREAD;
    }

    if(strrchr(path, ':')) {
        // Very likely a packaged image.
        // example/package.wad:myimage
    }

    if(_plDDSFormatCheck(fin)) {
        result = _plLoadDDSImage(fin, out);
    } else if(_plVTFFormatCheck(fin)) {
        result = _plLoadVTFImage(fin, out);
    } else if(_plDTXFormatCheck(fin)) {
        result = _plLoadDTXImage(fin, out);
    } else if(_plTIFFFormatCheck(fin)) {
        result = _plLoadTIFFImage(path, out);
    } else {
        const PLchar *extension = plGetFileExtension(path);
        if(plIsValidString(extension)) {
            if (!strncmp(extension, PLIMAGE_EXTENSION_FTX, 3)) {
                result = _plLoadFTXImage(fin, out);
            } else if (!strncmp(extension, PLIMAGE_EXTENSION_PPM, 3)) {
                result = _plLoadPPMImage(fin, out);
            }
        }
    }

    fclose(fin);

    if(result == PL_RESULT_SUCCESS) {
        strncpy(out->path, path, sizeof(out->path));
    }

    return result;
}

// Returns the number of samples per-pixel depending on the colour format.
PLuint plGetSamplesPerPixel(PLColourFormat format) {
    plFunctionStart();

    switch(format) {
        case PL_COLOURFORMAT_ABGR:
        case PL_COLOURFORMAT_ARGB:
        case PL_COLOURFORMAT_BGRA:
        case PL_COLOURFORMAT_RGBA:
            return 4;
        case PL_COLOURFORMAT_BGR:
        case PL_COLOURFORMAT_RGB:
            return 3;
    }

    return 0;
}

PLuint _plGetImageSize(PLImageFormat format, PLuint width, PLuint height) {
    plFunctionStart();

    switch(format) {
        case PL_IMAGEFORMAT_RGB_DXT1:   return (width * height) >> 1;
        case PL_IMAGEFORMAT_RGBA_DXT1:  return width * height * 4;
        case PL_IMAGEFORMAT_RGBA_DXT3:
        case PL_IMAGEFORMAT_RGBA_DXT5:  return width * height;

        case PL_IMAGEFORMAT_RGB5A1:     return width * height * 2;
        case PL_IMAGEFORMAT_RGB8:
        case PL_IMAGEFORMAT_RGB565:     return width * height * 3;
        case PL_IMAGEFORMAT_RGBA4:
        case PL_IMAGEFORMAT_RGBA8:      return width * height * 4;
        case PL_IMAGEFORMAT_RGBA16F:
        case PL_IMAGEFORMAT_RGBA16:     return width * height * 8;

        default:    return 0;
    }
}

void _plFreeImage(PLImage *image) {
    plFunctionStart();

    if (!image || !image->data) {
        return;
    }

    for(PLuint levels = 0; levels < image->levels; ++levels) {
        if(!image->data[levels]) {
            continue;
        }

        free(image->data[levels]);
    }

    free(image->data);
}

PLbool plIsValidImageSize(PLuint width, PLuint height) {
    plFunctionStart();

    if((width < 2) || (height < 2)) {
        return false;
    } else if(!plIsPowerOfTwo(width) || !plIsPowerOfTwo(height)) {
        return false;
    }

    return true;
}