// app.js
// XS40_CH552T 网页版主逻辑：树形(设备/层) + 5×8 网格 + 编辑弹窗 + 按键捕获 + 文件导入导出 + 多设备缓存。
import * as km from './keymap.js';
import * as hid from './hid.js';
import { KeyCapture } from './capture.js';

const ROWS = 5;
const COLS = 8;

// ---- 全局状态 ----
const state = {
  devices: [],                 // 已连接 HIDDevice[]
  deviceData: new Map(),       // device -> Uint8Array(160) 各键盘配置缓存
  currentDevice: null,         // HIDDevice | null
  currentLayer: 0,             // 0=mainKeyMap, 1=Fn0_keyMap
  data: km.makeDefaultKeymap(),// 当前显示/编辑的 160 字节
  editIdx: -1,
};
let capture = null;            // KeyCapture 实例（弹窗捕获时非 null）

// ---- DOM 工具 ----
const $ = (id) => document.getElementById(id);
function setStatus(msg) { $('statusbar').textContent = msg; }
function keyNameOf(code) {
  for (const [n, v] of Object.entries(km.KEY_NAMES)) if (v === code) return n;
  return '';
}

// ---- 初始化 ----
function init() {
  if (!hid.isWebHidSupported()) {
    setStatus('当前浏览器不支持 WebHID，请使用 Chrome / Edge，并通过 https 或 localhost 访问。');
    ['btnConnect', 'btnRead', 'btnWrite', 'btnRefresh'].forEach((id) => { $(id).disabled = true; });
  }
  buildModCheckboxes();
  buildKeynameDatalist();
  renderGrid();
  bindEvents();
}

function buildModCheckboxes() {
  const grid = $('modGrid');
  grid.innerHTML = '';
  for (const [name, val] of km.MOD_LIST) {
    const label = document.createElement('label');
    const cb = document.createElement('input');
    cb.type = 'checkbox';
    cb.id = 'mod_' + name;
    cb.value = val;
    label.append(cb, document.createTextNode(name));
    grid.append(label);
  }
}

function buildKeynameDatalist() {
  const dl = $('keynamelist');
  dl.innerHTML = '';
  for (const name of Object.keys(km.KEY_NAMES)) {
    const opt = document.createElement('option');
    opt.value = name;
    dl.append(opt);
  }
  for (const name of Object.keys(km.MOUSE_NAMES)) {
    const opt = document.createElement('option');
    opt.value = name;
    dl.append(opt);
  }
}

// 设备稳定标识（按厂商/产品/序列号/名称/用途，过滤掉多余接口导致的重复）
function devKey(dev) {
  return `${dev.vendorId}:${dev.productId}:${dev.serialNumber || ''}:` +
         `${dev.productName || ''}:${dev.usagePage || 0}:${dev.usage || 0}`;
}
function hasDevice(dev) {
  const k = devKey(dev);
  return state.devices.some((d) => devKey(d) === k);
}

// 左右手判定：固件把 "XS40 L/R Keyboard" 放在 iSerialNumber，
// productName 恒为 "CH552"，必须用 serialNumber 区分左右。
function deviceHand(dev) {
  return km.detectHandFromString(dev.serialNumber) ||
         km.detectHandFromString(dev.productName) || null;
}
// 设备显示名：优先 serialNumber（含 XS40 L/R），否则 productName。
function deviceLabel(dev) {
  return dev.serialNumber || dev.productName || 'XS40';
}

