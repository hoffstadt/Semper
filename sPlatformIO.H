#ifndef SEMPER_PLATFORMIO_H
#define SEMPER_PLATFORMIO_H

#ifndef S_PLATFORMIO_ALLOC
#define S_PLATFORMIO_ALLOC(x) malloc(x)
#endif

#ifndef S_PLATFORMIO_FREE
#define S_PLATFORMIO_FREE(x) free(x)
#endif

#ifndef S_PLATFORMIO_ASSERT
#include <assert.h>
#define S_PLATFORMIO_ASSERT(x) assert(x)
#endif

// includes
#include <string.h>
#include <cstdint>

#ifdef WIN32
// The following #defines disable a bunch of unused windows stuff. If you 
// get weird errors when trying to do some windows stuff, try removing some
// (or all) of these defines (it will increase build time though).
#ifndef FULL_WINDEFS
#define WIN32_LEAN_AND_MEAN
#define NORPC
#define NOPROXYSTUB
#define NOIMAGE
#define NOTAPE
#define NOGDICAPMASKS       // - CC_*, LC_*, PC_*, CP_*, TC_*, RC_
#define NOSYSMETRICS        // - SM_*
#define NOMENUS             // - MF_*
#define NOICONS             // - IDI_*
#define NOSYSCOMMANDS       // - SC_*
#define NORASTEROPS         // - Binary and Tertiary raster ops
#define OEMRESOURCE         // - OEM Resource values
#define NOATOM              // - Atom Manager routines
#define NOCLIPBOARD         // - Clipboard routines
#define NOCOLOR             // - Screen colors
#define NODRAWTEXT          // - DrawText() and DT_*
#define NOMEMMGR            // - GMEM_*, LMEM_*, GHND, LHND, associated routines
#define NOMETAFILE          // - typedef METAFILEPICT
#define NOMINMAX            // - Macros min(a,b) and max(a,b)
#define NOOPENFILE          // - OpenFile(), OemToAnsi, AnsiToOem, and OF_*
#define NOSCROLL            // - SB_* and scrolling routines
#define NOSERVICE           // - All Service Controller routines, SERVICE_ equates, etc.
#define NOSOUND             // - Sound driver routines
#define NOTEXTMETRIC        // - typedef TEXTMETRIC and associated routines
#define NOCOMM              // - COMM driver routines
#define NOKANJI             // - Kanji support stuff.
#define NOHELP              // - Help engine interface.
#define NOPROFILER          // - Profiler interface.
#define NODEFERWINDOWPOS    // - DeferWindowPos routines
#define NOMCX               // - Modem Configuration Extensions
#include <Windows.h>
#endif // FULL_WINDEFS
#elif defined(__APPLE__)
#else // linux
#include <xcb/xcb.h>
#include <X11/keysym.h>
#include <X11/XKBlib.h>     // sudo apt-get install libx11-dev
#include <X11/Xlib.h>
#include <X11/Xlib-xcb.h>  // sudo apt-get install libxkbcommon-x11-dev libx11-xcb-dev
#endif

#define S_PLATFORMIO_ARRAYSIZE(_ARR) ((int)(sizeof(_ARR) / sizeof(*(_ARR)))) // Size of a static C-style array. Don't use on pointers!

//-----------------------------------------------------------------------------
// [SECTION] Forward declarations and basic types
//-----------------------------------------------------------------------------

// forward declarations
struct sIOContext;
struct sIO;
struct sWindow;
struct sWindowMode;
struct sInputEvent;
struct sIOVec2;

// enums/flags
typedef int sInputEventType;   // -> enum sInputEventType_
typedef int sInputEventSource; // -> enum sInputEventSource_
typedef int sMouseButton;      // -> enum sMouseButton_
typedef int sKey;              // -> enum sKey_
typedef int sKeyModFlags;      // -> enum sKeyModFlags_


//-----------------------------------------------------------------------------
// [SECTION] Semper end-user API functions
//-----------------------------------------------------------------------------
namespace Semper
{
    // context creation & access
    void        create_io_context();
    void        destroy_io_context();
    sIOContext* get_io_context();

    // window operations
	sWindow* create_window        (const char* title, int cwidth, int cheight);
    sWindow* create_window        (const char* title, sWindowMode modes, int cwidth, int cheight);
	void     cleanup_window       (sWindow* window);
	void     process_window_events(sWindow& window);
	void     enable_cursor        (sWindow& window);
	void     disable_cursor       (sWindow& window);
    void     maximize_window      (sWindow& window);
    void     minimize_window      (sWindow& window);
    void     toggle_fullscreen    (sWindow& window);
    void     set_window_pos       (sWindow& window, int xpos, int ypos);
    void     set_window_title     (sWindow& window, const char* title);
    void     set_window_size      (sWindow& window, unsigned width, unsigned height);
    void     set_window_modes     (sWindow& window, sWindowMode modes);

    // main
    sIO& get_io();
    void new_frame();
    void end_frame();

    // input utilities: keyboard
    bool is_key_down           (sKey key);
    bool is_key_pressed        (sKey key, bool repeat = true);
    bool is_key_released       (sKey key);
    int  get_key_pressed_amount(sKey key, float repeat_delay, float rate);  // uses provided repeat rate/delay. return a count, most often 0 or 1 but might be >1 if RepeatRate is small enough that DeltaTime > RepeatRate

    // input utilities: mouse
    bool    is_mouse_down          (sMouseButton button);                      // is mouse button held?
    bool    is_mouse_clicked       (sMouseButton button, bool repeat = false); // did mouse button clicked? (went from !Down to Down). Same as GetMouseClickedCount() == 1.
    bool    is_mouse_released      (sMouseButton button);                      // did mouse button released? (went from Down to !Down)
    bool    is_mouse_double_clicked(sMouseButton button);                      // did mouse button double-clicked? Same as GetMouseClickedCount() == 2. (note that a double-click will also report IsMouseClicked() == true)
    int     get_mouse_clicked_count(sMouseButton button);                      // return the number of successive mouse-clicks at the time where a click happen (otherwise 0).
    bool    is_mouse_pos_valid     (const sIOVec2* mouse_pos = nullptr);      // by convention we use (-FLT_MAX,-FLT_MAX) to denote that there is no mouse available
    sIOVec2 get_mouse_pos();                                                   // shortcut to Semper::get_io().mousePos provided by user, to be consistent with other calls
    bool    is_mouse_dragging     (sMouseButton button, float lock_threshold = -1.0f); // is mouse dragging? (if lock_threshold < -1.0f, uses io.MouseDraggingThreshold)
    sIOVec2 get_mouse_drag_delta  (sMouseButton button = 0, float lock_threshold = -1.0f); // return the delta from the initial clicking position while the mouse button is pressed or was just released. This is locked and return 0.0f until the mouse moves past a distance threshold at least once (if lock_threshold < -1.0f, uses io.MouseDraggingThreshold)
    void    reset_mouse_drag_delta(sMouseButton button = 0);
}

//-----------------------------------------------------------------------------
// [SECTION] sIOVector
//-----------------------------------------------------------------------------
// Borrowed from Dear ImGui
// sIOVector<>
// Lightweight std::vector<>-like class to avoid dragging dependencies (also, some implementations of STL with debug enabled are absurdly slow, we bypass it so our code runs fast in debug).
//-----------------------------------------------------------------------------
// - You generally do NOT need to care or use this ever. But we need to make it available in imgui.h because some of our public structures are relying on it.
// - We use std-like naming convention here, which is a little unusual for this codebase.
// - Important: clear() frees memory, resize(0) keep the allocated buffer. We use resize(0) a lot to intentionally recycle allocated buffers across frames and amortize our costs.
// - Important: our implementation does NOT call C++ constructors/destructors, we treat everything as raw data! This is intentional but be extra mindful of that,
//   Do NOT use this class as a std::vector replacement in your own code! Many of the structures used by dear imgui can be safely initialized by a zero-memset.
//-----------------------------------------------------------------------------
template<typename T> void S_PLATFORMIO_DELETE(T* p)   { if (p) { p->~T(); S_PLATFORMIO_FREE(p); } }
template<typename T>
struct sIOVector
{
    int                 Size;
    int                 Capacity;
    T*                  Data;

    // Provide standard typedefs but we don't use them ourselves.
    typedef T                   value_type;
    typedef value_type*         iterator;
    typedef const value_type*   const_iterator;

    // Constructors, destructor
    inline sIOVector()                                       { Size = Capacity = 0; Data = NULL; }
    inline sIOVector(const sIOVector<T>& src)                { Size = Capacity = 0; Data = NULL; operator=(src); }
    inline sIOVector<T>& operator=(const sIOVector<T>& src)  { clear(); resize(src.Size); memcpy(Data, src.Data, (size_t)Size * sizeof(T)); return *this; }
    inline ~sIOVector()                                      { if (Data) S_PLATFORMIO_FREE(Data); } // Important: does not destruct anything

    inline void         clear()                             { if (Data) { Size = Capacity = 0; S_PLATFORMIO_FREE(Data); Data = NULL; } }  // Important: does not destruct anything
    inline void         clear_delete()                      { for (int n = 0; n < Size; n++) S_PLATFORMIO_DELETE(Data[n]); clear(); }     // Important: never called automatically! always explicit.
    inline void         clear_destruct()                    { for (int n = 0; n < Size; n++) Data[n].~T(); clear(); }           // Important: never called automatically! always explicit.

    inline bool         empty() const                       { return Size == 0; }
    inline int          size() const                        { return Size; }
    inline int          size_in_bytes() const               { return Size * (int)sizeof(T); }
    inline int          max_size() const                    { return 0x7FFFFFFF / (int)sizeof(T); }
    inline int          capacity() const                    { return Capacity; }
    inline T&           operator[](int i)                   { S_PLATFORMIO_ASSERT(i >= 0 && i < Size); return Data[i]; }
    inline const T&     operator[](int i) const             { S_PLATFORMIO_ASSERT(i >= 0 && i < Size); return Data[i]; }

    inline T*           begin()                             { return Data; }
    inline const T*     begin() const                       { return Data; }
    inline T*           end()                               { return Data + Size; }
    inline const T*     end() const                         { return Data + Size; }
    inline T&           front()                             { S_PLATFORMIO_ASSERT(Size > 0); return Data[0]; }
    inline const T&     front() const                       { S_PLATFORMIO_ASSERT(Size > 0); return Data[0]; }
    inline T&           back()                              { S_PLATFORMIO_ASSERT(Size > 0); return Data[Size - 1]; }
    inline const T&     back() const                        { S_PLATFORMIO_ASSERT(Size > 0); return Data[Size - 1]; }
    inline void         swap(sIOVector<T>& rhs)             { int rhs_size = rhs.Size; rhs.Size = Size; Size = rhs_size; int rhs_cap = rhs.Capacity; rhs.Capacity = Capacity; Capacity = rhs_cap; T* rhs_data = rhs.Data; rhs.Data = Data; Data = rhs_data; }

    inline int          _grow_capacity(int sz) const        { int new_capacity = Capacity ? (Capacity + Capacity / 2) : 8; return new_capacity > sz ? new_capacity : sz; }
    inline void         resize(int new_size)                { if (new_size > Capacity) reserve(_grow_capacity(new_size)); Size = new_size; }
    inline void         resize(int new_size, const T& v)    { if (new_size > Capacity) reserve(_grow_capacity(new_size)); if (new_size > Size) for (int n = Size; n < new_size; n++) memcpy(&Data[n], &v, sizeof(v)); Size = new_size; }
    inline void         shrink(int new_size)                { S_PLATFORMIO_ASSERT(new_size <= Size); Size = new_size; } // Resize a vector to a smaller size, guaranteed not to cause a reallocation
    inline void         reserve(int new_capacity)           { if (new_capacity <= Capacity) return; T* new_data = (T*)S_PLATFORMIO_ALLOC((size_t)new_capacity * sizeof(T)); if (Data) { memcpy(new_data, Data, (size_t)Size * sizeof(T)); S_PLATFORMIO_FREE(Data); } Data = new_data; Capacity = new_capacity; }

