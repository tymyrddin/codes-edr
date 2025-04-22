<#
.SYNOPSIS
Windows EDR Deployment Script
.DESCRIPTION
Deploys endpoint detection capabilities including:
- Process/Service monitoring
- Security event logging
- Attack surface reduction
.NOTES
Version: 1.2
Requires: Administrator privileges
#>

#region Configuration
$Config = @{
    EventLogName       = "EDR"
    LogSizeMB          = 1024  # Max log size
    RetentionDays      = 30    # Event retention
    ProtectedServices  = @("SamSs", "LanmanServer", "Dhcp", "DNS")
    AuditPaths         = @("C:\Windows\System32", "C:\Program Files")
}
#endregion

#region Pre-Flight Checks
function Test-Administrator {
    $identity = [Security.Principal.WindowsIdentity]::GetCurrent()
    $principal = New-Object Security.Principal.WindowsPrincipal($identity)
    return $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
}

if (-not (Test-Administrator)) {
    Write-Error "This script must be run as Administrator"
    exit 1
}

if ($PSVersionTable.PSVersion.Major -lt 5) {
    Write-Error "PowerShell 5.0 or later required"
    exit 1
}
#endregion

#region EDR Event Log Setup
try {
    if (-not [System.Diagnostics.EventLog]::SourceExists($Config.EventLogName)) {
        New-EventLog -LogName $Config.EventLogName -Source "EDRMonitor"
        Limit-EventLog -LogName $Config.EventLogName -MaximumSize ($Config.LogSizeMB * 1MB) -RetentionDays $Config.RetentionDays
        Write-Output "[+] Created EDR event log"
    }
}
catch {
    Write-Error "Failed to create event log: $_"
    exit 1
}
#endregion

#region Service Hardening
function Protect-ServiceAcl {
    param([string]$ServiceName)

    try {
        $service = Get-Service -Name $ServiceName -ErrorAction Stop
        $sd = sc.exe sdshow $ServiceName
        if ($sd -notmatch "D:.*?P") {
            Write-EventLog -LogName $Config.EventLogName -Source "EDRMonitor" -EntryType Warning -EventId 100 -Message "Service $ServiceName has permissive ACL: $sd"
        }
    }
    catch {
        Write-EventLog -LogName $Config.EventLogName -Source "EDRMonitor" -EntryType Error -EventId 101 -Message "Failed to check $ServiceName : $_"
    }
}

foreach ($service in $Config.ProtectedServices) {
    Protect-ServiceAcl -ServiceName $service
}
#endregion

#region Attack Surface Reduction
try {
    # Enable Windows Defender Attack Surface Reduction (ASR) rules
    Set-MpPreference -AttackSurfaceReductionRules_Ids @(
        "D4F940AB-401B-4EFC-AADC-AD5F3C50688A"  # Block Office macros
        "5BEB7EFE-FD9A-4556-801D-275E5FFC04CC"  # Block executable content
    ) -AttackSurfaceReductionRules_Actions Enabled

    # Enable auditing of sensitive files
    $paths = $Config.AuditPaths -join ","
    auditpol /set /subcategory:"File System" /success:enable /failure:enable
    Write-Output "[+] Enabled filesystem auditing for $paths"
}
catch {
    Write-EventLog -LogName $Config.EventLogName -Source "EDRMonitor" -EntryType Error -EventId 102 -Message "ASR configuration failed: $_"
}
#endregion

#region Process Monitor
$WMIQuery = @"
SELECT * FROM __InstanceCreationEvent
WITHIN 2
WHERE TargetInstance ISA 'Win32_Process'
"@

Register-WmiEvent -Query $WMIQuery -SourceIdentifier "ProcessCreate" -Action {
    $process = $EventArgs.NewEvent.TargetInstance
    $logMessage = "Process created: PID=$($process.ProcessId) Name=$($process.Name) Path=$($process.ExecutablePath) User=$($process.GetOwner().User)"

    # Check for suspicious characteristics
    if ($process.ExecutablePath -like "*temp*" -or $process.Name -eq "powershell.exe") {
        Write-EventLog -LogName $Config.EventLogName -Source "EDRMonitor" -EntryType Warning -EventId 200 -Message $logMessage
    }
} | Out-Null
#endregion

Write-Output @"
[+] Windows EDR deployment complete
    Event Log: $($Config.EventLogName)
    Protected Services: $($Config.ProtectedServices -join ', ')
    Audited Paths: $($Config.AuditPaths -join ', ')

Run 'Get-EventLog -LogName $($Config.EventLogName)' to view events
"@