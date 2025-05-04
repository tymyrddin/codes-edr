# Windows vulnerability scanner

## Features

* Checks for AlwaysInstallElevated vulnerability
* Identifies unquoted service paths
* Verifies critical service permissions
* Detects admin privileges

## Usage

Compile with:

```commandline
cl /EHsc windows_scanner.cpp /link advapi32.lib
```

Execute:

```commandline
windows_scanner.exe
```

## Adding new checks:

To add a new check to any `.cpp` scanner:

* Create a new function in the appropriate namespace
* Implement the check logic
* Add the function pointer to the checks vector in run_*_checks()

Example:

```commandline
namespace win_checks {
    void check_new_feature() {
        // Implementation here
    }
}

void run_windows_checks() {
    std::vector<void(*)()> checks = {
        // ... existing checks
        win_checks::check_new_feature
    };
    // ...
}
```