    // NB: It is illegal to call push_back/push_front/insert with a reference pointing inside the sIOVector data itself! e.g. v.push_back(v[10]) is forbidden.
    inline void         push_back(const T& v)               { if (Size == Capacity) reserve(_grow_capacity(Size + 1)); memcpy(&Data[Size], &v, sizeof(v)); Size++; }
    inline void         pop_back()                          { S_PLATFORMIO_ASSERT(Size > 0); Size--; }
    inline void         push_front(const T& v)              { if (Size == 0) push_back(v); else insert(Data, v); }
    inline T*           erase(const T* it)                  { S_PLATFORMIO_ASSERT(it >= Data && it < Data + Size); const ptrdiff_t off = it - Data; memmove(Data + off, Data + off + 1, ((size_t)Size - (size_t)off - 1) * sizeof(T)); Size--; return Data + off; }
    inline T*           erase(const T* it, const T* it_last){ S_PLATFORMIO_ASSERT(it >= Data && it < Data + Size && it_last >= it && it_last <= Data + Size); const ptrdiff_t count = it_last - it; const ptrdiff_t off = it - Data; memmove(Data + off, Data + off + count, ((size_t)Size - (size_t)off - (size_t)count) * sizeof(T)); Size -= (int)count; return Data + off; }
    inline T*           erase_unsorted(const T* it)         { S_PLATFORMIO_ASSERT(it >= Data && it < Data + Size);  const ptrdiff_t off = it - Data; if (it < Data + Size - 1) memcpy(Data + off, Data + Size - 1, sizeof(T)); Size--; return Data + off; }
    inline T*           insert(const T* it, const T& v)     { S_PLATFORMIO_ASSERT(it >= Data && it <= Data + Size); const ptrdiff_t off = it - Data; if (Size == Capacity) reserve(_grow_capacity(Size + 1)); if (off < (int)Size) memmove(Data + off + 1, Data + off, ((size_t)Size - (size_t)off) * sizeof(T)); memcpy(&Data[off], &v, sizeof(v)); Size++; return Data + off; }
    inline bool         contains(const T& v) const          { const T* data = Data;  const T* data_end = Data + Size; while (data < data_end) if (*data++ == v) return true; return false; }
    inline T*           find(const T& v)                    { T* data = Data;  const T* data_end = Data + Size; while (data < data_end) if (*data == v) break; else ++data; return data; }
    inline const T*     find(const T& v) const              { const T* data = Data;  const T* data_end = Data + Size; while (data < data_end) if (*data == v) break; else ++data; return data; }
    inline bool         find_erase(const T& v)              { const T* it = find(v); if (it < Data + Size) { erase(it); return true; } return false; }
    inline bool         find_erase_unsorted(const T& v)     { const T* it = find(v); if (it < Data + Size) { erase_unsorted(it); return true; } return false; }
    inline int          index_from_ptr(const T* it) const   { S_PLATFORMIO_ASSERT(it >= Data && it < Data + Size); const ptrdiff_t off = it - Data; return (int)off; }
};

//-----------------------------------------------------------------------------
// [SECTION] Enums
//-----------------------------------------------------------------------------

enum sInputSource_
{
    sInputSource_None = 0,
    sInputSource_Mouse,
    sInputSource_Keyboard,
    sInputSource_Gamepad,
    sInputSource_Nav,
    sInputSource_Clipboard,
    sInputSource_COUNT
};

enum sInputEventType_
{
    sInputEventType_None = 0,
    sInputEventType_MousePos,
    sInputEventType_MouseRelativePos,
    sInputEventType_MouseWheel,
    sInputEventType_MouseButton,
    sInputEventType_Key,
    sInputEventType_KeyMods,
    sInputEventType_Char,
    sInputEventType_Focus,
    sInputEventType_COUNT
};

enum sKeyModFlags_
{
    sKeyModFlags_None  = 0,
    sKeyModFlags_Ctrl  = 1 << 0,
    sKeyModFlags_Shift = 1 << 1,
    sKeyModFlags_Alt   = 1 << 2,
    sKeyModFlags_Super = 1 << 3 // Cmd/Super/Windows key
};

enum sMouseButton_
{
    sMouseButton_Left   = 0,
    sMouseButton_Right  = 1,
    sMouseButton_Middle = 2,
    sMouseButton_COUNT  = 5
};

enum sKey_
{
    sKey_None = 0,
    sKey_Tab,
    sKey_LeftArrow,
    sKey_RightArrow,
    sKey_UpArrow,
    sKey_DownArrow,
    sKey_PageUp,
    sKey_PageDown,
    sKey_Home,
    sKey_End,
    sKey_Insert,
    sKey_Delete,
    sKey_Backspace,
    sKey_Space,
    sKey_Enter,
    sKey_Escape,
    sKey_LeftCtrl, sKey_LeftShift, sKey_LeftAlt, sKey_LeftSuper,
    sKey_RightCtrl, sKey_RightShift, sKey_RightAlt, sKey_RightSuper,
    sKey_Menu,
    sKey_0, sKey_1, sKey_2, sKey_3, sKey_4, sKey_5, sKey_6, sKey_7, sKey_8, sKey_9,
    sKey_A, sKey_B, sKey_C, sKey_D, sKey_E, sKey_F, sKey_G, sKey_H, sKey_I, sKey_J,
    sKey_K, sKey_L, sKey_M, sKey_N, sKey_O, sKey_P, sKey_Q, sKey_R, sKey_S, sKey_T,
    sKey_U, sKey_V, sKey_W, sKey_X, sKey_Y, sKey_Z,
    sKey_F1, sKey_F2, sKey_F3, sKey_F4, sKey_F5, sKey_F6,
    sKey_F7, sKey_F8, sKey_F9, sKey_F10, sKey_F11, sKey_F12,
    sKey_Apostrophe,        // '
    sKey_Comma,             // ,
    sKey_Minus,             // -
    sKey_Period,            // .
    sKey_Slash,             // /
    sKey_Semicolon,         // ;
    sKey_Equal,             // =
    sKey_LeftBracket,       // [
    sKey_Backslash,         // \ (this text inhibit multiline comment caused by backslash)
    sKey_RightBracket,      // ]
    sKey_GraveAccent,       // `
    sKey_CapsLock,
    sKey_ScrollLock,
    sKey_NumLock,
    sKey_PrintScreen,
    sKey_Pause,
    sKey_Keypad0, sKey_Keypad1, sKey_Keypad2, sKey_Keypad3, sKey_Keypad4,
    sKey_Keypad5, sKey_Keypad6, sKey_Keypad7, sKey_Keypad8, sKey_Keypad9,
    sKey_KeypadDecimal,
    sKey_KeypadDivide,
    sKey_KeypadMultiply,
    sKey_KeypadSubtract,
    sKey_KeypadAdd,
    sKey_KeypadEnter,
    sKey_KeypadEqual,
    sKey_COUNT, // No valid sKey is ever greater than this value
};

//-----------------------------------------------------------------------------
// [SECTION] Structs
//-----------------------------------------------------------------------------

// FIXME: Structures in the union below need to be declared, as anonymous unions appears to be an extension?
//        'union member can't have a non-trivial default constructor'
struct sInputEventMousePos    { float posX, posY; };
struct sInputEventMouseWheel  { float wheelX, wheelY; };
struct sInputEventMouseButton { int button; bool down; };
struct sInputEventKey         { sKey key; bool down; };
struct sInputEventKeyMods     { sKeyModFlags mods; };
struct sInputEventText        { unsigned int c; };
struct sInputEventAppFocused  { bool focused; };

struct sInputEvent
{
    sInputEventType   type;
    sInputEventSource source;
    union
    {
        sInputEventMousePos     mousePos;       // if type == sInputEventType_MousePos, sInputEventType_MouseRelativePos
        sInputEventMouseWheel   mouseWheel;     // if type == sInputEventType_MouseWheel
        sInputEventMouseButton  mouseButton;    // if type == sInputEventType_MouseButton
        sInputEventKey          key;            // if type == sInputEventType_Key
        sInputEventKeyMods      keyMods;        // if type == sInputEventType_Modifiers
        sInputEventText         text;           // if type == sInputEventType_Text
        sInputEventAppFocused   appFocused;     // if type == sInputEventType_Focus
    };

    inline sInputEvent() { memset(this, 0, sizeof(*this)); }
};

struct sIOVec2
{
    float x, y;
    sIOVec2() = default;
    sIOVec2(float x, float y) : x(x), y(y) {}
};

struct sKeyData
{
    bool  down;             // True for if key is down
    float downDuration;     // Duration the key has been down (<0.0f: not pressed, 0.0f: just pressed, >0.0f: time held)
    float downDurationPrev; // Last frame duration the key has been down
};

//-----------------------------------------------------------------------------
// [SECTION] sWindow
//-----------------------------------------------------------------------------

struct sWindowMode
{
    bool vsync;
    bool resizable;
    bool alwaysOnTop;
    bool decorated;
    bool fullScreen;
};

struct sWindow
{
    const char* title;
    const char* smallIcon;
    const char* largeIcon;
    sWindowMode modes;
    int         width;
    int         height;
    int         client_width;
    int         client_height;
    unsigned    minwidth;
    unsigned    minheight;
    unsigned    maxwidth;
    unsigned    maxheight;
    int         xpos;
    int         ypos;
    bool        mouseTracked;
    bool        cursorEnabled;
    bool        rawMouseMotion;
    int         mouseButtonsDown;

    // flags
    bool running;
    bool titleDirty;
    bool modesDirty;
    bool sizeDirty;
    bool posDirty;
    bool sizeChanged;

    // platform specifics
#if defined(WIN32)
    HWND       _win32_handle;
    HWND       _win32_mouseHandle;
    WNDCLASSEX _win32_windowClass;
    DWORD      _win32_flags;
#elif defined(__APPLE__)
    NSWindow* handle;
    NSView* view;
    unsigned int sampleCount;
    MTL::Device* device;
#else // linux
    Display*          display;
    xcb_connection_t* connection;
    xcb_window_t      window;
    xcb_screen_t*     screen;
    xcb_atom_t        wm_protocols;
    xcb_atom_t        wm_delete_win;
#endif
};

//-----------------------------------------------------------------------------
// [SECTION] sIO
//-----------------------------------------------------------------------------

struct sIO
{

    //------------------------------------------------------------------
    // Configuration (fill once)                // Default value
    //------------------------------------------------------------------
    float       deltaTime;               // = 1.0f/60.0f     // Time elapsed since last frame, in seconds.
    float       mouseDoubleClickTime;    // = 0.30f          // Time for a double-click, in seconds.
    float       mouseDoubleClickMaxDist; // = 6.0f           // Distance threshold to stay in to validate a double-click, in pixels.
    float       mouseDragThreshold;      // = 6.0f           // Distance threshold before considering we are dragging.
    float       keyRepeatDelay;          // = 0.250f         // When holding a key/button, time before it starts repeating, in seconds (for buttons in Repeat mode, etc.).
    float       keyRepeatRate;           // = 0.050f         // When holding a key/button, rate at which it repeats, in seconds.
    bool        configInputTrickleEventQueue; // = true      // Enable input queue trickling: some types of events submitted during the same frame (e.g. button down + up) will be spread over multiple frames, improving interactions with low framerates.
    void*       userData;                // = NULL           // Store your own data for retrieval by callbacks.

    //------------------------------------------------------------------
    // Input - Call before calling NewFrame()
    //------------------------------------------------------------------

    // Input Functions
    void  pushKeyEvent(sKey key, bool down);           // queue a new key down/up event. Key should be "translated" (as in, generally ImGuiKey_A matches the key end-user would use to emit an 'A' character)
    void  pushKeyModsEvent(sKeyModFlags modifiers);    // queue a change of Ctrl/Shift/Alt/Super modifiers
    void  pushMousePosEvent(float x, float y);         // queue a mouse position update. Use -FLT_MAX,-FLT_MAX to signify no mouse (e.g. app not focused and not hovered)
    void  pushMouseRelativePosEvent(float x, float y); // raw Input
    void  pushMouseButtonEvent(int button, bool down); // queue a mouse button change
    void  pushMouseWheelEvent(float wh_x, float wh_y); // queue a mouse wheel update
    void  pushFocusEvent(bool focused);                // queue a gain/loss of focus for the application (generally based on OS/platform focus of your window)
    void  pushInputCharacter(unsigned int c);          // queue a new character input
    void  pushInputCharacterUTF16(unsigned short c);   // queue a new character input from an UTF-16 character, it can be a surrogate
    void  pushInputCharactersUTF8(const char* str);    // queue a new characters input from an UTF-8 string

    void  clearInputCharacters();                     // [Internal] clear the text input buffer manually
    void  clearInputKeys();                           // [Internal] release all keys

    //------------------------------------------------------------------
    // Output - Updated by NewFrame() or EndFrame()/Render()
    // (when reading from the io.WantCaptureMouse, io.WantCaptureKeyboard flags to dispatch your inputs, it is
    //  generally easier and more correct to use their state BEFORE calling NewFrame(). See FAQ for details!)
    //------------------------------------------------------------------
    float framerate;     // Rough estimate of application framerate, in frame per second. Solely for convenience. Rolling average estimation based on io.DeltaTime over 120 frames.
    sIOVec2 mouseDelta; // Mouse delta. Note that this is zero if either current or previous position are invalid (-FLT_MAX,-FLT_MAX), so a disappearing/reappearing mouse won't have a huge delta.

    //------------------------------------------------------------------
    // [Internal]
    //------------------------------------------------------------------

