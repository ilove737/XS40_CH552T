#!/usr/bin/env python3
"""
XS40_CH552T 键位映射配置工具 - CLI
通过 USB 厂商自定义请求（0x91）向键盘发送键位映射数据。

用法:
  ./set_keymap.py template                  # 生成默认模板文件
  ./set_keymap.py write keymap.txt           # 写入键位映射到设备
  ./set_keymap.py write keymap.bin --binary  # 写入二进制映射文件
  ./set_keymap.py check keymap.txt           # 检查映射文件格式
  ./set_keymap_gui.py                        # 启动图形界面

依赖: pyusb, libusb-1.0
需要 root 权限或 udev 规则访问 USB 设备。
"""

import argparse
import os
import sys

# =========================================================================
# USB HID 键码表
# =========================================================================
KEY_NAMES = {
    'NONE': 0x00, 'ERR_OVF': 0x01,
    'A': 0x04, 'B': 0x05, 'C': 0x06, 'D': 0x07, 'E': 0x08, 'F': 0x09,
    'G': 0x0a, 'H': 0x0b, 'I': 0x0c, 'J': 0x0d, 'K': 0x0e, 'L': 0x0f,
    'M': 0x10, 'N': 0x11, 'O': 0x12, 'P': 0x13, 'Q': 0x14, 'R': 0x15,
    'S': 0x16, 'T': 0x17, 'U': 0x18, 'V': 0x19, 'W': 0x1a, 'X': 0x1b,
    'Y': 0x1c, 'Z': 0x1d,
    '1': 0x1e, '2': 0x1f, '3': 0x20, '4': 0x21, '5': 0x22,
    '6': 0x23, '7': 0x24, '8': 0x25, '9': 0x26, '0': 0x27,
    'ENTER': 0x28, 'ESC': 0x29, 'BACKSPACE': 0x2a, 'TAB': 0x2b,
    'SPACE': 0x2c, 'MINUS': 0x2d, 'EQUAL': 0x2e,
    'LEFTBRACE': 0x2f, 'RIGHTBRACE': 0x30, 'BACKSLASH': 0x31,
    'HASHTILDE': 0x32, 'SEMICOLON': 0x33, 'APOSTROPHE': 0x34,
    'GRAVE': 0x35, 'COMMA': 0x36, 'DOT': 0x37, 'SLASH': 0x38,
    'CAPSLOCK': 0x39,
    'F1': 0x3a, 'F2': 0x3b, 'F3': 0x3c, 'F4': 0x3d, 'F5': 0x3e,
    'F6': 0x3f, 'F7': 0x40, 'F8': 0x41, 'F9': 0x42, 'F10': 0x43,
    'F11': 0x44, 'F12': 0x45,
    'SYSRQ': 0x46, 'SCROLLLOCK': 0x47, 'PAUSE': 0x48,
    'INSERT': 0x49, 'HOME': 0x4a, 'PAGEUP': 0x4b, 'DELETE': 0x4c,
    'END': 0x4d, 'PAGEDOWN': 0x4e,
    'RIGHT': 0x4f, 'LEFT': 0x50, 'DOWN': 0x51, 'UP': 0x52,
    'NUMLOCK': 0x53, 'KP_SLASH': 0x54, 'KP_ASTERISK': 0x55,
    'KP_MINUS': 0x56, 'KP_PLUS': 0x57, 'KP_ENTER': 0x58,
    'KP1': 0x59, 'KP2': 0x5a, 'KP3': 0x5b, 'KP4': 0x5c,
    'KP5': 0x5d, 'KP6': 0x5e, 'KP7': 0x5f, 'KP8': 0x60,
    'KP9': 0x61, 'KP0': 0x62, 'KP_DOT': 0x63,
    'LEFTCTRL': 0xe0, 'LEFTSHIFT': 0xe1, 'LEFTALT': 0xe2,
    'LEFTMETA': 0xe3, 'RIGHTCTRL': 0xe4, 'RIGHTSHIFT': 0xe5,
    'RIGHTALT': 0xe6, 'RIGHTMETA': 0xe7,
}

MOD = {
    'LCTRL': 0x01, 'LSHIFT': 0x02, 'LALT': 0x04, 'LMETA': 0x08,
    'RCTRL': 0x10, 'RSHIFT': 0x20, 'RALT': 0x40, 'RMETA': 0x80,
}

RED = '\033[91m'
GREEN = '\033[92m'
YELLOW = '\033[93m'
BOLD = '\033[1m'
RESET = '\033[0m'

