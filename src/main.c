#include <raylib.h>

#define HC_DATA_IMPL
#include "hc_data.h"

#define CLAY_IMPLEMENTATION
#include "clay.h"
#include "clay_raylib.c" // im sure this wont be an issue :)

void HandleClayErrors(Clay_ErrorData ed)
{
    printf("%s\n", ed.errorText.chars);
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

void RenderMessage(hc_MsgTree msg, int idx)
{
    CLAY((Clay_ElementDeclaration){
        .layout = {
            .layoutDirection = CLAY_LEFT_TO_RIGHT,
            .sizing = {
                .width = CLAY_SIZING_GROW(0),
            },
        },
    })
    {

        Clay_String name = HC_SLICE_TO_CLAY_STRING(msg.auther.name);
        Clay_String message = HC_SLICE_TO_CLAY_STRING(msg.content.text);

        CLAY((Clay_ElementDeclaration){
            .id = CLAY_IDI("msg contanor", idx),
            .border = {
                .color = RAYLIB_COLOR_TO_CLAY_COLOR(BLACK),
                .width = {
                    .bottom = Clay_Hovered() ? 1 : 0,
                    .top = Clay_Hovered() ? 1 : 0,
                },
            },
            .layout = {
                .padding = {
                    .top = 3,
                },
                .sizing = {
                    .height = CLAY_SIZING_FIT(0),
                    .width = CLAY_SIZING_GROW(0),
                },
            },
        })
        {

            CLAY((Clay_ElementDeclaration){
                .backgroundColor = UserColor(msg),
                .layout = {
                    .sizing = {
                        // .height = CLAY_SIZING_GROW(0),
                        .height = CLAY_SIZING_FIT(0),
                        .width = CLAY_SIZING_FIXED(80),
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

            CLAY((Clay_ElementDeclaration){
                .layout = {
                    .padding = {.left = 8},
                    .sizing = {
                        .height = CLAY_SIZING_FIT(0),
                        .width = CLAY_SIZING_GROW(0),
                    },
                },
            })
            {
                CLAY_TEXT(message, CLAY_TEXT_CONFIG((Clay_TextElementConfig){
                                       .fontId = FONTID_DEFAULT,
                                       .fontSize = 16,
                                       .textColor = {255, 255, 255, 255},
                                   }));
            }
        }
    }
}

void RenderMessageTree(hc_MsgTree *root, int i)
{
    i += 1;

    RenderMessage(*root, i);
    if (root->down)
    {
        RenderMessageTree(root->down, i);
    }
}

Clay_RenderCommandArray DoLayout()
{
    hc_MsgTree root =
        {
            .content = (hc_MsgContent){.text = HC_SLICE_FROM_CLIT("FIRST!!!")},
            .auther = (hc_MsgAuther){.name = HC_SLICE_FROM_CLIT("System")},
            .right = 0,
            .down = &(hc_MsgTree){
                .auther = (hc_MsgAuther){.name = HC_SLICE_FROM_CLIT("MX-5050")},
                .content = (hc_MsgContent){.text = HC_SLICE_FROM_CLIT("Hello, and welcome back.")},
                .down = &(hc_MsgTree){
                    .auther = (hc_MsgAuther){.name = HC_SLICE_FROM_CLIT("W01F-5088")},
                    .content = (hc_MsgContent){.text = HC_SLICE_FROM_CLIT("Hello world!")},
                    .down = &(hc_MsgTree){
                        .auther = (hc_MsgAuther){.name = HC_SLICE_FROM_CLIT("Queen Hour")},
                        .content = (hc_MsgContent){.text = HC_SLICE_FROM_CLIT("Admit it:\nThis is the 2nd line.")},
                        .down = &(hc_MsgTree){
                            .auther = (hc_MsgAuther){.name = HC_SLICE_FROM_CLIT("6625")},
                            .content = (hc_MsgContent){.text = HC_SLICE_FROM_CLIT("We are us are each other are it."
                                                                                  "We are us are each other are it. We are us are each other are it.")},
                            .down = &(hc_MsgTree){
                                .auther = (hc_MsgAuther){.name = HC_SLICE_FROM_CLIT("0013")},
                                .content = (hc_MsgContent){.text = HC_SLICE_FROM_CLIT("Mmmwwuufff.")},
                            },
                        },
                    },
                },
            },
        };

    Clay_BeginLayout();

    CLAY((Clay_ElementDeclaration){
        .id = CLAY_ID("msg_log"),
        .backgroundColor = RAYLIB_COLOR_TO_CLAY_COLOR(DARKGRAY),
        .layout = {
            .sizing = {.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIT(0)},
            .layoutDirection = CLAY_TOP_TO_BOTTOM},
    })
    {

        RenderMessageTree(&root, 0);

        // RenderMessage(root, 0);
        // RenderMessage(*root.down, 1);
        // RenderMessage(*root.down->down, 2);
        // RenderMessage(*root.down->down->down, 3);
        // RenderMessage(*root.down->down->down->down, 4);
        // RenderMessage(*root.down->down->down->down->down, 5);

        if (Render_Button(CLAY_STRING("Send")))
        {
            printf("Write Me!\n");
        };
    }

    return Clay_EndLayout();
}