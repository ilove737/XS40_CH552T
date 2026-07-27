#!/usr/bin/env python3
"""
XS40_CH552T 键位映射配置工具 - GUI 版
依赖: python3-gi (GTK3), hidapi
"""

import sys
import os
from pathlib import Path

# 确保可以导入同目录的 set_keymap 模块
sys.path.insert(0, str(Path(__file__).parent))
import gi

gi.require_version('Gtk', '3.0')
from gi.repository import Gtk, Gdk, GLib, Pango

# 导入 CLI 工具的模块
import set_keymap as km

# =========================================================================
# 常量
# =========================================================================
ROWS = 5
COLS = 8
ENTRIES = 40
KEYMAP_SIZE = 160

# 键名缩写（显示用）
SHORT_NAMES = {
    'NONE': '', 'ERR_OVF': 'OVF',
    'LEFTCTRL': 'LCtrl', 'LEFTSHIFT': 'LShift', 'LEFTALT': 'LAlt', 'LEFTMETA': 'LWin',
    'RIGHTCTRL': 'RCtrl', 'RIGHTSHIFT': 'RShift', 'RIGHTALT': 'RAlt', 'RIGHTMETA': 'RWin',
    'ENTER': 'Enter', 'BACKSPACE': 'BkSpc', 'CAPSLOCK': 'Caps', 'SCROLLLOCK': 'Scrlk',
    'NUMLOCK': 'NumLk', 'SPACE': 'Space', 'TAB': 'Tab', 'ESC': 'Esc',
    'DELETE': 'Del', 'INSERT': 'Ins', 'HOME': 'Home', 'END': 'End',
    'PAGEUP': 'PgUp', 'PAGEDOWN': 'PgDn', 'PRINTSCREEN': 'PrtSc', 'PAUSE': 'Pause',
    'LEFTBRACE': '[', 'RIGHTBRACE': ']', 'BACKSLASH': '\\', 'HASHTILDE': '#~',
    'SEMICOLON': ';', 'APOSTROPHE': "'", 'GRAVE': '`', 'COMMA': ',', 'DOT': '.',
    'SLASH': '/', 'MINUS': '-', 'EQUAL': '=',
    'UP': '↑', 'DOWN': '↓', 'LEFT': '←', 'RIGHT': '→',
    'KP_SLASH': 'KP/', 'KP_ASTERISK': 'KP*', 'KP_MINUS': 'KP-',
    'KP_PLUS': 'KP+', 'KP_ENTER': 'KPE', 'KP_DOT': 'KP.',
    'LEFTBRACE': '[', 'RIGHTBRACE': ']',
}

def short_name(code):
    """获取键码的短名称"""
    if code == 0:
        return ''
    # 查找 KEY_NAMES 中的名称
    for name, val in km.KEY_NAMES.items():
        if val == code:
            if name in SHORT_NAMES:
                return SHORT_NAMES[name]
            # 特殊键名截断
            if len(name) <= 5:
                return name
            if name.startswith('KP_'):
                return name[3:]
            return name[:4]
    return f'0x{code:02x}'


def mod_prefix(mod):
    """修饰符前缀"""
    parts = []
    if mod & 0x01: parts.append('LCtrl')
    if mod & 0x02: parts.append('LShift')
    if mod & 0x04: parts.append('LAlt')
    if mod & 0x08: parts.append('LWin')
    if mod & 0x10: parts.append('RCtrl')
    if mod & 0x20: parts.append('RShift')
    if mod & 0x40: parts.append('RAlt')
    if mod & 0x80: parts.append('RWin')
    return ''.join(parts) if parts else ''


# =========================================================================
# GDK keyval → USB HID 键码映射
# =========================================================================
from gi.repository import Gdk as _Gdk