VID = 0x413D
PID = 0x2107
VENDOR_SET_KEYMAP = 0x91
KEYMAP_SIZE = 160
KEY_ENTRIES = 40


def parse_key(name):
    name = name.strip().upper()
    if name.startswith('KEY_'):
        name = name[4:]
    if name.startswith('0X'):
        return int(name, 16)
    if name in KEY_NAMES:
        return KEY_NAMES[name]
    try:
        return int(name, 0)
    except ValueError:
        return None


def parse_mod(val):
    val = val.strip().upper()
    if val in MOD:
        return MOD[val]
    if val.startswith('0X'):
        return int(val, 16)
    try:
        return int(val, 0)
    except ValueError:
        return None


def make_default_keymap():
    main = [
        (0, 0x29), (0, 0x1e), (0, 0x1f), (0, 0x20), (0, 0x21), (0, 0x22), (0, 0x35), (2, 0x25),
        (0, 0x2b), (0, 0x14), (0, 0x1a), (0, 0x08), (0, 0x15), (0, 0x17), (0, 0x2f), (0, 0x30),
        (0, 0x39), (0, 0x04), (0, 0x16), (0, 0x07), (0, 0x09), (0, 0x0a), (2, 0x26), (2, 0x27),
        (2, 0x00), (2, 0x1f), (0, 0x1d), (0, 0x1b), (0, 0x06), (0, 0x19), (2, 0x24), (2, 0x20),
        (0xFF, 0x00), (8, 0x00), (4, 0x00), (0xFF, 0x00), (0, 0x2c), (1, 0x00), (1, 0x06), (1, 0x19),
    ]
    fn0 = [
        (0, 0x29), (0, 0x3a), (0, 0x3b), (0, 0x3c), (0, 0x3d), (0, 0x3e), (0, 0x35), (2, 0x25),
        (0, 0x2b), (0, 0x44), (0, 0x45), (0, 0x08), (0, 0x15), (0, 0x17), (0, 0x2f), (0, 0x30),
        (0, 0x39), (0, 0x04), (0, 0x16), (0, 0x07), (0, 0x09), (0, 0x0a), (2, 0x26), (2, 0x27),
        (2, 0x00), (2, 0x1f), (0, 0x1d), (0, 0x1b), (0, 0x06), (0, 0x19), (2, 0x24), (2, 0x20),
        (0xFF, 0x00), (8, 0x00), (4, 0x00), (0xFF, 0x00), (0, 0x2c), (1, 0x00), (2, 0x35), (2, 0x1e),
    ]
    raw = bytearray()
    for mod, key in main + fn0:
        raw.extend([mod, key])
    return bytes(raw)


def keycode_name(code):
    for name, val in KEY_NAMES.items():
        if val == code:
            return f'KEY_{name}'
    return f'0x{code:02x}'


def mod_name(mod):
    parts = []
    for name, val in MOD.items():
        if mod & val:
            parts.append(name)
    return '+'.join(parts) if parts else '0'


def format_keymap(data, title=''):
    lines = []
    if title:
        lines.append(f'{BOLD}# {title}{RESET}')
    lines.append('# 索引  修饰符  键码      注释')
    lines.append('# ------ ------  --------  -------------------------')
    for i in range(KEY_ENTRIES):
        mod = data[i * 2]
        key = data[i * 2 + 1]
        m = mod_name(mod)
        k = keycode_name(key)
        lines.append(f'  {i:2d}    0x{mod:02x}   0x{key:02x}    ({m:>8s}, {k})')
    return '\n'.join(lines)


def read_keymap_file(path):
    entries = {}
    with open(path) as f:
        for lineno, line in enumerate(f, 1):
            line = line.strip()
            if not line or line.startswith('#'):
                continue
            parts = line.split('#')[0].strip().split()
            if len(parts) < 3:
                print(f'{RED}格式错误{RESET} {path}:{lineno}: 需要 索引 修饰符 键码')
                sys.exit(1)
            try:
                idx = int(parts[0], 0)
            except ValueError:
                print(f'{RED}索引格式错误{RESET} {path}:{lineno}: {parts[0]}')
                sys.exit(1)
            if idx < 0 or idx >= KEY_ENTRIES * 2:
                print(f'{RED}索引越界{RESET} {path}:{lineno}: {idx}')
                sys.exit(1)
            mod = parse_mod(parts[1])
            if mod is None:
                print(f'{RED}修饰符格式错误{RESET} {path}:{lineno}: {parts[1]}')
                sys.exit(1)
            key = parse_key(parts[2])
            if key is None:
                print(f'{RED}键码格式错误{RESET} {path}:{lineno}: {parts[2]}')
                sys.exit(1)
            entries[idx] = (mod, key)
    raw = bytearray()
    for i in range(KEY_ENTRIES * 2):
        mod, key = entries.get(i, (0, 0))
        raw.extend([mod, key])
    return bytes(raw)