// ---- 设备连接 / 枚举 ----
async function onConnect() {
  try {
    setStatus('请求授权…（请在弹窗中同时勾选左手、右手两个条目）');
    const devs = await hid.requestDevice();
    if (!devs.length) { setStatus('已取消授权或未选择设备'); return; }
    let ok = 0, fail = 0;
    for (const dev of devs) {
      try {
        await hid.openDevice(dev);
        if (!hasDevice(dev)) state.devices.push(dev);
        ok++;
      } catch (e) {
        fail++;
        console.warn('打开设备失败:', dev, e);
      }
    }
    refreshTree();
    updateConnBadge();
    if (fail) {
      setStatus(`已连接 ${ok} 个键盘，但 ${fail} 个打开失败：请重新点「连接键盘」并在系统弹窗中勾选该设备。`);
    } else if (ok) {
      setStatus(`已连接 ${ok} 个键盘（左手 + 右手）`);
    } else {
      setStatus('未连接任何设备，请点「连接键盘」授权。');
    }
  } catch (e) {
    setStatus('连接失败: ' + e.message);
  }
}

function addDevice(dev) {
  if (!hasDevice(dev)) {
    if (!dev.opened) dev.open().catch(() => {});
    state.devices.push(dev);
  }
  refreshTree();
  updateConnBadge();
}

async function onRefresh() {
  try {
    const paired = await hid.getPairedDevices();
    let ok = 0, fail = 0;
    for (const d of paired) {
      if (hasDevice(d)) { ok++; continue; }
      try {
        await hid.openDevice(d);
        state.devices.push(d);
        ok++;
      } catch (e) {
        fail++;
        console.warn('打开设备失败:', d, e);
      }
    }
    refreshTree();
    updateConnBadge();
    if (state.devices.length === 0) {
      setStatus('未检测到已配对的 XS40 键盘，请点「连接键盘」授权。');
    } else if (fail) {
      setStatus(`已枚举 ${ok} 个键盘，但 ${fail} 个打开失败：重新点「连接键盘」并勾选该设备。`);
    } else {
      setStatus(`已枚举 ${ok} 个键盘`);
    }
  } catch (e) {
    setStatus('枚举失败: ' + e.message);
  }
}

function updateConnBadge() {
  const badge = $('connStatus');
  if (state.devices.length > 0) {
    badge.textContent = `已连接 ${state.devices.length}`;
    badge.className = 'status-badge on';
  } else {
    badge.textContent = '未连接';
    badge.className = 'status-badge off';
  }
}

// ---- 树形（设备 → 层）----
function refreshTree() {
  const tree = $('deviceTree');
  tree.innerHTML = '';
  if (state.devices.length === 0) {
    $('sideHint').hidden = false;
    return;
  }
  $('sideHint').hidden = true;

  state.devices.forEach((dev) => {
    const hand = deviceHand(dev);
    const hl = hand === 'L' ? '左手(L)' : hand === 'R' ? '右手(R)' : '未知';
    const devLi = document.createElement('li');
    devLi.className = 'tree-dev';
    devLi.dataset.kind = 'dev';
    devLi.textContent = `${hl}  ${deviceLabel(dev)}`;
    devLi.onclick = () => selectDevice(dev, 0);

    const ul = document.createElement('ul');
    const li0 = document.createElement('li');
    li0.className = 'tree-layer';
    li0.textContent = 'mainKeyMap (主层)';
    li0.onclick = (ev) => { ev.stopPropagation(); selectDevice(dev, 0); };
    const li1 = document.createElement('li');
    li1.className = 'tree-layer';
    li1.textContent = 'Fn0_keyMap (Fn层)';
    li1.onclick = (ev) => { ev.stopPropagation(); selectDevice(dev, 1); };
    ul.append(li0, li1);
    devLi.append(ul);
    tree.append(devLi);
  });
  highlightCurrent();
}

function highlightCurrent() {
  document.querySelectorAll('.tree-dev').forEach((devLi, i) => {
    const dev = state.devices[i];
    const isCur = dev === state.currentDevice;
    devLi.classList.toggle('active', isCur);
    devLi.querySelectorAll('.tree-layer').forEach((li, layer) => {
      li.classList.toggle('active', isCur && layer === state.currentLayer);
    });
  });
}

