#include "gui_utils.h"
#include "stdlib.h"
#include "font_tahoma.h"

GLFWwindow *create_gui(int width, int height, const char *title)
{
    if (!glfwInit())
        return NULL;

    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);

#if __APPLE__
    // GL 3.2 Core + GLSL 150
    const char *glsl_version = "#version 150";
#else
    // GL 3.2 + GLSL 130
    const char *glsl_version = "#version 130";
#endif

    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    GLFWwindow *window_ptr = NULL;

    window_ptr = glfwCreateWindow(width, height, title, NULL, NULL);
    if (!window_ptr)
    {
        printf("Failed to create window! Terminating!\n");
        glfwTerminate();
        return NULL;
    }

    glfwMakeContextCurrent(window_ptr);

    glfwSwapInterval(1);

    igCreateContext(NULL);

    printf("opengl version: %s\n", (char *)glGetString(GL_VERSION));

    ImGuiIO *ioptr = igGetIO();
    ioptr->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui_ImplGlfw_InitForOpenGL(window_ptr, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    igStyleColorsDark(NULL);

    ImFontAtlas_AddFontFromMemoryTTF(ioptr->Fonts, (void *)tahoma_ttf, TAHOMA_TTF_SIZE, 16.f, NULL, ImFontAtlas_GetGlyphRangesCyrillic(ioptr->Fonts));

    return window_ptr;
}

void destroy_gui(GLFWwindow *window_ptr)
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();

    igDestroyContext(NULL);

    glfwDestroyWindow(window_ptr);
    glfwTerminate();
}

void mainloop_gui(GLFWwindow *window_ptr, draw_imgui_func draw_func)
{
    ImGuiIO *ioptr = igGetIO();

    while (!glfwWindowShouldClose(window_ptr))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        igNewFrame();

        draw_func();

        igRender();

        glfwMakeContextCurrent(window_ptr);
        glViewport(0, 0, (int)ioptr->DisplaySize.x, (int)ioptr->DisplaySize.y);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(igGetDrawData());
        glfwSwapBuffers(window_ptr);
    }
}

inline ImVec2 imVec2_zero(void)
{
    static ImVec2 imVec;
    static bool init = false;

    if (!init)
    {
        imVec.x = 0;
        imVec.y = 0;
        init = true;
    }

    return imVec;
}

ImVec2 imVec2(float x, float y)
{
    ImVec2 imVec;
    imVec.x = x;
    imVec.y = y;

    return imVec;
}

array_list *get_ig_popups(void)
{
    static bool init = false;
    static array_list *list_of_popups = NULL;
    if (!init)
    {
        list_of_popups = array_list_new2(free, 16);
        init = true;
    }
    return list_of_popups;
}
void ig_popups_draw(void)
{
    array_list *popups = get_ig_popups();
    for (size_t i = 0; i < array_list_length(popups); i++)
    {
        t_ig_popup_drawer drawer = array_list_get_idx(popups, i);
        drawer();
    }
}
void ig_popup_add(t_ig_popup_drawer popup_drawer)
{
    array_list_add(get_ig_popups(), popup_drawer);
}

void ig_popups_free()
{
    array_list_free(get_ig_popups());
}

float ig_get_window_height()
{
    ImGuiStyle *style = igGetStyle();
    ImVec2 drawCursPos;

    igGetCursorPos(&drawCursPos);

    float base_height = igGetWindowHeight();

    return base_height - (drawCursPos.y + style->FramePadding.y + style->CellPadding.y + style->WindowPadding.y);
}
