/**
 * @file render.h
 * @brief The main render pass, data GPU uploads, and SSBO management.
 */
#ifndef RENDER_H
#define RENDER_H

#include "core/engine_types.h"
#include "ui/ui.h"

/**
 * @brief Binds SSBOs required for baking.
 */
void bind_baking_ssbos();

/**
 * @brief Unbinds SSBOs required for baking.
 */
void unbind_baking_ssbos();

/**
 * @brief Binds SSBOs required for rendering.
 */
void bind_render_ssbos();

/**
 * @brief Transfers an AABB to DebufVertex for a wireframe (GL_LINES).
 * @param out The outgoing vertices.
 * @param aabb The input AABB.
 * @param offset The offset in world space.
 * @todo Make a matrix transform instead of an offset. Or instance the AABBs and transform on the GPU.
 */
void aabb_to_vertices(DebugVertex * out, AABB * aabb, vec3 offset);

/**
 * @brief Adds Mesh AABB%s for each MeshNode child recursively to ::DEBUG_LINES.
 * @param mn The MeshNode root.
 */
void render_mesh_aabbs_from_root(MeshNode * mn);

/**
 * @brief Uploads Material data to the GPU as MaterialGPU.
 */
void upload_material_data();

/**
 * @brief Transfers a Light to a LightGPU.
 * @param l The light to copy from.
 * @param lgpu The target GPU light.
 */
void copy_light_to_gpu_light(LightGPU * lgpu, Light * l);

/**
 * @brief Uploads all current Light%s inside of Globals.lights to the GPU in a triple buffered ::LIGHT_SSBO.
 */
void upload_lights();

/**
 * @brief Adds a single Mesh to the ::MAIN_VBO.
 * @param m The Mesh to add.
 * @param layout The VertexLayout whose offset and counts during Mesh addition.
 */
void add_mesh_to_buffer(Mesh * m, VertexLayout * layout);

/**
 * @brief Adds all Mesh%es that have valid MeshNode%s.
 * @param layout The layout to use (::MAIN_VAO).
 * @critial Decide if we should upload ALL Mesh%es instead of just MeshNode%s here so that we can separate the two.
 */
void add_all_meshes_to_buffer(VertexLayout * layout);

/**
 * @brief Adds a TextBox to a vertex and index buffer, to be uploaded to ::TEXT_VBO and ::TEXT_EBO.
 * @param tvs The out vertex Vector.
 * @param tis The out index Vector.
 * @param tb The TextBox to upload.
 * @param current_box The index within the entire buffer, meaning a running count through each individual UIMenu%s.
 * @todo This current_box variable makes it so we can never reorder our ::TEXT_BOX_SSBO or else every vertex will be pointing to the wrong data. Resolve this.
 */
void add_text_box(Vector * tvs, Vector * tis, TextBox * tb, int current_box);

/**
 * @brief Adds all TextBox%es to ::TEXT_VBO and iterates through each character to push indices to ::TEXT_EBO.
 * @note This function is currently called on TextBox text change. Text should always be pre-loaded because this should not be called during "game-time."
 */
void add_all_text_boxes_to_draw_buffer();

/**
 * @brief Creates all DrawCommand%s based on unique SubMesh%es and sorts them via DrawCommand.sort_key.
 * @param dc_cnt The output count of DrawCommand%s.
 * @param dcs The output DrawCommand%s.
 */
void create_and_sort_draw_commands(int * dc_cnt, DrawCommand * dcs);

/**
 * @brief Uploads per-frame data to GPU and renders main render pass to a target FBO (omits Text, UI, and postprocessing that occurs after).
 * @param rprobe_cam The Camera to upload for the ::CAM_UBO.
 * @param render_fbo The main target FBO.
 * @param render_width The width of the target FBO.
 * @param probe_bake If this is baking ReflectionProbe%s. This flag controls if ReflectionProbe%s are sampled so that there is no accumulation.
 * @note This is used by the main pass and the ReflectionProbe cubemap rendering.
 */
void render_to_cubemap_face(Camera * rprobe_cam, GLuint render_fbo, int render_width, int render_height, bool probe_bake);

/**
 * @brief The main render pass. Calls render_to_cubemap_face() and then renders wireframes, text, ui, and postprocessing.
 */
void render();







#endif