def gdk_to_hid(keyval):
    """将 GDK keyval 映射为 USB HID 键码，不支持返回 None"""
    # 字母 A-Z（大小写均映射到同一 HID 码）
    if 0x41 <= keyval <= 0x5a:  # 大写 A-Z
        return keyval - 0x3d    # 'A'=0x41 → 0x04
    if 0x61 <= keyval <= 0x7a:  # 小写 a-z
        return keyval - 0x5d    # 'a'=0x61 → 0x04
    # 数字 0-9
    if 0x31 <= keyval <= 0x39:  # '1'-'9'
        return keyval - 0x13    # '1'=0x31 → 0x1e
    if keyval == 0x30:          # '0'
        return 0x27
    # 数字行上档符号 → 映射到对应数字的 HID 码
    if keyval == 0x21: return 0x1e  # ! → KEY_1
    if keyval == 0x40: return 0x1f  # @ → KEY_2
    if keyval == 0x23: return 0x20  # # → KEY_3
    if keyval == 0x24: return 0x21  # $ → KEY_4
    if keyval == 0x25: return 0x22  # % → KEY_5
    if keyval == 0x5e: return 0x23  # ^ → KEY_6
    if keyval == 0x26: return 0x24  # & → KEY_7
    if keyval == 0x2a: return 0x25  # * → KEY_8
    if keyval == 0x28: return 0x26  # ( → KEY_9
    if keyval == 0x29: return 0x27  # ) → KEY_0
    # 修饰键（单独按下时映射为 USB HID 修饰键码 0xe0-0xe7）
    if keyval == _Gdk.KEY_Shift_L:     return 0xe1
    if keyval == _Gdk.KEY_Shift_R:     return 0xe5
    if keyval == _Gdk.KEY_Control_L:   return 0xe0
    if keyval == _Gdk.KEY_Control_R:   return 0xe4
    if keyval == _Gdk.KEY_Alt_L:       return 0xe2
    if keyval == _Gdk.KEY_Alt_R:       return 0xe6
    if keyval == _Gdk.KEY_Super_L:     return 0xe3
    if keyval == _Gdk.KEY_Super_R:     return 0xe7
    if keyval == _Gdk.KEY_Meta_L:      return 0xe3
    if keyval == _Gdk.KEY_Meta_R:      return 0xe7
    # 小键盘
    if 0xFF80 <= keyval <= 0xFF89:  # KP_0 - KP_9
        return 0x62 - (keyval - 0xFF80)  # KP_0=0x62, ...
    if keyval == _Gdk.KEY_KP_Divide:    return 0x54
    if keyval == _Gdk.KEY_KP_Multiply:  return 0x55
    if keyval == _Gdk.KEY_KP_Subtract:  return 0x56
    if keyval == _Gdk.KEY_KP_Add:       return 0x57
    if keyval == _Gdk.KEY_KP_Enter:     return 0x58
    if keyval == _Gdk.KEY_KP_Decimal:   return 0x63
    # 功能键
    if _Gdk.KEY_F1 <= keyval <= _Gdk.KEY_F12:
        return 0x3a + (keyval - _Gdk.KEY_F1)
    # 方向键
    if keyval == _Gdk.KEY_Up:       return 0x52
    if keyval == _Gdk.KEY_Down:     return 0x51
    if keyval == _Gdk.KEY_Left:     return 0x50
    if keyval == _Gdk.KEY_Right:    return 0x4f
    # 编辑键
    if keyval == _Gdk.KEY_Insert:   return 0x49
    if keyval == _Gdk.KEY_Delete:   return 0x4c  # Forward Delete
    if keyval == _Gdk.KEY_Home:     return 0x4a
    if keyval == _Gdk.KEY_End:      return 0x4d
    if keyval == _Gdk.KEY_Page_Up:  return 0x4b
    if keyval == _Gdk.KEY_Page_Down: return 0x4e
    # 符号键（主键盘区）
    if keyval == _Gdk.KEY_minus:        return 0x2d
    if keyval == _Gdk.KEY_equal:        return 0x2e
    if keyval == _Gdk.KEY_bracketleft:  return 0x2f
    if keyval == _Gdk.KEY_bracketright: return 0x30
    if keyval == _Gdk.KEY_backslash:    return 0x31
    if keyval == _Gdk.KEY_semicolon:    return 0x33
    if keyval == _Gdk.KEY_apostrophe:   return 0x34
    if keyval == _Gdk.KEY_grave:        return 0x35
    if keyval == _Gdk.KEY_comma:        return 0x36
    if keyval == _Gdk.KEY_period:       return 0x37
    if keyval == _Gdk.KEY_slash:        return 0x38
    if keyval == _Gdk.KEY_space:        return 0x2c
    if keyval == _Gdk.KEY_Tab:          return 0x2b
    if keyval == _Gdk.KEY_Return:       return 0x28
    if keyval == _Gdk.KEY_Escape:       return 0x29
    if keyval == _Gdk.KEY_BackSpace:    return 0x2a
    if keyval == _Gdk.KEY_Caps_Lock:    return 0x39
    if keyval == _Gdk.KEY_Num_Lock:     return 0x53
    if keyval == _Gdk.KEY_Scroll_Lock:  return 0x47
    if keyval == _Gdk.KEY_Pause:        return 0x48
    if keyval == _Gdk.KEY_Sys_Req:      return 0x46
    return None


