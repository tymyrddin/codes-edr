# Windows EDR Deployment

What It Does

* Attack Surface Reduction (ASR) - Blocks Office macros, script-based attacks
* Service ACL Auditing - Flags vulnerable service permissions (e.g., unquoted paths)
* WMI Process Monitoring - Detects suspicious parent-child process chains

Why It Matters

* Microsoft Defender alone misses fileless/LOLBins attacks
* Critical for stopping:
  * Ransomware (Conti, REvil)
  * LSASS credential dumping
  * Living-off-the-land (PsExec, WMI abuse)

Key Protection Against:

* CVE-2023-23397 (Outlook NTLM theft)
* PrintNightmare exploits
* Mimikatz-style credential attacks

## Overview code

Production-grade PowerShell script that deploys:
- Real-time process monitoring via WMI
- Service ACL hardening
- Windows Defender ASR rules
- Centralized event logging

## Requirements

- Windows 10/11 or Server 2016+
- PowerShell 5.0+
- Administrator privileges
- Windows Defender (for ASR rules)

## Critical warnings !!!

1. **Service Modifications**:
   - Script audits but doesn't automatically modify service ACLs
   - Review event logs before making changes

2. **Process Monitoring**:
   - Creates persistent WMI event subscription
   - Monitor for Event ID 200 in `EDR` log

3. **ASR Rules**: Test in Audit mode first:

```powershell
Set-MpPreference -AttackSurfaceReductionRules_Actions AuditMode
```

## Usage

```powershell
# Execution Policy first (if needed)
Set-ExecutionPolicy RemoteSigned -Scope Process -Force

# Deploy EDR
.\deploy_windows_edr.ps1
```

## Verification

```powershell
# Check events
Get-EventLog -LogName EDR -Newest 10 | Format-Table -AutoSize

# Verify ASR rules
Get-MpPreference | Select-Object AttackSurfaceReductionRules_*
```

## Maintenance

```powershell
# Remove WMI monitor
Get-EventSubscriber -SourceIdentifier "ProcessCreate" | Unregister-Event

# Disable specific ASR rule
Set-MpPreference -AttackSurfaceReductionRules_Ids "D4F940AB-401B-4EFC-AADC-AD5F3C50688A" -AttackSurfaceReductionRules_Actions Disabled
```

## Recovery

If issues occur:

1. Check event logs: Get-EventLog -LogName EDR
2. Rollback ASR rules:

```powershell
Set-MpPreference -AttackSurfaceReductionRules_Actions Disabled
```

3. Remove WMI subscription:

```powershell
Unregister-Event -SourceIdentifier "ProcessCreate"
```

## Monitoring recommendations

* Forward EDR log to SIEM
* Alert on Event IDs:
    * 100: Permissive service ACL
    * 200: Suspicious process
    * 102: Configuration failure


## **Key Features**

1. **Defense-in-Depth**:
   - **Process Monitoring**: WMI-based real-time detection
   - **Service Hardening**: ACL checks for critical services
   - **ASR Rules**: Leverages built-in Windows Defender

2. **Safety Mechanisms**:
   - Pre-flight privilege checks
   - Non-destructive auditing first
   - Centralized event logging

3. **Enterprise Ready**:
   - PowerShell v5+ compatibility
   - Works with Group Policy
   - SIEM integration via event logs

4. **Documentation**:
   - Clear recovery procedures
   - Verification steps
   - ASR rule testing guidance

## **Deployment workflow**

1. **Test environment**
   
```powershell
# First run in audit mode
.\deploy_windows_edr.ps1
Get-EventLog -LogName EDR
```

2. Production deployment

```powershell
# Via Group Policy or SCCM
Start-Process -FilePath "powershell.exe" -ArgumentList "-File .\deploy_windows_edr.ps1" -Verb RunAs
```

3. Monitoring

```
# Forward events to Splunk/ELK
wevtutil epl EDR C:\EDR_Backup.evtx
```

