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

#include "graphics_private.h"

#if defined(PL_MODE_OPENGL)
#   define _PLGL_USE_VERTEX_BUFFER_OBJECTS
#endif

typedef struct MeshTranslatePrimitive {
    PLMeshPrimitive mode;

    unsigned int target;

    const char *name;
} MeshTranslatePrimitive;

MeshTranslatePrimitive primitives[] = {
#if defined (PL_MODE_OPENGL)
        {PLMESH_LINES, GL_LINES, "LINES"},
        {PLMESH_POINTS, GL_POINTS, "POINTS"},
        {PLMESH_TRIANGLES, GL_TRIANGLES, "TRIANGLES"},
        {PLMESH_TRIANGLE_FAN, GL_TRIANGLE_FAN, "TRIANGLE_FAN"},
        {PLMESH_TRIANGLE_FAN_LINE, GL_LINES, "TRIANGLE_FAN_LINE"},
        {PLMESH_TRIANGLE_STRIP, GL_TRIANGLE_STRIP, "TRIANGLE_STRIP"},
        {PLMESH_QUADS, GL_QUADS, "QUADS"}
#elif defined (VL_MODE_GLIDE)
{ PL_PRIMITIVE_LINES,					GR_LINES,			"LINES" },
    { PL_PRIMITIVE_LINE_STRIP,				GR_LINE_STRIP,		"LINE_STRIP" },
    { PL_PRIMITIVE_POINTS,					GR_POINTS,			"POINTS" },
    { PL_PRIMITIVE_TRIANGLES,				GR_TRIANGLES,		"TRIANGLES" },
    { PL_PRIMITIVE_TRIANGLE_FAN,			GR_TRIANGLE_FAN,	"TRIANGLE_FAN" },
    { PL_PRIMITIVE_TRIANGLE_FAN_LINE,		GR_LINES,			"TRIANGLE_FAN_LINE" },
    { PL_PRIMITIVE_TRIANGLE_STRIP,			GR_TRIANGLE_STRIP,	"TRIANGLE_STRIP" },
    { PL_PRIMITIVE_QUADS,					0,					"QUADS" }
#elif defined (VL_MODE_DIRECT3D)
#elif defined (VL_MODE_VULKAN)
    { PL_PRIMITIVE_LINES,					VK_PRIMITIVE_TOPOLOGY_LINE_LIST,		"LINES" },
    { PL_PRIMITIVE_POINTS,					VK_PRIMITIVE_TOPOLOGY_POINT_LIST,		"POINTS" },
    { PL_PRIMITIVE_TRIANGLES,				VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,	"TRIANGLES" },
    { PL_PRIMITIVE_TRIANGLE_FAN,			VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN,		"TRIANGLE_FAN" },
    { PL_PRIMITIVE_TRIANGLE_FAN_LINE,		VK_PRIMITIVE_TOPOLOGY_LINE_LIST,		"TRIANGLE_FAN_LINE" },
    { PL_PRIMITIVE_TRIANGLE_STRIP,			VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,	"TRIANGLE_STRIP" },
    { PL_PRIMITIVE_QUADS,					0,										"QUADS" }
#else
    { 0 }
#endif
};

unsigned int _plTranslatePrimitiveMode(PLMeshPrimitive mode) {
    if(mode == PL_PRIMITIVE_IGNORE) {
        return 0;
    }

    for (unsigned int i = 0; i < plArrayElements(primitives); i++) {
        if (mode == primitives[i].mode)
            return primitives[i].target;
    }

    // Hacky, but just return initial otherwise.
    return primitives[0].target;
}

unsigned int _plTranslateDrawMode(PLMeshDrawMode mode) {
#if defined(PL_MODE_OPENGL)
    switch(mode) {
        case PL_DRAW_DYNAMIC:   return GL_DYNAMIC_DRAW;
        case PL_DRAW_STATIC:    return GL_STATIC_DRAW;

        default: return 0;
    }
#else
    return mode;
#endif
}

