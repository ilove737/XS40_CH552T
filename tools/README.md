# XS40_CH552T 键位映射配置工具

通过 USB 厂商自定义控制传输，向键盘发送键位映射数据，写入 Flash 0x3600 地址。

## 依赖

```bash
# 安装 pyusb（libusb Python 封装）
pip3 install --user --break-system-packages pyusb

# libusb 通常系统已自带
dpkg -l | grep libusb-1.0
```

## 权限

直接运行需要 root 权限：

```bash
sudo ./set_keymap.py write keymap.txt
```

或配置 udev 规则以免除 sudo：

```bash
# /etc/udev/rules.d/99-xs40-keyboard.rules
SUBSYSTEM=="usb", ATTR{idVendor}=="413d", ATTR{idProduct}=="2107", MODE="0666"
```

然后重新插拔键盘。

## 用法

```text
./set_keymap.py template [文件]       生成默认模板文件
./set_keymap.py check   <文件>        检查映射文件格式
./set_keymap.py write   <文件>        写入键位映射到设备（需 sudo）
./set_keymap.py write   <文件> --binary  写入二进制映射文件
./set_keymap.py dump    [文件]        从设备读取当前映射

## GUI 图形界面

```bash
# 依赖 GTK3
sudo apt-get install python3-gi

# 启动图形界面
./set_keymap_gui.py
```
```

## 文件格式

文本文件，每行一个键位定义：

```
# 格式: 索引 修饰符 键码  [# 注释]
# 索引 0-39 为 mainKeyMap（默认层），40-79 为 Fn0_keyMap（Fn层）
# 修饰符: 0=无, 0x01=LCTRL, 0x02=LSHIFT, 0x04=LALT, 0x08=LMETA
#          0x10=RCTRL, 0x20=RSHIFT, 0x40=RALT, 0x80=RMETA
# 多个修饰符可相加，如 0x03 = LCTRL+LSHIFT
# 键码: 十六进制 (0xNN) 或 KEY_ 名称 (ESC, A, F1, SPACE 等)
# 0xFF 0x00 = Fn 切换键

 0    0x00   0x29    # KEY_ESC
 1    0x00   0x1e    # KEY_1
 2    0x00   0x1f    # KEY_2
...
37    0x01   0x00    # LCTRL（单独左Ctrl）
38    0x01   0x06    # LCTRL + KEY_C（Ctrl+C）
39    0x01   0x19    # LCTRL + KEY_V（Ctrl+V）
---
40    0x00   0x29    # Fn层: ESC
41    0x00   0x3a    # Fn层: F1
...
```

首先生成模板再修改：

```bash
./set_keymap.py template mylayout.txt
vim mylayout.txt
sudo ./set_keymap.py write mylayout.txt
```

## 二进制格式

160 字节，按 mainKeyMap(80B) + Fn0_keyMap(80B) 顺序排列。
每个键位占用 2 字节：`[修饰符, 键码]`，共 40 键 × 2 层。

## 工作原理

主机通过 USB 控制传输发送厂商自定义请求：

```
bmRequestType = 0x41  (厂商, 主机→设备, 设备)
bRequest      = 0x91  (自定义: 设置键位映射)
wValue        = 0x0000
wIndex        = 0x0000
wLength       = 160
Data          = 160 字节键位数据
```

键盘固件收到后关中断，将数据写入 Flash 地址 0x3600（mainKeyMap）和 0x3650（Fn0_keyMap），然后开中断。写入完成后立即生效，无需重启。

## 注意事项

- 写入过程中键盘会短暂无响应（约 4ms）
- 写入后数据持久保存，断电不丢失
- 如果写入失败，检查 USB 连接和权限
- 0x3600 位于 BootLoader 区之前，不会影响 ISP 下载