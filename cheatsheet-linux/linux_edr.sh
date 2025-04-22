#!/bin/bash
# Linux EDR Deployment Script
# Version: 1.2
# WARNING: This script modifies system security settings. Review all changes before execution.

set -euo pipefail

# ---- Configuration ----
FALCO_VERSION="1.0.0"
SSH_BACKUP_DIR="/etc/ssh/backups"
MAX_AUTH_TRIES=3
RATE_LIMIT=50  # Events per second

# ---- Pre-Flight Checks ----
echo "=== Linux EDR Deployment ==="
echo "[*] Running pre-flight checks..."

# Check root privileges
if [ "$(id -u)" -ne 0 ]; then
  echo "[-] Error: This script must be run as root" >&2
  exit 1
fi

# Verify SSH key access
if [ ! -f "$HOME/.ssh/authorized_keys" ] || [ ! -s "$HOME/.ssh/authorized_keys" ]; then
  echo "[-] CRITICAL: No SSH keys found in $HOME/.ssh/authorized_keys"
  echo "[-] Generate keys first: ssh-keygen -t ed25519"
  exit 1
fi

# ---- Falco Installation ----
echo "[*] Installing Falco..."
mkdir -p /etc/apt/keyrings
curl -fsSL https://falco.org/repo/falcosecurity-3672BA8F.asc | gpg --dearmor -o /etc/apt/keyrings/falco.gpg
echo "deb [signed-by=/etc/apt/keyrings/falco.gpg] https://download.falco.org/packages/deb stable main" | tee /etc/apt/sources.list.d/falcosecurity.list

apt-get update -qq
apt-get install -y falco=$FALCO_VERSION

# ---- SSH Hardening ----
echo "[*] Hardening SSH..."
mkdir -p "$SSH_BACKUP_DIR"
BACKUP_FILE="$SSH_BACKUP_DIR/sshd_config.$(date +%Y%m%d-%H%M%S)"
cp /etc/ssh/sshd_config "$BACKUP_FILE"
echo "[+] Backup saved to $BACKUP_FILE"

# Idempotent configuration
sed -i '/^PasswordAuthentication/d;/^PermitRootLogin/d;/^MaxAuthTries/d' /etc/ssh/sshd_config
echo "PasswordAuthentication no" >> /etc/ssh/sshd_config
echo "PermitRootLogin no" >> /etc/ssh/sshd_config
echo "MaxAuthTries $MAX_AUTH_TRIES" >> /etc/ssh/sshd_config

if ! sshd -t; then
  echo "[-] SSH config test failed. Restoring backup..."
  cp "$BACKUP_FILE" /etc/ssh/sshd_config
  exit 1
fi

systemctl restart sshd

# ---- eBPF Monitoring ----
echo "[*] Deploying eBPF monitor with rate limiting..."
cat > /usr/local/bin/execve_monitor.bt <<EOF
BEGIN {
  printf("Linux execve monitor started (rate limit: $RATE_LIMIT/s)\\n");
}

tracepoint:syscalls:sys_enter_execve {
  @[pid, comm] = count();
  if (@[pid, comm] > $RATE_LIMIT) {
    printf("RATE LIMIT EXCEEDED: pid=%d comm=%s\\n", pid, comm);
    exit();
  }
  printf("%-6d %-16s %s\\n", pid, comm, str(args->filename));
}
EOF

# Create systemd service
cat > /etc/systemd/system/execve_monitor.service <<EOF
[Unit]
Description=eBPF Execve Monitor
After=network.target

[Service]
ExecStart=/usr/bin/bpftrace /usr/local/bin/execve_monitor.bt
Restart=on-failure
StandardOutput=journal
StandardError=journal

[Install]
WantedBy=multi-user.target
EOF

systemctl daemon-reload
systemctl enable --now execve_monitor

echo "[+] Deployment complete"
echo "    - Falco logs: journalctl -u falco"
echo "    - eBPF logs: journalctl -u execve_monitor"
echo "    - SSH config backup: $BACKUP_FILE"