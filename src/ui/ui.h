/**
 * @file ui.h
 * @brief The standalone UI library that controls text rendering, ui rendering, and events.
 */

#ifndef UI_H
#define UI_H

#include "utilities/utilities.h"
#include "SDL2/SDL.h"

/**
 * @addtogroup ui_group 
 * @{
 */
/**
 * @name FBO Size
 * @brief The size of the rendering FBO, used to convert from screenspace to local UI space.
 * @{
 */
extern int TEXT_WIDTH;
extern int TEXT_HEIGHT;
/** @} */

extern Vector MENUS; /**<All UIMenu%s.*/

/**
 * @name UI Constants
 * @brief Sizes of UI arrays and UI Scale.
 * @todo Make UI scale dynamic.
 * @{
 */
#define MAX_UI_ELEMENTS 1024
#define MAX_UI_CONTAINERS 1024
#define UI_SCALE_WIDTH 2560
#define UI_SCALE_HEIGHT 1920
#define MAX_TEXT_BOXES 1024
#define MAX_TEXT_BOX_CHARACTERS 256
#define MAX_UI_CHILDREN 1024
#define MAX_GLYPHS 256
/** @} */


/**
 * @brief A single Glyph in a Font.
 * @note w_t and h_t are the texture size of the Font Image (parsed from json).
 * x and y are the positions on the texture.
 * w_c and h_c are the character sizes from json.
 * @see Font
 * @see TextBox
 */
typedef struct {

      int id;       /**<This ID == '<char>' when looking up which Glyph to use (ascii).*/            
    float u0;       /**<\f$x/w_t\f$.*/          
    float v0;       /**<\f$y/h_t\f$.*/          
    float u1;       /**<\f$(x+w_c)/w_t\f$.*/          
    float v1;       /**<\f$(y+h_c)/h_t\f$.*/          
      int width;    /**<\f$w_c\f$.*/
      int height;   /**<\f$h_c\f$.*/    
      int xoffset;  /**<The x offset before the character.*/
      int yoffset;  /**<The y offset before the character.*/ 
      int xadvance; /**<How much the x advances when it is typed.*/      

} Glyph;

/**
 * @brief A Font and its Glyph%s used for rendering TextBox%es.
 * @todo Figure out if highest_y is actually used.
 * @see Glyph
 * @see TextBox
 * @see ::FONTS
 */
typedef struct Font {

         int id;          /**<The ID of the Font in ::FONTS.*/
    size_t texture;       /**<The Image ID.*/
       Glyph * glyphs;    /**<Glyph%s for each character.*/
      size_t glyph_count; /**<The amount of Glyph%s.*/
         int tex_w;       /**<The texture dimensions.*/
         int tex_h;       /**<The texture dimensions.*/
         int highest_y;   /**<Used to determine the amount of space to cursor shift for newlines.*/
       int highest_dist; /**<Used to determine the amount of space to cursor shift for newlines.*/
      char name[256];    /**<The name of the file path.*/

} Font;

/**
 * @brief The GPU-side representation of a TextBox.
 * @see Glyph
 * @see TextBox
 * @see ::FONTS
 */
typedef struct TextBoxGPU
{

    uint32_t visible;
    uint32_t hovered;
    uint32_t shadow;
    float shadow_dist;
    uint32_t container_id;
    uint32_t mask;
    vec2 shadow_dir;
    uint32_t state;
    uint32_t pad0;
    uint32_t pad1;
    uint32_t pad2;
    vec4 color;
    vec4 fill;
    mat4 model;
    
} TextBoxGPU;

/**
 * @brief The GPU-side representation of a UIElement.
 * @see UIElement
 * @see UIContainer
 * @see UIMenu
 */
typedef struct UIElementGPU
{

    uint32_t visible;
    uint32_t hovered;
    uint32_t shadow;
    float shadow_dist;
    vec2 pos;
    vec2 size;
    vec2 shadow_dir;
    uint32_t state;
    uint32_t type;
    vec4 color;
    vec4 fill;
    vec4 padding;
    vec2 translation;
    vec2 scale;
    float radius;
    uint32_t masked;
    uint32_t container_id;
    uint32_t pad0;
    mat4 model;
    
    
} UIElementGPU;

