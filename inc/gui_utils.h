#ifndef UCHAT_GUI_UTILS_H
#define UCHAT_GUI_UTILS_H

#include "cimgui/cimgui.h"
#include "cimgui/cimgui_impl.h"
#include <GLFW/glfw3.h>
#include <stdio.h>
#ifdef _MSC_VER
#include <windows.h>
#endif
#include <GL/gl.h>

#include "json-c/arraylist.h"

#ifdef IMGUI_HAS_IMSTR
#define igBegin igBegin_Str
#define igSliderFloat igSliderFloat_Str
#define igCheckbox igCheckbox_Str
#define igColorEdit3 igColorEdit3_Str
#define igButton igButton_Str
#endif
typedef void (*draw_imgui_func)(void);


GLFWwindow *create_gui(int width, int height, const char *title);
void destroy_gui(GLFWwindow *window_ptr);
void mainloop_gui(GLFWwindow *window_ptr, draw_imgui_func func);

ImVec2 imVec2(float x, float y);
ImVec2 imVec2_zero(void);

typedef void(*t_ig_popup_drawer)(void);

array_list* get_ig_popups(void);
void ig_popups_draw(void);
void ig_popup_add(t_ig_popup_drawer popup_drawer);
void ig_popups_free(void);

float ig_get_window_height(void);

#endif
