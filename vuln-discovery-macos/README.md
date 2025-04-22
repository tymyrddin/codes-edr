# macOS vulnerability scanner

## Features

* Checks System Integrity Protection (SIP) status
* Verifies Gatekeeper settings
* Identifies dangerous SUID/SGID files
* Examines keychain security settings

## Usage

Compile with:

```commandline
clang++ -o macos_scanner macos_scanner.cpp -framework CoreFoundation -framework Security
```

Run:

```commandline
./macos_scanner
```

Severity indicators (`[*]`, `[!]`, `[+]`)

## Adding new checks:

To add a new check:

* Create a new function in the appropriate namespace
* Implement the check logic
* Add the function pointer to the checks vector in run_*_checks()
