// keymap.js
// XS40_CH552T 键位映射核心逻辑（移植自 tools/set_keymap.py）
// 纯前端版本：键码表、修饰符、解析/格式化、默认布局、显示辅助。

// USB HID 设备标识与键位数据布局
export const VID = 0x413D;
export const PID = 0x2107;
export const REPORT_ID = 0x01;     // 厂商 Feature Report ID
export const KEYMAP_SIZE = 160;    // mainKeyMap(80) + Fn0_keyMap(80)
export const KEY_ENTRIES = 40;     // 每层 40 键
export const LAYER_SIZE = 80;      // 每层 80 字节（40 键 * 2）

// =========================================================================
// USB HID 键码表（Usage ID）
// =========================================================================
export const KEY_NAMES = {
  NONE: 0x00, ERR_OVF: 0x01,
  A: 0x04, B: 0x05, C: 0x06, D: 0x07, E: 0x08, F: 0x09,
  G: 0x0a, H: 0x0b, I: 0x0c, J: 0x0d, K: 0x0e, L: 0x0f,
  M: 0x10, N: 0x11, O: 0x12, P: 0x13, Q: 0x14, R: 0x15,
  S: 0x16, T: 0x17, U: 0x18, V: 0x19, W: 0x1a, X: 0x1b,
  Y: 0x1c, Z: 0x1d,
  '1': 0x1e, '2': 0x1f, '3': 0x20, '4': 0x21, '5': 0x22,
  '6': 0x23, '7': 0x24, '8': 0x25, '9': 0x26, '0': 0x27,
  ENTER: 0x28, ESC: 0x29, BACKSPACE: 0x2a, TAB: 0x2b,
  SPACE: 0x2c, MINUS: 0x2d, EQUAL: 0x2e,
  LEFTBRACE: 0x2f, RIGHTBRACE: 0x30, BACKSLASH: 0x31,
  HASHTILDE: 0x32, SEMICOLON: 0x33, APOSTROPHE: 0x34,
  GRAVE: 0x35, COMMA: 0x36, DOT: 0x37, SLASH: 0x38,
  CAPSLOCK: 0x39,
  F1: 0x3a, F2: 0x3b, F3: 0x3c, F4: 0x3d, F5: 0x3e,
  F6: 0x3f, F7: 0x40, F8: 0x41, F9: 0x42, F10: 0x43,
  F11: 0x44, F12: 0x45,
  SYSRQ: 0x46, SCROLLLOCK: 0x47, PAUSE: 0x48,
  INSERT: 0x49, HOME: 0x4a, PAGEUP: 0x4b, DELETE: 0x4c,
  END: 0x4d, PAGEDOWN: 0x4e,
  RIGHT: 0x4f, LEFT: 0x50, DOWN: 0x51, UP: 0x52,
  NUMLOCK: 0x53, KP_SLASH: 0x54, KP_ASTERISK: 0x55,
  KP_MINUS: 0x56, KP_PLUS: 0x57, KP_ENTER: 0x58,
  KP1: 0x59, KP2: 0x5a, KP3: 0x5b, KP4: 0x5c,
  KP5: 0x5d, KP6: 0x5e, KP7: 0x5f, KP8: 0x60,
  KP9: 0x61, KP0: 0x62, KP_DOT: 0x63,
  LEFTCTRL: 0xe0, LEFTSHIFT: 0xe1, LEFTALT: 0xe2,
  LEFTMETA: 0xe3, RIGHTCTRL: 0xe4, RIGHTSHIFT: 0xe5,
  RIGHTALT: 0xe6, RIGHTMETA: 0xe7,
};

// 修饰符位（单字节内的 8 个标志位）
export const MOD = {
  LCTRL: 0x01, LSHIFT: 0x02, LALT: 0x04, LMETA: 0x08,
  RCTRL: 0x10, RSHIFT: 0x20, RALT: 0x40, RMETA: 0x80,
};

// 修饰符顺序（用于复选框布局）
export const MOD_LIST = [
  ['LCTRL', 0x01], ['LSHIFT', 0x02], ['LALT', 0x04], ['LMETA', 0x08],
  ['RCTRL', 0x10], ['RSHIFT', 0x20], ['RALT', 0x40], ['RMETA', 0x80],
];

// 显示用短名（移植自 GUI 的 SHORT_NAMES）
export const SHORT_NAMES = {
  NONE: '', ERR_OVF: 'OVF',
  LEFTCTRL: 'LCtrl', LEFTSHIFT: 'LShift', LEFTALT: 'LAlt', LEFTMETA: 'LWin',
  RIGHTCTRL: 'RCtrl', RIGHTSHIFT: 'RShift', RIGHTALT: 'RAlt', RIGHTMETA: 'RWin',
  ENTER: 'Enter', BACKSPACE: 'BkSpc', CAPSLOCK: 'Caps', SCROLLLOCK: 'Scrlk',
  NUMLOCK: 'NumLk', SPACE: 'Space', TAB: 'Tab', ESC: 'Esc',
  DELETE: 'Del', INSERT: 'Ins', HOME: 'Home', END: 'End',
  PAGEUP: 'PgUp', PAGEDOWN: 'PgDn', SYSRQ: 'PrtSc', PAUSE: 'Pause',
  LEFTBRACE: '[', RIGHTBRACE: ']', BACKSLASH: '\\', HASHTILDE: '#~',
  SEMICOLON: ';', APOSTROPHE: "'", GRAVE: '`', COMMA: ',', DOT: '.',
  SLASH: '/', MINUS: '-', EQUAL: '=',
  UP: '↑', DOWN: '↓', LEFT: '←', RIGHT: '→',
  KP_SLASH: 'KP/', KP_ASTERISK: 'KP*', KP_MINUS: 'KP-',
  KP_PLUS: 'KP+', KP_ENTER: 'KPE', KP_DOT: 'KP.',
};