typedef struct UIElement UIElement;

/**
 * @brief Options for Justify%ing TextBox%es inside of containers but also around the origin \f$(0,0)\f$.
 */
typedef enum Justify
{

    JUSTIFY_CENTER,
    JUSTIFY_LEFT,
    JUSTIFY_RIGHT,
    JUSTIFY_TOP_CENTER,
    JUSTIFY_TOP_LEFT,
    JUSTIFY_TOP_RIGHT,
    JUSTIFY_BOTTOM_CENTER,
    JUSTIFY_BOTTOM_LEFT,
    JUSTIFY_BOTTOM_RIGHT,
    JUSTIFY_MAX
   
} Justify;

/**
 * @brief The names of Justify for the Editor UI.
 */
static const char * JUSTIFY_NAMES[] =
{
    "JUSTIFY_CENTER",
    "JUSTIFY_LEFT",
    "JUSTIFY_RIGHT",
    "JUSTIFY_TOP_CENTER",
    "JUSTIFY_TOP_LEFT",
    "JUSTIFY_TOP_RIGHT",
    "JUSTIFY_BOTTOM_CENTER",
    "JUSTIFY_BOTTOM_LEFT",
    "JUSTIFY_BOTTOM_RIGHT"
};

/**
 * @brief UIState%s for UIElement%s.
 */
typedef enum UIState
{
    UISTATE_NONE,
    UISTATE_HOVERED,
    UISTATE_PRESSED,
    UISTATE_SELECTED /**<If the UIElement is selected in the Editor.*/
} UIState;

/**
 * @brief UIElement types for the union that describe unique functions.
 */
typedef enum UIInteractableType
{

    UI_INTERACTABLE_NONE,     /**<Normal UIElement.*/
    UI_INTERACTABLE_SLIDER,   /**<UISlider that can interpolate on a plane.*/
    UI_INTERACTABLE_DROPDOWN, /**<Unfinished type.*/
    UI_INTERACTABLE_LABEL,    /**<References a TextBox that is \ref Justify "Justified".*/
    UI_INTERACTABLE_MAX

} UIInteractableType;

/**
 * @brief UIElement interactable names for the union that describe unique functions.
 */
static const char * UI_INTERACTABLE_NAMES[] =
{
    "UI_INTERACTABLE_NONE",
    "UI_INTERACTABLE_SLIDER",
    "UI_INTERACTABLE_DROPDOWN",
    "UI_INTERACTABLE_LABEL"
};

/**
 * @brief UIContainer that contains both TextBox%es and UIElement%s and can mask and scroll them.
 * @see UIElement
 * @see TextBox
 */
typedef struct UIContainer
{

       int uID;                           /**<The ID inside of UIMenu.containers.*/
      bool free;                          /**<If this UIContainer is empty or can be replaced.*/

      vec2 scroll_offset;                 /**<The current offset from \f$(0,0)\f$.*/
      vec2 delta_scroll_offset;           /**<The difference between this frame's scroll offset and the current.*/
      vec2 scroll_dir;                    /**<The direction this container moves in when scrolled.*/

      bool wrap;                          /**<If UIElements and TextBox%es wrap around the container (unused).*/
      vec4 padding;                       /**<The padding before UIElement%s and TextBox%es are wrapped or masked.*/
      bool mask;                          /**<If the UIElement%s and TextBox%es contained should not be rendered if outside.*/
    
       int children[MAX_UI_CHILDREN];     /**<The IDs for UIElement%s contained.*/
    size_t child_count;                   /**<The amount of UIElement%s contained.*/
       int box_children[MAX_UI_CHILDREN]; /**<The IDs of TextBox%es contained.*/
    size_t box_count;                     /**<The amount of TextBox%es contained.*/

       int element;                       /**<Which UIElement owns this UIContainer @note -1 if empty.*/
       int scrollbar;                     /**<The scrollbar UISlider UIElement @note -1 if empty.*/
      char id[256];                       /**<The name of this UIContainer @note default is "default".*/
    
} UIContainer;