    // main Input State
    sIOVec2 mousePos;     // mouse position, in pixels. Set to ImVec2(-FLT_MAX, -FLT_MAX) if mouse is unavailable (on another screen, etc.)
    bool    mouseDown[5]; // mouse buttons: 0=left, 1=right, 2=middle + extras (sMouseButton_COUNT == 5). Dear ImGui mostly uses left and right buttons. Others buttons allows us to track if the mouse is being used by your application + available to user as a convenience via IsMouse** API.
    float   mouseWheel;   // mouse wheel Vertical: 1 unit scrolls about 5 lines text.
    float   mouseWheelH;  // mouse wheel Horizontal. Most users don't have a mouse with an horizontal wheel, may not be filled by all backends.
    bool    keyCtrl;      // keyboard modifier down: Control
    bool    keyShift;     // keyboard modifier down: Shift
    bool    keyAlt;       // keyboard modifier down: Alt
    bool    keySuper;     // keyboard modifier down: Cmd/Super/Windows
    
    // other state maintained from data above + IO function calls
    sKeyModFlags              keyMods;                            // key mods flags (same as io.KeyCtrl/KeyShift/KeyAlt/KeySuper but merged into flags), updated by NewFrame()
    sKeyModFlags              keyModsPrev;                        // key mods flags (from previous frame)
    sKeyData                  keysData[sKey_COUNT];               // key state for all known keys. Use IsKeyXXX() functions to access this.
    sIOVec2                   mousePosPrev;                       // Previous mouse position (note that MouseDelta is not necessary == MousePos-MousePosPrev, in case either position is invalid)
    sIOVec2                   mouseClickedPos[5];                 // Position at time of clicking
    double                    mouseClickedTime[5];                // Time of last click (used to figure out double-click)
    bool                      mouseClicked[5];                    // Mouse button went from !Down to Down (same as MouseClickedCount[x] != 0)
    bool                      mouseDoubleClicked[5];              // Has mouse button been double-clicked? (same as MouseClickedCount[x] == 2)
    unsigned short            mouseClickedCount[5];               // == 0 (not clicked), == 1 (same as MouseClicked[]), == 2 (double-clicked), == 3 (triple-clicked) etc. when going from !Down to Down
    unsigned short            mouseClickedLastCount[5];           // Count successive number of clicks. Stays valid after mouse release. Reset after another click is done.
    bool                      mouseReleased[5];                   // Mouse button went from Down to !Down
    bool                      mouseDownOwned[5];                  // Track if button was clicked inside a dear imgui window or over void blocked by a popup. We don't request mouse capture from the application if click started outside ImGui bounds.
    bool                      mouseDownOwnedUnlessPopupClose[5];  //Track if button was clicked inside a dear imgui window.
    float                     mouseDownDuration[5];               // Duration the mouse button has been down (0.0f == just clicked)
    float                     mouseDownDurationPrev[5];           // Previous time the mouse button has been down
    float                     mouseDragMaxDistanceSqr[5];         // Squared maximum distance of how much mouse has traveled from the clicking point (used for moving thresholds)
    bool                      appFocusLost;
    unsigned short            inputQueueSurrogate;                // for pushInputCharacterUTF16()
    sIOVector<unsigned short> inputQueueCharacters;               // queue of _characters_ input (obtained by platform backend). Fill using pushInputCharacter() helper.

    double                    restoreCursorPos[2];
    double                    lastCursorPos[2];
};

//-----------------------------------------------------------------------------
// [SECTION] sIOContext
//-----------------------------------------------------------------------------

struct sIOContext
{
    sIO                    IO;
    sIOVector<sInputEvent> inputEventsQueue; // Input events which will be trickled/written into IO structure.
    sIOVector<sInputEvent> inputEventsTrail; // Past input events processed in new_frame(). This is to allow domain-specific application to access e.g mouse/pen trail.
    int                    frameCount;
    double                 time;
    sIOVec2                mouseLastValidPos;
};

#endif // END OF HEADER

//-----------------------------------------------------------------------------
// [SECTION] Implementation
//-----------------------------------------------------------------------------
#ifdef SEMPER_PLATFORMIO_IMPLEMENTATION

#include <stdlib.h>
#include <string.h> // memcpy, strcmp

inline float _lengthSqrr(const sIOVec2& v) { return v.x*v.x + v.y*v.y;}
inline sIOVec2 operator-(sIOVec2 left, sIOVec2 right){ return sIOVec2(left.x - right.x, left.y - right.y);}

// Helper: Unicode defines
#define S_UNICODE_CODEPOINT_INVALID 0xFFFD     // Invalid Unicode code point (standard value).
#ifdef S_USE_WCHAR32
#define S_UNICODE_CODEPOINT_MAX     0x10FFFF   // Maximum Unicode code point supported by this build.
#else
#define S_UNICODE_CODEPOINT_MAX     0xFFFF     // Maximum Unicode code point supported by this build.
#endif

sIOContext* GSemperIOCtx = nullptr;

namespace Semper
{
	void      update_input_events(bool trickle_fast_inputs);
	void      update_keyboard_inputs();
	void      update_mouse_inputs();
	void      update_mouse_wheel();
	sKeyData* get_key_data(sKey key);
	int       calc_typematic_repeat_amount(float t0, float t1, float repeat_delay, float repeat_rate);
	bool      is_mouse_drag_past_threshold(sMouseButton button, float lock_threshold);

	// Helper: sBitArray
	inline bool sBitArrayTestBit(const uint32_t* arr, int n) { uint32_t mask = (uint32_t)1 << (n & 31); return (arr[n >> 5] & mask) != 0; }
	inline void sBitArrayClearBit(uint32_t* arr, int n) { uint32_t mask = (uint32_t)1 << (n & 31); arr[n >> 5] &= ~mask; }
	inline void sBitArraySetBit(uint32_t* arr, int n) { uint32_t mask = (uint32_t)1 << (n & 31); arr[n >> 5] |= mask; }
	inline void sBitArraySetBitRange(uint32_t* arr, int n, int n2) // Works on range [n..n2)
	{
		n2--;
		while (n <= n2)
		{
			int a_mod = (n & 31);
			int b_mod = (n2 > (n | 31) ? 31 : (n2 & 31)) + 1;
			uint32_t mask = (uint32_t)(((uint64_t)1 << b_mod) - 1) & ~(uint32_t)(((uint64_t)1 << a_mod) - 1);
			arr[n >> 5] |= mask;
			n = (n + 32) & ~31;
		}
	}

	// Helper: ImBitArray class (wrapper over ImBitArray functions)
	// Store 1-bit per value.
	template<int BITCOUNT>
	struct sBitArray
	{
		static const int size = BITCOUNT;
		uint32_t             storage[(BITCOUNT + 31) >> 5];
		sBitArray() { ClearAllBits(); }
		void            ClearAllBits() { memset(storage, 0, sizeof(storage)); }
		void            SetAllBits() { memset(storage, 255, sizeof(storage)); }
		bool            TestBit(int n) const { S_PLATFORMIO_ASSERT(n < BITCOUNT); return sBitArrayTestBit(storage, n); }
		void            SetBit(int n) { S_PLATFORMIO_ASSERT(n < BITCOUNT); sBitArraySetBit(storage, n); }
		void            ClearBit(int n) { S_PLATFORMIO_ASSERT(n < BITCOUNT); sBitArrayClearBit(storage, n); }
		void            SetBitRange(int n, int n2) { sBitArraySetBitRange(storage, n, n2); } // Works on range [n..n2)
	};
	template<int BITCOUNT>
	const int sBitArray<BITCOUNT>::size;

	// Convert UTF-8 to 32-bit character, process single character input.
	// A nearly-branchless UTF-8 decoder, based on work of Christopher Wellons (https://github.com/skeeto/branchless-utf8).
	// We handle UTF-8 decoding error by skipping forward.
	static int
	sTextCharFromUtf8(unsigned int* out_char, const char* in_text, const char* in_text_end)
	{
		static const char lengths[32] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 3, 3, 4, 0 };
		static const int masks[] = { 0x00, 0x7f, 0x1f, 0x0f, 0x07 };
		static const uint32_t mins[] = { 0x400000, 0, 0x80, 0x800, 0x10000 };
		static const int shiftc[] = { 0, 18, 12, 6, 0 };
		static const int shifte[] = { 0, 6, 4, 2, 0 };
		int len = lengths[*(const unsigned char*)in_text >> 3];
		int wanted = len + !len;

		if (in_text_end == NULL)
			in_text_end = in_text + wanted; // Max length, nulls will be taken into account.

		// Copy at most 'len' bytes, stop copying at 0 or past in_text_end. Branch predictor does a good job here,
		// so it is fast even with excessive branching.
		unsigned char s[4];
		s[0] = in_text + 0 < in_text_end ? in_text[0] : 0;
		s[1] = in_text + 1 < in_text_end ? in_text[1] : 0;
		s[2] = in_text + 2 < in_text_end ? in_text[2] : 0;
		s[3] = in_text + 3 < in_text_end ? in_text[3] : 0;

		// Assume a four-byte character and load four bytes. Unused bits are shifted out.
		*out_char = (uint32_t)(s[0] & masks[len]) << 18;
		*out_char |= (uint32_t)(s[1] & 0x3f) << 12;
		*out_char |= (uint32_t)(s[2] & 0x3f) << 6;
		*out_char |= (uint32_t)(s[3] & 0x3f) << 0;
		*out_char >>= shiftc[len];

		// Accumulate the various error conditions.
		int e = 0;
		e = (*out_char < mins[len]) << 6; // non-canonical encoding
		e |= ((*out_char >> 11) == 0x1b) << 7;  // surrogate half?
		e |= (*out_char > S_UNICODE_CODEPOINT_MAX) << 8;  // out of range?
		e |= (s[1] & 0xc0) >> 2;
		e |= (s[2] & 0xc0) >> 4;
		e |= (s[3]) >> 6;
		e ^= 0x2a; // top two bits of each tail byte correct?
		e >>= shifte[len];

		if (e)
		{
			// No bytes are consumed when *in_text == 0 || in_text == in_text_end.
			// One byte is consumed in case of invalid first byte of in_text.
			// All available bytes (at most `len` bytes) are consumed on incomplete/invalid second to last bytes.
			// Invalid or incomplete input may consume less bytes than wanted, therefore every byte has to be inspected in s.
			wanted = wanted < !!s[0] + !!s[1] + !!s[2] + !!s[3] ? wanted : !!s[0] + !!s[1] + !!s[2] + !!s[3];
			*out_char = S_UNICODE_CODEPOINT_INVALID;
		}

		return wanted;
	}
}

// platform specific forward declarations
#ifdef WIN32

// The following #defines disable a bunch of unused windows stuff. If you 
// get weird errors when trying to do some windows stuff, try removing some
// (or all) of these defines (it will increase build time though).
#ifndef FULL_WINDEFS
#define WIN32_LEAN_AND_MEAN
#define NORPC
#define NOPROXYSTUB
#define NOIMAGE
#define NOTAPE
#define NOGDICAPMASKS       // - CC_*, LC_*, PC_*, CP_*, TC_*, RC_
//#define NOVIRTUALKEYCODES // - VK_*
#define NOSYSMETRICS        // - SM_*
#define NOMENUS             // - MF_*
#define NOICONS             // - IDI_*
//#define NOKEYSTATES       // - MK_*
#define NOSYSCOMMANDS       // - SC_*
#define NORASTEROPS         // - Binary and Tertiary raster ops
#define OEMRESOURCE         // - OEM Resource values
#define NOATOM              // - Atom Manager routines
#define NOCLIPBOARD         // - Clipboard routines
#define NOCOLOR             // - Screen colors
#define NODRAWTEXT          // - DrawText() and DT_*
//#define NOGDI             // - All GDI defines and routines
//#define NOKERNEL          // - All KERNEL defines and routines
//#define NOUSER            // - All USER defines and routines
//#define NONLS             // - All NLS defines and routines
//#define NOMB              // - MB_* and MessageBox()
#define NOMEMMGR            // - GMEM_*, LMEM_*, GHND, LHND, associated routines
#define NOMETAFILE          // - typedef METAFILEPICT
#define NOMINMAX            // - Macros min(a,b) and max(a,b)
//#define NOMSG             // - typedef MSG and associated routines
#define NOOPENFILE          // - OpenFile(), OemToAnsi, AnsiToOem, and OF_*
#define NOSCROLL            // - SB_* and scrolling routines
#define NOSERVICE           // - All Service Controller routines, SERVICE_ equates, etc.
#define NOSOUND             // - Sound driver routines
#define NOTEXTMETRIC        // - typedef TEXTMETRIC and associated routines
//#define NOWH              // - SetWindowsHook and WH_*
//#define NOWINOFFSETS      // - GWL_*, GCL_*, associated routines
#define NOCOMM              // - COMM driver routines
#define NOKANJI             // - Kanji support stuff.
#define NOHELP              // - Help engine interface.
#define NOPROFILER          // - Profiler interface.
#define NODEFERWINDOWPOS    // - DeferWindowPos routines
#define NOMCX               // - Modem Configuration Extensions
//#define NOSHOWWINDOW      // - SW_*
//#define NOCTLMGR          // - Control and Dialog routines
//#define NOWINMESSAGES     // - WM_*, EM_*, LB_*, CB_*
//#define NOWINSTYLES       // - WS_*, CS_*, ES_*, LBS_*, SBS_*, CBS_*
#endif

