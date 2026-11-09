#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>



typedef struct vec2 {float x, y;} vec2;
typedef struct vec3 {float x, y, z;} vec3;
typedef struct vec4 {float x, y, z, w;} vec4;
typedef struct ivec2 {int x, y;} ivec2;
typedef struct ivec3 {int x, y, z;} ivec3;
typedef struct ivec4 {int x, y, z, w;} ivec4;
typedef struct uvec2 {uint32_t x, y;} uvec2;
typedef struct uvec3 {uint32_t x, y, z;} uvec3;
typedef struct uvec4 {uint32_t x, y, z, w;} uvec4;
typedef struct mat3 {float a0, a1, a2, b0, b1, b2, c0, c1, c2;} mat3;
typedef struct mat4 {float a0, a1, a2, a3, b0, b1, b2, b3, c0, c1, c2, c3, d0, d1, d2, d3;} mat4;


typedef enum TRI_POINT
{
    A=0, B=1, C=2, D=3
} TRI_POINT;

typedef enum AXIS
{
    X, Y, Z
} AXIS;


typedef struct AABB
{
    float min_x, max_x;
    float min_y, max_y;
    float min_z, max_z;

    vec3 center;
    vec3 extents;

} AABB;

typedef struct OBB
{

    mat3 rot;
    vec3 center;
    vec3 extents;

} OBB;

/**
 * @addtogroup editor_group
 * @{
 */
typedef enum UndoStructureType
{

    UNDOSTRUCT_MN,
    UNDOSTRUCT_UI_ELEMENT,
    UNDOSTRUCT_UI_CONTAINER,
    UNDOSTRUCT_TEXTBOX,
    UNDOSTRUCT_LIGHT,
    UNDOSTRUCT_GLOBAL,
    UNDOSTRUCT_EDITOR_SETTINGS,
    UNDOSTRUCT_ENTITY,
    UNDOSTRUCT_MATERIAL,
    UNDOSTRUCT_RPROBE,
    UNDOSTRUCT_LPROBE,
    UNDOSTRUCT_APROBE,
    UNDOSTRUCT_AEMITTER,
    UNDOSTRUCT_APORTAL,
    UNDOSTRUCT_MENU,
    UNDOSTRUCT_ASSETINSTANCE,
    
} UndoStructureType;



typedef enum UndoValue
{

    UNDOVAL_NONE,
    UNDOVAL_INT,
    UNDOVAL_FLOAT,
    UNDOVAL_VEC2,
    UNDOVAL_VEC3,
    UNDOVAL_VEC4,
    
} UndoValue;

#define MAX_UNDO_BYTES 50000
#define MAX_UNDOS 256

typedef struct Undo Undo;

#define CAST_SPECIAL(type, special) ((UndoSpecial)((type)special))

