$ErrorActionPreference = "Stop"

$repoUrl = "https://raw.githubusercontent.com/wtvsad/amtool/main/nmcost.cpp"
$installDir = Join-Path $HOME ".local\bin"
$sourceFile = Join-Path $env:TEMP "petcalc-source.cpp"
$tempExe = Join-Path $env:TEMP "petcalc.exe"
$targetExe = Join-Path $installDir "petcalc.exe"

Write-Host "Installing Adopt Me Pet Cost Calculator..."

# Check for a C++ compiler.
$compiler = Get-Command clang++ -ErrorAction SilentlyContinue

if (-not $compiler) {
    $compiler = Get-Command g++ -ErrorAction SilentlyContinue
}

if (-not $compiler) {
    Write-Host ""
    Write-Host "A C++ compiler is required."
    Write-Host "Install Visual Studio Build Tools with the C++ workload,"
    Write-Host "or install LLVM/MinGW and ensure the compiler is on PATH."
    Write-Host "Then reopen PowerShell and run this installer again."
    exit 1
}

try {
    Write-Host "Downloading calculator source..."
    Invoke-WebRequest -Uri $repoUrl -OutFile $sourceFile

    Write-Host "Compiling calculator..."
    & $compiler.Source -std=c++17 -O2 $sourceFile -o $tempExe

    if ($LASTEXITCODE -ne 0) {
        throw "Compilation failed. Check the compiler installation and source code."
    }

    New-Item -ItemType Directory -Force -Path $installDir | Out-Null
    Copy-Item $tempExe $targetExe -Force

    # Add the installation folder to the user's PATH.
    $userPath = [Environment]::GetEnvironmentVariable("Path", "User")
    $pathEntries = @($userPath -split ";" | Where-Object { $_ })

    if ($pathEntries -notcontains $installDir) {
        $newPath = (@($pathEntries) + $installDir) -join ";"
        [Environment]::SetEnvironmentVariable("Path", $newPath, "User")
    }

    # Update PATH in this PowerShell session as well.
    if (($env:Path -split ";") -notcontains $installDir) {
        $env:Path = "$installDir;$env:Path"
    }

    Write-Host ""
    Write-Host "Installation successful!"
    Write-Host "Run: petcalc"
}
finally {
    Remove-Item $sourceFile, $tempExe -Force -ErrorAction SilentlyContinue
}
