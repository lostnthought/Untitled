#ifndef UTILITIES_H
#define UTILITIES_H

#include "utilities/types.h"
#include "utilities/mathf.h"
#include <stdio.h>
#include <string.h>

int vector_push(Vector * v, void * o);
void * vector_expand(Vector * v, size_t i);

mat4 compose_2d_transform(vec2 transform, float angle, vec2 scale);

uint16_t float32_to_float16(float f);
void init_vector(Vector * v, size_t i, size_t type_size);
float triangle_area(vec3 a, vec3 b, vec3 c);
bool is_similar_plane(vec3 n1, vec3 n2, vec3 p1, vec3 p2, float epsilon);
bool test_sphere_aabb(vec3 light_pos, float light_radius, vec3 min, vec3 max);

void compute_convex_hull(vec3 *points, int count, vec3 *out_hull, int *out_count, vec3 normal);
void compute_hull_faces(vec3 * points, size_t * indices, size_t count, size_t * out_hull, size_t * out_count, vec3 normal);
vec3 compute_hull_centroid(vec3 *hull_points, int hull_count, float bottom, float top);
void calculate_aabb_extents(AABB * aabb);
void calculate_aabb_bounds(AABB * aabb);
void compute_aabb_from_points(vec3 * points, int point_count, AABB * aabb);
bool aabbs_intersect(AABB * a, AABB * b);
AABB transform_aabb_by_position(AABB aabb, vec3 pos);
AABB transform_aabb_by_model_matrix(AABB aabb, mat4 mat);
AABB transform_aabb_by_model_matrix_rotation(AABB aabb, mat4 mat);

float fbm (vec3 p);

bool vec4_equal(vec4 a, vec4 b, float eps);
bool vec3_equal(vec3 a, vec3 b, float eps);
bool triangles_are_contiguous(Triangle * a, Triangle * b);
bool compute_barycentric(const vec2 uv0, const vec2 uv1, const vec2 uv2, const vec2 p, float * b0, float * b1, float * b2);

bool ray_intersects_triangle(vec3 origin, vec3 dir, vec3 a, vec3 b, vec3 c, float *out_t, vec3 *out_hit_point, vec3 * barycentric);

void compute_3d_barycentric(vec3 a, vec3 b, vec3 c, vec3 p, float * wA, float * wB, float * wC);

vec3 generate_bounce(uint32_t i, uint32_t N);
vec3 generate_sphere_dir(int i, int N);

bool mesh_faces_are_contiguous(MeshFace * a, vec3 * a_verts, MeshFace * b, vec3 * b_verts);



#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

#define M_TAU (2.0f * M_PI)

static inline float wrap_angle_pi(float angle) {
    angle = fmodf(angle + M_PI, M_TAU);
    if (angle < 0.0f) {
        angle += M_TAU;
    }
    return angle - M_PI;
}

static inline float SDF_circle(vec2 p, vec2 center, float radius) {
    return vec2_length(vec2_sub(p, center)) - radius;
}
static inline float SDF_rect(vec2 p, vec2 center, vec2 size, float r) {
    vec2 local = vec2_sub(p, center);
    vec2 q = vec2_sub((vec2){fabsf(local.x), fabsf(local.y)}, vec2_scale(size, 0.5));
    q = vec2_add(q, (vec2){r,r});
    return vec2_length((vec2){fmaxf(q.x, 0.0f), fmaxf(q.y, 0.0f)}) + fminf(fmaxf(q.x, q.y), 0.0f) - r;
}

static inline vec3 vec3_min(vec3 a, vec3 b)
{
    a.x = MIN(a.x, b.x);
    a.y = MIN(a.y, b.y);
    a.z = MIN(a.z, b.z);
    return a;
}

static inline vec3 vec3_max(vec3 a, vec3 b)
{
    a.x = MAX(a.x, b.x);
    a.y = MAX(a.y, b.y);
    a.z = MAX(a.z, b.z);
    return a;
}

static inline vec3 vec3_clamp(vec3 a, vec3 b, vec3 c)
{
    a.x = clamp(a.x, b.x, c.x);
    a.y = clamp(a.y, b.y, c.y);
    a.z = clamp(a.z, b.z, c.z);
    return a;
}


static inline bool vec2_inside_bounds(vec2 p, vec2 min, vec2 max)
{

    if (p.x >= min.x && p.x <= max.x && p.y >= min.y && p.y <= max.y)
    {
        return true;
    }

    return false;
    
}


int clip_against_plane(
    const vec3 *in,
    int in_count,
    vec3 *out,
    vec3 plane_point,
    vec3 plane_normal);