#include <Windows.h>

#include <windowsx.h> // GET_X_LPARAM(), GET_Y_LPARAM()
#include <objbase.h>  // For COM headers
#include <tchar.h>
#include <dwmapi.h>

#define IM_VK_KEYPAD_ENTER (VK_RETURN + 256)

inline bool is_vk_down(int vk) {return (::GetKeyState(vk) & 0x8000) != 0;}
inline bool is_window_focused(sWindow& window){ return window._win32_handle == ::GetActiveWindow();}
static sKey VirtualKeyToSemperKey(WPARAM wParam);

static void _enable_mouse_raw_motion(sWindow& window)
{
	window.rawMouseMotion = true;

	//const RAWINPUTDEVICE rid = { 0x01, 0x02, RIDEV_NOLEGACY, window.platform.handle };
	const RAWINPUTDEVICE rid = { 0x01, 0x02, RIDEV_NOLEGACY, NULL };

	if (!::RegisterRawInputDevices(&rid, 1, sizeof(rid)))
	{
		S_PLATFORMIO_ASSERT(false && "Win32: Failed to register raw input device");
	}
}

static void _disable_mouse_raw_motion(sWindow& window)
{
	window.rawMouseMotion = false;

	const RAWINPUTDEVICE rid = { 0x01, 0x02, RIDEV_REMOVE, NULL };
	if (!::RegisterRawInputDevices(&rid, 1, sizeof(rid)))
	{
		S_PLATFORMIO_ASSERT(false && "Win32: Failed to register raw input device");
	}
}

static int _get_horizontal_shift(const HWND window_handle)
{
	RECT window_rectangle, frame_rectangle;
	::GetWindowRect(window_handle, &window_rectangle);
	::DwmGetWindowAttribute(window_handle,
		DWMWA_EXTENDED_FRAME_BOUNDS, &frame_rectangle, sizeof(RECT));

	return frame_rectangle.left - window_rectangle.left;
}

#if defined SEMPER_PLATFORMIO_INCLUDE_IMGUI
#include "imgui_impl_win32.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif
static LRESULT CALLBACK HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#elif defined(__APPLE__)
#else
static sKey
VirtualKeyToSemperKey(unsigned x_keycode)
{
	switch (x_keycode)
	{
        case XK_d:
        case XK_D: return sKey_D;
        case XK_e:
        case XK_E: return sKey_E;
    }

    return sKey_None;
}
#endif

void
Semper::create_io_context()
{
    if (GSemperIOCtx)
    {
        S_PLATFORMIO_ASSERT(false && "Context already created.");
        return;
    }

    GSemperIOCtx = (sIOContext*)S_PLATFORMIO_ALLOC(sizeof(sIOContext));
	GSemperIOCtx->frameCount = 0;
	GSemperIOCtx->IO.deltaTime = 1.0f / 60.0f;
	GSemperIOCtx->IO.mouseDoubleClickTime = 0.30f;
	GSemperIOCtx->IO.mouseDoubleClickMaxDist = 6.0f;
	GSemperIOCtx->IO.mouseDragThreshold = 6.0f;
	GSemperIOCtx->IO.keyRepeatDelay = 0.250f;
	GSemperIOCtx->IO.keyRepeatRate = 0.050f;
	GSemperIOCtx->IO.configInputTrickleEventQueue = true;
	GSemperIOCtx->IO.userData = nullptr;
	GSemperIOCtx->inputEventsQueue.Data = nullptr;
	GSemperIOCtx->inputEventsTrail.Data = nullptr;
	GSemperIOCtx->IO.inputQueueCharacters.Data = nullptr;

	GSemperIOCtx->inputEventsQueue = sIOVector<sInputEvent>();
	GSemperIOCtx->inputEventsTrail = sIOVector<sInputEvent>();
	GSemperIOCtx->IO.inputQueueCharacters = sIOVector<unsigned short>();

	for (int i = 0; i < sKey_COUNT; i++)
	{
		GSemperIOCtx->IO.keysData[i].down = false;
		GSemperIOCtx->IO.keysData[i].downDuration = -1.0f;
		GSemperIOCtx->IO.keysData[i].downDurationPrev = -1.0f;
	}
}

void
Semper::destroy_io_context()
{
    if(GSemperIOCtx)
    {
        S_PLATFORMIO_FREE(GSemperIOCtx);
        GSemperIOCtx = nullptr;
        return;
    }
    S_PLATFORMIO_ASSERT(false && "Context already destroyed.");
}

sIOContext*
Semper::get_io_context()
{
    return GSemperIOCtx;
}

sIO&
Semper::get_io()
{
	S_PLATFORMIO_ASSERT(GSemperIOCtx != nullptr && "No current context. Did you call Semper::create_semper_context() and ImGui::SetCurrentContext() ?");
	return GSemperIOCtx->IO;
}

void
Semper::new_frame()
{
	sIOContext* ctx = get_io_context();
	sIO& io = get_io();

	#if defined(WIN32)
    // Left & right Shift keys: when both are pressed together, Windows tend to not generate the WM_KEYUP event for the first released one.
    if (is_key_down(sKey_LeftShift) && !is_vk_down(VK_LSHIFT))
        io.pushKeyEvent(sKey_LeftShift, false);
    if (is_key_down(sKey_RightShift) && !is_vk_down(VK_RSHIFT))
        io.pushKeyEvent(sKey_RightShift, false);

    // Sometimes WM_KEYUP for Win key is not passed down to the app (e.g. for Win+V on some setups, according to GLFW).
    if (is_key_down(sKey_LeftSuper) && !is_vk_down(VK_LWIN))
        io.pushKeyEvent(sKey_LeftSuper, false);
    if (is_key_down(sKey_RightSuper) && !is_vk_down(VK_RWIN))
        io.pushKeyEvent(sKey_RightSuper, false);
	#endif

	ctx->time += ctx->IO.deltaTime;
	ctx->frameCount += 1;

	// Process input queue (trickle as many events as possible)
	ctx->inputEventsTrail.resize(0);
	update_input_events(ctx->IO.configInputTrickleEventQueue);

	// Update keyboard input state
	update_keyboard_inputs();

	// Update mouse input state
	update_mouse_inputs();

	// Mouse wheel scrolling, scale
	update_mouse_wheel();
}

void
Semper::end_frame()
{
	sIOContext* ctx = get_io_context();
	sIO& io = get_io();

	// Clear Input data for next frame
	io.mouseWheel = io.mouseWheelH = 0.0f;
	io.inputQueueCharacters.resize(0);
	io.keyModsPrev = io.keyMods; // doing it here is better than in new_frame() as we'll tolerate backend writing to keyMods. If we want to firmly disallow it we should detect it.
}

sKeyData*
Semper::get_key_data(sKey key)
{
	sIOContext* ctx = get_io_context();
	int index;
	index = key-1;
	return &ctx->IO.keysData[index];
}

bool
Semper::is_key_down(sKey key)
{
	const sKeyData* key_data = get_key_data(key);
	return key_data->down;
}

bool
Semper::is_key_pressed(sKey key, bool repeat)
{
	sIOContext& ctx = *GSemperIOCtx;
	const sKeyData* key_data = get_key_data(key);
	const float t = key_data->downDuration;
	if (t == 0.0f)
		return true;
	if (repeat && t > ctx.IO.keyRepeatDelay)
		return get_key_pressed_amount(key, ctx.IO.keyRepeatDelay, ctx.IO.keyRepeatRate) > 0;
	return false;
}

int
Semper::get_key_pressed_amount(sKey key, float repeat_delay, float repeat_rate)
{
	sIOContext& ctx = *GSemperIOCtx;
	const sKeyData* key_data = get_key_data(key);
	const float t = key_data->downDuration;
	return calc_typematic_repeat_amount(t - ctx.IO.deltaTime, t, repeat_delay, repeat_rate);
}

bool
Semper::is_key_released(sKey key)
{
	const sKeyData* key_data = get_key_data(key);
	return key_data->downDurationPrev >= 0.0f && !key_data->down;
}

bool  
Semper::is_mouse_down(sMouseButton button)
{
	sIOContext& ctx = *GSemperIOCtx;
	S_PLATFORMIO_ASSERT(button >= 0 && button < S_PLATFORMIO_ARRAYSIZE(ctx.IO.mouseDown));
	return ctx.IO.mouseDown[button];
}

bool  
Semper::is_mouse_clicked(sMouseButton button, bool repeat)
{
	sIOContext& ctx = *GSemperIOCtx;
	S_PLATFORMIO_ASSERT(button >= 0 && button < S_PLATFORMIO_ARRAYSIZE(ctx.IO.mouseDown));
	const float t = ctx.IO.mouseDownDuration[button];
	if (t == 0.0f)
		return true;

	if (repeat && t > ctx.IO.keyRepeatDelay)
	{
		int amount = calc_typematic_repeat_amount(t - ctx.IO.deltaTime, t, ctx.IO.keyRepeatDelay, ctx.IO.keyRepeatRate * 0.50f);
		if (amount > 0)
			return true;
	}
	return false;
}

bool  
Semper::is_mouse_released(sMouseButton button)
{
	sIOContext& ctx = *GSemperIOCtx;
	S_PLATFORMIO_ASSERT(button >= 0 && button < S_PLATFORMIO_ARRAYSIZE(ctx.IO.mouseDown));
	return ctx.IO.mouseReleased[button];
}

bool  
Semper::is_mouse_double_clicked(sMouseButton button)
{
	sIOContext& ctx = *GSemperIOCtx;
	S_PLATFORMIO_ASSERT(button >= 0 && button < S_PLATFORMIO_ARRAYSIZE(ctx.IO.mouseDown));
	return ctx.IO.mouseClickedCount[button] == 2;
}

int   
Semper::get_mouse_clicked_count(sMouseButton button)
{
	sIOContext& ctx = *GSemperIOCtx;
	S_PLATFORMIO_ASSERT(button >= 0 && button < S_PLATFORMIO_ARRAYSIZE(ctx.IO.mouseDown));
	return ctx.IO.mouseClickedCount[button];
}

bool  
Semper::is_mouse_pos_valid(const sIOVec2* mouse_pos)
{
	// The assert is only to silence a false-positive in XCode Static Analysis.
	// Because GImGui is not dereferenced in every code path, the static analyzer assume that it may be NULL (which it doesn't for other functions).
	S_PLATFORMIO_ASSERT(GSemperIOCtx != nullptr);
	const float MOUSE_INVALID = -256000.0f;
	sIOVec2 p = mouse_pos ? *mouse_pos : GSemperIOCtx->IO.mousePos;
	return p.x >= MOUSE_INVALID && p.y >= MOUSE_INVALID;
}

sIOVec2
Semper::get_mouse_pos()
{
	sIOContext& ctx = *GSemperIOCtx;
	return ctx.IO.mousePos;
}

// Return if a mouse click/drag went past the given threshold. Valid to call during the MouseReleased frame.
// [Internal] This doesn't test if the button is pressed
bool 
Semper::is_mouse_drag_past_threshold(sMouseButton button, float lock_threshold)
{
	sIOContext& ctx = *GSemperIOCtx;
	S_PLATFORMIO_ASSERT(button >= 0 && button < S_PLATFORMIO_ARRAYSIZE(ctx.IO.mouseDown));
	if (lock_threshold < 0.0f)
		lock_threshold = ctx.IO.mouseDragThreshold;
	return ctx.IO.mouseDragMaxDistanceSqr[button] >= lock_threshold * lock_threshold;
}

bool  
Semper::is_mouse_dragging(sMouseButton button, float lock_threshold)
{
	sIOContext& ctx = *GSemperIOCtx;
	S_PLATFORMIO_ASSERT(button >= 0 && button < S_PLATFORMIO_ARRAYSIZE(ctx.IO.mouseDown));
	if (!ctx.IO.mouseDown[button])
		return false;
	return is_mouse_drag_past_threshold(button, lock_threshold);
}

// Return the delta from the initial clicking position while the mouse button is clicked or was just released.
// This is locked and return 0.0f until the mouse moves past a distance threshold at least once.
// NB: This is only valid if is_mouse_pos_valid(). backends in theory should always keep mouse position valid when dragging even outside the client window.
sIOVec2
Semper::get_mouse_drag_delta(sMouseButton button, float lock_threshold)
{
	sIOContext& ctx = *GSemperIOCtx;
	S_PLATFORMIO_ASSERT(button >= 0 && button < S_PLATFORMIO_ARRAYSIZE(ctx.IO.mouseDown));
	if (lock_threshold < 0.0f)
		lock_threshold = ctx.IO.mouseDragThreshold;
	if (ctx.IO.mouseDown[button] || ctx.IO.mouseReleased[button])
		if (ctx.IO.mouseDragMaxDistanceSqr[button] >= lock_threshold * lock_threshold)
			if (is_mouse_pos_valid(&ctx.IO.mousePos) && is_mouse_pos_valid(&ctx.IO.mouseClickedPos[button]))
			{
				sIOVec2 result(ctx.IO.mousePos.x - ctx.IO.mouseClickedPos[button].x, ctx.IO.mousePos.y - ctx.IO.mouseClickedPos[button].y);
				return result;
			}
	return sIOVec2(0.0f, 0.0f);
}