/**
 * @brief A structure holding a Font ID and text to be rendered and transformed with its properties.
 * @see Font
 * @see UIMenu
 * @todo Go through some of these members like hovered and see if necessary.
 */
typedef struct TextBox
{

        bool hovered;          /**<Currently this indicates if this TextBox is hovered since it does not receive events.*/
        bool shadow;           /**<If there is a drop shadow.*/
        bool visible;          /**<If the TextBox is visible.*/
        bool free;             /**<If this TextBox is empty and can be overwritten.*/
         int font_id;          /**<The Font ID inside of ::FONTS.*/
    uint32_t uID;              /**<The ID inside of UIMenu.text_boxes.*/
     Justify justify;          /**<How the text is centered in relation to \f$(0,0)\f$ and/or its UILabel.*/
      char * text;             /**<The characters to render. @note Allocated, set to NULL.*/
         int text_cap;         /**<The amount of characters allocated.*/
       float text_size;        /**<The size to render the Font.*/
       float w;                /**<The width of the box after pseudo-rendering in determine_text_box_dimensions().*/
       float h;                /**<The height of the box after pseudo-rendering in determine_text_box_dimensions().*/
       float shadow_dist;      /**<How far the drop shadow is.*/
        vec2 shadow_dir;       /**<Which direction to offset the drop shadow in.*/

        /**
         * @name Matrix Transform Components
         * @brief These transform the TextBox on the GPU, after the CPU Justify%ing and placement around \f$(0,0)\f$.
         * @{
         */
        vec2 translation;      /**<Where to place the center of the TextBox on the GPU.*/
       float angle;            /**<The angle to transform this box to.*/
        vec2 scale;            /**<The scale for this box on the GPU.*/
        /** @} */

        vec2 center;           /**<The center in relation to \f$(0,0)\f$.*/
        vec2 start;            /**<The start of the text in relation to \f$(0,0)\f$.*/
        vec4 color;            /**<The color of the text.*/
        vec4 fill;             /**<The fill of the text. (unused)*/
    uint32_t layer;            /**<Rendering sort key (lower drawn first).*/
     UIState state;            /**<Only currently indicates if this TextBox is selected in the editor UI.*/
         int menu;             /**<The UIMenu that this box is within.*/
         int parent_container; /**<The UIContainer that contains this TextBox. @note Defaults to -1.*/
        char id[256];          /**<The name of this TextBox. @note Default is "default".*/
    
} TextBox;


/**
 * @brief Types of SDF shapes that are handled on the GPU.
 * @see ui/ui.frag
 * @see UIElement
 */
typedef enum UIElementType
{
    UIELEMENT_RECT, UIELEMENT_CIRCLE, UIELEMENT_TYPE_MAX
} UIElementType;


/**
 * @brief Types of values that may be controlled by a UI event function pointer.
 * @see UIElement
 * @see process_ui_event()
 */
typedef enum UIValueType
{

    UIVAL_NONE,
    UIVAL_BOOL,
    UIVAL_INT,
    UIVAL_FLOAT,
    UIVAL_CALLBACK
    
} UIValueType;

/**
 * @brief Types of ways that UIValue%s may be controlled when they reach a minimum or maximum value.
 * @see UIElement
 * @see process_ui_event()
 */
typedef enum UIValueBehavior
{

    UIVALBEHAVIOR_NONE,
    UIVALBEHAVIOR_WRAP,
    UIVALBEHAVIOR_CLAMP
    
} UIValueBehavior;


/**
 * @brief A value that is manipulated in some way when a UIElement receives an event.
 * @see UIElement
 * @see process_ui_event()
 */
