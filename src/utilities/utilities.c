
#include "mathf.h"
#include "types.h"
#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include "utilities.h"
#include <string.h>



int vector_alloc(Vector * v, size_t i)
{
     if (v->type_size == 0)
     {
         printf("ERR: NO TYPE SIZE FOR VECTOR SPECIFIED\n");
         return 0;
     }   
     v->buffer = malloc(v->type_size * i);
     if (!v->buffer) return 0;

     return 1;
}

int vector_push(Vector * v, void * o)
{
    
    if (!v->buffer)
    {
        
        if (!vector_alloc(v, 1)) return 0;
    }
    else
    {
        
        if (v->count + 1 >= v->capacity)
        {
            v->capacity = (v->capacity + 1) * 2;
            v->buffer = realloc(v->buffer, v->type_size * v->capacity);
        }
    }

    size_t offset = v->count * v->type_size;
    memcpy(v->buffer, o, v->type_size);
    
    return v->count++;
}

void * vector_expand(Vector * v, size_t i)
{
    
    if (!v->buffer)
    {
        if (!vector_alloc(v, i)) return NULL;
    }
    else
    {
        
        if (v->count + i >= v->capacity)
        {
            v->capacity = ((v->capacity + 1) * 2 + v->count + i) * 2;
            v->buffer = realloc(v->buffer, v->type_size * v->capacity);
        }
        if (!v->buffer) printf("BUFFER WITHOUT SIZE\n");
    }

    
    
    int original_count = v->count;
    v->count += i;
    return (v->buffer + original_count * v->type_size);
}

void init_vector(Vector * v, size_t i, size_t type_size)
{
    v->type_size = type_size;

    if (i)
    {
        v->buffer = malloc(type_size * i);
    }
    else
    {
        v->buffer = NULL;
    }
    v->count = i;
    v->capacity = i;
}

void free_vector(Vector * v)
{
    free(v->buffer);
    
}


mat4 compose_2d_transform(vec2 transform, float angle, vec2 scale) {
    float c = cosf(angle);
    float s = sinf(angle);

    mat4 out;
    out.a0 = c * scale.x;  out.b0 = -s * scale.y; out.c0 = 0.0f; out.d0 = transform.x;
    out.a1 = s * scale.x;  out.b1 =  c * scale.y; out.c1 = 0.0f; out.d1 = transform.y;
    out.a2 = 0.0f;    out.b2 = 0.0f;    out.c2 = 1.0f; out.d2 = 0.0f;
    out.a3 = 0.0f;    out.b3 = 0.0f;    out.c3 = 0.0f; out.d3 = 1.0f;
    return out;
}


uint16_t float32_to_float16(float f) {
    uint32_t x = *(uint32_t*)&f;
    
    uint32_t sign = (x >> 16) & 0x8000;
    int32_t exponent = ((x >> 23) & 0xff) - 127;
    uint32_t mantissa = x & 0x7fffff;
    
    if (exponent <= -15) {
        return sign; 
    }
    if (exponent >= 16) {
        return sign | 0x7c00; 
    }
    
    uint32_t new_exponent = (exponent + 15) << 10;
    uint32_t new_mantissa = mantissa >> 13;
    
    return sign | new_exponent | new_mantissa;
}



float
    mod289
    ( float x )
{ return x - floorf(x * (1.0f / 289.0f)) * 289.0f; }

vec4
    perm
    ( vec4 x )

{

    vec4 p = vec4_mul(
        vec4_add(
            vec4_scale(x, 34.0f),
            (vec4){ 1.0f, 1.0f, 1.0f, 1.0f }
        ),
        x
    );

    p.x = mod289(p.x);
    p.y = mod289(p.y);
    p.z = mod289(p.z);
    p.w = mod289(p.w);
    return p;

}

vec3
    vec3_fract
    (vec3 x)

{ 

    vec3 p;
    p.x = x.x - floorf(x.x);
    p.y = x.y - floorf(x.y);
    p.z = x.z - floorf(x.z);
    
    return p; 

}

vec4
    vec4_fract
    ( vec4 x )

{ 

    vec4 p;
    p.x = x.x - floorf(x.x);
    p.y = x.y - floorf(x.y);
    p.z = x.z - floorf(x.z);
    p.w = x.w - floorf(x.w);
    
    return p; 

}


float
    fbm_noise
    ( vec3 p )
    
{
    
    vec3 a = (vec3){floorf(p.x), floorf(p.y), floorf(p.z)};
    vec3 d = vec3_sub(p, a);

    d = vec3_mul(vec3_mul(d, d), vec3_sub((vec3){3.0f, 3.0f, 3.0f}, vec3_scale(d, 2.0f)));

    vec4 b = vec4_add((vec4){a.x, a.x, a.y, a.y}, (vec4){0.0f, 1.0f, 0.0f, 1.0f});
    vec4 k1 = perm((vec4){b.x, b.y, b.x, b.y});
    vec4 k2 = perm(vec4_add((vec4){k1.x, k1.y, k1.x, k1.y}, (vec4){b.z, b.z, b.w, b.w}));

    vec4 c = vec4_add(k2, (vec4){a.z, a.z, a.z, a.z});
    vec4 k3 = perm(c);
    vec4 k4 = perm(vec4_add(c, (vec4){1.0f, 1.0f, 1.0f, 1.0f}));

    vec4 o1 = vec4_fract(vec4_scale(k3, (1.0f / 41.0f)));
    vec4 o2 = vec4_fract(vec4_scale(k4, (1.0f / 41.0f)));

    vec4 o3 = vec4_add(vec4_scale(o2, d.z), vec4_scale(o1, (1.0f - d.z)));
    vec2 o4 = vec2_add(vec2_scale((vec2){o3.y, o3.w}, d.x), vec2_scale((vec2){o3.x, o3.z}, (1.0f - d.x)));
    
    return o4.y * d.y + o4.x * (1.0 - d.y);

} 

