# iOS vulnerability scanner

## Features

* Detects jailbreak indicators
* Checks for debugger attachment
* Identifies access to sensitive files
* Verifies pasteboard security
* Tests SSL validation

## Usage

### Integration

* Add this class to your iOS project
* Requires iOS 12.0+ for full functionality

### Basic usage

```
let scanner = iOSVulnerabilityScanner()
scanner.run_scan()
```

Required entitlements (add to your entitlements file):

```
<key>keychain-access-groups</key>
<array>
    <string>$(AppIdentifierPrefix)com.your.bundle</string>
</array>
```

### Extending functionality

```
// To add a new check:
checks.append {
    print("\n[*] Running custom check...")
    // Implementation here
}
```

### Output interpretation

```
[*] indicates an informational message
[!] indicates a potential vulnerability
[+] indicates scan completion
```

## Special Notes

* Some checks may trigger App Store review warnings
* Jailbreak detection methods may require justification
* SSL pinning checks need proper entitlements