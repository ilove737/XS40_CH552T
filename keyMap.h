#ifndef KEY_MAP
#define KEY_MAP

#include "scanKey.h"

// #define KEY_MOD_LCTRL 0x01
// #define KEY_MOD_LSHIFT 0x02
// #define KEY_MOD_LALT 0x04
// #define KEY_MOD_LMETA 0x08

// 这个就是 左 键盘布局
#if 1

UINT8X keyMap[40][2]={
    {0,KEY_ESC}, {0,KEY_1}, {0,KEY_2}, {0,KEY_3}, {0,KEY_4}, {0,KEY_5}, {0,KEY_GRAVE}, {2,KEY_8}, 
    {0,KEY_TAB}, {0,KEY_Q}, {0,KEY_W}, {0,KEY_E}, {0,KEY_R}, {0,KEY_T}, {0,KEY_LEFTBRACE}, {0,KEY_RIGHTBRACE}, 
    {0,KEY_CAPSLOCK}, {0,KEY_A}, {0,KEY_S}, {0,KEY_D}, {0,KEY_F}, {0,KEY_G}, {2,KEY_9}, {2,KEY_0}, 
    {2,0}, {2,KEY_2}, {0,KEY_Z}, {0,KEY_X}, {0,KEY_C}, {0,KEY_V}, {2,KEY_7}, {2,KEY_3}, 
    {0,KEY_Fn0}, {8,0}, {4,0}, {0,KEY_Fn0}, {0,KEY_SPACE}, {1,0}, {2,KEY_GRAVE}, {2,KEY_1},
};

UINT8X Fn0_keyMap[40][2]={
    {0,KEY_ESC}, {0,KEY_1}, {0,KEY_2}, {0,KEY_3}, {0,KEY_4}, {0,KEY_5}, {0,KEY_GRAVE}, {2,KEY_8}, 
    {0,KEY_TAB}, {0,KEY_Q}, {0,KEY_W}, {0,KEY_E}, {0,KEY_R}, {0,KEY_T}, {0,KEY_LEFTBRACE}, {0,KEY_RIGHTBRACE}, 
    {0,KEY_CAPSLOCK}, {0,KEY_A}, {0,KEY_S}, {0,KEY_D}, {0,KEY_F}, {0,KEY_G}, {2,KEY_9}, {2,KEY_0}, 
    {2,0}, {2,KEY_2}, {0,KEY_Z}, {0,KEY_X}, {0,KEY_C}, {0,KEY_V}, {2,KEY_7}, {2,KEY_3}, 
    {0,KEY_Fn0}, {8,0}, {4,0}, {0,KEY_Fn0}, {0,KEY_SPACE}, {1,0}, {2,KEY_GRAVE}, {2,KEY_1},
};

#else
// 这个就是 右 键盘布局

unsigned char xdata keyMap[40] ={
    SHIFT_MINUS, SHIFT_EQUAL, KEY_6,  KEY_7,  KEY_8,  KEY_9,  KEY_0,  KEY_BACKSPACE,
    KEY_EQUAL, KEY_BACKSLASH, KEY_Y,  KEY_U,  KEY_I,  KEY_O,  KEY_P,  KEY_DELETE,
    KEY_SEMICOLON, KEY_COMMA, KEY_H,  KEY_J,  KEY_K,  KEY_L,  KEY_MINUS,   KEY_ENTER,
    KEY_APOSTROPHE, KEY_DOT, KEY_B,  KEY_N,  KEY_M,  KEY_SLASH,  KEY_UP,   KEY_RIGHTSHIFT,
    SHIFT_5, SHIFT_SLASH, KEY_RIGHTCTRL, KEY_SPACE,  KEY_Fn0,  KEY_LEFT,  KEY_DOWN,  KEY_RIGHT,
};

unsigned char xdata Fn0_keyMap[40] ={
    SHIFT_MINUS, SHIFT_EQUAL, KEY_F6,  KEY_F7,  KEY_F8,  KEY_F9,  KEY_F10,  KEY_BACKSPACE,
    KEY_EQUAL, KEY_BACKSLASH, KEY_KP7,  KEY_KP8,  SHIFT_BACKSLASH,  KEY_KPSLASH,  KEY_P,  KEY_DELETE,
    KEY_SEMICOLON, KEY_COMMA, KEY_KP4,  SHIFT_APOSTROPHE,  SHIFT_SEMICOLON,  KEY_KPASTERISK,  KEY_MINUS,   KEY_ENTER,
    KEY_APOSTROPHE, KEY_DOT, KEY_KP1,  SHIFT_LEFTBRACE,  SHIFT_RIGHTBRACE,  KEY_KPMINUS,  KEY_UP,   KEY_RIGHTSHIFT,
    SHIFT_5, SHIFT_SLASH, KEY_KPENTER, KEY_KP0,  KEY_Fn0,  KEY_KPPLUS,  KEY_DOWN,  KEY_RIGHT,
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