// ---- 选择设备 / 层 ----
async function selectDevice(dev, layer) {
  const prevDev = state.currentDevice;
  const prevLayer = state.currentLayer;
  // 保存上一设备编辑
  if (prevDev && state.deviceData.has(prevDev)) {
    state.deviceData.set(prevDev, new Uint8Array(state.data));
  }
  state.currentDevice = dev;
  state.currentLayer = layer;

  if (state.deviceData.has(dev)) {
    state.data = new Uint8Array(state.deviceData.get(dev));
    afterLoad();
    setStatus(`已载入 ${deviceLabel(dev)} 的 ${layer ? 'Fn0' : '主'}层`);
  } else {
    setStatus(`正在读取 ${deviceLabel(dev)}...`);
    try {
      const data = await hid.readKeymap(dev);
      state.deviceData.set(dev, new Uint8Array(data));
      state.data = new Uint8Array(data);
      afterLoad();
      setStatus(`已读取 ${deviceLabel(dev)}`);
    } catch (e) {
      // 读取失败：不污染缓存（避免把上一台的数据错存到本台），
      // 回退到上一台设备的显示，并提示错误。
      state.currentDevice = prevDev;
      state.currentLayer = prevLayer;
      afterLoad();
      setStatus('读取失败: ' + e.message + '（保持上一键盘显示）');
    }
  }
  highlightCurrent();
}

function afterLoad() {
  $('layerTitle').textContent = state.currentLayer ? 'Fn0_keyMap (Fn层)' : 'mainKeyMap (主层)';
  updateHandLabel();
  renderGrid();
}

function updateHandLabel() {
  const dev = state.currentDevice;
  const hand = dev ? deviceHand(dev) : null;
  const lbl = $('handLabel');
  if (hand === 'L') lbl.textContent = 'L 左手';
  else if (hand === 'R') lbl.textContent = 'R 右手';
  else lbl.textContent = '? 未检测';
}

// ---- 网格渲染 ----
function keyDisplay(mod, key) {
  if (mod === 0xff && key === 0x00) return { text: 'Fn0', cls: 'key-fn' };
  if (km.isMouseAction(mod)) {
    const name = km.mouseShortName(key);
    return { text: '🖱' + (name ? '\n' + name : ''), cls: 'key-mouse' };
  }
  const prefix = km.modPrefix(mod);
  const name = km.shortName(key);
  let text;
  if (prefix) text = name ? prefix + '\n' + name : prefix;
  else text = name || '--';
  let cls = 'key-normal';
  if (mod !== 0) cls = 'key-mod';
  else if (key === 0) cls = 'key-none';
  return { text, cls };
}

function renderGrid() {
  const grid = $('grid');
  grid.innerHTML = '';
  for (let row = 0; row < ROWS; row++) {
    for (let col = 0; col < COLS; col++) {
      const idx = row * COLS + col;
      const [mod, key] = km.getKey(state.data, state.currentLayer, idx);
      const btn = document.createElement('button');
      btn.className = 'key-btn';
      const disp = keyDisplay(mod, key);
      btn.classList.add(disp.cls);
      btn.textContent = disp.text;
      btn.title = `索引 ${idx} (行${row} 列${col})  修饰符 0x${mod.toString(16).padStart(2, '0')} 键码 0x${key.toString(16).padStart(2, '0')}`;
      btn.onclick = () => openEdit(idx);
      grid.append(btn);
    }
  }
}

// ---- 编辑弹窗 ----
function switchEditMode(mode) {
  const isMouse = mode === 'mouse';
  $('mouseFieldset').hidden = !isMouse;
  $('modFieldset').hidden = isMouse;
  $('keycodeFieldset').hidden = isMouse;
  $('btnCapture').hidden = isMouse;
}