int clip_pts_against_plane( ClipVertex *in, int in_count, ClipVertex *out, vec3 plane_point, vec3 plane_normal);

static inline float calculate_polygon_area(vec3 p[], int n) {
    float area = 0.0f;
    
    int j = n - 1; 
    
    for (int i = 0; i < n; i++) {
        area += (p[j].x + p[i].x) * (p[j].y - p[i].y);
        j = i;
    }
    
    return fabsf(area / 2.0f);
}

#define CAST_VEC(vec, type) ((type*)((vec).buffer))


static inline void print_mat4 ( mat4 *m )

{

    printf("MAT4:\n"
           " % .6f % .6f % .6f % .6f\n"
           " % .6f % .6f % .6f % .6f\n"
           " % .6f % .6f % .6f % .6f\n"
           " % .6f % .6f % .6f % .6f\n",
           m->a0,m->a1,m->a2,m->a3,
           m->b0,m->b1,m->b2,m->b3,
           m->c0,m->c1,m->c2,m->c3,
           m->d0,m->d1,m->d2,m->d3);

}
static inline void print_mat3 ( mat3 *m )
{

    printf("MAT3:\n"
           " % .6f % .6f % .6f\n"
           " % .6f % .6f % .6f\n"
           " % .6f % .6f % .6f\n",
           m->a0,m->a1,m->a2,
           m->b0,m->b1,m->b2,
           m->c0,m->c1,m->c2);
           

}
static inline vec3 vec4_to_vec3(vec4 v) {return (vec3){v.x, v.y, v.z};}
static inline vec4 vec3_to_vec4(vec3 v) {return (vec4){v.x, v.y, v.z, 0.0};}

static inline void print_aabb(AABB * aabb)
{
    printf("MIN: %f %f %f\n MAX: %f %f %f\n CENTER: %f %f %f\n EXTENTS: %f %f %f\n", aabb->min_x, aabb->min_y, aabb->min_z, aabb->max_x, aabb->max_y, aabb->max_z, aabb->center.x, aabb->center.y, aabb->center.z, aabb->extents.x, aabb->extents.y, aabb->extents.z);
    
}


static inline void print_obb(OBB * obb)
{
    print_mat3(&obb->rot);
    printf("CENTER: %f %f %f\n EXTENTS: %f %f %f\n", obb->center.x, obb->center.y, obb->center.z, obb->extents.x, obb->extents.y, obb->extents.z);
    
}


static inline void frisvad_basis(const vec3 n, vec3 *b1, vec3 *b2) {
    if (n.z < -0.99998796f) { 
        b1->x = 0.0f; b1->y = -1.0f; b1->z = 0.0f;
        b2->x = -1.0f; b2->y = 0.0f; b2->z = 0.0f;
        return;
    }

    const float a = 1.0f / (1.0f + n.z);
    const float b = -n.x * n.y * a;

    b1->x = 1.0f - n.x * n.x * a;
    b1->y = b;
    b1->z = -n.x;

    b2->x = b;
    b2->y = 1.0f - n.y * n.y * a;
    b2->z = -n.y;
}
static inline AABB translate_aabb(AABB a, vec3 b)
{
    a.min_x += b.x;
    a.max_x += b.x;
    a.min_y += b.y;
    a.max_y += b.y;
    a.min_z += b.z;
    a.max_z += b.z;
    a.center = vec3_add(a.center, b);
    return a;
    
}

static inline bool point_in_aabb(vec3 p, AABB * aabb)
{


    if (p.x >= aabb->min_x && p.x <= aabb->max_x && p.y >= aabb->min_y && p.y <= aabb->max_y && p.z >= aabb->min_z && p.z <= aabb->max_z) return true;
    return false;
    
}


// copy a into b
static inline void copy_vector(Vector * a, Vector * b)
{
    
    if (b->buffer) free(b->buffer);
    *b = *a;
    b->buffer = malloc(b->type_size * b->capacity);
    memcpy(b->buffer, a->buffer, sizeof(b->type_size) * b->capacity);

}



static inline float find_hull_point_z_from_plane(Plane p, vec2 xy)
{

    return -(p.normal.x * xy.x + p.normal.y * xy.y + p.d) / p.normal.z;
    
}

static inline void compute_triangle_normal(Triangle * tri)
{

    vec3 AB = vec3_sub(tri->pts[B].pos, tri->pts[A].pos);
    vec3 AC = vec3_sub(tri->pts[C].pos, tri->pts[A].pos);
    
    
    tri->normal = vec3_normalize(vec3_cross(AB, AC));
}