float
    fbm
    ( vec3 p )
    
{
    
	float v = 0.0;
	float a = 0.5;
	float f = 1.0;
    
	vec3 shift = (vec3){100, 100, 100};
	for (int i = 0; i < 5; ++i) {

		v += a * fbm_noise(vec3_scale(p, f));
		
		p = vec3_add(vec3_scale(p, 2.0), shift);
		f *= 2.5;
		a *= 0.45;

	}
	
	return v;

}


bool test_sphere_aabb(vec3 light_pos, float light_radius, vec3 min, vec3 max)
{
    
    vec3 closest_point = vec3_clamp(light_pos, min, max);
    
    vec3 delta = vec3_sub(light_pos, closest_point);
    
    return vec3_dot(delta, delta) <= (light_radius * light_radius);
}



void init_aabb(AABB * a, vec3 center, vec3 extents)
{

    a->center = center;
    a->extents = extents;
    
    a->min_x = a->center.x - a->extents.x;
    a->max_x = a->center.x + a->extents.x;
    a->min_y = a->center.y - a->extents.y;
    a->max_y = a->center.y + a->extents.y;
    a->min_z = a->center.z - a->extents.z;
    a->max_z = a->center.z + a->extents.z;

}




bool aabbs_intersect(AABB * a, AABB * b)
{

    if (
        a->min_x <= b->max_x &&
        a->max_x >= b->min_x &&
        a->min_y <= b->max_y &&
        a->max_y >= b->min_y &&
        a->min_z <= b->max_z &&
        a->max_z >= b->min_z
        )
    {
        return true;
    }
    else
    {
        return false;
    }
    
}


void calculate_aabb_extents(AABB * aabb)
{
    
    aabb->extents = (vec3){fabsf(aabb->max_x - aabb->min_x) / 2, fabsf(aabb->max_y - aabb->min_y) / 2, fabsf(aabb->max_z - aabb->min_z) / 2};
    aabb->center = (vec3){aabb->min_x + aabb->extents.x, aabb->min_y + aabb->extents.y, aabb->min_z + aabb->extents.z};
    
}
void calculate_aabb_bounds(AABB * aabb)
{
    vec3 center = aabb->center;
    aabb->min_x = center.x - aabb->extents.x;
    aabb->max_x = center.x + aabb->extents.x;
    aabb->min_y = center.y - aabb->extents.y;
    aabb->max_y = center.y + aabb->extents.y;
    aabb->min_z = center.z - aabb->extents.z;
    aabb->max_z = center.z + aabb->extents.z;

    
}

float triangle_area(vec3 a, vec3 b, vec3 c) {
    vec3 ab = {b.x - a.x, b.y - a.y, b.z - a.z};
    vec3 ac = {c.x - a.x, c.y - a.y, c.z - a.z};
    vec3 cross = {
        ab.y * ac.z - ab.z * ac.y,
        ab.z * ac.x - ab.x * ac.z,
        ab.x * ac.y - ab.y * ac.x
    };
    return sqrtf(cross.x * cross.x + cross.y * cross.y + cross.z * cross.z) * 0.5f;
}


bool is_similar_plane(vec3 n1, vec3 n2, vec3 p1, vec3 p2, float epsilon) {

    float dot = vec3_dot(n1, n2);
    if (dot < 1 - epsilon) return false;

    float d1 = -vec3_dot(n1, p1);
    float d2 = -vec3_dot(n2, p2);
    return fabsf(d1 - d2) < epsilon;

}

bool ray_intersects_triangle(vec3 origin, vec3 dir, vec3 a, vec3 b, vec3 c, float *out_t, vec3 *out_hit_point, vec3 * barycentric) {

    vec3 edge1 = vec3_sub(b, a);
    vec3 edge2 = vec3_sub(c, a);
    vec3 h = vec3_cross(dir, edge2);
    float dA = vec3_dot(edge1, h);

    if (fabsf(dA) < 0.000001f) {
        return false;
    }

    float f = 1.0f / dA;
    vec3 s = vec3_sub(origin, a);
    float u = f * vec3_dot(s, h);
    if (u < 0.0f || u > 1.0f) {
        return false;
    }

    vec3 q = vec3_cross(s, edge1);
    float v = f * vec3_dot(dir, q);
    if (v < 0.0f || u + v > 1.0f) {
        return false;
    }

    float t = f * vec3_dot(edge2, q);
    if (t > 0.0f) {
        *out_t = t;

        float w = 1.0f - u - v;
        *out_hit_point = vec3_add(
                            vec3_add(vec3_scale(a, w), 
                                    vec3_scale(b, u)),
                            vec3_scale(c, v));
        barycentric->x = w;
        barycentric->y = u;
        barycentric->z = v;
        
        return true;
    }

    // printf("REJECT: t < 0\n");
    return false;
}

