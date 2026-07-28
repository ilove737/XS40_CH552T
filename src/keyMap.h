#ifndef __KEY_MAP_H__
#define __KEY_MAP_H__

#include "CH552.H"
#include "scanKey.h"

// 默认左手布局
#ifndef KEYBOARD_LAYOUT
#define KEYBOARD_LAYOUT 0
#endif

#if KEYBOARD_LAYOUT == 0

// 左手布局 键位映射表
UINT8C __at(0x3600) mainKeyMap[40][2] = {
    {0,KEY_ESC}, {0,KEY_1}, {0,KEY_2}, {0,KEY_3}, {0,KEY_4}, {0,KEY_5}, {0,KEY_GRAVE}, {2,KEY_8}, 
    {0,KEY_TAB}, {0,KEY_Q}, {0,KEY_W}, {0,KEY_E}, {0,KEY_R}, {0,KEY_T}, {0,KEY_LEFTBRACE}, {0,KEY_RIGHTBRACE}, 
    {0,KEY_CAPSLOCK}, {0,KEY_A}, {0,KEY_S}, {0,KEY_D}, {0,KEY_F}, {0,KEY_G}, {2,KEY_9}, {2,KEY_0}, 
    {2,0}, {2,KEY_2}, {0,KEY_Z}, {0,KEY_X}, {0,KEY_C}, {0,KEY_V}, {2,KEY_7}, {2,KEY_3}, 
    {KEY_FnX,0}, {8,0}, {4,0}, {KEY_FnX,0}, {0,KEY_SPACE}, {1,0}, {1,KEY_C}, {1,KEY_V},
};

// 左手布局 Fn0 键位映射表
// 修饰键字节 0xFE 表示鼠标动作，键码为 MOUSE_* 宏定义
UINT8C __at(0x3650) Fn0_keyMap[40][2] = {
    {0,KEY_ESC}, {0,KEY_F1}, {0,KEY_F2}, {0,KEY_F3}, {0,KEY_F4}, {0,KEY_F5}, {0,KEY_GRAVE}, {2,KEY_8}, 
    {0,KEY_TAB}, {0,KEY_F11}, {0,KEY_F12}, {0,KEY_E}, {0,KEY_R}, {0,KEY_T}, {0,KEY_LEFTBRACE}, {0,KEY_RIGHTBRACE}, 
    {0,KEY_CAPSLOCK}, {0xFE,MOUSE_LCLICK}, {0xFE,MOUSE_UP}, {0xFE,MOUSE_RCLICK}, {0,KEY_F}, {0,KEY_G}, {2,KEY_9}, {2,KEY_0},
    {2,0}, {0xFE,MOUSE_LEFT}, {0xFE,MOUSE_DOWN}, {0xFE,MOUSE_RIGHT}, {0,KEY_C}, {0,KEY_V}, {2,KEY_7}, {2,KEY_3},
    {KEY_FnX,0}, {8,0}, {4,0}, {KEY_FnX,0}, {0,KEY_SPACE}, {1,0}, {2,KEY_GRAVE}, {2,KEY_1},
};

#else
// 右手布局

UINT8C __at(0x3600) mainKeyMap[40][2] = {
    {2,KEY_MINUS}, {2,KEY_EQUAL}, {0,KEY_6}, {0,KEY_7}, {0,KEY_8}, {0,KEY_9}, {0,KEY_0}, {0,KEY_BACKSPACE}, 
    {0,KEY_EQUAL}, {0,KEY_BACKSLASH}, {0,KEY_Y}, {0,KEY_U}, {0,KEY_I}, {0,KEY_O}, {0,KEY_P}, {0,KEY_DELETE}, 
    {0,KEY_SEMICOLON}, {0,KEY_COMMA}, {0,KEY_H}, {0,KEY_J}, {0,KEY_K}, {0,KEY_L}, {0,KEY_MINUS}, {0,KEY_ENTER}, 
    {0,KEY_APOSTROPHE}, {0,KEY_DOT}, {0,KEY_B}, {0,KEY_N}, {0,KEY_M}, {2,KEY_SLASH}, {0,KEY_UP}, {0x20,0}, 
    {2,KEY_5}, {0,KEY_SLASH}, {0,KEY_SPACE}, {0,KEY_SPACE}, {KEY_FnX,0}, {0,KEY_LEFT}, {0,KEY_DOWN}, {0,KEY_RIGHT},
};