void plGenerateMeshNormals(PLMesh *mesh) {
    plAssert(mesh);

#if 1 // per face...
    for (unsigned int j = 0; j < mesh->num_triangles; j++) {
        mesh->triangles[j].normal = plGenerateVertexNormal(
                mesh->vertices[mesh->triangles[j].indices[0]].position,
                mesh->vertices[mesh->triangles[j].indices[1]].position,
                mesh->vertices[mesh->triangles[j].indices[2]].position
        );
    }
#else // per vertex... todo
    for (PLVertex *vertex = &mesh->vertices[0]; vertex; ++vertex) {
            for (PLTriangle *triangle = &mesh->triangles[0]; triangle; ++triangle) {

            }
        }
#endif
}

PLVector3 plGenerateVertexNormal(PLVector3 a, PLVector3 b, PLVector3 c) {
#if 0
    PLVector3D x = c - b;
    PLVector3D y = a - b;
    return x.CrossProduct(y).Normalize();
#else
    return plNormalizeVector3(
            plVector3CrossProduct(
                    PLVector3(
                            c.x - b.x, c.y - b.y, c.z - b.z
                    ),
                    PLVector3(
                            a.x - b.x, a.y - b.y, a.z - b.z
                    )
            )
    );
#endif
}

void plApplyMeshLighting(PLMesh *mesh, PLLight *light, PLVector3 position) {
    PLVector3 distvec = position;
    plSubtractVector3(&distvec, light->position);
    float distance = (light->colour.a - plVector3Length(distvec)) / 100.f;

    for(unsigned int i = 0; i < mesh->num_verts; i++) {
        PLVector3 normal = mesh->vertices[i].normal;
        float angle = (distance * ((normal.x * distvec.x) + (normal.y * distvec.y) + (normal.z * distvec.z)));
        if(angle < 0) {
            plClearColour(&mesh->vertices[i].colour);
        } else {
            mesh->vertices[i].colour.r = light->colour.r * plFloatToByte(angle);
            mesh->vertices[i].colour.g = light->colour.g * plFloatToByte(angle);
            mesh->vertices[i].colour.b = light->colour.b * plFloatToByte(angle);
        }
    }

#if 0
        /*
        x = Object->Vertices_normalStat[count].x;
        y = Object->Vertices_normalStat[count].y;
        z = Object->Vertices_normalStat[count].z;

        angle = (LightDist*((x * Object->Spotlight.x) + (y * Object->Spotlight.y) + (z * Object->Spotlight.z) ));
        if (angle<0 )
        {
        Object->Vertices_screen[count].r = 0;
        Object->Vertices_screen[count].b = 0;
        Object->Vertices_screen[count].g = 0;
        }
        else
        {
        Object->Vertices_screen[count].r = Object->Vertices_local[count].r * angle;
        Object->Vertices_screen[count].b = Object->Vertices_local[count].b * angle;
        Object->Vertices_screen[count].g = Object->Vertices_local[count].g * angle;
        }
        */
#endif
}

void _plDrawArrays(PLMeshPrimitive mode, unsigned int first, unsigned int count) {
    plAssert(count); plAssert(first > count);

#if defined(PL_MODE_OPENGL)
    glDrawArrays(_plTranslatePrimitiveMode(mode), first, count);
#endif
}

void _plDrawElements(PLMeshPrimitive mode, unsigned int count, unsigned int type, const void *indices) {
    plAssert(count); plAssert(indices);

#if defined(PL_MODE_OPENGL)
    glDrawElements(_plTranslatePrimitiveMode(mode), count, type, indices);
#endif
}

