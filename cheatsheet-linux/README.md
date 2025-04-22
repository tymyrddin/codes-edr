# Linux EDR deployment

What It Does

* Falco Runtime Security - Monitors system calls for suspicious activity (container escapes, crypto miners)
* SSH Hardening - Enforces key-based auth, blocks root login, and limits brute force attempts
* eBPF Monitoring - Tracks process execution with rate-limiting to prevent log flooding

Why It Matters

* Catches fileless attacks that bypass traditional AV
* Prevents common privilege escalation paths (e.g., via vulnerable services)
* Minimal performance overhead (<5% CPU) vs. auditd

Key Protection Against:

* Kinsing malware (crypto miners)
* CVE-2021-4034 (Polkit privilege escalation)
* SSH brute force attacks

## Overview code

Production-ready script for deploying:
- Falco runtime security
- SSH hardening
- eBPF-based execve monitoring

## Requirements

- Linux kernel ≥ 4.14
- Root privileges
- Existing SSH key access

## Critical warnings !!!

1. **SSH Hardening**:
   - Ensure you have working SSH key authentication
   - Backup of original config stored in `/etc/ssh/backups/`

2. **eBPF Monitoring**:
   - Rate-limited to 50 events/sec by default
   - Adjust via `RATE_LIMIT` variable

## Usage

```bash
chmod +x linux_edr.sh
sudo ./linux_edr.sh
```

## Recovery

If SSH access breaks:

1. Connect via local console or cloud provider's web shell
2. Restore backup:

```bash
sudo cp /etc/ssh/backups/latest_backup /etc/ssh/sshd_config
sudo systemctl restart sshd
```

## Monitoring

```bash
# Falco alerts
journalctl -u falco -f

# eBPF monitor
journalctl -u execve_monitor -f
```
