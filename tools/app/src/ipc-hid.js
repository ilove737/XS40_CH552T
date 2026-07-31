// ipc-hid.js
// XS40_CH552T HID 通信层 - Tauri IPC 版
// 通过 Tauri 的 invoke 调用 Rust 后端 hidapi，替代浏览器 WebHID。
import { invoke } from '@tauri-apps/api/core';

export const KEYMAP_SIZE = 160;

// =========================================================================
// 设备枚举
// =========================================================================

/** 枚举所有已连接的 XS40 键盘（VID=0x413D, PID=0x2107） */
export async function listDevices() {
  return await invoke('list_devices');
}

// =========================================================================
// 设备打开 / 关闭
// =========================================================================

/** 打开设备（按路径） */
export async function openDevice(path) {
  return await invoke('open_device', { path });
}

/** 关闭设备（按路径） */
export async function closeDevice(path) {
  return await invoke('close_device', { path });
}

// =========================================================================
// 键位映射读写
// =========================================================================

/** 读取键位映射，返回 Uint8Array(160) */
export async function readKeymap(path) {
  const arr = await invoke('read_keymap', { path });
  return new Uint8Array(arr);
}

/** 写入键位映射，data 为 Uint8Array(160) */
export async function writeKeymap(path, data) {
  return await invoke('write_keymap', { path, data: Array.from(data) });
}

// =========================================================================
// 辅助函数
// =========================================================================

/** 设备稳定标识（按路径，路径唯一） */
export function deviceId(info, fallbackIndex = 0) {
  return info.path || `device:${fallbackIndex}`;
}

/** 设备显示名：优先 serialNumber，否则 productName */
export function deviceLabel(info) {
  return info.serial_number || info.product_name || 'XS40';
}

/** 左右手判定：优先 serialNumber，回退到 productName */
export function deviceHand(info) {
  const s = (info.serial_number || '').toUpperCase();
  if (s.includes('XS40 L') || s.includes('L')) return 'L';
  if (s.includes('XS40 R') || s.includes('R')) return 'R';
  const p = (info.product_name || '').toUpperCase();
  if (p.includes('XS40 L') || p.includes('L')) return 'L';
  if (p.includes('XS40 R') || p.includes('R')) return 'R';
  return null;
}