# =========================================================================
# 按键编辑对话框
# =========================================================================

class KeyEditDialog(Gtk.Dialog):
    def __init__(self, parent, idx, mod, keycode, layer_name):
        super().__init__(title=f'编辑键位 {idx} ({layer_name})', parent=parent,
                         flags=Gtk.DialogFlags.MODAL)
        self.set_default_size(350, 250)
        self.add_button('取消', Gtk.ResponseType.CANCEL)
        self.add_button('确定', Gtk.ResponseType.OK)

        self.capturing = True

        # 捕获按键事件
        ctrl = Gtk.EventControllerKey.new(self)
        ctrl.connect('key-pressed', self.on_key_pressed)
        self._key_controller = ctrl

        box = self.get_content_area()
        box.set_spacing(8)
        box.set_margin_top(12)
        box.set_margin_bottom(12)
        box.set_margin_start(12)
        box.set_margin_end(12)

        # 索引信息
        lbl = Gtk.Label(label=f'<b>索引 {idx}</b>  ({layer_name}, 行{idx//COLS} 列{idx%COLS})')
        lbl.set_use_markup(True)
        box.add(lbl)

        # 修饰符
        mod_frame = Gtk.Frame(label='修饰符')
        mod_box = Gtk.Grid()
        mod_box.set_column_spacing(4)
        mod_box.set_row_spacing(2)
        mod_box.set_margin_top(6)
        mod_box.set_margin_bottom(6)
        mod_box.set_margin_start(6)
        mod_box.set_margin_end(6)

        self.mod_check = {}
        mod_names = [
            ('LCTRL', 0x01), ('LSHIFT', 0x02), ('LALT', 0x04), ('LMETA', 0x08),
            ('RCTRL', 0x10), ('RSHIFT', 0x20), ('RALT', 0x40), ('RMETA', 0x80),
        ]
        for i, (name, val) in enumerate(mod_names):
            cb = Gtk.CheckButton(label=name)
            cb.set_active(bool(mod & val))
            mod_box.attach(cb, i % 4, i // 4, 1, 1)
            self.mod_check[name] = cb
        mod_frame.add(mod_box)
        box.add(mod_frame)

        # 键码
        key_frame = Gtk.Frame(label='键码')
        key_box = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=6)
        key_box.set_margin_top(6)
        key_box.set_margin_bottom(6)
        key_box.set_margin_start(6)
        key_box.set_margin_end(6)

        key_box.add(Gtk.Label(label='十六进制:'))
        self.hex_entry = Gtk.Entry(max_length=4, width_chars=6)
        self.hex_entry.set_text(f'0x{keycode:02x}')
        key_box.add(self.hex_entry)

        key_box.add(Gtk.Label(label='  名称:'))
        self.name_entry = Gtk.Entry(max_length=20, width_chars=16)
        for name, val in km.KEY_NAMES.items():
            if val == keycode:
                self.name_entry.set_text(name)
                break
        key_box.add(self.name_entry)

        key_frame.add(key_box)
        box.add(key_frame)

        # 捕获按键
        capture_box = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=6)
        capture_box.set_margin_top(4)
        self.capture_btn = Gtk.Button(label='⏳ 捕获中...')
        self.capture_btn.set_sensitive(False)
        self.capture_btn.connect('clicked', self.on_capture)
        capture_box.add(self.capture_btn)
        self.capture_label = Gtk.Label(label='')
        self.capture_label.set_markup('<span color="blue">按任意键...</span>')
        capture_box.add(self.capture_label)
        box.add(capture_box)

        # 提示
        box.add(Gtk.Label(
            label='提示: 如果组合键和系统全局热键冲突，\n'
                  '则可能无法捕获到按键。',
            xalign=0))

        self.show_all()

    def get_result(self):
        mod = 0
        for name, val in [
            ('LCTRL', 0x01), ('LSHIFT', 0x02), ('LALT', 0x04), ('LMETA', 0x08),
            ('RCTRL', 0x10), ('RSHIFT', 0x20), ('RALT', 0x40), ('RMETA', 0x80),
        ]:
            if self.mod_check[name].get_active():
                mod |= val

        # 解析键码
        hex_text = self.hex_entry.get_text().strip()
        name_text = self.name_entry.get_text().strip().upper()
        if name_text and name_text in km.KEY_NAMES:
            key = km.KEY_NAMES[name_text]
        else:
            try:
                key = int(hex_text, 0)
            except ValueError:
                key = 0
        return mod, key

    def on_capture(self, btn):
        """进入捕获模式 — 下一个按键将填入此对话框"""
        self.capturing = True
        self.capture_btn.set_sensitive(False)
        self.capture_btn.set_label('⏳ 请按目标键...')
        self.capture_label.set_markup('<span color="blue">等待按键...</span>')
        self.grab_focus()

    def on_key_pressed(self, ctrl, keyval, keycode, state):
        """GTK 按键事件 — 捕获模式下记录键值"""
        if not self.capturing:
            return False  # 继续正常事件处理

        # 映射 GDK keyval → USB HID 键码
        hid = gdk_to_hid(keyval)
        if hid is None:
            self.capture_label.set_markup('<span color="red">不支持的按键</span>')
            return True

        # 映射 GDK 修饰符 → USB 修饰符位
        usb_mod = 0
        # state 中的 Shift/Ctrl/Alt 对应左侧
        if state & Gdk.ModifierType.SHIFT_MASK:   usb_mod |= 0x02
        if state & Gdk.ModifierType.CONTROL_MASK: usb_mod |= 0x01
        if state & Gdk.ModifierType.MOD1_MASK:    usb_mod |= 0x04  # Alt
        if state & Gdk.ModifierType.SUPER_MASK:   usb_mod |= 0x08  # Win

        # 修饰键 (0xe0-0xe7) 也合并到修饰符位
        if 0xe0 <= hid <= 0xe7:
            modbit = {0xe0: 0x01, 0xe1: 0x02, 0xe2: 0x04, 0xe3: 0x08,
                      0xe4: 0x10, 0xe5: 0x20, 0xe6: 0x40, 0xe7: 0x80}
            usb_mod |= modbit.get(hid, 0)

        # 更新修饰符复选框
        for name, val in [('LCTRL',0x01),('LSHIFT',0x02),('LALT',0x04),('LMETA',0x08),
                          ('RCTRL',0x10),('RSHIFT',0x20),('RALT',0x40),('RMETA',0x80)]:
            if name in self.mod_check:
                self.mod_check[name].set_active(bool(usb_mod & val))

        # 如果是纯修饰键 (0xe0-0xe7)，更新修饰符后继续等待主键
        if 0xe0 <= hid <= 0xe7:
            self.capture_label.set_markup(
                f'<span color="blue">已捕获修饰符，继续按主键...</span>')
            return True

        # 非修饰键：填充键码并结束捕获
        self.hex_entry.set_text(f'0x{hid:02x}')
        for name, val in km.KEY_NAMES.items():
            if val == hid:
                self.name_entry.set_text(name)
                break

        self.capture_label.set_markup(f'<span color="green">已捕获 0x{hid:02x}</span>')
        self._capture_end()
        return True

    def _capture_end(self):
        """退出捕获模式"""
        self.capturing = False
        self.capture_btn.set_sensitive(True)
        self.capture_btn.set_label('🎯 重新捕获')