typedef struct {
    
        UIValueType type;     /**<The type of value that is inside of data.*/
    UIValueBehavior behavior; /**<The control upon reaching min or max.*/
    /**
     * @brief The minimum value data can become.
     */
    union 
    {
        int min_int;
        float min_flt;
    } min_val; 

    /**
     * @brief The maximum value data can become.
     */
    union 
    {
        int max_int;
        float max_flt;
    } max_val; 
    /**
     * @brief How much data should be incremented.
     */
    union 
    {
        int inc_int;
        float inc_flt;
    } inc_val; 
    void * data; /**<The data, which may be simply a pointer that is passed into a function instead of manipulated directly.*/
    
} UIValue;

/**
 * @brief Describes function pointers for UIElement%s that determine behavior on events.
 * @see UIElement
 * @see UIValue
 * @see process_ui_event()
 */
typedef struct {

       bool bind_self;                 /**<If the UIElement should bind itself to this callback.*/
       bool one_shot;                  /**<If this event only occurs on initial event.*/
    UIValue val;                       /**<The value to change.*/
       void (*action_fn)(void *);      /**<The function called on action (if there is one).*/
       void (*unaction_fn)(void *);    /**<The function called when action is stopped (if there is one).*/
     void * (*bind_fn)(void*, void *); /**<The function that binds data to data on initialization.*/
     void * data;                      /**<The bind data.*/
       void (*free_data)(void*);       /**<The function that frees the bound data.*/
       char id[256];                   /**<The name of this UICallback inside of its registry.*/
    
} UICallback;

/**
 * @brief UIElement sliders that interpolate between two points on a line. AKA scrollbars. ::UI_INTERACTABLE_SLIDER
 * @see UIElement
 * @see UIContainer
 * @see UIInteractableType
 * @see process_ui_event()
 * @see update_ui_state()
 * @see compute_slider_bounds()
 */
 //#ffff0
typedef struct UISlider {

     vec2 dir;                /**<The direction it slides in.*/
     vec2 min;                /**<The min in pixel space.*/
     vec2 max;                /**<The max in pixel space.*/
    float len;                /**<The length in pixel space.*/

    /**
     * @note
     * \f$d_s\f$ = UISlider.drag_mouse_start<br>
     * \f$v\f$ = UISlider.dir<br>
     * \f$m_p\f$ = UISlider.max_proj<br>
     * \f$s\f$ = UISlider.drag_start_scalar<br>
     * \f$m\f$ = \f$\mathrm{Mouse\ Coords} * (\mathrm{UI\ Scale} / \mathrm{FBO\ Size})\f$<br>
     * \f$n_v\f$ = UISlider.val<br>
     * \f$n_v = clamp(((m - d_s) \cdot v) + s, 0, m_p) / m_p\f$
     */

    float val;                /**<The current value between min and max val.*/
    float prev_val;           /**<The previous value.*/
    float min_val;            /**<The minimum value of the slider.*/
    float max_val;            /**<The maximum value of the slider.*/
    float scroll_scale;       /**<How much the scroll is supposed to influence the UIContainer.*/
     bool dragging;           /**<If we are dragging this UISlider.*/
     vec2 drag_mouse_start;   /**<The mouse position on initial drag.*/
     vec2 drag_center_start;  /**<The translation of the UISlider on initial drag.*/

    /**
     * @note
     * \f$e\f$ = UIElement.translation<br>
     * \f$x_0\f$ = UISlider.min<br>
     * \f$v\f$ = UISlider.dir<br>
     * \f$m_p\f$ = UISlider.max_proj<br>
     * \f$s\f$ = UISlider.drag_start_scalar<br>
     * \f$s = clamp((e - x_0) \cdot v, 0, m_p)\f$
     */
    
    float drag_start_scalar;  /**<The scalar value the slider starts at based on initial drag position.*/

    /**
     * @note
     * \f$x_0\f$ = UISlider.min<br>
     * \f$x_1\f$ = UISlider.max<br>
     * \f$m_p\f$ = UISlider.max_proj<br>
     * \f$m_p = ||x_1 - x_0||\f$
     */
    float max_proj;           /**<The maximum projected scalar value.*/

      int container;          /**<The UIContainer that this UISlider scrolls. @note Defaults to -1.*/
    
} UISlider;