function openEdit(idx) {
  state.editIdx = idx;
  const [mod, key] = km.getKey(state.data, state.currentLayer, idx);
  const layerName = state.currentLayer ? 'Fn0_keyMap' : 'mainKeyMap';
  $('modalTitle').textContent =
    `编辑键位 ${idx} (${layerName}, 行${Math.floor(idx / COLS)} 列${idx % COLS})`;

  const isMouse = km.isMouseAction(mod);
  // 设置 radio 选中状态
  document.querySelector('input[name="editMode"][value="' + (isMouse ? 'mouse' : 'keyboard') + '"]').checked = true;
  switchEditMode(isMouse ? 'mouse' : 'keyboard');

  if (isMouse) {
    // 鼠标动作模式：设置下拉框选中项
    const sel = $('mouseSelect');
    let found = false;
    for (const opt of sel.options) {
      if (parseInt(opt.value, 0) === key) {
        sel.value = opt.value;
        found = true;
        break;
      }
    }
    if (!found) sel.value = '1';
  } else {
    // 普通键盘模式：设置修饰符复选框和键码输入
    for (const [name] of km.MOD_LIST) {
      $('mod_' + name).checked = !!(mod & km.MOD[name]);
    }
    $('hexInput').value = '0x' + key.toString(16).padStart(2, '0');
    $('nameInput').value = keyNameOf(key);
  }

  $('captureLabel').textContent = '点击「捕获按键」后按目标键…';
  $('captureLabel').className = 'capture-label';
  $('modal').hidden = false;
  if (capture) { capture.stop(); capture = null; }
}

function closeEdit() {
  $('modal').hidden = true;
  if (capture) { capture.stop(); capture = null; }
}

function collectMod() {
  // 鼠标动作模式：返回 0xFE
  if (document.querySelector('input[name="editMode"]:checked').value === 'mouse')
    return 0xFE;
  let mod = 0;
  for (const [name, val] of km.MOD_LIST) {
    if ($('mod_' + name).checked) mod |= val;
  }
  return mod & 0xff;
}

function collectKey() {
  // 鼠标动作模式：从下拉框取值
  if (document.querySelector('input[name="editMode"]:checked').value === 'mouse')
    return parseInt($('mouseSelect').value, 0) & 0xff;
  const name = $('nameInput').value.trim().toUpperCase();
  if (name) {
    const k = km.parseKey(name);
    if (k !== null) return k & 0xff;
  }
  const hex = $('hexInput').value.trim();
  try { return parseInt(hex, 0) & 0xff; } catch { return 0; }
}

function onOk() {
  const mod = collectMod();
  const key = collectKey();
  km.setKey(state.data, state.currentLayer, state.editIdx, mod, key);
  if (state.currentDevice) {
    state.deviceData.set(state.currentDevice, new Uint8Array(state.data));
  }
  closeEdit();
  renderGrid();
  setStatus(`已修改键位 ${state.editIdx}: mod=0x${mod.toString(16).padStart(2, '0')} key=0x${key.toString(16).padStart(2, '0')}`);
}

function applyModToCheckboxes(mod) {
  for (const [name, val] of km.MOD_LIST) {
    $('mod_' + name).checked = !!(mod & val);
  }
}

// ---- 按键捕获 ----
function onCaptureStart() {
  if ($('modal').hidden) return;
  capture = new KeyCapture();
  capture.start();
  $('captureLabel').textContent = '请按目标键…';
  $('captureLabel').className = 'capture-label info';
}

document.addEventListener('keydown', (e) => {
  if (!capture || !capture.capturing) return;
  if (e.key === 'Escape') return; // 交给关闭逻辑处理，不作为按键捕获
  const r = capture.handle(e);
  if (!r) return;
  if (r.type === 'modifier') {
    applyModToCheckboxes(r.mod);
    $('captureLabel').textContent = '已捕获修饰符，继续按主键…';
    $('captureLabel').className = 'capture-label info';
  } else if (r.type === 'key') {
    applyModToCheckboxes(r.mod);
    $('hexInput').value = '0x' + r.key.toString(16).padStart(2, '0');
    $('nameInput').value = keyNameOf(r.key);
    $('captureLabel').textContent = `已捕获 0x${r.key.toString(16).padStart(2, '0')}`;
    $('captureLabel').className = 'capture-label ok';
    capture.stop();
  }
});