bool compute_barycentric(const vec2 uv0, const vec2 uv1, const vec2 uv2,
                         const vec2 p, float * b0, float * b1, float * b2)
{
    float x0 = uv0.x, y0 = uv0.y;
    float x1 = uv1.x, y1 = uv1.y;
    float x2 = uv2.x, y2 = uv2.y;
    float x = p.x, y = p.y;

    float denom = (y1 - y2)*(x0 - x2) + (x2 - x1)*(y0 - y2);
    if (fabsf(denom) < 1e-6f)
    {
        return false;
    } 


    float inv_denom = 1.0f / denom;

    *b0 = ((y1 - y2)*(x - x2) + (x2 - x1)*(y - y2)) * inv_denom;
    *b1 = ((y2 - y0)*(x - x2) + (x0 - x2)*(y - y2)) * inv_denom;
    *b2 = 1.0f - *b0 - *b1;

    return true;
}

void compute_3d_barycentric(vec3 a, vec3 b, vec3 c, vec3 p, float * wA, float * wB, float * wC)
{

    vec3 v0 = vec3_sub(b, a);
    vec3 v1 = vec3_sub(c, a);
    vec3 v2 = vec3_sub(p, a);

    vec3 tn = vec3_cross(v0, v1);
    float ta = vec3_length(tn);

    float wwC = vec3_length(vec3_cross(v0, v2)) / ta;
    float wwB = vec3_length(vec3_cross(v2, v1)) / ta;
    float wwA = 1.0f - wwB - wwC;
    *wA = wwA;
    *wB = wwB;
    *wC = wwC;
        
    
}

vec3 generate_bounce(uint32_t i, uint32_t N) {


    int grid = 8;

    int ix = i % grid;
    int iy = i / grid;

    float u = (ix + (float)rand() / RAND_MAX) / grid;
    float v = (iy + (float)rand() / RAND_MAX) / grid;
    float r = sqrt(u);
    float theta = 2.0f * M_PI * v;

    float x = r * cos(theta);
    float y = r * sin(theta);
    float z = sqrt(fmaxf(0, 1.0f - x*x - y*y));
    return (vec3){x,y,z};
}


vec3 generate_sphere_dir(int i, int N)
{
    
    float phi = M_PI * (3.0f - sqrtf(5.0f));

    float y = 1.0f - 2.0f*(i+0.5f)/N;

    float r = sqrtf(1-y*y);

    float theta = phi*i;

    return (vec3){
    cos(theta)*r,
    y,
    sin(theta)*r};
}

AABB transform_aabb_by_position(AABB aabb, vec3 pos)
{
    aabb.center = vec3_add(aabb.center, pos);
    calculate_aabb_bounds(&aabb); 
    return aabb;

}

AABB transform_aabb_by_model_matrix(AABB aabb, mat4 mat)
{

    aabb.center = mat4_mul_point(mat, aabb.center);

    vec3 AX = mat4_mul_direction(mat, (vec3){1,0,0});
    vec3 AY = mat4_mul_direction(mat, (vec3){0,1,0});
    vec3 AZ = mat4_mul_direction(mat, (vec3){0,0,1});


    vec3 new_extents;
    new_extents.x = fabsf(AX.x) * aabb.extents.x + fabsf(AY.x) * aabb.extents.y + fabsf(AZ.x) * aabb.extents.z;
    new_extents.y = fabsf(AX.y) * aabb.extents.x + fabsf(AY.y) * aabb.extents.y + fabsf(AZ.y) * aabb.extents.z;
    new_extents.z = fabsf(AX.z) * aabb.extents.x + fabsf(AY.z) * aabb.extents.y + fabsf(AZ.z) * aabb.extents.z;
    aabb.extents = new_extents;
    aabb.min_x = aabb.center.x - new_extents.x;
    aabb.max_x = aabb.center.x + new_extents.x;
    aabb.min_y = aabb.center.y - new_extents.y;
    aabb.max_y = aabb.center.y + new_extents.y;
    aabb.min_z = aabb.center.z - new_extents.z;
    aabb.max_z = aabb.center.z + new_extents.z;

    return aabb;
    
}
AABB transform_aabb_by_model_matrix_rotation(AABB aabb, mat4 mat)
{
    vec3 AX = mat4_mul_direction(mat, (vec3){1,0,0});
    vec3 AY = mat4_mul_direction(mat, (vec3){0,1,0});
    vec3 AZ = mat4_mul_direction(mat, (vec3){0,0,1});


    vec3 new_extents;
    new_extents.x = fabsf(AX.x) * aabb.extents.x + fabsf(AY.x) * aabb.extents.y + fabsf(AZ.x) * aabb.extents.z;
    new_extents.y = fabsf(AX.y) * aabb.extents.x + fabsf(AY.y) * aabb.extents.y + fabsf(AZ.y) * aabb.extents.z;
    new_extents.z = fabsf(AX.z) * aabb.extents.x + fabsf(AY.z) * aabb.extents.y + fabsf(AZ.z) * aabb.extents.z;
    aabb.extents = new_extents;
    aabb.min_x = aabb.center.x - new_extents.x;
    aabb.max_x = aabb.center.x + new_extents.x;
    aabb.min_y = aabb.center.y - new_extents.y;
    aabb.max_y = aabb.center.y + new_extents.y;
    aabb.min_z = aabb.center.z - new_extents.z;
    aabb.max_z = aabb.center.z + new_extents.z;

    return aabb;
}