/**
 * @brief Unfinished structure for a dropdown that spawns UIElements and TextBox%es when opened.
 */
typedef struct UIDropdown {
    
    // int selected_index;
    // struct TextBox * selected_option;
    // char selected_option_id[128];
    // char options[MAX_DROPDOWN_ELEMENTS][MAX_CHARACTERS];
    // size_t option_count;
    // bool expanded;
    // struct UIElement * children[MAX_DROPDOWN_ELEMENTS];
    // size_t child_count;

    // TextAlign alignment;
    // float text_size;
    // int font_id;
    // RGBA color;
    // RGBA fill;
    // int render_group_id;
    // int material_id;
    

} UIDropdown;

/**
 * @brief UIElement UILabel%s that contain a TextBox reference. ::UI_INTERACTABLE_LABEL
 * @see UIElement
 * @see UIContainer
 * @see UIInteractableType
 * @see TextBox
 */
typedef struct UILabel {

        int box;             /**<The TextBox index inside of the UIMenu.*/
    Justify align;           /**<How the TextBox is justified within this UIElement.*/
        int parent_dropdown; /**<The dropdown parent. @note Defaults to -1.*/
    
} UILabel;

/**
 * @brief UIElement%s that are contained within a UIMenu that may receive events, be contained within UIContainer%s, or be UIContainer%s themselves.
 * @see UIContainer
 * @see UIMenu
 * @see UIInteractableType
 * @todo Clear unused fields.
 */
struct UIElement
{

                  bool shadow;           /**<Unused - If the UIElement has a shadow.*/ 
                  vec2 shadow_dir;       /**<Unused.*/
                 float shadow_dist;      /**<Unused.*/ 
                  bool visible;          /**<If the UIElement is visible.*/ 
                  bool free;             /**<If the UIElement can be replaced or is empty.*/ 
         UIElementType type;             /**<The type of SDF function to render.*/ 
              uint32_t uID;              /**<The ID inside of UIMenu.*/ 
                 float radius;           /**<The radius of the circle OR the radius of a rectangle's corners.*/ 
                /**
                 * @name Matrix Transform Components
                 * @brief These transform the UIElement on the GPU, after the CPU places at UIElement.center.
                 * @{
                 */
                  vec2 translation;      /**<X and Y coords where Y+ is down.*/ 
                  vec2 base_pos;         /**<Where the element was before it was scrolled or manipulated somehow. So that we have an anchor point.*/ 
                 float angle;            /**<Angle of rotation.*/ 
                  vec2 scale;            /**<Scale.*/ 
                /** @} */
                
                  vec2 center;           /**<The center. Should be at \f$(0,0)\f$.*/ 
                  vec2 size;             /**<Size of the SDF. @see ui/ui.frag*/ 
                  vec4 color;            /**<Color of the shape.*/ 
                  vec4 fill;             /**<Unused.*/ 

              uint32_t layer;            /**<The layer of the UIElement. Lower = bottom-most and drawn first. Higher elements consume events.*/ 
            UICallback action_callback;  /**<The event to occur on action (press or hold) or unaction.*/ 
            UICallback hover_callback;   /**<The event to occur on hover or unhover.*/ 
                  bool already_hovered;  /**<To avoid triggering hover events multiple times if not necessary.*/ 
               UIState state;            /**<The current state of the element.*/ 

                   int menu;             /**<The UIMenu this UIElement is inside. @warning Defaults to undefined value.*/ 
                   int container;        /**<The UIContainer this UIElement may control. @note Defaults to -1.*/ 
                   int parent_container; /**<The UIContainer that contains this UIElement. @note Defaults to -1.*/ 

                  char id[256];          /**<The name of this UIElement. @note Defaults to "default".*/ 