// ---- 设备读写 / 默认 ----
async function onWrite() {
  if (!state.currentDevice) { setStatus('请先连接并选择键盘'); return; }
  setStatus('正在写入设备...');
  try {
    await hid.sendKeymap(state.currentDevice, new Uint8Array(state.data));
    setStatus('写入成功');
  } catch (e) {
    setStatus('写入失败: ' + e.message);
  }
}

async function onRead() {
  if (!state.currentDevice) { setStatus('请先连接并选择键盘'); return; }
  await selectDevice(state.currentDevice, state.currentLayer);
}

function onDefault() {
  state.data = km.makeDefaultKeymap();
  if (state.currentDevice) {
    state.deviceData.set(state.currentDevice, new Uint8Array(state.data));
  }
  afterLoad();
  setStatus('已恢复默认键位映射');
}

// ---- 文件导入导出 ----
function onImport() { $('fileInput').click(); }

$('fileInput').addEventListener('change', (e) => {
  const file = e.target.files[0];
  if (!file) return;
  const reader = new FileReader();
  reader.onload = () => {
    try {
      let data;
      if (file.name.toLowerCase().endsWith('.bin')) {
        const buf = new Uint8Array(reader.result);
        if (buf.length !== km.KEYMAP_SIZE) throw new Error(`文件大小错误: ${buf.length}B`);
        data = buf;
      } else {
        data = km.readKeymapText(reader.result);
      }
      state.data = data;
      if (state.currentDevice) {
        state.deviceData.set(state.currentDevice, new Uint8Array(data));
      }
      afterLoad();
      setStatus(`已导入: ${file.name}`);
    } catch (err) {
      setStatus('导入失败: ' + err.message);
    }
  };
  if (file.name.toLowerCase().endsWith('.bin')) reader.readAsArrayBuffer(file);
  else reader.readAsText(file);
  e.target.value = '';
});

function downloadBlob(blob, filename) {
  const url = URL.createObjectURL(blob);
  const a = document.createElement('a');
  a.href = url;
  a.download = filename;
  document.body.append(a);
  a.click();
  a.remove();
  setTimeout(() => URL.revokeObjectURL(url), 1000);
}

function exportTxt() {
  const text = km.formatKeymapText(state.data);
  downloadBlob(new Blob([text], { type: 'text/plain' }), 'keymap.txt');
  setStatus('已导出 keymap.txt');
}

function exportBin() {
  downloadBlob(new Blob([state.data], { type: 'application/octet-stream' }), 'keymap.bin');
  setStatus('已导出 keymap.bin');
}

// ---- 事件绑定 ----
function bindEvents() {
  $('btnConnect').onclick = onConnect;
  $('btnRefresh').onclick = onRefresh;
  $('btnRead').onclick = onRead;
  $('btnWrite').onclick = onWrite;
  $('btnDefault').onclick = onDefault;
  $('btnImport').onclick = onImport;
  $('btnExportTxt').onclick = exportTxt;
  $('btnExportBin').onclick = exportBin;
  $('btnCapture').onclick = onCaptureStart;
  $('btnCancel').onclick = closeEdit;
  $('btnOk').onclick = onOk;

  // 编辑模式切换（键盘/鼠标）
  document.querySelectorAll('input[name="editMode"]').forEach((radio) => {
    radio.addEventListener('change', () => {
      if (radio.checked) switchEditMode(radio.value);
    });
  });

  // 点击弹窗背景不关闭，避免误丢编辑；仅取消/确定生效
  window.addEventListener('keydown', (e) => {
    if (e.key === 'Escape' && !$('modal').hidden) closeEdit();
  });
}

init();