UINT8C __at(0x3650) Fn0_keyMap[40][2] = {
    {2,KEY_MINUS}, {2,KEY_EQUAL}, {0,KEY_F6}, {0,KEY_F7}, {0,KEY_F8}, {0,KEY_F9}, {0,KEY_F10}, {0,KEY_BACKSPACE}, 
    {0,KEY_EQUAL}, {0,KEY_BACKSLASH}, {0,KEY_Y}, {0,KEY_U}, {0,KEY_I}, {0,KEY_O}, {0,KEY_P}, {0,KEY_DELETE}, 
    {0,KEY_SEMICOLON}, {0x20,KEY_COMMA}, {0,KEY_H}, {2,KEY_APOSTROPHE}, {2,KEY_SEMICOLON}, {0,KEY_L}, {0,KEY_MINUS}, {0,KEY_ENTER}, 
    {0,KEY_APOSTROPHE}, {0x20,KEY_DOT}, {1,KEY_B}, {2,KEY_LEFTBRACE}, {2,KEY_RIGHTBRACE}, {0xFE,MOUSE_LCLICK}, {0xFE,MOUSE_UP}, {0xFE,MOUSE_RCLICK},
    {2,KEY_5}, {0,KEY_SLASH}, {0,KEY_SPACE}, {0,KEY_SPACE}, {KEY_FnX,0}, {0xFE,MOUSE_LEFT}, {0xFE,MOUSE_DOWN}, {0xFE,MOUSE_RIGHT},
};

#endif

/*
http://www.keyboard-layout-editor.com/#/

[{c:"#6495ed"},"Esc",{c:"#cccccc"},"!\n1","@\n2","#\n3","$\n4","%\n5",{c:"#ff4500"},"~\n`","*",{x:1},"_","+",{c:"#cccccc"},"^\n6","&\n7","*\n8","(\n9",")\n0",{c:"#bb0000"},"Bs"],
[{c:"#444444"},"Tab",{c:"#cccccc"},"Q","W","E","R","T",{c:"#444444"},"{\n[","}\n]",{x:1},"+\n=","|\n\\",{c:"#cccccc"},"Y","U","I","O","P",{c:"#444444"},"del"],
[{c:"#cccccc"},"Caps Lock","A","S","D","F","G","(",{c:"#444444"},")",{x:1},":\n;",{c:"#cccccc"},"<\n,","H","J","K","L","_\n-","Enter"],
[{c:"#ff4500"},"Shift",{c:"#6495ed"},"@",{c:"#cccccc"},"Z","X","C","V","&",{c:"#444444"},"#",{x:1},"\"\n'",{c:"#cccccc"},">\n.","B","N","M","?\n/",{c:"#bb0000"},"↑",{c:"#ff4500"},"RShift"],
[{c:"#6495ed"},"Fn0","Win","Alt",{c:"#ff4500"},"Shift",{c:"#cccccc"},"Space",{c:"#444444"},"Ctrl","~","!",{x:1},"%","?","Ctrl",{c:"#cccccc"},"Space",{c:"#ff4500"},"Fn0",{c:"#bb0000"},"←","↓","→"],

[{y:0.5,c:"#6495ed"},"F0",{c:"#cccccc"},"F1","F2","F3","F4","F5",{c:"#ff4500"},"Num Lock","",{x:1},"","",{c:"#cccccc"},"F6","F7","F8","F9","F10",{c:"#bb0000"},""],
[{c:"#444444"},"select",{c:"#cccccc"},"F11","F12","mail",{a:7},"","",{c:"#444444",a:4},"","",{x:1},"","",{c:"#cccccc"},"7\nHome",{a:4},"8\n↑","|",{a:4},"/","",{c:"#444444"},""],
[{c:"#cccccc"},"prev","play","next","stop",{a:7},"","",{a:4},"",{c:"#444444"},"",{x:1},"",{c:"#cccccc"},"","4\n←",{a:4},"\"",{a:4},":",{a:4},"*","",""],
[{c:"#ff4500",a:4},"vol dn",{c:"#6495ed"},"mute",{c:"#cccccc"},"vol up","app",{a:7},"","",{a:4},"",{c:"#444444"},"",{x:1},"",{c:"#cccccc"},"","1\nEnd","{","}","-",{c:"#bb0000",a:4},"",{c:"#ff4500"},""],
[{c:"#6495ed"},"Fn0","Fn2","Fn4",{c:"#ff4500",a:7},"",{c:"#cccccc"},"",{c:"#444444"},"",{a:4},"","",{x:1},"","","PEnter",{c:"#cccccc"},"0\nIns",{c:"#ff4500"},"Fn0",{c:"#bb0000"},"+","",""]

*/
#endif // KEY_MAP