# =========================================================================
# 主窗口
# =========================================================================

class KeymapGUI(Gtk.Window):
    def __init__(self):
        super().__init__(title='XS40_CH552T 键位映射编辑器')
        self.set_default_size(700, 500)
        self.set_border_width(6)

        # 数据: 160 字节
        self.data = bytearray(km.make_default_keymap())
        self.current_layer = 0  # 0=mainKeyMap, 1=Fn0_keyMap
        self.file_path = None

        # 主布局
        vbox = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=4)
        self.add(vbox)

        # 工具栏
        toolbar = Gtk.Toolbar()
        toolbar.set_style(Gtk.ToolbarStyle.ICONS)
        vbox.add(toolbar)

        # 图层切换
        self.layer_btn = Gtk.ToggleToolButton()
        self.layer_btn.set_label('Layer')
        self.layer_btn.set_active(False)
        self.layer_btn.set_tooltip_text('切换 mainKeyMap / Fn0_keyMap 图层')
        self.layer_btn.connect('toggled', self.on_layer_toggle)
        toolbar.add(self.layer_btn)

        self.layer_label = Gtk.ToolItem()
        lbl = Gtk.Label(label='  mainKeyMap  ')
        lbl.set_markup('<b>mainKeyMap</b>')
        self.layer_label_icon = lbl
        self.layer_label.add(lbl)
        toolbar.add(self.layer_label)

        toolbar.add(Gtk.SeparatorToolItem())

        # 文件操作
        btn_load = Gtk.ToolButton.new_from_stock(Gtk.STOCK_OPEN)
        btn_load.set_label('加载')
        btn_load.set_tooltip_text('从文件加载键位映射')
        btn_load.connect('clicked', self.on_load)
        toolbar.add(btn_load)

        btn_save = Gtk.ToolButton.new_from_stock(Gtk.STOCK_SAVE)
        btn_save.set_label('保存')
        btn_save.set_tooltip_text('保存到当前文件')
        btn_save.connect('clicked', self.on_save)
        toolbar.add(btn_save)

        btn_saveas = Gtk.ToolButton.new_from_stock(Gtk.STOCK_SAVE_AS)
        btn_saveas.set_label('另存')
        btn_saveas.set_tooltip_text('另存为新文件')
        btn_saveas.connect('clicked', self.on_save_as)
        toolbar.add(btn_saveas)

        toolbar.add(Gtk.SeparatorToolItem())

        # 设备操作
        btn_write = Gtk.ToolButton.new_from_stock(Gtk.STOCK_MEDIA_RECORD)
        btn_write.set_label('写入设备')
        btn_write.set_tooltip_text('将当前映射写入键盘 Flash（需 udev 权限）')
        btn_write.connect('clicked', self.on_write_device)
        toolbar.add(btn_write)

        btn_read = Gtk.ToolButton.new_from_stock(Gtk.STOCK_REFRESH)
        btn_read.set_label('读取')
        btn_read.set_tooltip_text('从键盘读取当前键位映射')
        btn_read.connect('clicked', self.on_read_device)
        toolbar.add(btn_read)

        btn_reset = Gtk.ToolButton.new_from_stock(Gtk.STOCK_CLEAR)
        btn_reset.set_label('默认')
        btn_reset.set_tooltip_text('恢复为默认键位映射')
        btn_reset.connect('clicked', self.on_reset)
        toolbar.add(btn_reset)

        # 键盘网格
        grid_frame = Gtk.Frame(label=' 键盘布局 ')
        vbox.add(grid_frame)

        self.grid = Gtk.Grid()
        self.grid.set_row_spacing(2)
        self.grid.set_column_spacing(2)
        self.grid.set_margin_top(6)
        self.grid.set_margin_bottom(6)
        self.grid.set_margin_start(6)
        self.grid.set_margin_end(6)

        # 创建 40 个按键
        self.key_buttons = []
        for row in range(ROWS):
            row_btns = []
            for col in range(COLS):
                btn = Gtk.Button(label='', width_request=76, height_request=76)
                btn.get_child().set_ellipsize(Pango.EllipsizeMode.END)
                btn.get_child().set_max_width_chars(4)
                btn.get_child().set_alignment(0.5, 0.5)
                btn.connect('clicked', self.on_key_clicked, row * COLS + col)
                self.grid.attach(btn, col, row, 1, 1)
                row_btns.append(btn)
            self.key_buttons.append(row_btns)

        grid_frame.add(self.grid)

        # 状态栏
        self.statusbar = Gtk.Statusbar()
        self.statusbar.set_margin_top(4)
        ctx = self.statusbar.get_context_id('main')
        self.status_ctx = ctx
        vbox.add(self.statusbar)

        self.update_all()
        self.set_status('就绪')

        self.show_all()

    # ---- 数据访问 ----
    def get_layer_data(self, layer):
        offset = layer * 80
        return self.data[offset:offset + 80]

    def set_layer_data(self, layer, data):
        offset = layer * 80
        self.data[offset:offset + 80] = data[:80]

    def get_key(self, layer, idx):
        return self.get_layer_data(layer)[idx * 2], self.get_layer_data(layer)[idx * 2 + 1]

    def set_key(self, layer, idx, mod, key):
        offset = layer * 80 + idx * 2
        self.data[offset] = mod & 0xFF
        self.data[offset + 1] = key & 0xFF

    # ---- 界面更新 ----
    def update_all(self):
        layer_data = self.get_layer_data(self.current_layer)
        for row in range(ROWS):
            for col in range(COLS):
                idx = row * COLS + col
                mod = layer_data[idx * 2]
                key = layer_data[idx * 2 + 1]
                btn = self.key_buttons[row][col]

                name = short_name(key)
                prefix = mod_prefix(mod)
                # Fn 切换键显示 Fn0
                if mod == 0xFF and key == 0x00:
                    text = 'Fn0'
                elif prefix:
                    text = f'{prefix}\n{name}' if name else prefix
                else:
                    text = name or '--'

                btn.set_label(text)

                # 颜色
                style = btn.get_style_context()
                for cls in ['key-normal', 'key-fn', 'key-mod', 'key-none']:
                    style.remove_class(cls)
                if mod == 0xFF and key == 0x00:
                    style.add_class('key-fn')
                elif mod != 0:
                    style.add_class('key-mod')
                elif key == 0:
                    style.add_class('key-none')
                else:
                    style.add_class('key-normal')

        # 图层标签
        name = 'mainKeyMap' if self.current_layer == 0 else 'Fn0_keyMap'
        self.layer_label_icon.set_markup(f'<b>{name}</b>')

    def set_status(self, msg):
        self.statusbar.push(self.status_ctx, msg)

    # ---- 事件处理 ----
    def on_layer_toggle(self, btn):
        self.current_layer = 1 if btn.get_active() else 0
        self.update_all()
        self.set_status(f'切换到 {"Fn0_keyMap" if self.current_layer else "mainKeyMap"}')

    def on_key_clicked(self, btn, idx):
        mod, key = self.get_key(self.current_layer, idx)
        layer_name = 'Fn0_keyMap' if self.current_layer else 'mainKeyMap'
        dialog = KeyEditDialog(self, idx, mod, key, layer_name)
        if dialog.run() == Gtk.ResponseType.OK:
            new_mod, new_key = dialog.get_result()
            self.set_key(self.current_layer, idx, new_mod, new_key)
            self.update_all()
            self.set_status(f'已修改键位 {idx}: mod=0x{new_mod:02x} key=0x{new_key:02x}')
        dialog.destroy()

    def on_load(self, btn):
        dialog = Gtk.FileChooserDialog(
            title='加载映射文件', parent=self,
            action=Gtk.FileChooserAction.OPEN)
        dialog.add_button('取消', Gtk.ResponseType.CANCEL)
        dialog.add_button('打开', Gtk.ResponseType.OK)

        filt_txt = Gtk.FileFilter()
        filt_txt.set_name('文本映射 (*.txt)')
        filt_txt.add_pattern('*.txt')
        dialog.add_filter(filt_txt)

        filt_bin = Gtk.FileFilter()
        filt_bin.set_name('二进制映射 (*.bin)')
        filt_bin.add_pattern('*.bin')
        dialog.add_filter(filt_bin)

        filt_all = Gtk.FileFilter()
        filt_all.set_name('所有文件')
        filt_all.add_pattern('*')
        dialog.add_filter(filt_all)

        if dialog.run() == Gtk.ResponseType.OK:
            path = dialog.get_filename()
            filt = dialog.get_filter()
            is_binary = '二进制' in filt.get_name()
            try:
                if is_binary:
                    with open(path, 'rb') as f:
                        raw = f.read()
                    if len(raw) != KEYMAP_SIZE:
                        raise ValueError(f'大小错误: {len(raw)}B, 期望 {KEYMAP_SIZE}B')
                    self.data = bytearray(raw)
                else:
                    raw = km.read_keymap_file(path)
                    self.data = bytearray(raw)
                self.file_path = path
                self.update_all()
                self.set_status(f'已加载: {path}')
            except Exception as e:
                self.set_status(f'加载失败: {e}')
        dialog.destroy()

    def on_save(self, btn):
        if self.file_path:
            self._do_save(self.file_path)
        else:
            self.on_save_as(btn)

    def on_save_as(self, btn):
        dialog = Gtk.FileChooserDialog(
            title='保存映射文件', parent=self,
            action=Gtk.FileChooserAction.SAVE)
        dialog.add_button('取消', Gtk.ResponseType.CANCEL)
        dialog.add_button('保存', Gtk.ResponseType.OK)
        dialog.set_current_name('keymap.txt')

        filt_txt = Gtk.FileFilter()
        filt_txt.set_name('文本映射 (*.txt)')
        filt_txt.add_pattern('*.txt')
        dialog.add_filter(filt_txt)

        filt_bin = Gtk.FileFilter()
        filt_bin.set_name('二进制映射 (*.bin)')
        filt_bin.add_pattern('*.bin')
        dialog.add_filter(filt_bin)

        if dialog.run() == Gtk.ResponseType.OK:
            path = dialog.get_filename()
            self._do_save(path)
            self.file_path = path
        dialog.destroy()

    def _do_save(self, path):
        try:
            if path.endswith('.bin'):
                with open(path, 'wb') as f:
                    f.write(self.data)
            else:
                if not path.endswith('.txt'):
                    path += '.txt'
                with open(path, 'w') as f:
                    f.write('# XS40_CH552T 键位映射\n')
                    f.write('# 索引 修饰符 键码\n')
                    f.write('\n# mainKeyMap\n')
                    for i in range(ENTRIES):
                        mod, key = self.get_key(0, i)
                        f.write(f'{i:2d}    0x{mod:02x}   0x{key:02x}\n')
                    f.write('\n# Fn0_keyMap\n')
                    for i in range(ENTRIES):
                        mod, key = self.get_key(1, i)
                        f.write(f'{i+ENTRIES:2d}    0x{mod:02x}   0x{key:02x}\n')
            self.set_status(f'已保存: {path}')
        except Exception as e:
            self.set_status(f'保存失败: {e}')

    def on_write_device(self, btn):
        try:
            self.set_status('正在写入设备...')
            import threading

            def do_write():
                try:
                    km.send_keymap(bytes(self.data))
                    GLib.idle_add(self.set_status, '写入成功')
                except Exception as e:
                    GLib.idle_add(self.set_status, f'写入失败: {e}')

            t = threading.Thread(target=do_write, daemon=True)
            t.start()
        except Exception as e:
            self.set_status(f'写入失败: {e}')

    def on_read_device(self, btn):
        try:
            self.set_status('正在读取设备...')
            import threading

            def do_read():
                try:
                    data = km.read_keymap()
                    self.data = bytearray(data)
                    GLib.idle_add(self.update_all)
                    GLib.idle_add(self.set_status, '读取成功')
                except Exception as e:
                    GLib.idle_add(self.set_status, f'读取失败: {e}')

            t = threading.Thread(target=do_read, daemon=True)
            t.start()
        except Exception as e:
            self.set_status(f'读取失败: {e}')

    def on_reset(self, btn):
        dialog = Gtk.MessageDialog(
            parent=self, flags=Gtk.DialogFlags.MODAL,
            type=Gtk.MessageType.QUESTION,
            buttons=Gtk.ButtonsType.YES_NO,
            message_format='恢复默认键位映射？')
        dialog.format_secondary_text('当前修改将丢失。')
        if dialog.run() == Gtk.ResponseType.YES:
            self.data = bytearray(km.make_default_keymap())
            self.update_all()
            self.set_status('已恢复默认')
        dialog.destroy()


# =========================================================================
# CSS 样式
# =========================================================================

CSS = b"""
.key-normal, .key-mod, .key-fn, .key-none {
    min-width: 72px;
    min-height: 72px;
    font-size: 14px;
    padding: 0;
    border-radius: 8px;
}
.key-normal {
    background-color: #f0f0f0;
}
.key-mod {
    background-color: #c8e6ff;
}
.key-fn {
    background-color: #ffe0b0;
}
.key-none {
    background-color: #e0e0e0;
    color: #999;
}
.key-normal:hover, .key-mod:hover, .key-fn:hover, .key-none:hover {
    background-color: #b0d0ff;
}
"""


# =========================================================================
# 入口
# =========================================================================

def main():
    # 加载 CSS
    screen = Gdk.Screen.get_default()
    provider = Gtk.CssProvider()
    provider.load_from_data(CSS)
    Gtk.StyleContext.add_provider_for_screen(
        screen, provider, Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION)

    win = KeymapGUI()
    win.connect('destroy', Gtk.main_quit)
    Gtk.main()


if __name__ == '__main__':
    main()