typedef enum UndoSpecial
{
    UNDOSPECIAL_NONE,
    UNDOEDITORSETTINGS_FULLSCREENOPT,
    UNDOEDITORSETTINGS_SCREEN_RES,
    UNDOEDITORSETTINGS_RENDER_RES,
    UNDOEDITORSETTINGS_SSAO_RES,
    UNDOEDITORSETTINGS_DOF_RES,
    UNDOEDITORSETTINGS_BLOOM_RES,
    UNDOEDITORSETTINGS_HALFSIZE_EFFECTS,
    UNDOASSETINSTANCE_ROTATION_ANGLE,
    UNDOASSETINSTANCE_MAT,
    UNDOMN_ROTATION_ANGLE,
    UNDOMN_MAT,
    UNDOMN_CHANGE_MATERIAL,
    UNDOAPORTAL_SPAWN,
    UNDOAPORTAL_DELETE,
    UNDOAEMITTER_SPAWN,
    UNDOAEMITTER_DELETE_SOUND_OBJ,
    UNDOAEMITTER_LOAD_SOUND_OBJ,
    UNDOAEMITTER_DELETE_EMITTER,
    UNDOAEMITTER_PLAY_EMITTER,
    UNDOAEMITTER_STOP_EMITTER,
    UNDOAEMITTER_OBB_ROT,
    UNDOAPROBE_RESET_APVS,
    UNDOAPROBE_SPAWN_APV,
    UNDOAPROBE_RESET_APV_SIZE,
    UNDOAPROBE_DELETE_APV,
    UNDOAPROBE_BAKE_APV,
    UNDOAPROBE_OBB_ROT,
    UNDOLPROBE_SPAWN,
    UNDOLPROBE_RESET_LPVS,
    REDOLPROBE_RESET_LPVS,
    UNDOLPROBE_RESET_LPV_SIZE,
    UNDOLPROBE_DELETE,
    UNDOLPROBE_BAKE,
    UNDOLPROBE_OBB_ROT,
    UNDORPROBE_SPAWN,
    UNDORPROBE_DELETE,
    UNDOLIGHT_SPAWN,
    UNDOLIGHT_DELETE,
    UNDOGLOBAL_SPAWN_ASSET_INSTANCE,
    UNDOUIELEMENT_CREATE_ELEMENT,
    UNDOUIELEMENT_DELETE_ELEMENT,
    UNDOUIELEMENT_CHANGE_CONTAINER,
    UNDOUIELEMENT_REMOVE_FROM_CONTAINER,
    REDOUIELEMENT_ADD_TO_CONTAINER,
    UNDOUIELEMENT_CREATE_CONTAINER,
    UNDOUIELEMENT_DELETE_CONTAINER,
    UNDOUIELEMENT_CREATE_LABEL,
    UNDOUIELEMENT_JUSTIFY_LABEL,
    UNDOUICONTAINER_CHANGE_SCROLLBAR,
    UNDOTEXTBOX_CREATE_TEXT_BOX,
    UNDOTEXTBOX_DELETE_TEXT_BOX,
    UNDOTEXTBOX_CHANGE_TEXT,
    UNDOTEXTBOX_CHANGE_CONTAINER,
    UNDOTEXTBOX_REMOVE_FROM_CONTAINER,
    UNDOTEXTBOX_ADD_TO_CONTAINER,
    
    
} UndoSpecial;



typedef struct UndoStructure
{

    UndoStructureType type;
    UndoSpecial special;
    int ids[8];
    
} UndoStructure;


struct Undo
{

    bool free;
    size_t special_count;
    size_t byte_size;
    size_t byte_offset;
    uint32_t id;
    Undo * prev;
    Undo * next;
    uint8_t ** special_bytes;
    UndoStructure structure;
    uint8_t bytes[MAX_UNDO_BYTES];
    
};


typedef struct NKSlider
{

    bool is_held;
    uint8_t bytes[MAX_UNDO_BYTES];
    size_t byte_size;
    
} NKSlider;



/** @} */

typedef struct Vector
{

    size_t count, capacity, type_size;
    void * buffer;
        
} Vector;




typedef struct Point
{
    
    vec3 pos;
    vec3 normal;
    vec4 tangent;
    vec3 color;
    vec2 uv;

    ivec4 joints;
    vec4 weights;

} Point;


typedef struct Edge
{

    vec3 a;
    vec3 b;
    
} Edge;

typedef struct HorizonEdge
{

    int ab[2];
    
} HorizonEdge;

typedef enum HitType
{
    HIT_NONE,
    HIT_STATIC,
    HIT_DYNAMIC,

} HitType;

typedef struct RayHit {

    float t;
    vec3 pos;
    size_t mesh_node_idx;
    size_t tri;
    HitType hit;
    float b0;
    float b1;
    float b2;
    bool occluded;
    
} RayHit;


typedef struct Triangle
{

    Point pts[3];

    vec3 normal;
    
} Triangle;

typedef struct MeshFace
{

    // indices into hull->vertices
    size_t indices[3];
    vec3 normal;
    int material_id;
    
} MeshFace;



typedef struct Plane
{
    vec3 normal;
    float d;
    
} Plane;

typedef struct SupportVertex
{

    vec3 a, b, p;
    size_t index_a;
    size_t index_b;
    
} SupportVertex;


typedef struct Simplex
{
    SupportVertex points[4];
    int count;
    
    vec3 normals[4];
    vec3 centroid;

} Simplex;

#define MAX_HULL_FACE_VERTICES 64
typedef struct HullFace
{

    size_t indices[MAX_HULL_FACE_VERTICES];
    size_t count;
    vec3 normal;
    
} HullFace;

typedef struct ClipVertex
{
    vec3 p;
    int edge0, edge1;
} ClipVertex;
typedef struct {
    uint64_t start;
    uint64_t freq;
} Timer;


#endif