bool vec4_equal(vec4 a, vec4 b, float eps)
{
    if (fabsf(a.x - b.x) < eps &&
        fabsf(a.y - b.y) < eps &&
        fabsf(a.z - b.z) < eps &&
        fabsf(a.w - b.w) < eps)
    {
        
        return true;
    }
    else
    {
        
        return false;
    }
}
bool vec3_equal(vec3 a, vec3 b, float eps)
{
    if (fabsf(a.x - b.x) < eps &&
        fabsf(a.y - b.y) < eps &&
        fabsf(a.z - b.z) < eps)
    {
        
        return true;
    }
    else
    {
        
        return false;
    }
}

bool triangles_are_contiguous(Triangle * a, Triangle * b)
{

    float eps = 0.001f;
    int eq = 0;
    if (vec3_equal(a->pts[A].pos, b->pts[A].pos, 0.001f)) eq++;
    if (vec3_equal(a->pts[A].pos, b->pts[B].pos, 0.001f)) eq++;
    if (vec3_equal(a->pts[A].pos, b->pts[C].pos, 0.001f)) eq++;
    if (vec3_equal(a->pts[B].pos, b->pts[A].pos, 0.001f)) eq++;
    if (vec3_equal(a->pts[B].pos, b->pts[B].pos, 0.001f)) eq++;
    if (vec3_equal(a->pts[B].pos, b->pts[C].pos, 0.001f)) eq++;
    if (vec3_equal(a->pts[C].pos, b->pts[A].pos, 0.001f)) eq++;
    if (vec3_equal(a->pts[C].pos, b->pts[B].pos, 0.001f)) eq++;
    if (vec3_equal(a->pts[C].pos, b->pts[C].pos, 0.001f)) eq++;

    // Why 3?
    if (eq >= 3) return true;
    return false;
    
    
}
bool mesh_faces_are_contiguous(MeshFace * a, vec3 * a_verts, MeshFace * b, vec3 * b_verts)
{

    float eps = 0.001f;
    int eq = 0;
    for (int i = 0; i < 3; i++)
    {
        size_t i0 = a->indices[i];
        size_t i1 = a->indices[(i+1)%3];
        vec3 a0 = a_verts[i0];
        vec3 a1 = a_verts[i1];
        for (int j = 0; j < 3; j++)
        {
            size_t j0 = b->indices[j];
            size_t j1 = b->indices[(j+1)%3];
            vec3 b0 = b_verts[j0];
            vec3 b1 = b_verts[j1];
            if ((vec3_equal(a0, b0, 0.001f) && vec3_equal(a1, b1, 0.001f)) ||
                (vec3_equal(a0, b1, 0.001f) && vec3_equal(a1, b0, 0.001f)))
            {
                return true;
            }

        }
        
    }
    
    return false;
}




int leftmost_point(vec3 *points, int count) {
    int min_idx = 0;
    for (int i = 1; i < count; i++) {
        if (points[i].x < points[min_idx].x || 
            (points[i].x == points[min_idx].x && points[i].y < points[min_idx].y)) {
            min_idx = i;
        }
    }
    return min_idx;
}
int leftmost_pointr(vec3 *points, int count) {
    int min_idx = 0;
    for (int i = 1; i < count; i++) {
        if (points[i].z < points[min_idx].z || 
            (points[i].z == points[min_idx].z && points[i].y < points[min_idx].y)) {
            min_idx = i;
        }
    }
    return min_idx;
}
size_t leftmost_pointi(vec3 *points, size_t * indices, size_t count) {
    size_t min_idx = 0;
    for (int i = 1; i < count; i++) {
        if (points[indices[i]].x < points[indices[min_idx]].x || 
            (points[indices[i]].x == points[indices[min_idx]].x && points[indices[i]].y < points[indices[min_idx]].y)) {
            min_idx = i;
        }
    }
    return min_idx;
}
size_t leftmost_pointri(vec3 *points, size_t * indices, size_t count) {
    size_t min_idx = 0;
    for (int i = 1; i < count; i++) {
        if (points[indices[i]].z < points[indices[min_idx]].z || 
            (points[indices[i]].z == points[indices[min_idx]].z && points[indices[i]].y < points[indices[min_idx]].y)) {
            min_idx = i;
        }
    }
    return min_idx;
}
size_t leftmost_pointv2(vec2 *points, size_t count) {
    size_t min_idx = 0;
    for (int i = 1; i < count; i++) {
        if (points[i].x < points[min_idx].x || 
            (points[i].x == points[min_idx].x && points[i].y < points[min_idx].y)) {
            min_idx = i;
        }
    }
    return min_idx;
}

bool ccwv2(vec2 p, vec2 a, vec2 b) {
    float cross = (a.x - p.x) * (b.y - p.y) - (a.y - p.y) * (b.x - p.x);
    return cross > 0;
}

bool ccw(vec3 p, vec3 a, vec3 b) {
    float cross = (a.x - p.x) * (b.y - p.y) - (a.y - p.y) * (b.x - p.x);
    return cross > 0;
}
bool ccwr(vec3 p, vec3 a, vec3 b) {
    float cross = (a.z - p.z) * (b.y - p.y) - (a.y - p.y) * (b.z - p.z);
    return cross > 0;
}

