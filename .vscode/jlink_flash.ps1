# J-Link Flash Script for STM32L071CB
# Usage: jlink_flash.ps1 <path_to_elf_file>

param(
    [Parameter(Mandatory=$true)]
    [string]$ElfPath
)

# Check if ELF file exists
if (-not (Test-Path $ElfPath)) {
    Write-Error "ELF file not found: $ElfPath"
    Write-Error "Please build the project first."
    exit 1
}

Write-Host "Flashing: $ElfPath" -ForegroundColor Cyan

# Find J-Link executable - search for any JLink_V* folders
$JLinkExe = $null
$JLinkBasePaths = @(
    "C:\Program Files\SEGGER",
    "C:\Program Files (x86)\SEGGER"
)

foreach ($basePath in $JLinkBasePaths) {
    if (Test-Path $basePath) {
        # Find all JLink_V* folders and sort by version (descending) to get the latest
        $jlinkFolders = Get-ChildItem -Path $basePath -Directory -Filter "JLink*" | Sort-Object Name -Descending
        foreach ($folder in $jlinkFolders) {
            $candidatePath = Join-Path $folder.FullName "JLink.exe"
            if (Test-Path $candidatePath) {
                $JLinkExe = $candidatePath
                break
            }
        }
        if ($JLinkExe) { break }
    }
}

if (-not $JLinkExe) {
    Write-Error "J-Link not found. Please install SEGGER J-Link software."
    Write-Error "Download from: https://www.segger.com/downloads/jlink/"
    exit 1
}

Write-Host "Using J-Link: $JLinkExe" -ForegroundColor Green

# Create temporary J-Link command file
$tempJLinkScript = Join-Path $PSScriptRoot "jlink_flash_temp.jlink"

# Escape backslashes for J-Link command file
$elfPathEscaped = $ElfPath -replace '\\', '/'

$jlinkCommands = @"
r
h
erase
loadfile "$elfPathEscaped"
r
g
qc
"@

$jlinkCommands | Out-File -FilePath $tempJLinkScript -Encoding ASCII

Write-Host "J-Link commands:" -ForegroundColor Yellow
Write-Host $jlinkCommands

# Execute J-Link (using same settings as launch.json debug configuration)
& $JLinkExe -device STM32L071CBTx -if SWD -speed 4000 -autoconnect 1 -CommandFile $tempJLinkScript

# Clean up
Remove-Item $tempJLinkScript -ErrorAction SilentlyContinue

if ($LASTEXITCODE -eq 0) {
    Write-Host "`nFlash completed successfully!" -ForegroundColor Green
} else {
    Write-Error "`nFlash failed with exit code: $LASTEXITCODE"
    exit $LASTEXITCODE
}
