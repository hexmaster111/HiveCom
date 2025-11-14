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
    // SetExitKey(KEY_NULL);

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

    Clay_Raylib_Close();

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
        .layout = {.padding = {16, 16 /*, 8, 8*/}},
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
        .id = CLAY_IDI("RenderMessage", idx),
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

void RenderMessageTree(hc_MsgTree *root, int *i)
{
    RenderMessage(*root, *i);
    *i += 1;

    if (root->right)
    {
        CLAY((Clay_ElementDeclaration){
            .layout = {
                .sizing = {.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIT(0)},
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
                .padding = {.left = 20},
            },
        })
        {
            RenderMessageTree(root->right, i);
        }
    }

    if (root->down)
    {
        RenderMessageTree(root->down, i);
    }
}

void Render_TextBox(char *buffer, size_t bufferSize)
{
    int32_t len = strlen(buffer);

    CLAY_TEXT(CLAY_STRING("Text we will send."), CLAY_TEXT_CONFIG((Clay_TextElementConfig){
                                                     .fontId = FONTID_DEFAULT,
                                                     .fontSize = 16,
                                                     .textColor = {255, 255, 255, 255},
                                                 }));
}

void RenderChatBar()
{
    CLAY((Clay_ElementDeclaration){
        .layout = {
            .layoutDirection = CLAY_LEFT_TO_RIGHT,
            .sizing = {
                .height = CLAY_SIZING_FIT(0),
                .width = CLAY_SIZING_GROW(100),
            },
        },
    })
    {
        CLAY((Clay_ElementDeclaration){
            .backgroundColor = RAYLIB_COLOR_TO_CLAY_COLOR(BROWN),
            .layout = {
                .padding = {.right = 8},
                .sizing = {
                    .height = CLAY_SIZING_FIT(0),
                    .width = CLAY_SIZING_GROW(100),
                },
            },
        })
        {

            static char buffer[512] = "Text we will send";

            Render_TextBox(buffer, sizeof(buffer) - 1); // keep a null terminator in there (-1)
        }

        if (Render_Button(CLAY_STRING("Send")))
        {
            printf("Write Me!\n");
        };
    }
}

#pragma region // demo data

const char *demo_names[] = {
    "System",
    "MX-5050",
    "6625",
    "W01F-5088",
    "0013",
    "PZ-9447",
    "4440",
    "9790",
    "0001",
    "0002",
    "Queen Hour",
    "0003",
    "0004",
    "0005",
    "0007",
};

const char *demo_msgs[] = {
    "First!!",
    "Hello world!",
    "Hello, and welcome back!",
    "Goshhh.",
    "*purrs*",
    "Afafafaf",
    "Yeahhhh~",
    "Admit it:\nthis is the 2nd line.",
    "We are us are each other are it",
    "Mmmwwuufff.",
};

const char *RandomName() { return demo_names[GetRandomValue(0, (sizeof(demo_names) / sizeof(demo_names[0]) - 1))]; }
const char *RandomMessage() { return demo_msgs[GetRandomValue(0, (sizeof(demo_msgs) / sizeof(demo_msgs[0]) - 1))]; }

hc_MsgTree BuildDemoTree()
{
    hc_MsgTree ret = {0};

    hc_MsgTree *n = &ret;

    n->auther.name = hc_slice_from(demo_names[0]);
    n->content.text = hc_slice_from(demo_msgs[0]);

    int downs = 20;

    for (size_t i = 0; i < downs; i++)
    {
        n->down = malloc(sizeof(hc_MsgTree));
        memset(n->down, 0, sizeof(hc_MsgTree));
        n->down->auther.name = hc_slice_from(RandomName());
        n->down->content.text = hc_slice_from(RandomMessage());
        n = n->down;
    }

    for (size_t branches = 0; branches < 3; branches++)
    {
        n = &ret;
        int randval = GetRandomValue(0, downs - 1);

        for (size_t i = 0; i < randval; i++)
        {
            if (n->down == NULL)
                break;

            n = n->down;
        }

        randval = GetRandomValue(1, 5);
        for (size_t i = 0; i < randval; i++)
        {
            n->right = malloc(sizeof(hc_MsgTree));
            memset(n->right, 0, sizeof(hc_MsgTree));
            n->right->auther.name = hc_slice_from(RandomName());
            n->right->content.text = hc_slice_from(RandomMessage());
            n = n->right;
        }
    }

    return ret;
}

#pragma endregion // demo data

Clay_RenderCommandArray DoLayout()
{

    Clay_BeginLayout();

    static hc_MsgTree *root = 0;
    if (root == NULL || IsKeyPressed(KEY_F1))
    {
        // LEAKing root if its not null... (who cares, its demo data)

        hc_MsgTree tree = BuildDemoTree();
        root = malloc(sizeof(hc_MsgTree));
        *root = tree;
    }

    CLAY((Clay_ElementDeclaration){
        .id = CLAY_ID("comp_msg"),
        .layout = {
            .sizing = {.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0)},
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
        },
    })
    {
        CLAY((Clay_ElementDeclaration){
            .id = CLAY_ID("Messages"),
            .clip = {.vertical = true, .childOffset = Clay_GetScrollOffset()},
            .backgroundColor = RAYLIB_COLOR_TO_CLAY_COLOR(DARKGRAY),
            .layout = {
                .sizing = {.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIT(0)},
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
            },
        })
        {
            int idx = 0;
            RenderMessageTree(root, &idx);
            // printf("%d messages\n", idx);
        }

        CLAY((Clay_ElementDeclaration){.id = CLAY_ID("FILL SPACE"), .layout = {.sizing = {.height = CLAY_SIZING_GROW(0)}}}) {}

        RenderChatBar();
    }

    return Clay_EndLayout();
}
