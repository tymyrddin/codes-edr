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

Example in a Java/Kotlin implementation context:

```
// MainActivity.java
public class MainActivity extends AppCompatActivity {
    private TextView resultText;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        resultText = findViewById(R.id.scan_results);
        
        Button scanButton = findViewById(R.id.scan_button);
        scanButton.setOnClickListener(v -> runScan());
    }

    private void runScan() {
        // Redirect console output to TextView
        System.setOut(new PrintStream(new TextOutputStream(resultText)));
        
        AndroidVulnerabilityScanner scanner = new AndroidVulnerabilityScanner(this);
        scanner.run_scan();
    }

    // Custom stream to capture console output
    private class TextOutputStream extends OutputStream {
        private TextView textView;
        private StringBuilder buffer = new StringBuilder();

        public TextOutputStream(TextView textView) {
            this.textView = textView;
        }

        @Override
        public void write(int b) {
            buffer.append((char) b);
            if ((char) b == '\n') {
                textView.post(() -> {
                    textView.append(buffer.toString());
                    buffer.setLength(0);
                });
            }
        }
    }
}
```

Required permissions (add to AndroidManifest.xml):

```xml
<uses-permission android:name="android.permission.INTERNET" />
<uses-permission android:name="android.permission.READ_LOGS" />
<uses-permission android:name="android.permission.ACCESS_SUPERUSER" />
<uses-permission android:name="android.permission.QUERY_ALL_PACKAGES" />
```

Runtime Permissions (Android 11+)

```
// In Activity:
if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
    requestPermissions(arrayOf(Manifest.permission.QUERY_ALL_PACKAGES), 0)
}
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