static inline vec3 compute_tri_normal(vec3 a, vec3 b, vec3 c)
{

    vec3 AB = vec3_sub(b, a);
    vec3 AC = vec3_sub(c, a);
    
    
    return vec3_normalize(vec3_cross(AB, AC));
}

static inline void print_triangle(Triangle * t)
{
    
    printf("A: %f %f %f B: %f %f %f C: %f %f %f\n", t->pts[0].pos.x, t->pts[0].pos.y, t->pts[0].pos.z, t->pts[1].pos.x, t->pts[1].pos.y, t->pts[1].pos.z, t->pts[2].pos.x, t->pts[2].pos.y, t->pts[2].pos.z);
}

static inline void print_tri(vec3 a, vec3 b, vec3 c)
{
    
    printf("A: %f %f %f B: %f %f %f C: %f %f %f\n", a.x, a.y, a.z, b.x, b.y, b.z, c.x, c.y, c.z);
}

#define REMOVE_FROM_VEC(vec, index, type) \
    type * buf = CAST_VEC(*vec, type); \
    for (int i = index; i < (int)(vec)->count-1; i++) \
    { \
        buf[i] = buf[i+1]; \
    } \
    (vec)->count--; \

#define SHIFT_VEC_RIGHT(vec, index, type) \
    type * buf = CAST_VEC(*vec, type); \
    for (int i = (int)(vec)->count; i > index; i--) \
    { \
        buf[i] = buf[i-1]; \
    } \
    (vec)->count++; \

#define REMOVE_FROM_ARR(arr, index, count) \
    for (int i = index; i < (int)(*count)-1; i++) \
    { \
        arr[i] = arr[i+1]; \
    } \
    (*count)--; \

