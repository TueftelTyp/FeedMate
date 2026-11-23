param(
    [Parameter()]
    [string]$Port = 'COM6'
)

$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
Push-Location $projectRoot
try {
    function Invoke-PioStep {
        param([string[]]$Arguments)
        & pio @Arguments
        if ($LASTEXITCODE -ne 0) {
            throw "PlatformIO failed ($LASTEXITCODE): pio $($Arguments -join ' ')"
        }
    }

    Invoke-PioStep @('run', '-e', 'esp32c3_supermini', '-t', 'clean')
    Invoke-PioStep @('run', '-e', 'esp32c3_supermini', '-t', 'erase', '--upload-port', $Port)
    Invoke-PioStep @('run', '-e', 'esp32c3_supermini', '-t', 'upload', '--upload-port', $Port)
    Invoke-PioStep @('run', '-e', 'esp32c3_supermini', '-t', 'uploadfs', '--upload-port', $Port)
    Write-Host 'Full FeedMate flash completed. LittleFS data and saved settings were erased.'
}
finally {
    Pop-Location
}