void compute_convex_hull(vec3 *points, int count, vec3 *out_hull, int *out_count, vec3 normal) {
    if (count < 3) return;

    

    bool z_deg = 1 - fabsf(normal.z) < 0.001f;
    int left = z_deg ? leftmost_pointr(points, count) : leftmost_point(points, count);
    int p = left, q;
    int max_steps = count + 2;  
    int steps = 0;
    do {
        if (++steps > max_steps) {
            fprintf(stderr, "Error: convex hull infinite loop detected.\n");
            break;
        }
        out_hull[(*out_count)++] = points[p];
        q = (p + 1) % count;
        for (int i = 0; i < count; i++) {
                
            if (z_deg)
            {
                
                if (ccwr(points[p], points[i], points[q])) {

                    q = i;
                }
            }
            else
            {
                
                if (ccw(points[p], points[i], points[q])) {
                    q = i;
                }
            }
        }
        p = q;
    } while (p != left);
}




void compute_hull_faces(vec3 * points, size_t * indices, size_t count, size_t * out_hull, size_t * out_count, vec3 normal) {

    if (count < 3) return;


    vec3 N = vec3_normalize(normal);    
    vec3 up;

    if (fabsf(N.z) < 0.9f)
        up = (vec3){0,0,1};
    else
        up = (vec3){1,0,0};
    
    vec3 T = vec3_normalize(vec3_cross(up, N));
    vec3 B = vec3_cross(N, T);    

    vec2 * local_points = malloc(sizeof(vec2) * count);

    for (int i = 0; i < count; i++)
    {
        vec3 p = points[indices[i]];

        local_points[i].x = vec3_dot(p, T);
        local_points[i].y = vec3_dot(p, B);
    }

    bool z_deg = 1 - fabsf(normal.z) < 0.001f;
    int left = leftmost_pointv2(local_points, count);
    int p = left, q;
    int max_steps = count + 2;  
    int steps = 0;
    do {
        if (++steps > max_steps) {
            fprintf(stderr, "Error: convex hull infinite loop detected.\n");
            break;
        }
        out_hull[(*out_count)++] = indices[p];
        q = (p + 1) % count;
        for (int i = 0; i < count; i++) {
                
                
            if (ccwv2(local_points[p], local_points[i], local_points[q])) {
                q = i;
            }
        }
        p = q;
    } while (p != left);
    free(local_points);
}

void compute_aabb_from_points(vec3 * points, int point_count, AABB * aabb)
{
    aabb->min_x = INFINITY;
    aabb->min_y = INFINITY;
    aabb->min_z = INFINITY;
    aabb->max_x = -INFINITY;
    aabb->max_y = -INFINITY;
    aabb->max_z = -INFINITY;
    
    for (int i = 0; i < point_count; i++)
    {
        vec3 p = points[i];
        if (p.x < aabb->min_x) aabb->min_x = p.x;
        if (p.y < aabb->min_y) aabb->min_y = p.y;
        if (p.z < aabb->min_z) aabb->min_z = p.z;
        if (p.x > aabb->max_x) aabb->max_x = p.x;
        if (p.y > aabb->max_y) aabb->max_y = p.y;
        if (p.z > aabb->max_z) aabb->max_z = p.z;
    }

    calculate_aabb_extents(aabb);
    
}



int clip_against_plane(
    const vec3 *in,
    int in_count,
    vec3 *out,
    vec3 plane_point,
    vec3 plane_normal)
{
    int out_count = 0;

    for(int i=0;i<in_count;i++)
    {
        int j = (i+1)%in_count;

        vec3 A = in[i];
        vec3 B = in[j];

        float da = vec3_dot(vec3_sub(A, plane_point), plane_normal);
        float db = vec3_dot(vec3_sub(B, plane_point), plane_normal);

        bool Ain = da > 0;
        bool Bin = db > 0;

        if(Ain && Bin)
        {
            out[out_count++] = B;
        }
        else if(Ain && !Bin)
        {
            float t = da/(da-db);
            out[out_count++] = vec3_add(A,
                    vec3_scale(vec3_sub(B, A), t));
        }
        else if(!Ain && Bin)
        {
            float t = da/(da-db);
            out[out_count++] = vec3_add(A,
                    vec3_scale(vec3_sub(B, A), t));
            out[out_count++] = B;
        }
    }

    return out_count;
}

int clip_pts_against_plane( ClipVertex *in, int in_count, ClipVertex *out, vec3 plane_point, vec3 plane_normal)
{
    int out_count = 0;

    for(int i=0;i<in_count;i++)
    {
        int j = (i+1)%in_count;

        vec3 A = in[i].p;
        vec3 B = in[j].p;

        float da = vec3_dot(vec3_sub(A, plane_point), plane_normal);
        float db = vec3_dot(vec3_sub(B, plane_point), plane_normal);

        bool Ain = da > 0;
        bool Bin = db > 0;

        if(Ain && Bin)
        {
            out[out_count].p = B;
            out[out_count].edge0 = in[j].edge0;
            out[out_count].edge1 = in[j].edge1;
            out_count++;
        }
        else if(Ain && !Bin)
        {
            float t = da/(da-db);
            out[out_count].p = vec3_add(A, vec3_scale(vec3_sub(B, A), t));

            out[out_count].edge0 = in[i].edge0;
            out[out_count].edge1 = in[i].edge1;
            out_count++;
        }
        else if(!Ain && Bin)
        {
            float t = da/(da-db);
            out[out_count].p = vec3_add(A, vec3_scale(vec3_sub(B, A), t));
            out[out_count].edge0 = in[i].edge0;
            out[out_count].edge1 = in[i].edge1;
            out_count++;
            out[out_count].p = B;
            out[out_count].edge0 = in[j].edge0;
            out[out_count].edge1 = in[j].edge1;
            out_count++;
        }
    }

    return out_count;
}