PLMesh *plCreateMesh(PLMeshPrimitive primitive, PLMeshDrawMode mode, unsigned int num_tris, unsigned int num_verts) {
    plAssert(num_verts);

    if((primitive == PL_PRIMITIVE_IGNORE) || (primitive < PL_PRIMITIVE_IGNORE) ||
            (primitive > PL_NUM_PRIMITIVES)) {
        ReportError(PL_RESULT_DRAW_PRIMITIVE, "Invalid mesh primitive, %d!\n", primitive);
        return NULL;
    }

    PLuint umode = _plTranslateDrawMode(mode);
    if(!umode && (mode != PL_DRAW_IMMEDIATE)) {
        ReportError(PL_RESULT_DRAW_MODE, "Invalid mesh draw mode, %d!\n", mode);
        return NULL;
    }

    PLMesh *mesh = (PLMesh*)calloc(1, sizeof(PLMesh));
    if(mesh == NULL) {
        ReportError(PL_RESULT_MEMORYALLOC, "Failed to allocate memory for Mesh, %d!\n", sizeof(PLMesh));
        return NULL;
    }

    mesh->primitive = mesh->primitive_restore = primitive;
    mesh->num_triangles = num_tris;
    mesh->num_verts = num_verts;
    mesh->mode = mode;

    if(num_tris > 0) {
        mesh->triangles = (PLTriangle*)calloc(num_tris, sizeof(PLTriangle));
        if(!mesh->triangles) {
            ReportError(PL_RESULT_MEMORYALLOC, "Failed to allocate memory for Triangle, %d!\n",
                           sizeof(PLTriangle) * num_tris);

            plDeleteMesh(mesh);
            return NULL;
        }
    }
    mesh->vertices = (PLVertex*)calloc(num_verts, sizeof(PLVertex));
    if(mesh->vertices == NULL) {
        ReportError(PL_RESULT_MEMORYALLOC, "Failed to allocate memory for Vertex, %d!\n",
            sizeof(PLVertex) * num_verts);

        plDeleteMesh(mesh);
        return NULL;
    }

#if defined(PL_MODE_OPENGL) && defined(_PLGL_USE_VERTEX_BUFFER_OBJECTS)
    if(mode != PL_DRAW_IMMEDIATE) {
        glGenBuffers(_PLGL_BUFFERS, mesh->_buffers);
    }
#endif

    return mesh;
}

void plDeleteMesh(PLMesh *mesh) {
    if(!mesh) {
        return;
    }

    if(mesh->vertices) {
        free(mesh->vertices);
    }

    if(mesh->triangles) {
        free(mesh->triangles);
    }

    free(mesh);
}

void plClearMesh(PLMesh *mesh) {
    plAssert(mesh);

    // Reset the data contained by the mesh, if we're going to begin a new draw.
    memset(mesh->vertices, 0, sizeof(PLVertex) * mesh->num_verts);
    memset(mesh->triangles, 0, sizeof(PLTriangle) * mesh->num_triangles);
}

void plSetMeshVertexPosition(PLMesh *mesh, unsigned int index, PLVector3 vector) {
    mesh->vertices[index].position = vector;
}

void plSetMeshVertexPosition3f(PLMesh *mesh, unsigned int index, float x, float y, float z) {
    mesh->vertices[index].position = PLVector3(x, y, z);
}

void plSetMeshVertexPosition2f(PLMesh *mesh, unsigned int index, float x, float y) {
    mesh->vertices[index].position = PLVector3(x, y, 0);
}

void plSetMeshVertexPosition3fv(PLMesh *mesh, unsigned int index, unsigned int size, const float *v) {
    size += index;
    if(size > mesh->num_verts) {
        size -= (size - mesh->num_verts);
    }

    for(unsigned int i = index; i < size; i++) {
        mesh->vertices[i].position.x = v[0];
        mesh->vertices[i].position.y = v[1];
        mesh->vertices[i].position.z = v[2];
    }
}

void plSetMeshVertexNormal3f(PLMesh *mesh, unsigned int index, float x, float y, float z) {
    mesh->vertices[index].normal = PLVector3(x, y, z);
}

void plSetMeshVertexST(PLMesh *mesh, unsigned int index, float s, float t) {
    mesh->vertices[index].st[0] = PLVector2(s, t);
}