    UIInteractableType interactable;     /**<The special interactable type this UIElement may be.*/ 
    union
    {
        UIDropdown dropdown;
        UILabel label;
        UISlider slider;
    } as;
    
};


/**
 * @brief The top-most UI structure that contains UIElement%s, TextBox%es, and UIContainer%s.
 * @see UIContainer
 * @see UIElement
 * @see TextBox
 */
typedef struct UIMenu
{
    
       uint32_t uID;                           /**<The index inside of MENUS.*/
           char id[256];                       /**<The unique name of this menu.*/
      UIElement elements[MAX_UI_ELEMENTS];     /**<The UIElement%s.*/
        TextBox text_boxes[MAX_TEXT_BOXES];    /**<The TextBox%es.*/
    UIContainer containers[MAX_UI_CONTAINERS]; /**<The UIContainer%s.*/
    
} UIMenu;


/**
 * @name Default Structures
 * @brief Safe defaults for each UI structure.
 * @{
 */

static UICallback DEFAULT_CALLBACK =
{
    .id = "empty",
    .action_fn = NULL,
    .unaction_fn = NULL,
    .bind_fn = NULL,
    .bind_self = true,
    .data = NULL,
    .free_data = NULL,
    .val.data = NULL,
    .val.type = UIVAL_NONE,
    .one_shot = false
};

static UIElement DEFAULT_UI_ELEMENT=
{
    .free = 0,
    .color = (vec4){0,0,1,1},
    .fill = (vec4){0,0,0,0},
    .size = (vec2){0.5,0.5},
    .center = (vec2){0,0},
    .uID = 0,
    .angle = 0,
    .scale = (vec2){50,50},
    .shadow_dir = (vec2){0.5, 0.5},
    .shadow = true,
    .shadow_dist = 20,
    .translation = (vec2){20,100},
    .base_pos = (vec2){20,100},
    .type = UIELEMENT_RECT,
    .radius = 0.0f,
    .visible = true,
    .layer = 0,
    .already_hovered = false,
    .state = UISTATE_NONE,
    .container = -1,
    .parent_container = -1,
    .id = "default",
    .interactable = UI_INTERACTABLE_NONE
};

static UISlider DEFAULT_SLIDER =
{

    .container = -1,
    .len = 100,
    .dir = (vec2){0, 1},
    .scroll_scale = 1,
    .min_val = 0,
    .max_val = 100
    
};

static UILabel DEFAULT_LABEL =
{

    .box = -1,
    .parent_dropdown = -1,
    .align = JUSTIFY_CENTER
    
};


static UIContainer DEFAULT_UI_CONTAINER =
{
    .free = 0,
    .uID = -1,
    .scroll_offset = (vec2){0,0},
    .delta_scroll_offset = (vec2){0,0},
    .scroll_dir = (vec2){0,1},
    .wrap = false,
    .padding = (vec4){0,0,0,0},
    .mask  = true,
    .children = {0},
    .child_count = 0,
    .box_children = {0},
    .box_count = 0,
    .element = -1,
    .scrollbar = -1,
    .id = "default"
};

static TextBox DEFAULT_TEXT_BOX =
{
    .free = 0,
    .color = (vec4){1,1,1,1},
    .fill = (vec4){0,0,0,0},
    .font_id = 0,
    .w = 500,
    .h = 500,
    .start = (vec2){0,0},
    .center = (vec2){0,0},
    .uID = 0,
    .text_size = 0.6,
    .angle = 0,
    .scale = (vec2){1,1},
    .shadow_dir = (vec2){0.5, -0.5},
    .shadow = true,
    .shadow_dist = 20,
    .translation = (vec2){0,0},
    .text = NULL,
    .justify = JUSTIFY_CENTER,
    .visible = true,
    .parent_container = -1,
    .menu = -1,
    .id = "default",
    .text_cap = 0,
    .layer = 0
};

/** @} */

