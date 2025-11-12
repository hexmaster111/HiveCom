#include <raylib.h>

#include "hc_data.h"

#define CLAY_IMPLEMENTATION
#include "clay.h"
#include "clay_raylib.c" // im sure this wont be an issue :)

void HandleClayErrors(Clay_ErrorData ed)
{
    printf("%s", ed.errorText.chars);
    // if (errorData.errorType == CLAY_ERROR_TYPE_ELEMENTS_CAPACITY_EXCEEDED)
    // {
    //     reinitializeClay = true;
    //     Clay_SetMaxElementCount(Clay_GetMaxElementCount() * 2);
    // }
    // else if (errorData.errorType == CLAY_ERROR_TYPE_TEXT_MEASUREMENT_CAPACITY_EXCEEDED)
    // {
    //     reinitializeClay = true;
    //     Clay_SetMaxMeasureTextCacheWordCount(Clay_GetMaxMeasureTextCacheWordCount() * 2);
    // }
}

Clay_RenderCommandArray DoLayout();

#define FONTID_DEFAULT (0)

Font fonts[1] = {0};

#include "font_8_16.h"
#include "font_16_32.h"

extern unsigned char FONT_8_16_DATA[590716];

void LoadFonts()
{

    Image img = (Image){
        .data = FONT_8_16_DATA,
        .width = FONT_8_16_WIDTH,
        .height = FONT_8_16_HEIGHT,
        .format = FONT_8_16_FORMAT,
        .mipmaps = 1};

    fonts[FONTID_DEFAULT] = LoadFontFromImage(img, MAGENTA, '!');

    if (!IsFontValid(fonts[FONTID_DEFAULT]))
    {
        TraceLog(LOG_WARNING, "Failed to load font!");
        fonts[FONTID_DEFAULT] = GetFontDefault();
    }
}

int main(int argc, char *argv[])
{

    uint64_t totalMemorySize = Clay_MinMemorySize();
    Clay_Arena clayMemory = Clay_CreateArenaWithCapacityAndMemory(totalMemorySize, malloc(totalMemorySize));
    Clay_Initialize(clayMemory, (Clay_Dimensions){(float)GetScreenWidth(), (float)GetScreenHeight()}, (Clay_ErrorHandler){HandleClayErrors, 0});

    Clay_Raylib_Initialize(800, 600, "Hive Com", FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    SetTargetFPS(GetMonitorRefreshRate(GetCurrentMonitor()));

    LoadFonts();
    SetExitKey(KEY_NULL);

    Clay_SetMeasureTextFunction(Raylib_MeasureText, fonts);
    //    EnableEventWaiting();

    bool debug_enabled = false;

    while (!WindowShouldClose())
    {
        Vector2 mouseWheelDelta = GetMouseWheelMoveV();
        float mouseWheelX = mouseWheelDelta.x;
        float mouseWheelY = mouseWheelDelta.y;

        Clay_Vector2 mousePosition = RAYLIB_VECTOR2_TO_CLAY_VECTOR2(GetMousePosition());
        Clay_SetPointerState(mousePosition, IsMouseButtonDown(MOUSE_BUTTON_LEFT));
        Clay_SetLayoutDimensions((Clay_Dimensions){(float)GetScreenWidth(), (float)GetScreenHeight()});
        Clay_UpdateScrollContainers(true, (Clay_Vector2){mouseWheelX, mouseWheelY}, GetFrameTime());

        if (IsKeyPressed(KEY_D))
        {
            debug_enabled = !debug_enabled;
            Clay_SetDebugModeEnabled(debug_enabled);
        }

        Clay_RenderCommandArray render_commands = DoLayout();

        BeginDrawing();
        ClearBackground(BLACK);
        Clay_Raylib_Render(render_commands, fonts);

        // DrawFPS(GetScreenHeight()-20, GetScreenWidth()-20);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}

hc_MsgTree root = {
    .content = (hc_MsgContent){
        .text = (hc_slice){.base = "Hello, world", .len = sizeof("Hello, world") - 1},
    },
    .auther = (hc_MsgAuther){.name = (hc_slice){.base = "System", .len = sizeof("System") - 1}},
    .right = 0,
    .down = &(hc_MsgTree){
        .auther = (hc_MsgAuther){.name = (hc_slice){.base = "MX-5050", .len = sizeof("MX-5050") - 1}},
        .content = (hc_MsgContent){
            .text = (hc_slice){.base = "Hello, world", .len = sizeof("Hello, world") - 1},
        },
    },
};

#define RAYLIB_COLOR_TO_CLAY_COLOR(RAYCOLOR) \
    (Clay_Color) { .r = (RAYCOLOR).r, .g = (RAYCOLOR).g, .b = (RAYCOLOR).b, .a = (RAYCOLOR).a, }

void RenderHeaderButton(Clay_String text)
{
    CLAY((Clay_ElementDeclaration){
        .layout = {.padding = {16, 16, 8, 8}},
        .backgroundColor = {Clay_Hovered() ? 160 : 140, 140, 140, 255},
        .cornerRadius = CLAY_CORNER_RADIUS(5)})
    {
        CLAY_TEXT(text, CLAY_TEXT_CONFIG({.fontId = FONTID_DEFAULT,
                                          .fontSize = 16,
                                          .textColor = {255, 255, 255, 255}}));
    }
}
Clay_RenderCommandArray DoLayout()
{
    Clay_BeginLayout();

    CLAY((Clay_ElementDeclaration){
        .id = CLAY_ID("content"),
        .backgroundColor = RAYLIB_COLOR_TO_CLAY_COLOR(LIGHTGRAY),
        .layout = {
            .sizing = {.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIT(0)},
            .layoutDirection = CLAY_TOP_TO_BOTTOM},
    })
    {
        RenderHeaderButton(CLAY_STRING("Test"));
    }

    return Clay_EndLayout();
}