void plSetMeshVertexSTv(PLMesh *mesh, uint8_t unit, unsigned int index, unsigned int size, const float *st) {
    size += index;
    if(size > mesh->num_verts) {
        size -= (size - mesh->num_verts);
    }

    for(unsigned int i = index; i < size; i++) {
        mesh->vertices[i].st[unit].x = st[0];
        mesh->vertices[i].st[unit].y = st[1];
    }
}

void plSetMeshVertexColour(PLMesh *mesh, unsigned int index, PLColour colour) {
    mesh->vertices[index].colour = colour;
}

void plUploadMesh(PLMesh *mesh) {
#if defined(PL_MODE_OPENGL) && defined(_PLGL_USE_VERTEX_BUFFER_OBJECTS)
    if((mesh->mode == PL_DRAW_IMMEDIATE) || (mesh->primitive == PLMESH_QUADS)) {
        // todo, eventually just convert quad primitives to triangles
        return;
    }

    // Fill our buffer with data.
    glBindBuffer(GL_ARRAY_BUFFER, mesh->_buffers[_PLGL_BUFFER_VERTICES]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(PLVertex), &mesh->vertices[0].position.x, _plTranslateDrawMode(mesh->mode));
#endif
}

void plDrawMesh(PLMesh *mesh) {
    plAssert(mesh->num_verts);

#if defined(PL_MODE_OPENGL)
#if !defined(PL_MODE_OPENGL_CORE) //&& !defined(_PLGL_USE_VERTEX_BUFFER_OBJECTS)
    if(mesh->mode == PL_DRAW_IMMEDIATE) {
#if 1
        glBegin(_plTranslatePrimitiveMode(mesh->primitive));
        for(unsigned int i = 0; i < mesh->num_verts; i++) {
            glVertex3f(mesh->vertices[i].position.x, mesh->vertices[i].position.y, mesh->vertices[i].position.z);
            glTexCoord2f(mesh->vertices[i].st[0].x, mesh->vertices[i].st[0].y);
            glColor3ub(mesh->vertices[i].colour.r, mesh->vertices[i].colour.g, mesh->vertices[i].colour.b);
            glNormal3f(mesh->vertices[i].normal.x, mesh->vertices[i].normal.y, mesh->vertices[i].normal.z);
        }
        glEnd();
#else
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY);

        PLVertex *vert = &mesh->vertices[0];
        glVertexPointer(3, GL_FLOAT, 0, &vert->position);
        glColorPointer(4, GL_FLOAT, 0, &vert->colour);
        glNormalPointer(GL_FLOAT, 0, &vert->normal);
        for(int i = 0; i < plGetMaxTextureUnits(); i++) {
            if (gfx_state.tmu[i].active) {
                glClientActiveTexture((GLenum) GL_TEXTURE0 + i);
                glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                glTexCoordPointer(2, GL_FLOAT, 0, vert->st);
            }
        }

        if(mesh->primitive == PL_PRIMITIVE_TRIANGLES) {
            _plDrawElements
                    (
                            mesh->primitive,
                            mesh->numtriangles * 3,
                            GL_UNSIGNED_BYTE,
                            mesh->indices
                    );
        } else {
            _plDrawArrays(mesh->primitive, 0, mesh->numverts);
        }

        glDisableClientState(GL_COLOR_ARRAY);
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_NORMAL_ARRAY);
        for(int i = 0; i < plGetMaxTextureUnits(); i++) {
            if(gfx_state.tmu[i].active) {
                glClientActiveTexture((GLenum)GL_TEXTURE0 + i);
                glDisableClientState(GL_TEXTURE_COORD_ARRAY);
            }
        }
#endif
    } else if(mesh->primitive != PLMESH_QUADS) {
#else
    {
#endif
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, mesh->_buffers[_PLGL_BUFFER_VERTICES]);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

        if (mesh->primitive == PLMESH_TRIANGLES) {
            _plDrawElements(
                    mesh->primitive,
                    mesh->num_triangles * 3,
                    GL_UNSIGNED_BYTE,
                    mesh->indices
            );
        } else {
            _plDrawArrays(mesh->primitive, 0, mesh->num_verts);
        }

        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
#endif
}

