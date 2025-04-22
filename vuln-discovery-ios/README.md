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

Special notes:

* Some checks may trigger App Store review warnings
* Jailbreak detection methods may require justification
* SSL pinning checks need proper entitlements

Integration:

```
let scanner = iOSVulnerabilityScanner()
scanner.run_scan()
```

Example in a SwiftUI Project:

```
// ContentView.swift
struct ContentView: View {
    @State private var scanResults = ""
    let scanner = iOSVulnerabilityScanner()
    
    var body: some View {
        VStack {
            ScrollView {
                Text(scanResults)
                    .font(.system(.body, design: .monospaced))
                    .padding()
            }
            
            Button("Run Scan") {
                scanResults = ""
                DispatchQueue.global(qos: .userInitiated).async {
                    let output = captureConsoleOutput {
                        scanner.run_scan()
                    }
                    DispatchQueue.main.async {
                        scanResults = output
                    }
                }
            }
        }
    }
    
    // Helper to capture console output
    func captureConsoleOutput(closure: () -> Void) -> String {
        let pipe = Pipe()
        let dup = dup(STDOUT_FILENO)
        
        setvbuf(stdout, nil, _IONBF, 0)
        dup2(pipe.fileHandleForWriting.fileDescriptor, STDOUT_FILENO)
        
        closure()
        
        dup2(dup, STDOUT_FILENO)
        close(dup)
        
        let data = pipe.fileHandleForReading.readDataToEndOfFile()
        return String(data: data, encoding: .utf8) ?? ""
    }
}
```

Required entitlements (add to your entitlements file):

```
<key>keychain-access-groups</key>
<array>
    <string>$(AppIdentifierPrefix)com.your.bundle</string>
</array>
<key>com.apple.security.cs.allow-unsigned-executable-memory</key>
<true/>
<key>com.apple.security.cs.disable-library-validation</key>
<true/>
```

Privacy Manifest (iOS 17+), create PrivacyInfo.xcprivacy with:

```
<dict>
    <key>NSPrivacyAccessedAPITypes</key>
    <array>
        <dict>
            <key>NSPrivacyAccessedAPIType</key>
            <string>NSPrivacyAccessedAPICategoryFileTimestamp</string>
        </dict>
    </array>
</dict>
```

App Store considerations:

* Justify jailbreak detection in App Review notes
* Use #if DEBUG for aggressive checks:
    swift

```
#if DEBUG
checks.append(check_debugger_attach)
#endif
```
Performance optimization:

```
DispatchQueue.concurrentPerform(iterations: checks.count) { i in
    checks[i]() // Parallel execution
}
```

Extending functionality:

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

