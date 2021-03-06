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
#include <PL/platform_image.h>
#include <PL/platform_filesystem.h>
#include <PL/platform_math.h>

PLresult plLoadImagef(FILE *fin, const char *path, PLImage *out) {
    if(!fin) {
        ReportError(PL_RESULT_FILEREAD, "invalid file handle");
        return PL_RESULT_FILEREAD;
    }

    PLresult result = PL_RESULT_FILETYPE;
    if(DDSFormatCheck(fin)) {
        result = LoadDDSImage(fin, out);
    } else if(TIMFormatCheck(fin)) {
        result = LoadTIMImage(fin, out);
    } else if(_plVTFFormatCheck(fin)) {
        result = _plLoadVTFImage(fin, out);
    } else if(_plDTXFormatCheck(fin)) {
        result = _plLoadDTXImage(fin, out);
    } else if(_plBMPFormatCheck(fin)) {
        result = _plLoadBMPImage(fin, out);
    } else {
        const char *extension = plGetFileExtension(path);
        if(plIsValidString(extension)) {
            if (!strncmp(extension, PLIMAGE_EXTENSION_FTX, 3)) {
                result = _plLoadFTXImage(fin, out);
            } else if (!strncmp(extension, PLIMAGE_EXTENSION_PPM, 3)) {
                result = _plLoadPPMImage(fin, out);
            }
        }
    }

    if(result == PL_RESULT_SUCCESS) {
        strncpy(out->path, path, sizeof(out->path));
    }

    return result;
}

bool plLoadImage(const char *path, PLImage *out) {
    if (!plIsValidString(path)) {
        ReportError(PL_RESULT_FILEPATH, "Invalid path, %s, passed for image!\n", path);
        return false;
    }

    FILE *fin = fopen(path, "rb");
    if(fin == NULL) {
        ReportError(PL_RESULT_FILEREAD, "Failed to load image, %s!\n", path);
        return false;
    }

    if(strrchr(path, ':')) {
        // Very likely a packaged image.
        // example/package.wad:myimage
    }

    PLresult result = plLoadImagef(fin, path, out);

    fclose(fin);

    return result;
}

PLresult plWriteImage(const PLImage *image, const char *path) {
    if (!plIsValidString(path)) {
        return PL_RESULT_FILEPATH;
    }

    PLresult result = PL_RESULT_FILETYPE;
#if 0
    const char *extension = plGetFileExtension(path);
    if(plIsValidString(extension)) {
        if (!strncmp(extension, PLIMAGE_EXTENSION_TIFF, 3)) {
            result = plWriteTIFFImage(image, path);
        } else {
            // todo, Write BMP or some other easy-to-go format.
        }
    }
#endif

    return result;
}

// Returns the number of samples per-pixel depending on the colour format.
unsigned int plGetSamplesPerPixel(PLColourFormat format) {
    switch(format) {
        case PL_COLOURFORMAT_ABGR:
        case PL_COLOURFORMAT_ARGB:
        case PL_COLOURFORMAT_BGRA:
        case PL_COLOURFORMAT_RGBA: {
            return 4;
        }

        case PL_COLOURFORMAT_BGR:
        case PL_COLOURFORMAT_RGB: {
            return 3;
        }
    }

    return 0;
}

bool plConvertPixelFormat(PLImage *image, PLImageFormat new_format) {
    /* TODO: Make this thing more extensible... */
    if(image->format == PL_IMAGEFORMAT_RGB5A1 && new_format == PL_IMAGEFORMAT_RGBA8) {
        uint8_t *levels[image->levels];
        memset(levels, 0, image->levels * sizeof(uint8_t*));

        /* Make a new copy of each detail level in the new format. */

        unsigned int lw = image->width;
        unsigned int lh = image->height;

        for(unsigned int l = 0; l < image->levels; ++l) {
            levels[l] = _plImageDataRGB5A1toRGBA8(image->data[l], lw * lh);
            if(levels[l] == NULL) {
                /* Memory allocation failed, ditch any buffers we've created so far. */
                for(unsigned int m = 0; m < image->levels; ++m) {
                    free(levels[m]);
                }

                ReportError(PL_RESULT_MEMORYALLOC, "Couldn't allocate memory for image data");
                return false;
            }

            lw /= 2;
            lh /= 2;
        }

        /* Now that all levels have been converted, free and replace the old buffers. */

        for(unsigned int l = 0; l < image->levels; ++l) {
            free(image->data[l]);
            image->data[l] = levels[l];
        }

        image->format = new_format;
        /* TODO: Update colour_format */

        return true;
    }

    ReportError(PL_RESULT_IMAGEFORMAT, "Unsupported image format conversion");
    return false;
}

unsigned int _plGetImageSize(PLImageFormat format, unsigned int width, unsigned int height) {
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

void _plAllocateImage(PLImage *image, PLuint size, PLuint levels) {
    image->data = (uint8_t**)calloc(levels, sizeof(uint8_t));
}

void plFreeImage(PLImage *image) {
    plFunctionStart();

    if (image == NULL || image->data == NULL) {
        return;
    }

    for(unsigned int levels = 0; levels < image->levels; ++levels) {
        if(image->data[levels] == NULL) {
            continue;
        }

        free(image->data[levels]);
        image->data[levels] = NULL;
    }

    free(image->data);
    image->data = NULL;
}

bool plIsValidImageSize(unsigned int width, unsigned int height) {
    if(((width < 2) || (height < 2)) || (!plIsPowerOfTwo(width) || !plIsPowerOfTwo(height))) {
        return false;
    }

    return true;
}

bool plIsCompressedImageFormat(PLImageFormat format) {
    switch (format) {
        default:    return false;
        case PL_IMAGEFORMAT_RGBA_DXT1:
        case PL_IMAGEFORMAT_RGBA_DXT3:
        case PL_IMAGEFORMAT_RGBA_DXT5:
        case PL_IMAGEFORMAT_RGB_DXT1:
        case PL_IMAGEFORMAT_RGB_FXT1:
            return true;
    }
}

#define scale_5to8(i) ((((double)(i)) / 31) * 255)

uint8_t *_plImageDataRGB5A1toRGBA8(const uint8_t *src, size_t n_pixels) {
    uint8_t *dst = malloc(n_pixels * 4);
    if(dst == NULL) {
        return NULL;
    }

    uint8_t *dp = dst;

    for(size_t i = 0; i < n_pixels; ++i) {
        /* Red */
        *(dp++) = scale_5to8((src[0] & 0xF8) >> 3);

        /* Green */
        *(dp++) = scale_5to8(((src[0] & 0x07) << 2) | ((src[1] & 0xC0) >> 6));

        /* Blue */
        *(dp++) = scale_5to8((src[1] & 0x3E) >> 1);

        /* Alpha */
        *(dp++) = (src[1] & 0x01) ? 255 : 0;

        src += 2;
    }

    return dst;
}