def check_keymap(path):
    data = read_keymap_file(path)
    main = data[:KEYMAP_SIZE // 2]
    fn0 = data[KEYMAP_SIZE // 2:]
    print(f'{GREEN}格式正确{RESET}')
    print()
    print(format_keymap(main, 'mainKeyMap'))
    print()
    print(format_keymap(fn0, 'Fn0_keyMap'))


def write_template(path):
    data = make_default_keymap()
    main = data[:KEYMAP_SIZE // 2]
    fn0 = data[KEYMAP_SIZE // 2:]
    with open(path, 'w') as f:
        f.write('# XS40_CH552T 键位映射配置文件\n')
        f.write('# 格式: 索引 修饰符 键码  [# 注释]\n')
        f.write(f'# 索引 0-{KEY_ENTRIES-1} mainKeyMap, {KEY_ENTRIES}-{KEY_ENTRIES*2-1} Fn0_keyMap\n')
        f.write('# 修饰符: 0=无, 1=LCTRL, 2=LSHIFT, 4=LALT, 8=LMETA, 0x10=RCTRL, 0x20=RSHIFT, 0x40=RALT, 0x80=RMETA\n')
        f.write('# 键码: 见 USB HID Usage Table，或用 KEY_ 名称\n')
        f.write('# 0xFF修饰符+0x00键码 = Fn 切换键\n\n')
        f.write('# ===== mainKeyMap =====\n')
        for i in range(KEY_ENTRIES):
            mod = main[i * 2]
            key = main[i * 2 + 1]
            f.write(f'{i:2d}    0x{mod:02x}   {keycode_name(key):<12s}  # ({mod_name(mod)})\n')
        f.write('\n# ===== Fn0_keyMap =====\n')
        for i in range(KEY_ENTRIES):
            mod = fn0[i * 2]
            key = fn0[i * 2 + 1]
            f.write(f'{i+KEY_ENTRIES:2d}    0x{mod:02x}   {keycode_name(key):<12s}  # ({mod_name(mod)})\n')
    print(f'{GREEN}已生成模板{RESET} {path}')


def send_keymap(data):
    import usb.core
    import usb.util
    dev = usb.core.find(idVendor=VID, idProduct=PID)
    if dev is None:
        print(f'{RED}未找到设备{RESET}')
        sys.exit(1)
    if dev.is_kernel_driver_active(0):
        dev.detach_kernel_driver(0)
    try:
        dev.set_configuration()
        ret = dev.ctrl_transfer(0x41, VENDOR_SET_KEYMAP, 0, 0, data, timeout=5000)
        if ret == len(data):
            print(f'{GREEN}写入成功{RESET} ({ret} 字节)')
        else:
            print(f'{RED}写入失败{RESET}')
    except usb.core.USBError as e:
        print(f'{RED}USB 错误{RESET}: {e}')
    finally:
        usb.util.dispose_resources(dev)


# =========================================================================
# 主入口
# =========================================================================

def main():
    parser = argparse.ArgumentParser(
        description='XS40_CH552T 键位映射配置工具')
    parser.add_argument('action', nargs='?',
                        choices=['template', 'write', 'check'],
                        help='操作: template(生成模板), write(写入), check(检查)')
    parser.add_argument('file', nargs='?', help='文件名')
    parser.add_argument('--binary', action='store_true',
                        help='以二进制格式读写文件')
    args = parser.parse_args()

    if not args.action:
        parser.print_help()
        return

    if args.action == 'template':
        write_template(args.file or 'keymap.txt')

    elif args.action == 'check':
        if not args.file:
            print(f'{RED}请指定映射文件{RESET}')
            sys.exit(1)
        check_keymap(args.file)

    elif args.action == 'write':
        if not args.file:
            print(f'{RED}请指定映射文件{RESET}')
            sys.exit(1)
        if args.binary:
            with open(args.file, 'rb') as f:
                data = f.read()
            if len(data) != KEYMAP_SIZE:
                print(f'{RED}文件大小错误{RESET}: {len(data)}B')
                sys.exit(1)
        else:
            data = read_keymap_file(args.file)
        print(f'查找设备 {VID:04X}:{PID:04X}...')
        send_keymap(data)


if __name__ == '__main__':
    main()