// =========================================================================
// 解析 / 格式化
// =========================================================================

// 名称或十六进制字符串 → HID 键码（数字）。非法返回 null。
export function parseKey(name) {
  if (name == null) return null;
  name = String(name).trim().toUpperCase();
  if (name.startsWith('KEY_')) name = name.slice(4);
  if (name.startsWith('0X')) return parseInt(name, 16);
  if (name in KEY_NAMES) return KEY_NAMES[name];
  // 单个字符（如 'A'）
  if (name.length === 1 && name in KEY_NAMES) return KEY_NAMES[name];
  try {
    const v = parseInt(name, 0);
    if (!isNaN(v)) return v & 0xff;
  } catch (e) { /* ignore */ }
  return null;
}

export function parseMod(val) {
  if (val == null) return null;
  val = String(val).trim().toUpperCase();
  if (val in MOD) return MOD[val];
  if (val.startsWith('0X')) return parseInt(val, 16);
  try {
    const v = parseInt(val, 0);
    if (!isNaN(v)) return v & 0xff;
  } catch (e) { /* ignore */ }
  return null;
}

// 键码 → 名称（如 'KEY_A' 或 '0x04' 形式的 HEX）
export function keycodeName(code) {
  for (const [name, val] of Object.entries(KEY_NAMES)) {
    if (val === code) return `KEY_${name}`;
  }
  return `0x${(code & 0xff).toString(16).padStart(2, '0')}`;
}

// 修饰符位 → 名称串（如 'LCTRL+LSHIFT'，0 返回 '0'）
export function modName(mod) {
  const parts = [];
  for (const [name, val] of Object.entries(MOD)) {
    if (mod & val) parts.push(name);
  }
  return parts.length ? parts.join('+') : '0';
}

// 键码 → 短名（用于网格显示）
export function shortName(code) {
  if (code === 0) return '';
  for (const [name, val] of Object.entries(KEY_NAMES)) {
    if (val === code) {
      if (name in SHORT_NAMES) return SHORT_NAMES[name];
      if (name.length <= 5) return name;
      if (name.startsWith('KP_')) return name.slice(3);
      return name.slice(0, 4);
    }
  }
  return `0x${(code & 0xff).toString(16).padStart(2, '0')}`;
}

// 修饰符位 → 前缀（每个修饰符一行，用换行分隔）
export function modPrefix(mod) {
  const parts = [];
  if (mod & 0x01) parts.push('LCtrl');
  if (mod & 0x02) parts.push('LShift');
  if (mod & 0x04) parts.push('LAlt');
  if (mod & 0x08) parts.push('LWin');
  if (mod & 0x10) parts.push('RCtrl');
  if (mod & 0x20) parts.push('RShift');
  if (mod & 0x40) parts.push('RAlt');
  if (mod & 0x80) parts.push('RWin');
  return parts.join('\n');
}

// 默认键位（main + Fn0），返回 Uint8Array(160)
export function makeDefaultKeymap() {
  const main = [
    [0, 0x29], [0, 0x1e], [0, 0x1f], [0, 0x20], [0, 0x21], [0, 0x22], [0, 0x35], [2, 0x25],
    [0, 0x2b], [0, 0x14], [0, 0x1a], [0, 0x08], [0, 0x15], [0, 0x17], [0, 0x2f], [0, 0x30],
    [0, 0x39], [0, 0x04], [0, 0x16], [0, 0x07], [0, 0x09], [0, 0x0a], [2, 0x26], [2, 0x27],
    [2, 0x00], [2, 0x1f], [0, 0x1d], [0, 0x1b], [0, 0x06], [0, 0x19], [2, 0x24], [2, 0x20],
    [0xff, 0x00], [8, 0x00], [4, 0x00], [0xff, 0x00], [0, 0x2c], [1, 0x00], [1, 0x06], [1, 0x19],
  ];
  const fn0 = [
    [0, 0x29], [0, 0x3a], [0, 0x3b], [0, 0x3c], [0, 0x3d], [0, 0x3e], [0, 0x35], [2, 0x25],
    [0, 0x2b], [0, 0x44], [0, 0x45], [0, 0x08], [0, 0x15], [0, 0x17], [0, 0x2f], [0, 0x30],
    [0, 0x39], [0, 0x04], [0, 0x16], [0, 0x07], [0, 0x09], [0, 0x0a], [2, 0x26], [2, 0x27],
    [2, 0x00], [2, 0x1f], [0, 0x1d], [0, 0x1b], [0, 0x06], [0, 0x19], [2, 0x24], [2, 0x20],
    [0xff, 0x00], [8, 0x00], [4, 0x00], [0xff, 0x00], [0, 0x2c], [1, 0x00], [2, 0x35], [2, 0x1e],
  ];
  const raw = new Uint8Array(KEYMAP_SIZE);
  let off = 0;
  for (const [m, k] of main.concat(fn0)) {
    raw[off++] = m & 0xff;
    raw[off++] = k & 0xff;
  }
  return raw;
}