void  
Semper::reset_mouse_drag_delta(sMouseButton button)
{
	sIOContext& ctx = *GSemperIOCtx;
	S_PLATFORMIO_ASSERT(button >= 0 && button < S_PLATFORMIO_ARRAYSIZE(ctx.IO.mouseDown));
	// NB: We don't need to reset g.IO.MouseDragMaxDistanceSqr
	ctx.IO.mouseClickedPos[button] = ctx.IO.mousePos;
}

void
sIO::pushKeyEvent(sKey key, bool down)
{
	if (key == sKey_None)
		return;

	sIOContext* ctx = Semper::get_io_context();

	sInputEvent e;
	e.type = sInputEventType_Key;
	e.source = sInputSource_Keyboard;
	e.key.key = key;
	e.key.down = down;
	ctx->inputEventsQueue.push_back(e);
}

void
sIO::pushKeyModsEvent(sKeyModFlags modifiers)
{
	sIOContext* ctx = Semper::get_io_context();

	sInputEvent e;
	e.type = sInputEventType_KeyMods;
	e.source = sInputSource_Keyboard;
	e.keyMods.mods = modifiers;
	ctx->inputEventsQueue.push_back(e);
}

void
sIO::pushMousePosEvent(float x, float y)
{
	sIOContext* ctx = Semper::get_io_context();

	sInputEvent e;
	e.type = sInputEventType_MousePos;
	e.source = sInputSource_Mouse;
	e.mousePos.posX = x;
	e.mousePos.posY = y;
	ctx->inputEventsQueue.push_back(e);
}

void
sIO::pushMouseRelativePosEvent(float x, float y)
{
	sIOContext* ctx = Semper::get_io_context();

	sInputEvent e;
	e.type = sInputEventType_MouseRelativePos;
	e.source = sInputSource_Mouse;
	e.mousePos.posX = x;
	e.mousePos.posY = y;
	ctx->inputEventsQueue.push_back(e);
}

void
sIO::pushMouseButtonEvent(int mouse_button, bool down)
{
	sIOContext* ctx = Semper::get_io_context();

	assert(mouse_button >= 0 && mouse_button < sMouseButton_COUNT);

	sInputEvent e;
	e.type = sInputEventType_MouseButton;
	e.source = sInputSource_Mouse;
	e.mouseButton.button = mouse_button;
	e.mouseButton.down = down;
	ctx->inputEventsQueue.push_back(e);
}

void
sIO::pushMouseWheelEvent(float wheel_x, float wheel_y)
{
	sIOContext* ctx = Semper::get_io_context();

	if (wheel_x == 0.0f && wheel_y == 0.0f)
		return;

	sInputEvent e;
	e.type = sInputEventType_MouseWheel;
	e.source = sInputSource_Mouse;
	e.mouseWheel.wheelX = wheel_x;
	e.mouseWheel.wheelY = wheel_y;
	ctx->inputEventsQueue.push_back(e);
}

void
sIO::pushFocusEvent(bool focused)            
{
	sIOContext* ctx = Semper::get_io_context();

	sInputEvent e;
	e.type = sInputEventType_Focus;
	e.appFocused.focused = focused;
	ctx->inputEventsQueue.push_back(e);
}

// Pass in translated ASCII characters for text input.
// - with glfw you can get those from the callback set in glfwSetCharCallback()
// - on Windows you can get those using ToAscii+keyboard state, or via the WM_CHAR message
// FIXME: Should in theory be called "AddCharacterEvent()" to be consistent with new API
void
sIO::pushInputCharacter(unsigned int c)        
{
	sIOContext* ctx = Semper::get_io_context();

	if (c == 0)
		return;

	sInputEvent e;
	e.type = sInputEventType_Char;
	e.source = sInputSource_Keyboard;
	e.text.c = c;
	ctx->inputEventsQueue.push_back(e);
}

// UTF16 strings use surrogate pairs to encode codepoints >= 0x10000, so
// we should save the high surrogate.
void 
sIO::pushInputCharacterUTF16(unsigned short c)
{
	sIOContext* ctx = Semper::get_io_context();

	if (c == 0 && inputQueueSurrogate == 0)
		return;

	if ((c & 0xFC00) == 0xD800) // High surrogate, must save
	{
		if (inputQueueSurrogate != 0)
			pushInputCharacter(S_UNICODE_CODEPOINT_INVALID);
		inputQueueSurrogate = c;
		return;
	}

	unsigned char cp = c;
	if (inputQueueSurrogate != 0)
	{
		if ((c & 0xFC00) != 0xDC00) // Invalid low surrogate
		{
			pushInputCharacter(S_UNICODE_CODEPOINT_INVALID);
		}
		else
		{
			#if S_UNICODE_CODEPOINT_MAX == 0xFFFF
			cp = S_UNICODE_CODEPOINT_INVALID; // Codepoint will not fit in ImWchar
			#else
			cp = (unsigned char)(((inputQueueSurrogate - 0xD800) << 10) + (c - 0xDC00) + 0x10000);
			#endif
		}

		inputQueueSurrogate = 0;
	}
	pushInputCharacter((unsigned)cp);
}

void
sIO::pushInputCharactersUTF8(const char* utf8_chars)
{
	sIOContext* ctx = Semper::get_io_context();

	while (*utf8_chars != 0)
	{
		unsigned int c = 0;
		utf8_chars += Semper::sTextCharFromUtf8(&c, utf8_chars, NULL);
		if (c != 0)
			pushInputCharacter(c);
	}

}

void 
sIO::clearInputCharacters()
{
	inputQueueCharacters.resize(0);
}

void 
sIO::clearInputKeys()
{
	for (int n = 0; n < sKey_COUNT; n++)
	{
		keysData[n].down = false;
		keysData[n].downDuration = -1.0f;
		keysData[n].downDurationPrev = -1.0f;
	}
	keyCtrl = keyShift = keyAlt = keySuper = false;
	keyMods = keyModsPrev = sKeyModFlags_None;
}

// t0 = previous time (e.g.: g.Time - g.IO.DeltaTime)
// t1 = current time (e.g.: g.Time)
// An event is triggered at:
//  t = 0.0f     t = repeat_delay,    t = repeat_delay + repeat_rate*N
int
Semper::calc_typematic_repeat_amount(float t0, float t1, float repeat_delay, float repeat_rate)
{
	if (t1 == 0.0f)
		return 1;
	if (t0 >= t1)
		return 0;
	if (repeat_rate <= 0.0f)
		return (t0 < repeat_delay) && (t1 >= repeat_delay);
	const int count_t0 = (t0 < repeat_delay) ? -1 : (int)((t0 - repeat_delay) / repeat_rate);
	const int count_t1 = (t1 < repeat_delay) ? -1 : (int)((t1 - repeat_delay) / repeat_rate);
	const int count = count_t1 - count_t0;
	return count;
}

void
Semper::update_input_events(bool trickleFastInputs)
{
	sIOContext& ctx = *GSemperIOCtx;
	sIO& io = ctx.IO;

	bool mouse_moved = false;
	bool mouse_wheeled = false;
	bool key_changed = false;
	bool text_inputed = false;
	int  mouse_button_changed = 0x00;
	int key_mods_changed = 0x00;

	sBitArray<sKey_COUNT> key_changed_mask;

	int event_n = 0;
	for (; event_n < ctx.inputEventsQueue.size(); event_n++)
	{
		const sInputEvent* e = &ctx.inputEventsQueue[event_n];
		if (e->type == sInputEventType_MousePos)
		{
			sIOVec2 event_pos(e->mousePos.posX, e->mousePos.posY);

			if (is_mouse_pos_valid(&event_pos))
				event_pos = sIOVec2(floorf(event_pos.x), floorf(event_pos.y)); // Apply same flooring as UpdateMouseInputs()
			if (io.mousePos.x != event_pos.x || io.mousePos.y != event_pos.y)
			{
				// Trickling Rule: Stop processing queued events if we already handled a mouse button change
				if (trickleFastInputs && (mouse_button_changed != 0 || mouse_wheeled || key_changed || key_mods_changed || text_inputed))
					break;
				io.mousePos = event_pos;
				mouse_moved = true;
			}
		}
		else if (e->type == sInputEventType_MouseRelativePos)
		{
			sIOVec2 event_pos(e->mousePos.posX, e->mousePos.posY);
			event_pos = sIOVec2(floorf(event_pos.x), floorf(event_pos.y)); // Apply same flooring as UpdateMouseInputs()

			if (io.mousePos.x != event_pos.x || io.mousePos.y != event_pos.y)
			{
				// Trickling Rule: Stop processing queued events if we already handled a mouse button change
				if (trickleFastInputs && (mouse_button_changed != 0 || mouse_wheeled || key_changed || key_mods_changed || text_inputed))
					break;
				io.mousePos.x+= event_pos.x;
				io.mousePos.y+= event_pos.y;
				mouse_moved = true;
			}
		}
		else if (e->type == sInputEventType_MouseButton)
		{
			const sMouseButton button = e->mouseButton.button;
			S_PLATFORMIO_ASSERT(button >= 0 && button < sMouseButton_COUNT);
			if (io.mouseDown[button] != e->mouseButton.down)
			{
				// Trickling Rule: Stop processing queued events if we got multiple action on the same button
				if (trickleFastInputs && ((mouse_button_changed & (1 << button)) || mouse_wheeled))
					break;
				io.mouseDown[button] = e->mouseButton.down;
				mouse_button_changed |= (1 << button);
			}
		}
		else if (e->type == sInputEventType_MouseWheel)
		{
			if (e->mouseWheel.wheelX != 0.0f || e->mouseWheel.wheelY != 0.0f)
			{
				// Trickling Rule: Stop processing queued events if we got multiple action on the event
				if (trickleFastInputs && (mouse_wheeled || mouse_button_changed != 0))
					break;
				io.mouseWheelH += e->mouseWheel.wheelX;
				io.mouseWheel += e->mouseWheel.wheelY;
				mouse_wheeled = true;
			}
		}
		else if (e->type == sInputEventType_Key)
		{
			S_PLATFORMIO_ASSERT(e->key.key != sKey_None);
			const int keydata_index = (e->key.key - 1);
			sKeyData* keydata = &io.keysData[keydata_index];
			if (keydata->down != e->key.down)
			{
				// Trickling Rule: Stop processing queued events if we got multiple action on the same button
				if (trickleFastInputs && (key_changed_mask.TestBit(keydata_index) || text_inputed || mouse_button_changed != 0))
					break;
				keydata->down = e->key.down;
				key_changed = true;
				key_changed_mask.SetBit(keydata_index);
			}
		}
		else if (e->type == sInputEventType_KeyMods)
		{
			const sKeyModFlags modifiers = e->keyMods.mods;
			if (io.keyMods != modifiers)
			{
				// Trickling Rule: Stop processing queued events if we got multiple action on the same button
				sKeyModFlags modifiers_that_are_changing = (io.keyMods ^ modifiers);
				if (trickleFastInputs && (key_mods_changed & modifiers_that_are_changing) != 0)
					break;
				io.keyMods = modifiers;
				io.keyCtrl = (modifiers & sKeyModFlags_Ctrl) != 0;
				io.keyShift = (modifiers & sKeyModFlags_Shift) != 0;
				io.keyAlt = (modifiers & sKeyModFlags_Alt) != 0;
				io.keySuper = (modifiers & sKeyModFlags_Super) != 0;
				key_mods_changed |= modifiers_that_are_changing;
			}
		}
		else if (e->type == sInputEventType_Char)
		{
			// Trickling Rule: Stop processing queued events if keys/mouse have been interacted with
			if (trickleFastInputs && (key_changed || mouse_button_changed != 0 || mouse_moved || mouse_wheeled))
				break;
			unsigned int c = e->text.c;
			io.inputQueueCharacters.push_back(c <= S_UNICODE_CODEPOINT_MAX ? (uint16_t)c : S_UNICODE_CODEPOINT_INVALID);
			text_inputed = true;
		}
		else if (e->type == sInputEventType_Focus)
		{
			// We intentionally overwrite this and process lower, in order to give a chance
			// to multi-viewports backends to queue AddFocusEvent(false) + AddFocusEvent(true) in same frame.
			io.appFocusLost = !e->appFocused.focused;
		}
		else
		{
			S_PLATFORMIO_ASSERT(0 && "Unknown event!");
		}
	}

	// Record trail (for domain-specific applications wanting to access a precise trail)
	for (int n = 0; n < event_n; n++)
		ctx.inputEventsTrail.push_back(ctx.inputEventsQueue[n]);

	// Remaining events will be processed on the next frame
	if (event_n == ctx.inputEventsQueue.size())
		ctx.inputEventsQueue.resize(0);
	else
		ctx.inputEventsQueue.erase(ctx.inputEventsQueue.begin(), ctx.inputEventsQueue.begin() + event_n);
		//ctx.inputEventsQueue.erase(ctx.inputEventsQueue.data(), ctx.inputEventsQueue.data() + event_n);

	// Clear buttons state when focus is lost
	// (this is useful so e.g. releasing Alt after focus loss on Alt-Tab doesn't trigger the Alt menu toggle)
	if (ctx.IO.appFocusLost)
	{
		ctx.IO.clearInputKeys();
		ctx.IO.appFocusLost = false;
	}
}