// vec3 compute_hull_centroid(vec3 *hull_points, int hull_count, float bottom, float top) {
//     double signed_area = 0.0;
//     double cx = 0.0, cy = 0.0;

//     for (int i = 0; i < hull_count; i++) {
//         int j = (i + 1) % hull_count;

//         double xi = hull_points[i].x;
//         double yi = hull_points[i].y;
//         double xj = hull_points[j].x;
//         double yj = hull_points[j].y;

//         double a = xi * yj - xj * yi;
//         signed_area += a;

//         cx += (xi + xj) * a;
//         cy += (yi + yj) * a;
//     }

//     signed_area *= 0.5;

//     vec3 centroid;
//     if (fabs(signed_area) < 1e-6) {
//         centroid.x = 0;
//         centroid.y = 0;
//         for (int i = 0; i < hull_count; i++) {
//             centroid.x += hull_points[i].x;
//             centroid.y += hull_points[i].y;
//         }
//         centroid.x /= hull_count;
//         centroid.y /= hull_count;
//     } else {
//         centroid.x = cx / (6.0 * signed_area);
//         centroid.y = cy / (6.0 * signed_area);
//     }

//     centroid.z = (bottom + top) * 0.5f;


//     return centroid;
// }



int clip_tri_against_plane(Vector * points, Vector * faces, vec3 plane, vec3 plane_point, vec3 face_points[3], vec3 j_normal)
{
    vec3 * pts = CAST_VEC(*points, vec3);

    float d[3];
    bool s[3] = {0};
    vec3 p = plane_point;
    vec3 normal = plane;

    int splits = 0;
    for (int k = 0; k < 3; k++)
    {

        vec3 inside = vec3_sub(face_points[k], p);
        d[k] = vec3_dot(inside, normal);
    
            
    
        if (d[k] <= 0.0f)
        {
            s[k] = true;
            splits++;
        }

    }
    if (splits == 0)
    {

        return 0;

    }
    else if (splits == 3)
    {
        // dont add face, it is completely outside
        return 3;
        
    }
    else
    {

        vec3 new_points[3];
        int new_point_count = 0;
        for (int k = 0; k < 3; k++)
        {
            float da = d[k];
            float db = d[(k+1) % 3];
            if (s[k] != s[(k+1) % 3])
            {
                // for edge ab
                // da = dot(n, a-p)
                // db = dot(n, b-p)

                // t = da / (da - db)
                // point = a + t(b-a)
                float t = da / (da - db);
                new_points[new_point_count++] =
                    vec3_add(face_points[k],
                            vec3_scale(vec3_sub(face_points[(k+1) % 3], face_points[k]), t));
    
            }
        }
        
        vec3 * inside = vector_expand(points, 2);
        inside[0] = new_points[0];
        inside[1] = new_points[1];
        size_t ib = points->count - 2;
        size_t ic = points->count - 1;

        int opp_outside_tri_emitted = 0;
        int opp_inside_tri_emitted = 0;


        int in = 0;
        int in_idx[3] = {0};
        int in_idx_cnt = 0;
        for (int k = 0; k < 3; k++)
        {

            size_t ia = 0;
            size_t oa = 0;
            if (!s[k])
            {
                in++;
                in_idx[in_idx_cnt++] = k;
     
            }
        }
        if (in > 1)
        {


            inside = vector_expand(points, 1);
            inside[0] = face_points[in_idx[0]];
            int ia = points->count - 1;

            MeshFace * i0 = vector_expand(faces, 1);
            i0->indices[0] = ia;
            i0->indices[1] = ic;
            i0->indices[2] = ib;
            i0->normal = j_normal;

        }
        else
        {
            inside = vector_expand(points, 1);
            inside[0] = face_points[in_idx[0]];
            int ia = points->count - 1;

            MeshFace * i0 = vector_expand(faces, 1);
            i0->indices[0] = ia;
            i0->indices[1] = ib;
            i0->indices[2] = ic;
            i0->normal = j_normal;
     
            inside = vector_expand(points, 1);
            inside[0] = face_points[in_idx[1]];
            size_t id = points->count - 1;

            i0 = vector_expand(faces, 1);
            size_t ic1 = 0;
            if (vec3_length(vec3_sub(new_points[0], inside[0])) > vec3_length(vec3_sub(new_points[1], inside[0])))
            {
                ic1 = ic;
            }
            else
            {
                ic1 = ib;
            }

            i0->indices[0] = id;
            i0->indices[1] = ic1;
            i0->indices[2] = ia;
            i0->normal = j_normal;

        
        }


        
        return 1;

    }

    
}