#define PRINT_VEC2N(vec) \
    printf("%s x: %.3f y: %.3f\n", #vec, vec.x, vec.y); \

#define PRINT_VEC3(vec, a, b, c) \
    printf("%s %s: %.3f %s: %.3f %s: %.3f\n", #vec, a, vec.x, b, vec.y, c, vec.z); \

#define PRINT_VEC3N(vec) \
    printf("%s x: %.3f y: %.3f z: %.3f\n", #vec, vec.x, vec.y, vec.z); \

#define PRINT_VEC4N(vec) \
    printf("%s x: %.3f y: %.3f z: %.3f w: %.3f\n", #vec, vec.x, vec.y, vec.z, vec.w); \



static inline vec3 mat4_mul_point(mat4 m, vec3 v) {
    vec4 t = mul_m4_v4(m, (vec4){v.x, v.y, v.z, 1.0f});
    if (t.w == 0) t.w = 1;
    return (vec3){ t.x / t.w, t.y / t.w, t.z / t.w };
}
static inline vec3 mat4_mul_pointr(const mat4 m, vec3 v) {
    vec4 t = mul_m4_v4r(m, (vec4){v.x, v.y, v.z, 1.0f});
    return (vec3){ t.x / t.w, t.y / t.w, t.z / t.w };
}

static inline vec3 mat4_mul_direction(const mat4 m, vec3 v) {
    vec4 t = mul_m4_v4(m, (vec4){v.x, v.y, v.z, 0.0f});
    return (vec3){ t.x, t.y, t.z };
}


static inline mat3 quat_to_mat3(const vec4 q) {
    float w = q.x;
    float x = q.y;
    float y = q.z;
    float z = q.w;

    float x2 = x * x;
    float y2 = y * y;
    float z2 = z * z;
    float xy = x * y;
    float xz = x * z;
    float yz = y * z;
    float wx = w * x;
    float wy = w * y;
    float wz = w * z;

    mat3 mat;
    mat.a0 = 1.0f - 2.0f * (y2 + z2);
    mat.a1 = 2.0f * (xy - wz);
    mat.a2 = 2.0f * (xz + wy);

    mat.b0 = 2.0f * (xy + wz);
    mat.b1 = 1.0f - 2.0f * (x2 + z2);
    mat.b2 = 2.0f * (yz - wx);

    mat.c0 = 2.0f * (xz - wy);
    mat.c1 = 2.0f * (yz + wx);
    mat.c2 = 1.0f - 2.0f * (x2 + y2);
    return mat;
}


static inline vec4 quat_from_axis_angle(vec3 axis, float angle) {
  float s = sin(angle/2);
  vec4 quat;
  quat.x = axis.x * s;
  quat.y = axis.y * s;
  quat.z = axis.z * s;
  quat.w = cos(angle/2);
  return quat;
}

static inline vec4 quat_copy_sign(vec4 a, vec4 b){
    
    float dot = a.x * b.x + a.y * b.y + a.z * b.z + a.w * a.w;
    if (dot < 0){
        a.w = -a.w;
        a.x = -a.x;
        a.y = -a.y;
        a.z = -a.z;
    } 
    return a;
}


static inline vec4 quat_mul(vec4 a, vec4 b) {
    return (vec4){
        a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y, 
        a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x, 
        a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w,
        a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z 
    };
}


static inline vec3 get_mat4_axis_x(mat4 a) {
    return (vec3){a.a0, a.a1, a.a2};    
}
static inline vec3 get_mat4_axis_y(mat4 a) {
    return (vec3){a.b0, a.b1, a.b2};    
}
static inline vec3 get_mat4_axis_z(mat4 a) {
    return (vec3){a.c0, a.c1, a.c2};    
}
static inline vec3 get_mat4_axis_w(mat4 a) {
    return (vec3){a.d0, a.d1, a.d2};    
}

static inline mat4 mat4_mul_flt(mat4 a, float b)
{
    a.a0 *= b;
    a.a1 *= b;
    a.a2 *= b;
    a.a3 *= b;
    a.b0 *= b;
    a.b1 *= b;
    a.b2 *= b;
    a.b3 *= b;
    a.c0 *= b;
    a.c1 *= b;
    a.c2 *= b;
    a.c3 *= b;
    a.d0 *= b;
    a.d1 *= b;
    a.d2 *= b;
    a.d3 *= b;
    return a;
    
}


static inline mat4 mat4_identity() {
    return (mat4){
        1,0,0,0,
        0,1,0,0,
        0,0,1,0,
        0,0,0,1
    };
}
static inline mat3 mat3_identity() {
    return (mat3){
        1,0,0,
        0,1,0,
        0,0,1,
    };
}
static inline mat4 mat4_translation(vec3 t) {
    mat4 m = mat4_identity();
    m.d0 = t.x;
    m.d1 = t.y;
    m.d2 = t.z;
    return m;
}
static inline mat4 mat4_scale(vec3 s) {
    mat4 m = mat4_identity();
    m.a0 = s.x;
    m.b1 = s.y;
    m.c2 = s.z; 
    return m;
}
static inline mat4 mat4_from_quat(vec4 q) {
    float x = q.x, y = q.y, z = q.z, w = q.w;
    float xx = x * x, yy = y * y, zz = z * z;
    float xy = x * y, xz = x * z, yz = y * z;
    float wx = w * x, wy = w * y, wz = w * z;

    mat4 m = mat4_identity();

    m.a0 = 1.0f - 2.0f * (yy + zz);
    m.a1 = 2.0f * (xy + wz);
    m.a2 = 2.0f * (xz - wy);

    m.b0 = 2.0f * (xy - wz);
    m.b1 = 1.0f - 2.0f * (xx + zz);
    m.b2 = 2.0f * (yz + wx);

    m.c0 = 2.0f * (xz + wy);
    m.c1 = 2.0f * (yz - wx);
    m.c2 = 1.0f - 2.0f * (xx + yy);

    return m;
}
static inline mat4 compose_trs_from_vectors(vec3 pos, vec4 rot, vec3 scale) {
    mat4 T = mat4_translation(pos);
    mat4 R = mat4_from_quat(rot);
    mat4 S = mat4_scale(scale);

    return m4_mul(T, m4_mul(R, S));
}


static inline mat3 mat3_from_mat4(const mat4 *m) {
    return (mat3){
        m->a0, m->a1, m->a2,
        m->b0, m->b1, m->b2,
        m->c0, m->c1, m->c2
    };
}

static inline vec3 mat3_mul_vec3(mat3 m, vec3 v) {
    return (vec3){
        m.a0*v.x + m.a1*v.y + m.a2*v.z,
        m.b0*v.x + m.b1*v.y + m.b2*v.z,
        m.c0*v.x + m.c1*v.y + m.c2*v.z
    };
}
static inline vec3 mat3_mul_vec3r(mat3 m, vec3 v) {
    return (vec3){
        m.a0*v.x + m.b0*v.y + m.c0*v.z,
        m.a1*v.x + m.b1*v.y + m.c1*v.z,
        m.a2*v.x + m.b2*v.y + m.c2*v.z
    };
}

static inline void mat4_transpose(float * dst, const float * src)
{
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            dst[j * 4 + i] = src[i * 4 + j];
        }
    }
}


static inline mat4 m4_transpose(mat4 in){
    mat4 out;
    out.a0 = in.a0;
    out.a1 = in.b0;
    out.a2 = in.c0;
    out.a3 = in.d0;
    out.b0 = in.a1;
    out.b1 = in.b1;
    out.b2 = in.c1;
    out.b3 = in.d1;
    out.c0 = in.a2;
    out.c1 = in.b2;
    out.c2 = in.c2;
    out.c3 = in.d2;
    out.d0 = in.a3;
    out.d1 = in.b3;
    out.d2 = in.c3;
    out.d3 = in.d3;
    return out;
}
static inline mat3 mat3_transpose(mat3 in){
    mat3 out;
    out.a0 = in.a0;
    out.a1 = in.b0;
    out.a2 = in.c0;
    out.b0 = in.a1;
    out.b1 = in.b1;
    out.b2 = in.c1;
    out.c0 = in.a2;
    out.c1 = in.b2;
    out.c2 = in.c2;
    return out;
}

static inline mat3 mat3_mul(mat3 M, mat3 N) {
    mat3 R;
    
    R.a0 = M.a0 * N.a0 + M.a1 * N.b0 + M.a2 * N.c0;
    R.a1 = M.a0 * N.a1 + M.a1 * N.b1 + M.a2 * N.c1;
    R.a2 = M.a0 * N.a2 + M.a1 * N.b2 + M.a2 * N.c2;
    
    R.b0 = M.b0 * N.a0 + M.b1 * N.b0 + M.b2 * N.c0;
    R.b1 = M.b0 * N.a1 + M.b1 * N.b1 + M.b2 * N.c1;
    R.b2 = M.b0 * N.a2 + M.b1 * N.b2 + M.b2 * N.c2;
    
    R.c0 = M.c0 * N.a0 + M.c1 * N.b0 + M.c2 * N.c0;
    R.c1 = M.c0 * N.a1 + M.c1 * N.b1 + M.c2 * N.c1;
    R.c2 = M.c0 * N.a2 + M.c1 * N.b2 + M.c2 * N.c2;
    
    return R;
}
static inline mat3 mat3_inverse_transpose(mat3 m) {
    float det =
        m.a0*(m.b1*m.c2 - m.b2*m.c1) -
        m.a1*(m.b0*m.c2 - m.b2*m.c0) +
        m.a2*(m.b0*m.c1 - m.b1*m.c0);

    float invDet = (det != 0.0f) ? 1.0f/det : 1.0f;

    mat3 r;
    r.a0 =  (m.b1*m.c2 - m.b2*m.c1) * invDet;
    r.a1 = -(m.a1*m.c2 - m.a2*m.c1) * invDet;
    r.a2 =  (m.a1*m.b2 - m.a2*m.b1) * invDet;

    r.b0 = -(m.b0*m.c2 - m.b2*m.c0) * invDet;
    r.b1 =  (m.a0*m.c2 - m.a2*m.c0) * invDet;
    r.b2 = -(m.a0*m.b2 - m.a2*m.b0) * invDet;

    r.c0 =  (m.b0*m.c1 - m.b1*m.c0) * invDet;
    r.c1 = -(m.a0*m.c1 - m.a1*m.c0) * invDet;
    r.c2 =  (m.a0*m.b1 - m.a1*m.b0) * invDet;

    return r;
}

static inline void OBB_vectors(mat3 rot, vec3 * fwd, vec3 * right, vec3 * up)
{
    *fwd = (vec3){rot.a0, rot.b0, rot.c0};
    *right = (vec3){rot.a1, rot.b1, rot.c1};
    *up = (vec3){rot.a2, rot.b2, rot.c2};
}

static inline vec3 OBB_min_corner(OBB * obb)
{
    
    vec3 fwd; vec3 right; vec3 up;
    OBB_vectors(obb->rot, &fwd, &right, &up);

    vec3 pos = vec3_add(obb->center, 
    vec3_add(vec3_scale(fwd, -obb->extents.x),
    vec3_add(vec3_scale(right, -obb->extents.y),
    vec3_scale(up, -obb->extents.z))));
    return pos;

}



static inline bool point_in_obb(OBB * obb, vec3 pos)
{

    vec3 local = vec3_sub(pos, obb->center);
    vec3 norm = mat3_mul_vec3(mat3_inverse_transpose(obb->rot), local);

    if (fabsf(norm.x) <= obb->extents.x &&
        fabsf(norm.y) <= obb->extents.y &&
        fabsf(norm.z) <= obb->extents.z)
    {
        return true;
    }

    return false;
}


static inline vec3 triangle_centroid(vec3 A, vec3 B, vec3 C)
{
    return vec3_scale(vec3_add(vec3_add(A, B), C), 0.333333f);
}

#endif
