#!/usr/bin/env bash
# 一键启动 XS40 WebHID 配置工具：起本地 HTTP 服务并以 Chrome app 模式打开
set -euo pipefail

PORT="${XS40_WEB_PORT:-8000}"
DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$DIR"

# 若端口已被占用（上次的服务还在），直接复用，不重复起
if curl -s -o /dev/null "http://localhost:$PORT/" 2>/dev/null; then
  echo "检测到 http://localhost:$PORT 已在运行，直接打开。"
else
  python3 -m http.server "$PORT" &
  SERVER_PID=$!
  trap "kill $SERVER_PID 2>/dev/null || true" EXIT INT TERM
  for _ in $(seq 1 30); do
    curl -s -o /dev/null "http://localhost:$PORT/" 2>/dev/null && break
    sleep 0.3
  done
fi

# 选择浏览器
if command -v google-chrome >/dev/null 2>&1; then
  BROWSER=google-chrome
elif command -v chromium >/dev/null 2>&1; then
  BROWSER=chromium
elif command -v chromium-browser >/dev/null 2>&1; then
  BROWSER=chromium-browser
else
  echo "未找到 Chrome/Chromium，请手动在浏览器打开 http://localhost:$PORT/"
  xdg-open "http://localhost:$PORT/" 2>/dev/null || true
  exit 1
fi

echo "正在以应用模式打开 http://localhost:$PORT/ （关闭窗口后按 Ctrl+C 退出后台服务）"
"$BROWSER" --app="http://localhost:$PORT/" &
wait