void
Semper::update_keyboard_inputs()
{
	sIOContext& ctx = *GSemperIOCtx;
	sIO& io = get_io();

	// Synchronize io.KeyMods with individual modifiers io.KeyXXX bools
	io.keyMods = sKeyModFlags_None;
	if (io.keyCtrl)  { io.keyMods |= sKeyModFlags_Ctrl; }
	if (io.keyShift) { io.keyMods |= sKeyModFlags_Shift; }
	if (io.keyAlt)   { io.keyMods |= sKeyModFlags_Alt; }
	if (io.keySuper) { io.keyMods |= sKeyModFlags_Super; }

	// Update keys
	for (int i = 0; i < S_PLATFORMIO_ARRAYSIZE(io.keysData); i++)
	{
		sKeyData& key_data = io.keysData[i];
		key_data.downDurationPrev = key_data.downDuration;
		key_data.downDuration = key_data.down ? (key_data.downDuration < 0.0f ? 0.0f : key_data.downDuration + io.deltaTime) : -1.0f;
	}
}

void
Semper::update_mouse_inputs()
{
	sIOContext& ctx = *GSemperIOCtx;
	sIO& io = get_io();

	// Round mouse position to avoid spreading non-rounded position (e.g. UpdateManualResize doesn't support them well)
	if (is_mouse_pos_valid(&io.mousePos))
		io.mousePos = ctx.mouseLastValidPos = sIOVec2(floorf(io.mousePos.x), floorf(io.mousePos.y));

	// If mouse just appeared or disappeared (usually denoted by -FLT_MAX components) we cancel out movement in MouseDelta
	if (is_mouse_pos_valid(&io.mousePos) && is_mouse_pos_valid(&io.mousePosPrev))
		io.mouseDelta = io.mousePos - io.mousePosPrev;
	else
		io.mouseDelta = sIOVec2(0.0f, 0.0f);

	io.mousePosPrev = io.mousePos;
	for (int i = 0; i < S_PLATFORMIO_ARRAYSIZE(io.mouseDown); i++)
	{
		io.mouseClicked[i] = io.mouseDown[i] && io.mouseDownDuration[i] < 0.0f;
		io.mouseClickedCount[i] = 0; // Will be filled below
		io.mouseReleased[i] = !io.mouseDown[i] && io.mouseDownDuration[i] >= 0.0f;
		io.mouseDownDurationPrev[i] = io.mouseDownDuration[i];
		io.mouseDownDuration[i] = io.mouseDown[i] ? (io.mouseDownDuration[i] < 0.0f ? 0.0f : io.mouseDownDuration[i] + io.deltaTime) : -1.0f;
		if (io.mouseClicked[i])
		{
			bool is_repeated_click = false;
			if ((float)(ctx.time - io.mouseClickedTime[i]) < io.mouseDoubleClickTime)
			{
				sIOVec2 delta_from_click_pos = is_mouse_pos_valid(&io.mousePos) ? (io.mousePos - io.mouseClickedPos[i]) : sIOVec2(0.0f, 0.0f);
				if (_lengthSqrr(delta_from_click_pos) < io.mouseDoubleClickMaxDist * io.mouseDoubleClickMaxDist)
					is_repeated_click = true;
			}
			if (is_repeated_click)
				io.mouseClickedLastCount[i]++;
			else
				io.mouseClickedLastCount[i] = 1;
			io.mouseClickedTime[i] = ctx.time;
			io.mouseClickedPos[i] = io.mousePos;
			io.mouseClickedCount[i] = io.mouseClickedLastCount[i];
			io.mouseDragMaxDistanceSqr[i] = 0.0f;
		}
		else if (io.mouseDown[i])
		{
			// Maintain the maximum distance we reaching from the initial click position, which is used with dragging threshold
			float delta_sqr_click_pos = is_mouse_pos_valid(&io.mousePos) ? _lengthSqrr(io.mousePos - io.mouseClickedPos[i]) : 0.0f;
			io.mouseDragMaxDistanceSqr[i] = io.mouseDragMaxDistanceSqr[i] > delta_sqr_click_pos ? io.mouseDragMaxDistanceSqr[i] : delta_sqr_click_pos;
		}

		// We provide io.MouseDoubleClicked[] as a legacy service
		io.mouseDoubleClicked[i] = (io.mouseClickedCount[i] == 2);
	}
}

void
Semper::update_mouse_wheel()
{
	sIOContext& ctx = *GSemperIOCtx;
	sIO& io = get_io();

	if (ctx.IO.mouseWheel == 0.0f && ctx.IO.mouseWheelH == 0.0f)
		return;
}

void
Semper::cleanup_window(sWindow* window)
{
#ifdef WIN32
    ::CoUninitialize();
    ::DestroyWindow(window->_win32_handle);
    UnregisterClass(window->_win32_windowClass.lpszClassName, window->_win32_windowClass.hInstance);
    window->_win32_handle = nullptr;
    window->_win32_mouseHandle = nullptr;
    window->_win32_windowClass = { 0 };
#elif defined(__APPLE__)
#else
        // Turn key repeats back on since this is global for the OS... just... wow.
        XAutoRepeatOn(window->display);
        xcb_destroy_window(window->connection, window->window);
#endif
	S_PLATFORMIO_FREE(window);
	window = nullptr;
}

