# Android vulnerability scanner

## Features

* Detects root access and superuser apps
* Identifies debuggable applications
* Checks for world-writable files
* Scans for dangerous permissions
* Verifies Android version vulnerabilities

## Usage

### Integration

* Add the class to your Android project (Java or Kotlin)
* Requires Android API level 21+ for full functionality

### Basic usage

```
// In your Activity or Service:
AndroidVulnerabilityScanner scanner = new AndroidVulnerabilityScanner(getApplicationContext());
scanner.run_scan();
```

Required permissions (add to AndroidManifest.xml):

```xml
<uses-permission android:name="android.permission.INTERNET" />
<uses-permission android:name="android.permission.READ_LOGS" />
<uses-permission android:name="android.permission.ACCESS_SUPERUSER" />
```

### Extending functionality

```
// To add a new check:
checks.add(() -> {
    System.out.println("\n[*] Running custom check...");
    // Implementation here
});
```

### Output interpretation

```
[*] indicates an informational message
[!] indicates a potential vulnerability
[+] indicates scan completion
```

