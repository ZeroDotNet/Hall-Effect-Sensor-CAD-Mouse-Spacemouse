param(
    [string]$SerialPort,
    [int]$BaudRate = 250000,
    [string]$UdpHost = "127.0.0.1",
    [int]$UdpPort = 47269,
    [switch]$DryRun,
    [string[]]$TestLine
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

function Convert-LineToTeleplotPayload {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Line
    )

    $trimmed = $Line.Trim("`r", "`n", " ", "`t")
    if ([string]::IsNullOrWhiteSpace($trimmed)) {
        return $null
    }

    $timestamp = [DateTimeOffset]::UtcNow.ToUnixTimeMilliseconds()
    $normalized = $trimmed.Replace("||", ",")
    $payloadLines = New-Object System.Collections.Generic.List[string]

    foreach ($field in ($normalized -split ",")) {
        $entry = $field.Trim()
        if ($entry -match '^(?<name>[^:|]+):(?<value>-?(?:\d+(?:\.\d+)?|\.\d+)(?:[eE][+-]?\d+)?)$') {
            $name = $Matches['name'].Trim()
            $value = $Matches['value']
            $payloadLines.Add("${name}:${timestamp}:${value}")
        }
    }

    if ($payloadLines.Count -gt 0) {
        return ($payloadLines -join "`n")
    }

    if ($trimmed.StartsWith(">")) {
        return $trimmed
    }

    return ">${timestamp}:${trimmed}"
}

function Send-TeleplotPayload {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Payload,
        [System.Net.Sockets.UdpClient]$UdpClient,
        [string]$RemoteHost,
        [int]$Port,
        [switch]$DryRunOutput
    )

    if ($DryRunOutput) {
        Write-Output $Payload
        return
    }

    $bytes = [System.Text.Encoding]::UTF8.GetBytes($Payload)
    [void]$UdpClient.Send($bytes, $bytes.Length, $RemoteHost, $Port)
}

if ($TestLine -and $TestLine.Count -gt 0) {
    $udpClient = if ($DryRun) { $null } else { [System.Net.Sockets.UdpClient]::new() }
    try {
        foreach ($line in $TestLine) {
            $payload = Convert-LineToTeleplotPayload -Line $line
            if ($null -ne $payload) {
                Send-TeleplotPayload -Payload $payload -UdpClient $udpClient -RemoteHost $UdpHost -Port $UdpPort -DryRunOutput:$DryRun
            }
        }
    }
    finally {
        if ($null -ne $udpClient) {
            $udpClient.Dispose()
        }
    }
    exit 0
}

if ([string]::IsNullOrWhiteSpace($SerialPort)) {
    throw "-SerialPort is required unless -TestLine is used."
}

$serial = [System.IO.Ports.SerialPort]::new($SerialPort, $BaudRate, [System.IO.Ports.Parity]::None, 8, [System.IO.Ports.StopBits]::One)
$serial.NewLine = "`n"
$serial.ReadTimeout = 500
$serial.DtrEnable = $true
$serial.RtsEnable = $true
$udpClient = [System.Net.Sockets.UdpClient]::new()

try {
    $serial.Open()
    Write-Host "Forwarding $SerialPort -> udp://$UdpHost`:$UdpPort"
    while ($true) {
        try {
            $line = $serial.ReadLine()
        }
        catch [System.TimeoutException] {
            continue
        }

        $payload = Convert-LineToTeleplotPayload -Line $line
        if ($null -ne $payload) {
            Send-TeleplotPayload -Payload $payload -UdpClient $udpClient -RemoteHost $UdpHost -Port $UdpPort -DryRunOutput:$DryRun
        }
    }
}
finally {
    if ($serial.IsOpen) {
        $serial.Close()
    }
    $serial.Dispose()
    $udpClient.Dispose()
}