void
Semper::process_window_events(sWindow& window)
{
#ifdef WIN32
    if (window.posDirty)
    {
        int horizontal_shift = _get_horizontal_shift(window._win32_handle);
        ::SetWindowPos(window._win32_handle, window.modes.alwaysOnTop ? HWND_TOPMOST : HWND_TOP, 
            window.xpos - horizontal_shift, window.ypos, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE);
        window.posDirty = false;
    }

    if (window.sizeDirty)
    {
        ::SetWindowPos(window._win32_handle, window.modes.alwaysOnTop ? HWND_TOPMOST : HWND_TOP, 
            0, 0, window.width, window.height, SWP_SHOWWINDOW | SWP_NOMOVE);
        window.sizeDirty = false;
    }

    if (window.modesDirty)
    {
        window._win32_flags = WS_OVERLAPPED;

        if (window.modes.resizable && window.modes.decorated) window._win32_flags |= WS_THICKFRAME;
        if (window.modes.decorated) window._win32_flags |= WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;

        SetWindowLongPtr(window._win32_handle, GWL_STYLE, window._win32_flags);
        ::SetWindowPos(window._win32_handle, window.modes.alwaysOnTop ? HWND_TOPMOST : HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
        window.modesDirty = false;
    }

    if (window.titleDirty)
    {
        SetWindowTextA(window._win32_handle, window.title);
        window.titleDirty = false;
    }

    MSG msg = {};

    // while queue has messages, remove and dispatch them (but do not block on empty queue)
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
    {
        // check for quit because peekmessage does not signal this via return val
        if (msg.message == WM_QUIT)
        {
            window.running = false;
            return;
        }

        // TranslateMessage will post auxilliary WM_CHAR messages from key msgs
        ::TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
#elif defined(__APPLE__)
#else
        sIO& io = GSemperIOCtx->IO;

        xcb_generic_event_t* event;
        xcb_client_message_event_t* cm;

        // Poll for events until null is returned.
        while ((event = xcb_poll_for_event(window.connection))) 
        {
            // Input events
            switch (event->response_type & ~0x80) 
            {

                case XCB_KEY_PRESS:
                case XCB_KEY_RELEASE: 
                {

                    #if defined SEMPER_INCLUDE_IMGUI
                    if (ImGui::GetIO().WantCaptureKeyboard) break;
                    #endif

                    // Key press event - xcb_key_press_event_t and xcb_key_release_event_t are the same
                    xcb_key_press_event_t* kb_event = (xcb_key_press_event_t*)event;
                    bool pressed = event->response_type == XCB_KEY_PRESS;
                    xcb_keycode_t code = kb_event->detail;
                    KeySym key_sym = XkbKeycodeToKeysym(
                        window.display,
                        (KeyCode)code,  //event.xkey.keycode,
                        0,
                        code & ShiftMask ? 1 : 0);

                    //keys key = translate_keycode(key_sym);

                    const sKey key = VirtualKeyToSemperKey(key_sym);
                    // Pass to the input subsystem for processing.
                    //input_process_key(key, pressed);
                    if (key != sKey_None)
				        io.pushKeyEvent(key, is_key_down);
                    break;
                }

                case XCB_BUTTON_PRESS:
                case XCB_BUTTON_RELEASE: 
                {
                    xcb_button_press_event_t* mouse_event = (xcb_button_press_event_t*)event;
                    bool pressed = event->response_type == XCB_BUTTON_PRESS;
                    // buttons mouse_button = BUTTON_MAX_BUTTONS;
                    // switch (mouse_event->detail) {
                    //     case XCB_BUTTON_INDEX_1:
                    //         mouse_button = BUTTON_LEFT;
                    //         break;
                    //     case XCB_BUTTON_INDEX_2:
                    //         mouse_button = BUTTON_MIDDLE;
                    //         break;
                    //     case XCB_BUTTON_INDEX_3:
                    //         mouse_button = BUTTON_RIGHT;
                    //         break;
                    // }

                    // Pass over to the input subsystem.
                    // if (mouse_button != BUTTON_MAX_BUTTONS) {
                    //     input_process_button(mouse_button, pressed);
                    // }
                    break;
                }

                case XCB_MOTION_NOTIFY: 
                {

                    #if defined SEMPER_INCLUDE_IMGUI
                    // We need to call TrackMouseEvent in order to receive WM_MOUSELEAVE events
                    if (ImGui::GetIO().WantCaptureMouse) break;
                    #endif

                    // Mouse move
                    xcb_motion_notify_event_t* move_event = (xcb_motion_notify_event_t*)event;

                    // Pass over to the input subsystem.
                    io.pushMousePosEvent((float)move_event->event_x, (float)move_event->event_y);
                    break;
                }

                case XCB_CONFIGURE_NOTIFY: 
                {
                    // Resizing - note that this is also triggered by moving the window, but should be
                    // passed anyway since a change in the x/y could mean an upper-left resize.
                    // The application layer can decide what to do with this.
                    xcb_configure_notify_event_t* configure_event = (xcb_configure_notify_event_t*)event;

                    // Fire the event. The application layer should pick this up, but not handle it
                    // as it shouldn be visible to other parts of the application.
                    // event_context context;
                    // context.data.u16[0] = configure_event->width;
                    // context.data.u16[1] = configure_event->height;
                    // event_fire(EVENT_CODE_RESIZED, 0, context);
                    window.sizeChanged = true;
                    window.width = configure_event->width;
                    window.height = configure_event->height;
                    break;

                }

                case XCB_CLIENT_MESSAGE: 
                {
                    cm = (xcb_client_message_event_t*)event;

                    // Window close
                    if (cm->data.data32[0] == window.wm_delete_win) 
                    {
                        window.running = false;
                    }
                    break;
                } 
                default:
                    // Something else
                    break;
            }

            free(event);
        }
#endif
}

void
Semper::enable_cursor(sWindow& window)
{
	if (window.cursorEnabled)
		return;

	window.cursorEnabled = true;
    sIO& io = get_io();
#ifdef WIN32
    
    if (window.rawMouseMotion)
        _disable_mouse_raw_motion(window);

    ::ClipCursor(nullptr);

    ::SetCursorPos((int)io.restoreCursorPos[0], (int)io.restoreCursorPos[1]);

    ::ShowCursor(true);
    ::SetCursor(::LoadCursor(nullptr, IDC_ARROW));

#elif defined(__APPLE__)
#else
#endif
}

void
Semper::disable_cursor(sWindow& window)
{
	if (!window.cursorEnabled)
		return;

	sIO& io = get_io();

	window.cursorEnabled = false;

#ifdef WIN32
    if (is_window_focused(window))
    {
        POINT pos;

        if (::GetCursorPos(&pos))
        {
            //::ScreenToClient(window.platform.handle, &pos);
            io.restoreCursorPos[0] = pos.x;
            io.restoreCursorPos[1] = pos.y;
        }
    }

    ::ShowCursor(false);
    ::SetCursor(NULL);

    {
        int width, height;

        RECT area;
        ::GetClientRect(window._win32_handle, &area);

        POINT pos = { (LONG)area.right / 2, (LONG)area.bottom / 2 };

        // Store the new position so it can be recognized later
        io.lastCursorPos[0] = pos.x;
        io.lastCursorPos[1] = pos.y;

        ::ClientToScreen(window._win32_handle, &pos);
        ::SetCursorPos(pos.x, pos.y);
    }

    // update clip rect
    {
        RECT clipRect;
        ::GetClientRect(window._win32_handle, &clipRect);
        ::ClientToScreen(window._win32_handle, (POINT*)&clipRect.left);
        ::ClientToScreen(window._win32_handle, (POINT*)&clipRect.right);
        ::ClipCursor(&clipRect);
    }

    if(!window.rawMouseMotion)
        _enable_mouse_raw_motion(window);
#elif defined(__APPLE__)
#else
#endif
}

void
Semper::maximize_window(sWindow& window)
{
#ifdef WIN32
    ::ShowWindow(window._win32_handle, SW_MAXIMIZE);
#elif defined(__APPLE__)
#else
#endif
}

void
Semper::minimize_window(sWindow& window)
{
#ifdef WIN32
    ::ShowWindow(window._win32_handle, SW_MINIMIZE);
#elif defined(__APPLE__)
#else
#endif
}

void
Semper::toggle_fullscreen(sWindow& window)
{
#ifdef WIN32
    static size_t storedWidth = 0;
    static size_t storedHeight = 0;
    static int    storedXPos = 0;
    static int    storedYPos = 0;

    size_t width = ::GetSystemMetrics(SM_CXSCREEN);
    size_t height = ::GetSystemMetrics(SM_CYSCREEN);

    if (window.modes.fullScreen)
    {
        RECT rect;
        rect.left = storedXPos;
        rect.top = storedYPos;
        rect.right = storedXPos + storedWidth;
        rect.bottom = storedYPos + storedHeight;
        ::SetWindowLongPtr(window._win32_handle, GWL_STYLE, WS_OVERLAPPEDWINDOW | WS_VISIBLE);
        ::AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
        ::MoveWindow(window._win32_handle, storedXPos, storedYPos, storedWidth, storedHeight, TRUE);
        window.modes.fullScreen = false;
    }
    else
    {
        storedWidth = window.width;
        storedHeight = window.height;
        storedXPos = window.xpos;
        storedYPos = window.ypos;

        SetWindowLongPtr(window._win32_handle, GWL_STYLE, WS_SYSMENU | WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE);
        ::MoveWindow(window._win32_handle, 0, 0, width, height, TRUE);
        window.modes.fullScreen = true;
    }
#elif defined(__APPLE__)
#else
#endif
}

void
Semper::set_window_pos(sWindow& window, int xpos, int ypos)
{
	window.xpos = xpos;
	window.ypos = ypos;
	window.posDirty = true;
}

void
Semper::set_window_title(sWindow& window, const char* title)
{
	window.title = title;
	window.titleDirty = true;
}

void
Semper::set_window_size(sWindow& window, unsigned width, unsigned height)
{
	window.width = width;
	window.height = height;
	window.sizeChanged = true;
	window.sizeDirty = true;
}

void
Semper::set_window_modes(sWindow& window, sWindowMode modes)
{
	window.modes = modes;
	window.modesDirty = true;
}

sWindow*
Semper::create_window(const char* title, sWindowMode modes, int cwidth, int cheight)
{
	auto window = (sWindow*)S_PLATFORMIO_ALLOC(sizeof(sWindow));

	window->client_width = cwidth;
	window->client_height = cheight;
	window->xpos = 0;
	window->ypos = 0;
	window->running = true;
	window->sizeChanged = false;
	window->modesDirty = false;
	window->sizeDirty = false;
	window->posDirty = false;
	window->mouseButtonsDown = 0;
	window->mouseTracked = false;
	window->cursorEnabled = true;
	window->minwidth = 250u;
	window->minheight = 250u;
	window->maxwidth = 10000u;
	window->maxheight = 10000u;
	window->modes = modes;
	window->title = title;
    window->smallIcon = nullptr;
    window->largeIcon = nullptr;

#ifdef WIN32
    window->_win32_windowClass = { 0 };
    window->_win32_windowClass.cbSize = sizeof(WNDCLASSEX);
    window->_win32_windowClass.style = CS_HREDRAW | CS_VREDRAW;
    window->_win32_windowClass.lpfnWndProc = HandleMsgSetup;
    window->_win32_windowClass.cbClsExtra = 0;
    window->_win32_windowClass.cbWndExtra = 0;
    window->_win32_windowClass.hInstance = GetModuleHandle(nullptr);
    window->_win32_windowClass.hIcon = nullptr;
    window->_win32_windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    window->_win32_windowClass.hbrBackground = nullptr;
    window->_win32_windowClass.lpszMenuName = nullptr;
    window->_win32_windowClass.lpszClassName = "Semper Window";
    window->_win32_windowClass.hIconSm = nullptr;
    RegisterClassEx(&window->_win32_windowClass);

    // calculate window size based on desired client region size
    RECT wr;
    wr.left = window->xpos;
    wr.right = cwidth + wr.left;
    wr.top = window->ypos;
    wr.bottom = cheight + wr.top;
    ::AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

	window->_win32_flags = WS_OVERLAPPED;
	if (modes.resizable && modes.decorated) window->_win32_flags |= WS_THICKFRAME;
	if (modes.decorated) window->_win32_flags |= WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;

    // create window & get hWnd
    window->_win32_handle = ::CreateWindowA(
        "Semper Window",
        _T(window->title),
        window->_win32_flags,
        window->xpos, window->ypos, wr.right - wr.left, wr.bottom - wr.top,
        nullptr,
        nullptr,
        ::GetModuleHandle(nullptr),
        window // app data
    );

    window->width = wr.right - wr.left;
    window->height = wr.bottom - wr.top;

    // set platform modes
    if (window->smallIcon)
    {
        HANDLE hIcon = ::LoadImage(0, _T(window->smallIcon), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);
        if (hIcon)
        {
            ::SendMessage(window->_win32_handle, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
            ::SendMessage(::GetWindow(window->_win32_handle, GW_OWNER), WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
        }
    }

    if (window->largeIcon)
    {
        HANDLE hIcon = ::LoadImage(0, _T(window->largeIcon), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);
        if (hIcon)
        {
            ::SendMessage(window->_win32_handle, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
            ::SendMessage(::GetWindow(window->_win32_handle, GW_OWNER), WM_SETICON, ICON_BIG, (LPARAM)hIcon);
        }
    }

    int cmdShow = SW_SHOWDEFAULT;

    //if (minimized)
    //	cmdShow = SW_MINIMIZE;
    //else if (maximized)
    //	cmdShow = SW_MAXIMIZE;

    ::ShowWindow(window->_win32_handle, cmdShow);
    ::UpdateWindow(window->_win32_handle);

    if (window->modes.alwaysOnTop)
        ::SetWindowPos(window->_win32_handle, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);   
#elif defined(__APPLE__)
#else
        // connect to x
        window->display = XOpenDisplay(nullptr);

        // turn off key repeats
        XAutoRepeatOff(window->display);

        // retrieve connection from display
        window->connection = XGetXCBConnection(window->display);

        if(xcb_connection_has_error(window->connection))
        {
            S_PLATFORMIO_ASSERT(false && "Failed to connect to X server via XCB.");
            return window;
        }

        // get data from x server
        const xcb_setup_t* setup = xcb_get_setup(window->connection);

        // Loop through screens using iterator
        xcb_screen_iterator_t it = xcb_setup_roots_iterator(setup);
        int screen_p = 0;
        for (int s = screen_p; s > 0; s--) 
        {
            xcb_screen_next(&it);
        }

        // After screens have been looped through, assign it.
        window->screen = it.data;

        // Allocate a XID for the window to be created.
        window->window = xcb_generate_id(window->connection);

        // Register event types.
        // XCB_CW_BACK_PIXEL = filling then window bg with a single colour
        // XCB_CW_EVENT_MASK is required.
        unsigned int event_mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;

        // Listen for keyboard and mouse buttons
        unsigned int  event_values = XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE |
                        XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE |
                        XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_POINTER_MOTION |
                        XCB_EVENT_MASK_STRUCTURE_NOTIFY;

        // Values to be sent over XCB (bg colour, events)
        unsigned int  value_list[] = {window->screen->black_pixel, event_values};

        // Create the window
        xcb_create_window(
            window->connection,
            XCB_COPY_FROM_PARENT,  // depth
            window->window,
            window->screen->root,        // parent
            window->xpos,                              //x
            window->ypos,                              //y
            cwidth,                          //width
            cheight,                         //height
            0,                              // No border
            XCB_WINDOW_CLASS_INPUT_OUTPUT,  //class
            window->screen->root_visual,
            event_mask,
            value_list);

        // Change the title
        xcb_change_property(
            window->connection,
            XCB_PROP_MODE_REPLACE,
            window->window,
            XCB_ATOM_WM_NAME,
            XCB_ATOM_STRING,
            8,  // data should be viewed 8 bits at a time
            strlen("Semper Window"),
            "Semper Window");

        // Tell the server to notify when the window manager
        // attempts to destroy the window.
        xcb_intern_atom_cookie_t wm_delete_cookie = xcb_intern_atom(
            window->connection,
            0,
            strlen("WM_DELETE_WINDOW"),
            "WM_DELETE_WINDOW");
        xcb_intern_atom_cookie_t wm_protocols_cookie = xcb_intern_atom(
            window->connection,
            0,
            strlen("WM_PROTOCOLS"),
            "WM_PROTOCOLS");
        xcb_intern_atom_reply_t* wm_delete_reply = xcb_intern_atom_reply(
            window->connection,
            wm_delete_cookie,
            NULL);
        xcb_intern_atom_reply_t* wm_protocols_reply = xcb_intern_atom_reply(
            window->connection,
            wm_protocols_cookie,
            NULL);
        window->wm_delete_win = wm_delete_reply->atom;
        window->wm_protocols = wm_protocols_reply->atom;

        xcb_change_property(
            window->connection,
            XCB_PROP_MODE_REPLACE,
            window->window,
            wm_protocols_reply->atom,
            4,
            32,
            1,
            &wm_delete_reply->atom);

        // Map the window to the screen
        xcb_map_window(window->connection, window->window);

        // Flush the stream
        int stream_result = xcb_flush(window->connection);
        if (stream_result <= 0) 
        {
            S_PLATFORMIO_ASSERT(false && "An error occurred when flusing the stream");
        }
#endif

	return window; 
}

sWindow*
Semper::create_window(const char* title, int cwidth, int cheight)
{
	sWindowMode modes;
	modes.vsync = true;
	modes.resizable = true;
	modes.alwaysOnTop = false;
	modes.decorated = true;
	modes.fullScreen = false;
	return create_window(title, modes, cwidth, cheight);
}

#ifdef WIN32

static LRESULT CALLBACK
HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
#if defined SEMPER_PLATFORMIO_INCLUDE_IMGUI
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;
#endif

	sIO& io = GSemperIOCtx->IO;
	sWindow* window;
	if (msg == WM_CREATE)
	{
		CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
		window = reinterpret_cast<sWindow*>(pCreate->lpCreateParams);
		::SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)window);
	}
	else
	{
		LONG_PTR ptr = ::GetWindowLongPtr(hWnd, GWLP_USERDATA);
		window = reinterpret_cast<sWindow*>(ptr);
	}

	switch (msg)
	{

	case WM_SIZE:
		if (wParam != SIZE_MINIMIZED)
		{
			RECT actual_rect;
			RECT client_rect;
			int actual_width = 0;
			int actual_height = 0;
			int client_width = 0;
			int client_height = 0;
			if (::GetWindowRect(hWnd, &actual_rect))
			{
				actual_width = actual_rect.right - actual_rect.left;
				actual_height = actual_rect.bottom - actual_rect.top;
			}

			if (::GetClientRect(hWnd, &client_rect))
			{
				client_width = client_rect.right - client_rect.left;
				client_height = client_rect.bottom - client_rect.top;
			}

			window->sizeChanged = true;
			window->width = actual_width;
			window->height = actual_height;
			window->client_width = client_width;
			window->client_height = client_height;
		}
		return 0;

	case WM_MOVING:
	{
		int horizontal_shift = _get_horizontal_shift(window->_win32_handle);
		RECT rect = *(RECT*)(lParam);
		window->xpos = rect.left + horizontal_shift;
		window->ypos = rect.top;
		break;
	}
	case WM_INPUT:
	{
	    UINT dwSize;

		::GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
		LPBYTE lpb = (BYTE*)S_PLATFORMIO_ALLOC(sizeof(BYTE)*dwSize);
		if (lpb == NULL) 
		{
			return 0;
		} 

		if (::GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize)
			::OutputDebugString (TEXT("GetRawInputData does not return correct size !\n")); 

		RAWINPUT* raw = (RAWINPUT*)lpb;

		if (raw->header.dwType == RIM_TYPEMOUSE) 
		{
			io.pushMouseRelativePosEvent((float)raw->data.mouse.lLastX, raw->data.mouse.lLastY);
		} 

		S_PLATFORMIO_FREE(lpb);
		return 0;	
	}

	case WM_MOUSEMOVE:

		#if defined SEMPER_INCLUDE_IMGUI
		// We need to call TrackMouseEvent in order to receive WM_MOUSELEAVE events
		if (ImGui::GetIO().WantCaptureMouse) break;
		#endif

		window->_win32_mouseHandle = window->_win32_handle;
		if (!window->mouseTracked)
		{
			TRACKMOUSEEVENT tme = { sizeof(tme), TME_LEAVE, window->_win32_handle, 0 };
			::TrackMouseEvent(&tme);
			window->mouseTracked = true;
		}
		if(window->cursorEnabled)
			io.pushMousePosEvent((float)GET_X_LPARAM(lParam), (float)GET_Y_LPARAM(lParam));
		break;
	case WM_MOUSELEAVE:

		#if defined SEMPER_INCLUDE_IMGUI
		// We need to call TrackMouseEvent in order to receive WM_MOUSELEAVE events
		if (ImGui::GetIO().WantCaptureMouse) break;
		#endif

		if (window->_win32_mouseHandle == window->_win32_handle)
			window->_win32_mouseHandle = NULL;
		window->mouseTracked = false;
		io.pushMousePosEvent(-FLT_MAX, -FLT_MAX);
		break;
	case WM_LBUTTONDOWN: case WM_LBUTTONDBLCLK:
	case WM_RBUTTONDOWN: case WM_RBUTTONDBLCLK:
	case WM_MBUTTONDOWN: case WM_MBUTTONDBLCLK:
	case WM_XBUTTONDOWN: case WM_XBUTTONDBLCLK:
	{

		#if defined SEMPER_INCLUDE_IMGUI
		// We need to call TrackMouseEvent in order to receive WM_MOUSELEAVE events
		if (ImGui::GetIO().WantCaptureMouse) break;
		#endif

		int button = 0;
		if (msg == WM_LBUTTONDOWN || msg == WM_LBUTTONDBLCLK) { button = 0; }
		if (msg == WM_RBUTTONDOWN || msg == WM_RBUTTONDBLCLK) { button = 1; }
		if (msg == WM_MBUTTONDOWN || msg == WM_MBUTTONDBLCLK) { button = 2; }
		if (msg == WM_XBUTTONDOWN || msg == WM_XBUTTONDBLCLK) { button = (GET_XBUTTON_WPARAM(wParam) == XBUTTON1) ? 3 : 4; }
		if (window->mouseButtonsDown == 0 && ::GetCapture() == NULL)
			::SetCapture(window->_win32_handle);
		window->mouseButtonsDown |= 1 << button;
		io.pushMouseButtonEvent(button, true);
		return 0;
	}

	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
	case WM_XBUTTONUP:
	{
		#if defined SEMPER_INCLUDE_IMGUI
		// We need to call TrackMouseEvent in order to receive WM_MOUSELEAVE events
		if (ImGui::GetIO().WantCaptureMouse) break;
		#endif

		int button = 0;
		if (msg == WM_LBUTTONUP) { button = 0; }
		if (msg == WM_RBUTTONUP) { button = 1; }
		if (msg == WM_MBUTTONUP) { button = 2; }
		if (msg == WM_XBUTTONUP) { button = (GET_XBUTTON_WPARAM(wParam) == XBUTTON1) ? 3 : 4; }
		window->mouseButtonsDown &= ~(1 << button);
		if (window->mouseButtonsDown == 0 && ::GetCapture() == window->_win32_handle)
			::ReleaseCapture();
		io.pushMouseButtonEvent(button, false);
		return 0;
	}

	case WM_MOUSEWHEEL:
		#if defined SEMPER_INCLUDE_IMGUI
		if (ImGui::GetIO().WantCaptureMouse) break;
		#endif
		io.pushMouseWheelEvent(0.0f, (float)GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA);
		return 0;
	case WM_MOUSEHWHEEL:
		#if defined SEMPER_INCLUDE_IMGUI
		if (ImGui::GetIO().WantCaptureMouse) break;
		#endif
		io.pushMouseWheelEvent((float)GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA, 0.0f);
		return 0;

	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
	{
		#if defined SEMPER_INCLUDE_IMGUI
		if (ImGui::GetIO().WantCaptureKeyboard) break;
		#endif
		const bool is_key_down = (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN);
		if (wParam < 256)
		{
			// submit modifiers
			sKeyModFlags key_mods =
				((is_vk_down(VK_CONTROL)) ? sKeyModFlags_Ctrl : 0) |
				((is_vk_down(VK_SHIFT)) ? sKeyModFlags_Shift : 0) |
				((is_vk_down(VK_MENU)) ? sKeyModFlags_Alt : 0) |
				((is_vk_down(VK_APPS)) ? sKeyModFlags_Super : 0);
			io.pushKeyModsEvent(key_mods);

			// obtain virtual key code
			// (keypad enter doesn't have its own... VK_RETURN with KF_EXTENDED flag means keypad enter, see IM_VK_KEYPAD_ENTER definition for details, it is mapped to ImGuiKey_KeyPadEnter.)
			int vk = (int)wParam;
			if ((wParam == VK_RETURN) && (HIWORD(lParam) & KF_EXTENDED))
				vk = IM_VK_KEYPAD_ENTER;

			// submit key event
			const sKey key = VirtualKeyToSemperKey(vk);
			const int scancode = (int)LOBYTE(HIWORD(lParam));
			if (key != sKey_None)
				io.pushKeyEvent(key, is_key_down);

			// submit individual left/right modifier events
			if (vk == VK_SHIFT)
			{
				// Important: shift keys tend to get stuck when pressed together, missing key-up events are corrected in ImGui_ImplWin32_ProcessKeyEventsWorkarounds()
				if (is_vk_down(VK_LSHIFT) == is_key_down) { io.pushKeyEvent(sKey_LeftShift, is_key_down); }
				if (is_vk_down(VK_RSHIFT) == is_key_down) { io.pushKeyEvent(sKey_RightShift, is_key_down); }
			}
			else if (vk == VK_CONTROL)
			{
				if (is_vk_down(VK_LCONTROL) == is_key_down) { io.pushKeyEvent(sKey_LeftCtrl, is_key_down); }
				if (is_vk_down(VK_RCONTROL) == is_key_down) { io.pushKeyEvent(sKey_RightCtrl, is_key_down); }
			}
			else if (vk == VK_MENU)
			{
				if (is_vk_down(VK_LMENU) == is_key_down) { io.pushKeyEvent(sKey_LeftAlt, is_key_down); }
				if (is_vk_down(VK_RMENU) == is_key_down) { io.pushKeyEvent(sKey_RightAlt, is_key_down); }
			}
		}
		return 0;
	}

	case WM_SETFOCUS:
	case WM_KILLFOCUS:
		io.pushFocusEvent(msg == WM_SETFOCUS);
		return 0;

	case WM_CHAR:

		#if defined SEMPER_INCLUDE_IMGUI
		if (ImGui::GetIO().WantCaptureKeyboard) break;
		#endif

		// You can also use ToAscii()+GetKeyboardState() to retrieve characters.
		if (wParam > 0 && wParam < 0x10000)
			io.pushInputCharacterUTF16((unsigned short)wParam);
		return 0;

	case WM_DESTROY:
	{
		window->running = false;
		PostQuitMessage(0);
		break;
	}

	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}

static sKey
VirtualKeyToSemperKey(WPARAM wParam)
{
	switch (wParam)
	{
	case VK_TAB: return sKey_Tab;
	case VK_LEFT: return sKey_LeftArrow;
	case VK_RIGHT: return sKey_RightArrow;
	case VK_UP: return sKey_UpArrow;
	case VK_DOWN: return sKey_DownArrow;
	case VK_PRIOR: return sKey_PageUp;
	case VK_NEXT: return sKey_PageDown;
	case VK_HOME: return sKey_Home;
	case VK_END: return sKey_End;
	case VK_INSERT: return sKey_Insert;
	case VK_DELETE: return sKey_Delete;
	case VK_BACK: return sKey_Backspace;
	case VK_SPACE: return sKey_Space;
	case VK_RETURN: return sKey_Enter;
	case VK_ESCAPE: return sKey_Escape;
	case VK_OEM_7: return sKey_Apostrophe;
	case VK_OEM_COMMA: return sKey_Comma;
	case VK_OEM_MINUS: return sKey_Minus;
	case VK_OEM_PERIOD: return sKey_Period;
	case VK_OEM_2: return sKey_Slash;
	case VK_OEM_1: return sKey_Semicolon;
	case VK_OEM_PLUS: return sKey_Equal;
	case VK_OEM_4: return sKey_LeftBracket;
	case VK_OEM_5: return sKey_Backslash;
	case VK_OEM_6: return sKey_RightBracket;
	case VK_OEM_3: return sKey_GraveAccent;
	case VK_CAPITAL: return sKey_CapsLock;
	case VK_SCROLL: return sKey_ScrollLock;
	case VK_NUMLOCK: return sKey_NumLock;
	case VK_SNAPSHOT: return sKey_PrintScreen;
	case VK_PAUSE: return sKey_Pause;
	case VK_NUMPAD0: return sKey_Keypad0;
	case VK_NUMPAD1: return sKey_Keypad1;
	case VK_NUMPAD2: return sKey_Keypad2;
	case VK_NUMPAD3: return sKey_Keypad3;
	case VK_NUMPAD4: return sKey_Keypad4;
	case VK_NUMPAD5: return sKey_Keypad5;
	case VK_NUMPAD6: return sKey_Keypad6;
	case VK_NUMPAD7: return sKey_Keypad7;
	case VK_NUMPAD8: return sKey_Keypad8;
	case VK_NUMPAD9: return sKey_Keypad9;
	case VK_DECIMAL: return sKey_KeypadDecimal;
	case VK_DIVIDE: return sKey_KeypadDivide;
	case VK_MULTIPLY: return sKey_KeypadMultiply;
	case VK_SUBTRACT: return sKey_KeypadSubtract;
	case VK_ADD: return sKey_KeypadAdd;
	case IM_VK_KEYPAD_ENTER: return sKey_KeypadEnter;
	case VK_LSHIFT: return sKey_LeftShift;
	case VK_LCONTROL: return sKey_LeftCtrl;
	case VK_LMENU: return sKey_LeftAlt;
	case VK_LWIN: return sKey_LeftSuper;
	case VK_RSHIFT: return sKey_RightShift;
	case VK_RCONTROL: return sKey_RightCtrl;
	case VK_RMENU: return sKey_RightAlt;
	case VK_RWIN: return sKey_RightSuper;
	case VK_APPS: return sKey_Menu;
	case '0': return sKey_0;
	case '1': return sKey_1;
	case '2': return sKey_2;
	case '3': return sKey_3;
	case '4': return sKey_4;
	case '5': return sKey_5;
	case '6': return sKey_6;
	case '7': return sKey_7;
	case '8': return sKey_8;
	case '9': return sKey_9;
	case 'A': return sKey_A;
	case 'B': return sKey_B;
	case 'C': return sKey_C;
	case 'D': return sKey_D;
	case 'E': return sKey_E;
	case 'F': return sKey_F;
	case 'G': return sKey_G;
	case 'H': return sKey_H;
	case 'I': return sKey_I;
	case 'J': return sKey_J;
	case 'K': return sKey_K;
	case 'L': return sKey_L;
	case 'M': return sKey_M;
	case 'N': return sKey_N;
	case 'O': return sKey_O;
	case 'P': return sKey_P;
	case 'Q': return sKey_Q;
	case 'R': return sKey_R;
	case 'S': return sKey_S;
	case 'T': return sKey_T;
	case 'U': return sKey_U;
	case 'V': return sKey_V;
	case 'W': return sKey_W;
	case 'X': return sKey_X;
	case 'Y': return sKey_Y;
	case 'Z': return sKey_Z;
	case VK_F1: return sKey_F1;
	case VK_F2: return sKey_F2;
	case VK_F3: return sKey_F3;
	case VK_F4: return sKey_F4;
	case VK_F5: return sKey_F5;
	case VK_F6: return sKey_F6;
	case VK_F7: return sKey_F7;
	case VK_F8: return sKey_F8;
	case VK_F9: return sKey_F9;
	case VK_F10: return sKey_F10;
	case VK_F11: return sKey_F11;
	case VK_F12: return sKey_F12;
	default: return sKey_None;
	}
}
#endif // win32

#endif