bool perform_clip(Vector * points, Vector * faces, vec3 planes[6], vec3 plane_points[6], vec3 face_points[3], vec3 normal)
{


    bool did_clip = false;
    int iter = 0;
    int safety = 2;
    bool has_not_split = true;
    bool is_out = false;
    int clipped = -1;
    vec3 pts_a[64];
    pts_a[0] = face_points[0];
    pts_a[1] = face_points[1];
    pts_a[2] = face_points[2];
    int a_cnt = 3;
    vec3 pts_b[64];
    int b_cnt = 0;
    
    b_cnt = clip_against_plane(pts_a, a_cnt, pts_b, plane_points[0], planes[0]);

    for(int i=1; i < b_cnt-1; i++)
    {

        vec3 e0 = vec3_sub(pts_b[i], pts_b[0]);
        vec3 e1 = vec3_sub(pts_b[i+1], pts_b[0]);

        float area2 = vec3_length(vec3_cross(e0, e1));
        if (area2 < 0.001f) continue;
        vec3 * pts = vector_expand(points, 3);
        pts[0] = pts_b[0];
        pts[1] = pts_b[i];
        pts[2] = pts_b[i+1];

        MeshFace * f = vector_expand(faces, 1);
        f->indices[0] = points->count-3;
        f->indices[1] = points->count-2;
        f->indices[2] = points->count-1;
        f->normal = normal;
    }
    return did_clip;
}



void clip_triangle(Vector * points, Vector * faces, vec3 planes[6], vec3 plane_points[6], vec3 a, vec3 b, vec3 c, vec3 normal)
{


    Vector new_verts;
    init_vector(&new_verts, 0, sizeof(vec3));
    Vector new_faces;
    init_vector(&new_faces, 0, sizeof(MeshFace));



    int did_cut = false;
    bool cannot_cut = false;

    int safety = 100;
    int iter = 0;
    perform_clip(&new_verts, &new_faces, planes, plane_points, (vec3[]){a,b,c}, normal);

    if (new_verts.count > 0 && new_faces.count > 0)
    {
        if (points->capacity > 0)
        {
        
            if (points->capacity < points->count + new_verts.count)
            {
                points->capacity = (points->capacity * 2) + new_verts.count + points->count;
                // printf("%p\n", points->buffer);
                points->buffer = realloc(points->buffer, sizeof(vec3) * points->capacity);
            }
        }
        else
        {
            points->capacity = new_verts.count * 2;
            points->buffer = malloc(sizeof(vec3) * points->capacity);
        }
        vec3 * pts = CAST_VEC(*points, vec3);
        vec3 * new_pts = CAST_VEC(new_verts, vec3);
        for (int i = 0; i < new_verts.count; i++)
        {
            int index = i + points->count;
            pts[index] = new_pts[i];
        }
        if (faces->capacity > 0)
        {
            
            if (faces->capacity < faces->count + new_faces.count)
            {
                faces->capacity = (faces->capacity * 2) + new_faces.count + faces->count;
                faces->buffer = realloc(faces->buffer, sizeof(MeshFace) * faces->capacity);
            }
        }
        else
        {
            
            faces->capacity = new_faces.count * 2;
            faces->buffer = malloc(sizeof(MeshFace) * faces->capacity);
        }
        MeshFace * f = CAST_VEC(*faces, MeshFace);

        MeshFace * mfs = CAST_VEC(new_faces, MeshFace);
        for (int i = 0; i < new_faces.count; i++)
        {
            int index = i + faces->count;
            f[index] = mfs[i];
            f[index].indices[0] += points->count;
            f[index].indices[1] += points->count;
            f[index].indices[2] += points->count;
        }
        points->count += new_verts.count;
        faces->count += new_faces.count;
        
        if (new_verts.buffer != NULL) free(new_verts.buffer);
        if (new_faces.buffer != NULL) free(new_faces.buffer);
    }

    
    
}




bool point_in_triangle(vec3 P, vec3 A, vec3 B, vec3 C)
{
    
    vec3 v0 = vec3_sub(B, A);
    vec3 v1 = vec3_sub(C, A);
    vec3 v2 = vec3_sub(P, A);

    float d00 = vec3_dot(v0, v0);
    float d01 = vec3_dot(v0, v1);
    float d11 = vec3_dot(v1, v1);
    float d20 = vec3_dot(v2, v0);
    float d21 = vec3_dot(v2, v1);


    float denom = d00 * d11 - d01 * d01;    

    float v = (d11 * d20 - d01 * d21) / denom;
    float w = (d00 * d21 - d01 * d20) / denom;
    float u = 1.0f - v - w;    


    const float eps = 0.01f;

    bool inside =
        u >= -eps &&
        v >= -eps &&
        w >= -eps;

    return inside;
}

vec3 project_point_plane(vec3 point, vec3 plane_point, vec3 normal)
{

    float d = vec3_dot(vec3_sub(point, plane_point), normal);

    return vec3_sub(point, vec3_scale(normal, d));

}

vec3 closest_point_segment(vec3 p, vec3 a, vec3 b)
{
    vec3 ab = vec3_sub(b, a);

    float t = vec3_dot(vec3_sub(p, a), ab) / vec3_dot(ab, ab);

    t = clamp(t, 0.0f, 1.0f);

    return vec3_add(a, vec3_scale(ab, t));

}

