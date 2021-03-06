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
#pragma once

typedef enum PLPrimitive {
    PL_PRIMITIVE_IGNORE,

    PLMESH_LINES,
    PL_PRIMITIVE_LINE_STRIP,
    PLMESH_POINTS,
    PLMESH_TRIANGLES,
    PLMESH_TRIANGLE_STRIP,
    PLMESH_TRIANGLE_FAN,
    PLMESH_TRIANGLE_FAN_LINE,
    PLMESH_QUADS,

    PL_NUM_PRIMITIVES
} PLMeshPrimitive;

typedef enum PLDrawMode {
    PL_DRAW_DYNAMIC,
    PL_DRAW_STATIC,
    PL_DRAW_IMMEDIATE,  // Not necessarily supported in all cases, will just revert to dynamic otherwise!

    PL_NUM_DRAWMODES
} PLMeshDrawMode;

typedef struct PLVertex {
    PLVector3 position, normal;
    PLVector2 st[16];

    PLColour colour;
} PLVertex;

typedef struct PLTriangle {
    PLVector3 normal;

    unsigned int indices[3];
} PLTriangle;

#if defined(PL_MODE_OPENGL)
enum {
    _PLGL_BUFFER_VERTICES,
    _PLGL_BUFFER_TEXCOORDS,

    _PLGL_BUFFERS
};
#endif

typedef struct PLMesh {
#if defined(PL_MODE_OPENGL)
    unsigned int _buffers[_PLGL_BUFFERS];
#endif

    PLVertex *vertices;
    PLTriangle *triangles;
    uint8_t *indices;

    unsigned int num_verts;
    unsigned int num_triangles;

    PLMeshPrimitive primitive, primitive_restore;
    PLMeshDrawMode mode;

    PLVector3 position, angles;
} PLMesh;

PL_EXTERN_C

PL_EXTERN PLMesh *plCreateMesh(PLMeshPrimitive primitive, PLMeshDrawMode mode, PLuint num_tris, PLuint num_verts);
PL_EXTERN void plDeleteMesh(PLMesh *mesh);

PL_EXTERN void plDrawRectangle(PLRectangle2D rect);
PL_EXTERN void plDrawTriangle(int x, int y, unsigned int w, unsigned int h);

PL_EXTERN void plClearMesh(PLMesh *mesh);
PL_EXTERN void plSetMeshVertexPosition(PLMesh *mesh, unsigned int index, PLVector3 vector);
PL_EXTERN void plSetMeshVertexPosition3f(PLMesh *mesh, unsigned int index, float x, float y, float z);
PL_EXTERN void plSetMeshVertexPosition2f(PLMesh *mesh, unsigned int index, float x, float y);
PL_EXTERN void plSetMeshVertexNormal3f(PLMesh *mesh, unsigned int index, float x, float y, float z);
PL_EXTERN void plSetMeshVertexST(PLMesh *mesh, unsigned int index, float s, float t);
PL_EXTERN void plSetMeshVertexColour(PLMesh *mesh, unsigned int index, PLColour colour);
PL_EXTERN void plUploadMesh(PLMesh *mesh);

PL_EXTERN void plDrawMesh(PLMesh *mesh);

PL_EXTERN PLAABB plCalculateMeshAABB(PLMesh *mesh);

PL_EXTERN PLVector3 plGenerateVertexNormal(PLVector3 a, PLVector3 b, PLVector3 c);

PL_EXTERN void plGenerateMeshNormals(PLMesh *mesh);

PL_EXTERN_C_END
