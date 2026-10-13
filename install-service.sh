#!/usr/bin/env bash
set -euo pipefail

BIN_NAME="light_bar_sens"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BIN_PATH="${SCRIPT_DIR}/${BIN_NAME}"
SERVICE_NAME="touchbar-backlight.service"
SERVICE_PATH="/etc/systemd/system/${SERVICE_NAME}"

if [[ ! -x "${BIN_PATH}" ]]; then
    echo "error: ${BIN_PATH} not found or not executable" >&2
    exit 1
fi

sudo tee "${SERVICE_PATH}" >/dev/null <<EOF
[Unit]
Description=Touch Bar keyboard backlight listener
After=multi-user.target

[Service]
Type=simple
ExecStart=${BIN_PATH}
Restart=on-failure
RestartSec=2
StandardOutput=journal
StandardError=journal

[Install]
WantedBy=multi-user.target
EOF

sudo systemctl daemon-reload
sudo systemctl enable --now "${SERVICE_NAME}"