float point_to_segment_dist(vec2 p, vec2 a, vec2 b) {

    vec2 ab = {b.x - a.x, b.y - a.y};
    vec2 ap = {p.x - a.x, p.y - a.y};

    float t = (ap.x * ab.x + ap.y * ab.y) / (ab.x * ab.x + ab.y * ab.y);
    t = fmaxf(0.0f, fminf(1.0f, t));

    vec2 closest = { a.x + t * ab.x, a.y + t * ab.y };
    float dx = p.x - closest.x;
    float dy = p.y - closest.y;

    return sqrtf(dx*dx + dy*dy);

}


vec3 segment_plane_distance(vec3 proposed, vec3 pts[3], vec3 normal)
{

    vec3 b = proposed;
    float da = vec3_dot(vec3_sub(proposed, pts[0]), normal);
    float db = vec3_dot(vec3_sub(b, pts[0]), normal);
    float dd = da * db;
    if (dd > 0) return (vec3){999,999,999};

    float t = da/(da-db);

    return vec3_add(proposed, vec3_scale(vec3_sub(b, proposed), t));    

}

vec3 point_triangle_distance(vec3 proposed, vec3 pts[3], vec3 normal)
{
    vec3 p = project_point_plane(proposed, pts[0], normal);



    vec3 ab = closest_point_segment(proposed, pts[0], pts[1]);
    vec3 bc = closest_point_segment(proposed, pts[1], pts[2]);
    vec3 ac = closest_point_segment(proposed, pts[2], pts[0]);

    float ab_dist = vec3_length(vec3_sub(proposed, ab));
    float bc_dist = vec3_length(vec3_sub(proposed, bc));
    float ac_dist = vec3_length(vec3_sub(proposed, ac));
    float p_dist = vec3_length(vec3_sub(p, proposed));

    if (ab_dist <= bc_dist && ab_dist <= ac_dist && ab_dist <= p_dist)
    {
        return ab;
    }
    if (bc_dist <= ab_dist && bc_dist <= ac_dist && bc_dist <= p_dist)
    {
        return bc;
    }
    if (ac_dist <= ab_dist && ac_dist <= bc_dist && ac_dist <= p_dist)
    {
        return ac;
        
    }
    if (p_dist <= ab_dist && p_dist <= bc_dist && p_dist <= ac_dist)
    {
        return p;
    }
    return p;

}



// From Ericson
vec3 closest_point_triangle(vec3 p, vec3 a, vec3 b, vec3 c)
{
    
    // Check if P in vertex region outside A
    vec3 ab = vec3_sub(b, a);
    vec3 ac = vec3_sub(c, a);
    vec3 ap = vec3_sub(p, a);
    float d1 = vec3_dot(ab, ap);
    float d2 = vec3_dot(ac, ap);
    if (d1 <= 0.0f && d2 <= 0.0f) return a; // barycentric coordinates (1,0,0)

    // Check if P in vertex region outside B
    vec3 bp = vec3_sub(p, b);
    float d3 = vec3_dot(ab, bp);
    float d4 = vec3_dot(ac, bp);
    if (d3 >= 0.0f && d4 <= d3) return b; // barycentric coordinates (0,1,0)

    // Check if P in edge region of AB, if so return projection of P onto AB
    float vc = d1*d4 - d3*d2;
    if (vc <= 0.0f && d1 >= 0.0f && d3 <= 0.0f) {

        float v = d1 / (d1 - d3);
        return vec3_add(a, vec3_scale(ab, v)); // barycentric coordinates (1-v,v,0)

    }

    // Check if P in vertex region outside C
    vec3 cp = vec3_sub(p, c);
    float d5 = vec3_dot(ab, cp);
    float d6 = vec3_dot(ac, cp);
    if (d6 >= 0.0f && d5 <= d6) return c; // barycentric coordinates (0,0,1)


    // // Check if P in edge region of AC, if so return projection of P onto AC
    float vb = d5*d2 - d1*d6;
    if (vb <= 0.0f && d2 >= 0.0f && d6 <= 0.0f) {
        float w = d2 / (d2 - d6);
        return vec3_add(a, vec3_scale(ac, w)); // barycentric coordinates (1-w,0,w)
    }
    // // Check if P in edge region of BC, if so return projection of P onto BC
    float va = d3*d6 - d5*d4;
    if (va <= 0.0f && (d4 - d3) >= 0.0f && (d5 - d6) >= 0.0f) {

        float w = (d4 - d3) / ((d4 - d3) + (d5 - d6));
        return vec3_add(b, vec3_scale(vec3_sub(c, b), w)); // barycentric coordinates (0,1-w,w)

    }
    // // P inside face region. Compute Q through its barycentric coordinates (u,v,w)
    float denom = 1.0f / (va + vb + vc);
    float v = vb * denom;
    float w = vc * denom;
    return vec3_add(vec3_add(a, vec3_scale(ab, v)), vec3_scale(ac, w)); // = u*a + v*b + w*c, u = va * denom = 1.0f - v - w
    
}
#define DEGREES_TO_RAD (3.14159265358979323846 / 180.0)
vec3 spherical_to_cartesian(float azimuth_deg, float elevation_deg, float radius) {
    
    float azimuth_rad = azimuth_deg * DEGREES_TO_RAD;
    float elevation_rad = elevation_deg * DEGREES_TO_RAD;

    float projected_rad = radius * cosf(elevation_rad);

    vec3 pos =
        {
            -projected_rad * sinf(azimuth_rad),
            radius * sinf(elevation_rad),
            -projected_rad * cosf(azimuth_rad)

        };


    return pos;
}