// Utility Functions

PLAABB plCalculateMeshAABB(PLMesh *mesh) {
    plAssert(mesh);

    static PLAABB bounds;
    memset(&bounds, 0, sizeof(PLAABB));

    for(unsigned int i = 0; i < mesh->num_verts; ++i) {
        if(bounds.maxs.x < mesh->vertices[i].position.x) {
            bounds.maxs.x = mesh->vertices[i].position.x;
        }

        if(bounds.mins.x > mesh->vertices[i].position.x) {
            bounds.mins.x = mesh->vertices[i].position.x;
        }

        if(bounds.maxs.y < mesh->vertices[i].position.y) {
            bounds.maxs.y = mesh->vertices[i].position.y;
        }

        if(bounds.mins.y > mesh->vertices[i].position.y) {
            bounds.mins.y = mesh->vertices[i].position.y;
        }

        if(bounds.maxs.z < mesh->vertices[i].position.z) {
            bounds.maxs.z = mesh->vertices[i].position.z;
        }

        if(bounds.mins.z > mesh->vertices[i].position.z) {
            bounds.mins.z = mesh->vertices[i].position.z;
        }
    }

    return bounds;
}

void plDrawCube() {} // todo

/*  Textured Rectangle Mesh  */

void plDrawRectangle(PLRectangle2D rect) {
    static PLMesh *mesh = NULL;
    if(!mesh) {
        mesh = plCreateMesh(
                PLMESH_TRIANGLE_STRIP,
                PL_DRAW_IMMEDIATE, // todo, update to dynamic
                2, 4
        );
    }

    plClearMesh(mesh);

    plSetMeshVertexPosition2f(mesh, 0, rect.xy.x, rect.xy.y + rect.wh.y);
    plSetMeshVertexPosition2f(mesh, 1, rect.xy.x, rect.xy.y);
    plSetMeshVertexPosition2f(mesh, 2, rect.xy.x + rect.wh.x, rect.xy.y + rect.wh.y);
    plSetMeshVertexPosition2f(mesh, 3, rect.xy.x + rect.wh.x, rect.xy.y);

    plSetMeshVertexColour(mesh, 0, rect.ll);
    plSetMeshVertexColour(mesh, 1, rect.ul);
    plSetMeshVertexColour(mesh, 2, rect.lr);
    plSetMeshVertexColour(mesh, 3, rect.ur);

    plSetMeshVertexST(mesh, 0, 0, 0);
    plSetMeshVertexST(mesh, 1, 0, 1);
    plSetMeshVertexST(mesh, 2, 1, 1);
    plSetMeshVertexST(mesh, 3, 1, 0);

    plUploadMesh(mesh);

    plDrawMesh(mesh);
}

/*  Triangle Mesh   */

void plDrawTriangle(int x, int y, unsigned int w, unsigned int h) {
    static PLMesh *mesh = NULL;
    if (!mesh) {
        mesh = plCreateMesh(
                PLMESH_TRIANGLE_FAN,
                PL_DRAW_IMMEDIATE, // todo, update to dynamic
                1, 3
        );
    }

    plClearMesh(mesh);

    plSetMeshVertexPosition2f(mesh, 0, x, y + h);
    plSetMeshVertexPosition2f(mesh, 1, x + w / 2, x);
    plSetMeshVertexPosition2f(mesh, 2, x + w, y + h);

    plSetMeshVertexColour(mesh, 0, PLColour(255, 0, 0, 255));
    plSetMeshVertexColour(mesh, 1, PLColour(0, 255, 0, 255));
    plSetMeshVertexColour(mesh, 2, PLColour(0, 0, 255, 255));

    plUploadMesh(mesh);

    plDrawMesh(mesh);
}
