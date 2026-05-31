param(
    [Parameter(Mandatory = $true)]
    [string]$SerialPort,
    [int]$BaudRate = 250000,
    [string]$UdpHost = "127.0.0.1",
    [int]$UdpPort = 47269,
    [switch]$DryRun
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$bridgeArgs = @(
    "-ExecutionPolicy", "Bypass",
    "-File", (Join-Path $PSScriptRoot "serial_to_teleplot.ps1"),
    "-SerialPort", $SerialPort,
    "-BaudRate", $BaudRate,
    "-UdpHost", $UdpHost,
    "-UdpPort", $UdpPort
)

$monitorArgs = @(
    "device", "monitor",
    "--port", $SerialPort,
    "--baud", $BaudRate
)

if ($DryRun) {
    Write-Output ("Bridge: pwsh " + ($bridgeArgs -join " "))
    Write-Output ("Monitor: pio " + ($monitorArgs -join " "))
    exit 0
}

$bridgeProcess = Start-Process -FilePath "pwsh" -ArgumentList $bridgeArgs -WindowStyle Hidden -PassThru

try {
    Write-Host "Bridge PID $($bridgeProcess.Id) forwarding $SerialPort -> udp://$UdpHost`:$UdpPort"
    & pio @monitorArgs
}
finally {
    if (-not $bridgeProcess.HasExited) {
        Stop-Process -Id $bridgeProcess.Id -Force
    }
}