/**
 * @brief Moves a UIElement from its current UIMenu to another UIMenu.
 * @param element_idx The UIElement index into the current UIMenu.
 * @param current_menu The current UIMenu.
 * @param target_menu The target UIMenu.
 * @returns The resulting index into the new UIMenu.
 */
int move_element_to_menu(int element_idx, int current_menu, int target_menu);

/**
 * @brief Moves a TextBox from its current UIMenu to another UIMenu.
 * @param tb_idx The TextBox index inside of the current UIMenu.
 * @param current_menu The current UIMenu.
 * @param target_menu The target UIMenu.
 * @returns The resulting index into the new UIMenu.
 */
int move_text_box_to_menu(int tb_idx, int current_menu, int target_menu);

/**
 * @brief Finds a TextBox based on a screen position.
 * @details Transfers the screenspace pos to UI space.
 * @param pos The screenspace position.
 * @param menu_idx The out pointer to a UIMenu index.
 * @returns The TextBox index.
 * @warning returns -1 if none found.
 */
int get_text_box_from_coords(vec2 pos, int * menu_idx);

/**
 * @brief Finds a UIElement based on a screen position.
 * @details Transfers the screenspace pos to UI space.
 * @param pos The screenspace position.
 * @param menu_idx The out pointer to a UIMenu index.
 * @returns The UIElement index.
 * @warning returns -1 if none found.
 */
int get_ui_element_from_coords(vec2 pos, int * menu_idx);


/**
 * @brief Creates a UIElement on a given UIMenu.
 * @param menu_id The target UIMenu.
 * @returns The UIElement index.
 */
int create_ui_element(int menu_id);

/**
 * @brief Creates a UIContainer from a given UIElement a given UIMenu.
 * @param el The target UIElement.
 * @param menu_id The target UIMenu.
 * @todo menu_id should be the same as the UIElement%'s menu_id. Just use the UIElement%'s.
 * @returns The pointer to the UIContainer.
 */
UIContainer * create_ui_container(UIElement * el, int menu_id);

/**
 * @brief Deletes a UIElement, removing it from its possible parent UIContainer and deleting its own possible UIContainer.
 * @param menu The target UIMenu.
 * @param id The target UIElement.
 */
void delete_ui_element(int menu, int id);

/**
 * @brief Deletes a UIContainer, removing all references to it from existing TextBox%es and UIElement%s on the same UIMenu.
 * @param el The pointer to the UIElement that owns the UIContainer.
 * @todo Possible that a pointer is too much to ask, but think through the cases.
 */
void delete_ui_container(UIElement * el);

/**
 * @brief Handles an incoming UI event.
 * @param event The SDL event to process.
 * @param mouse_x Current mouse x.
 * @param mouse_y Current mouse y.
 */
void handle_ui_event(SDL_Event event, int mouse_x, int mouse_y);

/**
 * @brief Handles the persistent UI state from previous frames.
 * @note This exists to process events that don't exist, such as a button being held down or not moving but still hovering.
 * @param mouse_x Current mouse x.
 * @param mouse_y Current mouse y.
 */
void update_ui_state(float mouse_x, float mouse_y);

/**
 * @brief Computes the minimum and maximum in UI space for a UISlider.
 * @param element The UIElement that is the UISlider.
 */
void compute_slider_bounds(UIElement * element);

/**
 * @brief Creates a UISlider from a UIElement.
 * @param element The target UIElement.
 * @param dir The UISlider direction.
 * @param len The length of the UISlider.
 */
void create_slider(UIElement * element, vec2 dir, float len);

/**
 * @brief Removes a UIElement from a UIContainer. Shifts the rest of the children array forward.
 * @param c The UIContainer.
 * @param el The UIElement.
 */
void remove_element_from_container(UIContainer * c, UIElement * el);

/**
 * @brief Removes a TextBox from a UIContainer. Shifts the rest of the children array forward.
 * @param c The TextBox.
 * @param el The UIElement.
 */
void remove_text_box_from_container(UIContainer * c, TextBox * tb);

