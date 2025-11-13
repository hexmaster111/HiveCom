#include <raylib.h>

#define HC_DATA_IMPL
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

#include "fonts.h"

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
    //    EnableEventWaiting();

    LoadFonts();
    SetExitKey(KEY_NULL);

    Clay_SetMeasureTextFunction(Raylib_MeasureText, fonts);

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

#define RAYLIB_COLOR_TO_CLAY_COLOR(RAYCOLOR) \
    (Clay_Color) { .r = (RAYCOLOR).r, .g = (RAYCOLOR).g, .b = (RAYCOLOR).b, .a = (RAYCOLOR).a, }

Clay_Color HeaderButtonColor()
{
    Clay_Color ret = {140, 140, 140, 255};

    if (Clay_Hovered())
    {
        ret.r = 160;

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            ret.g = 160;
        }
    }

    return ret;
}

bool Render_Button(Clay_String text)
{
    bool clicked = 0;

    CLAY((Clay_ElementDeclaration){
        .layout = {.padding = {16, 16, 8, 8}},
        .backgroundColor = HeaderButtonColor(),
        .cornerRadius = CLAY_CORNER_RADIUS(0)})
    {
        CLAY_TEXT(text, CLAY_TEXT_CONFIG({.fontId = FONTID_DEFAULT,
                                          .fontSize = 16,
                                          .textColor = {255, 255, 255, 255}}));

        clicked = Clay_Hovered() && IsMouseButtonReleased(MOUSE_BUTTON_LEFT);
    }

    return clicked;
}

#define HC_SLICE_TO_CLAY_STRING(HC_SLICE) \
    (Clay_String) { .chars = (HC_SLICE).base, .length = (HC_SLICE).len }

Clay_Color UserColor(hc_MsgTree msg)
{
    if (hc_slice_cmp(msg.auther.name, hc_slice_from("System")) == 0)
    {
        return RAYLIB_COLOR_TO_CLAY_COLOR(RED);
    }

    return RAYLIB_COLOR_TO_CLAY_COLOR(PURPLE);
}

void RenderMessage(hc_MsgTree msg)
{
    CLAY((Clay_ElementDeclaration){
        .layout = {
            .layoutDirection = CLAY_LEFT_TO_RIGHT,
        },
    })
    {

        Clay_String name = HC_SLICE_TO_CLAY_STRING(msg.auther.name);
        Clay_String message = HC_SLICE_TO_CLAY_STRING(msg.content.text);

        CLAY((Clay_ElementDeclaration){
            .backgroundColor = UserColor(msg),
            .layout = {
                .padding = {.right = 8},
                .sizing = {
                    .height = CLAY_SIZING_FIT(0),
                    .width = CLAY_SIZING_FIXED(64),
                },
            },
        })
        {

            CLAY_TEXT(name, CLAY_TEXT_CONFIG((Clay_TextElementConfig){
                                .fontId = FONTID_DEFAULT,
                                .fontSize = 16,
                                .textColor = {255, 255, 255, 255},
                            }));
        }

        CLAY_TEXT(message, CLAY_TEXT_CONFIG((Clay_TextElementConfig){
                               .fontId = FONTID_DEFAULT,
                               .fontSize = 16,
                               .textColor = {255, 255, 255, 255},
                           }));
    }
}

Clay_RenderCommandArray DoLayout()
{
    hc_MsgTree root =
        {
            .content = (hc_MsgContent){
                .text = (hc_slice){.base = "FIRST!!!!", .len = sizeof("FIRST!!!!") - 1},
            },
            .auther = (hc_MsgAuther){.name = (hc_slice){.base = "System", .len = sizeof("System") - 1}},
            .right = 0,
            .down = &(hc_MsgTree){
                .auther = (hc_MsgAuther){.name = (hc_slice){.base = "MX-5050", .len = sizeof("MX-5050") - 1}},
                .content = (hc_MsgContent){
                    .text = (hc_slice){.base = "Hello, world", .len = sizeof("Hello, world") - 1},
                },

                .down = &(hc_MsgTree){
                    .auther = (hc_MsgAuther){.name = (hc_slice){.base = "W01F-5088", .len = sizeof("W01F-5088") - 1}},
                    .content = (hc_MsgContent){
                        .text = (hc_slice){.base = "Hello, and welcome back", .len = sizeof("Hello, and welcome back") - 1},
                    },
                },

            },
        };

    Clay_BeginLayout();

    CLAY((Clay_ElementDeclaration){
        .id = CLAY_ID("content"),
        .backgroundColor = RAYLIB_COLOR_TO_CLAY_COLOR(LIGHTGRAY),
        .layout = {
            .sizing = {.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIT(0)},
            .layoutDirection = CLAY_TOP_TO_BOTTOM},
    })
    {
        RenderMessage(root);
        RenderMessage(*root.down);
        RenderMessage(*root.down->down);

        if (Render_Button(CLAY_STRING("Test")))
        {
            printf("Clicked Test!\n");
        };
    }

    return Clay_EndLayout();
}