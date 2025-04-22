# Linux vulnerability scanner

## Features

* Checks for dangerous SUID/SGID files
* Identifies world-writable sensitive files
* Scans for suspicious cron jobs
* Verifies system file permissions

## Usage

Compile with:

```commandline
g++ -o linux_scanner linux_scanner.cpp
```

Run as root for best results:

```commandline
sudo ./linux_scanner
```

## Adding new checks:

To add a new check:

* Create a new function in the appropriate namespace
* Implement the check logic
* Add the function pointer to the checks vector in run_*_checks()