/**
 * @brief Finds a UIElement inside of a UIMenu via name.
 * @param m The UIMenu to search.
 * @param id The name string to compare.
 * @returns The index of the UIElement.
 */
int find_ui_element_in_menu(UIMenu * m, const char * id);

/**
 * @brief Finds a TextBox inside of a UIMenu via name.
 * @param m The UIMenu to search.
 * @param id The name string to compare.
 * @returns The index of the TextBox.
 */
int find_text_box_in_menu(UIMenu * m, const char * id);

/**
 * @brief Finds a UIContainer inside of a UIMenu via name.
 * @param m The UIMenu to search.
 * @param id The name string to compare.
 * @returns The index of the UIContainer.
 */
int find_container_in_menu(UIMenu * m, const char * id);


/**
 * @brief Aligns a TextBox inside of a UILabel based on its Justify.
 * @param el The UIElement that is the UILabel.
 * @todo This still is off-center sometimes. It likely has to do with the resolution scaling.
 */
void align_text_box_label(UIElement * el);


/**
 * @brief Assigns a UIElement to be parented to a UIContainer.
 * @param el The UIElement to assign.
 * @param c The target parent UIContainer.
 */
void assign_element_to_container(UIElement * el, UIContainer * c);

/**
 * @brief Assigns a TextBox to be parented to a UIContainer.
 * @param el The TextBox to assign.
 * @param c The target parent UIContainer.
 */
void assign_text_box_to_container(TextBox * tb, UIContainer * c);

/**
 * @brief Assigns all children based on current UIContainer children.
 * @note This is used during redo, where all of the children have been changed and we need to relink them.
 * @param el The UIElement that is the UIContainer.
 */
void reassign_elements_to_container(UIElement * el);




/**
 * @brief Simply frees the MENUS buffer and reinitializes the Vector.
 */
static inline void UI_INIT()
{

    if (MENUS.buffer) free(MENUS.buffer);
    init_vector(&MENUS, 0, sizeof(UIMenu));

}

/**
 * @brief Creates a new UIMenu, expanding the MENUS Vector and sets all arrays to free.
 */
static inline UIMenu * create_menu() {

    UIMenu * menu = vector_expand(&MENUS, 1);

    for (int i = 0; i < MAX_UI_ELEMENTS; i++)
    {
        menu->elements[i].free = true;
    }
    for (int i = 0; i < MAX_TEXT_BOXES; i++)
    {
        menu->text_boxes[i].free = true;
    }
    for (int i = 0; i < MAX_UI_CONTAINERS; i++)
    {
        menu->containers[i].free = true;
    }
    
    menu->uID = MENUS.count-1;
    menu->id[0] = '\0';

    return menu;
}

/**
 * @brief Checks if a UIContainer and UIMenu ID pairing is valid.
 * @note Used in the Editor to see if UIElement%s have valid UIContainer%s.
 * @param container The UIContainer ID.
 * @param menu The UIMenu ID.
 * @returns True if the container is valid.
 */
static inline bool container_is_valid(int container, int menu)
{
    if (menu < 0 || menu >= MENUS.count) return false;

    UIMenu * ms = CAST_VEC(MENUS, UIMenu);
    UIMenu * m = &ms[menu];
    
    if (container >= 0 && container < MAX_UI_CONTAINERS)
    {

        UIContainer * conts = m->containers;

        UIContainer * c = &conts[container];
        if (c->uID != -1 && !c->free)
        {
            return true;
        }
        
    }

    return false;
    
}

/**
 * @brief Transfers a position to local UI space by multiplying it by a ratio of the two resolutions
 * @param pos The position to transfer.
 * @returns The UI space position.
 */
static inline vec2 screenspace_to_ui(vec2 pos)
{
    return (vec2)
    {
        pos.x * ((float)UI_SCALE_WIDTH / TEXT_WIDTH),
        pos.y * ((float)UI_SCALE_HEIGHT / TEXT_HEIGHT)
    };
}

/** @} */


#endif