// 解析文本映射文件 → Uint8Array(160)。格式: 索引 修饰符 键码 [# 注释]
// 索引 0-39 为 mainKeyMap，40-79 为 Fn0_keyMap。
export function readKeymapText(text) {
  const entries = {};
  const lines = text.split(/\r?\n/);
  lines.forEach((line, lineno) => {
    line = line.trim();
    if (!line || line.startsWith('#')) return;
    const parts = line.split('#')[0].trim().split(/\s+/);
    if (parts.length < 3) {
      throw new Error(`格式错误 行 ${lineno + 1}: 需要 "索引 修饰符 键码"`);
    }
    const idx = parseInt(parts[0], 0);
    if (isNaN(idx) || idx < 0 || idx >= KEY_ENTRIES * 2) {
      throw new Error(`索引越界 行 ${lineno + 1}: ${parts[0]}`);
    }
    const mod = parseMod(parts[1]);
    if (mod === null) throw new Error(`修饰符格式错误 行 ${lineno + 1}: ${parts[1]}`);
    const key = parseKey(parts[2]);
    if (key === null) throw new Error(`键码格式错误 行 ${lineno + 1}: ${parts[2]}`);
    entries[idx] = [mod & 0xff, key & 0xff];
  });
  const raw = new Uint8Array(KEYMAP_SIZE);
  for (let i = 0; i < KEY_ENTRIES * 2; i++) {
    const [m, k] = entries[i] || [0, 0];
    raw[i * 2] = m;
    raw[i * 2 + 1] = k;
  }
  return raw;
}

// 生成文本映射文件（与 set_keymap.py write_template 兼容）
export function formatKeymapText(data) {
  const main = data.slice(0, LAYER_SIZE);
  const fn0 = data.slice(LAYER_SIZE);
  let out = '';
  out += '# XS40_CH552T 键位映射配置文件\n';
  out += '# 格式: 索引 修饰符 键码  [# 注释]\n';
  out += `# 索引 0-${KEY_ENTRIES - 1} mainKeyMap, ${KEY_ENTRIES}-${KEY_ENTRIES * 2 - 1} Fn0_keyMap\n`;
  out += '# 修饰符: 0=无, 1=LCTRL, 2=LSHIFT, 4=LALT, 8=LMETA, 0x10=RCTRL, 0x20=RSHIFT, 0x40=RALT, 0x80=RMETA\n';
  out += '# 键码: 见 USB HID Usage Table，或用 KEY_ 名称\n';
  out += '# 0xFF修饰符+0x00键码 = Fn 切换键\n\n';
  out += '# ===== mainKeyMap =====\n';
  for (let i = 0; i < KEY_ENTRIES; i++) {
    const m = main[i * 2];
    const k = main[i * 2 + 1];
    out += `${String(i).padStart(2, ' ')}    0x${m.toString(16).padStart(2, '0')}   ` +
           `${keycodeName(k).padEnd(12, ' ')}  # (${modName(m)})\n`;
  }
  out += '\n# ===== Fn0_keyMap =====\n';
  for (let i = 0; i < KEY_ENTRIES; i++) {
    const m = fn0[i * 2];
    const k = fn0[i * 2 + 1];
    out += `${String(i + KEY_ENTRIES).padStart(2, ' ')}    0x${m.toString(16).padStart(2, '0')}   ` +
           `${keycodeName(k).padEnd(12, ' ')}  # (${modName(m)})\n`;
  }
  return out;
}

// 从产品字符串解析左右手: 'XS40 R Keyboard' -> 'R'
export function detectHandFromString(productString) {
  const ps = (productString || '').toUpperCase();
  const m = ps.match(/XS40\s+([LR])\b/);
  return m ? m[1] : null;
}

// 层 → 该层 80 字节视图
export function getLayerData(data, layer) {
  const off = layer * LAYER_SIZE;
  return data.slice(off, off + LAYER_SIZE);
}

export function setLayerData(data, layer, layerData) {
  const off = layer * LAYER_SIZE;
  data.set(layerData.subarray(0, LAYER_SIZE), off);
}

export function getKey(data, layer, idx) {
  const off = layer * LAYER_SIZE + idx * 2;
  return [data[off], data[off + 1]];
}

export function setKey(data, layer, idx, mod, key) {
  const off = layer * LAYER_SIZE + idx * 2;
  data[off] = mod & 0xff;
  data[off + 1] = key & 0xff;
}
