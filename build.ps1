<#
.SYNOPSIS
    Build VibiKey into a single-file executable using PyInstaller or Nuitka.

.DESCRIPTION
    Builds the native C core (vibikey_core.dll) for the requested arch/compiler,
    stages it, then packages the PySide6 app as ONE self-extracting executable
    via PyInstaller (--onefile) or Nuitka (--onefile). Output is a single file
    under dist/ named VibiKey-<arch>-<compiler>-<tool>.exe

.EXAMPLE
    .\build.ps1                       # PyInstaller onefile, x64, MSVC
    .\build.ps1 -Tool nuitka          # Nuitka onefile
    .\build.ps1 -t nuitka             # same, short form (-t == -Tool)
    .\build.ps1 -NoOneFile            # folder build (onedir/standalone) for debugging
    .\build.ps1 -Arch x86 -Compiler mingw
    .\build.ps1 -Clean
    .\build.ps1 -Help

.NOTES
    Requires: Python 3.11+ and a C compiler (MSVC vcvars64/32/arm64, or MSYS2
    mingw64/mingw32/clangarm64). For Nuitka onefile install: pip install "Nuitka[app]"
#>
[CmdletBinding()]
param(
    [switch]$Help,
    [Alias('t')]
    [ValidateSet('pyinstaller', 'nuitka')]
    [string]$Tool = 'pyinstaller',
    [switch]$NoOneFile,
    [switch]$Clean,
    [Alias('p')]
    [string]$Python = '',
    [Alias('a')]
    [ValidateSet('x64', 'x86', 'arm64')]
    [string]$Arch = 'x64',
    [Alias('c')]
    [ValidateSet('msvc', 'mingw')]
    [string]$Compiler = 'msvc'
)

if ($Help) {
    Get-Help $PSCommandPath -Detailed | Out-String | Write-Host
    exit 0
}

$ErrorActionPreference = 'Stop'
$Root = $PSScriptRoot
$Name = 'VibiKey'
$Entry = Join-Path $Root 'run_ui.py'
$CoreDir = Join-Path $Root 'vibikey_core'
$Locales = Join-Path $Root 'locales'
$OneFile = -not $NoOneFile

if (-not $Python) {
    $venvPy = Join-Path $Root '..\.venv\Scripts\python.exe'
    $Python = (Test-Path $venvPy) ? (Resolve-Path $venvPy).Path : 'python'
}

function Remove-Artifacts {
    foreach ($d in @('build', 'dist', '__pycache__', "$Name.build", "$Name.dist", "$Name.onefile-build", 'run_ui.build', 'run_ui.dist', 'run_ui.onefile-build')) {
        $p = Join-Path $Root $d
        if (Test-Path $p) { Remove-Item -Recurse -Force $p }
    }
    Get-ChildItem $Root -Filter '*.spec' | Remove-Item -Force -ErrorAction SilentlyContinue
}

if ($Clean) { Remove-Artifacts; Write-Host 'Cleaned.' -ForegroundColor Green; exit 0 }

$Dll = Join-Path $CoreDir 'vibikey_core.dll'
$srcs = @((Join-Path $CoreDir 'vietnamese_tep.c'), (Join-Path $CoreDir 'windows' 'vibikey_hook.c'))
if (-not (Test-Path $Dll)) {
    Write-Host "Building vibikey_core.dll ($Arch/$Compiler)..." -ForegroundColor Yellow
    if ($Compiler -eq 'msvc') {
        $vc = switch ($Arch) {
            'x86'    { 'vcvars32.bat' }
            'arm64'  { 'vcvarsarm64.bat' }
            default  { 'vcvars64.bat' }
        }
        $vcvars = Get-ChildItem "C:\Program Files\Microsoft Visual Studio",
            "C:\Program Files (x86)\Microsoft Visual Studio", "E:\Visual Studio" `
            -Filter $vc -Recurse -ErrorAction SilentlyContinue | Select-Object -First 1 -ExpandProperty FullName
        if (-not $vcvars) { throw "MSVC $vc not found." }
        $deff = Join-Path $CoreDir 'vibikey_core.def'
        $cmd = ('"{0}" && cl /nologo /w /std:c17 /utf-8 /LD /Fe:"{1}" {2} /link /DEF:"{3}" user32.lib' `
                -f $vcvars, $Dll, (($srcs | ForEach-Object { "`"$_`"" }) -join ' '), $deff)
        cmd /c $cmd | Out-Null
        if (-not (Test-Path $Dll)) { throw "MSVC build failed." }
    } else {
        $cc = if ($Arch -eq 'arm64') { 'clang' } else { 'gcc' }
        if (-not (Get-Command $cc -ErrorAction SilentlyContinue)) {
            throw "$cc not found on PATH (install MSYS2 + the matching toolchain first)."
        }
        Push-Location $CoreDir
        try {
            $relSrcs = ($srcs | ForEach-Object {
                $rel = [System.IO.Path]::GetRelativePath($CoreDir, $_) -replace '\\','/'
                '"{0}"' -f $rel
            }) -join ' '
            $cmd = "$cc -shared -O2 -o `"$Dll`" $relSrcs vibikey_core.def -Wl,--kill-at -luser32"
            & cmd /c $cmd 2>&1 | Write-Host
            if ($LASTEXITCODE -ne 0) { throw "MinGW ($Arch) build failed (exit $LASTEXITCODE)." }
        } finally {
            Pop-Location
        }
        if (-not (Test-Path $Dll)) { throw "MinGW ($Arch) build failed." }
    }
    Write-Host "core built: $Dll" -ForegroundColor Green
    & $Python -c @"
import ctypes, sys
try:
    lib = ctypes.CDLL(r'$Dll')
    _ = lib.vibikey_convert_word
    print('export OK: vibikey_convert_word')
except Exception as e:
    print('EXPORT CHECK FAILED:', e, file=sys.stderr); sys.exit(1)
"@
    if ($LASTEXITCODE -ne 0) { throw "Native core missing required exports." }
}

$Suffix = "$Arch-$Compiler"
$Tag = "$Name-$Suffix-$Tool"
$FinalExe = Join-Path $Root "dist\$Tag.exe"

Remove-Artifacts
New-Item -ItemType Directory -Path (Join-Path $Root 'dist') -Force | Out-Null

$Stage = Join-Path $env:TEMP "vibikey_core_stage"
if (Test-Path $Stage) { Remove-Item -Recurse -Force $Stage }
New-Item -ItemType Directory -Path $Stage | Out-Null
$libs = Get-ChildItem $CoreDir -File | Where-Object { $_.Extension -in '.dll', '.so', '.dylib' }
if (-not $libs) { throw "No native libraries (*.dll/*.so/*.dylib) found in $CoreDir to bundle." }
$libs | Copy-Item -Destination $Stage
Write-Host "Staged native libs: $(($libs | ForEach-Object Name) -join ', ')" -ForegroundColor Cyan
Write-Host "Mode: $(if ($OneFile) { 'ONEFILE (single .exe)' } else { 'folder' }) / tool=$Tool" -ForegroundColor Cyan

if ($Tool -eq 'pyinstaller') {
    & $Python -m pip install --quiet --upgrade pyinstaller
    $args = @(
        '-m', 'PyInstaller', '--noconfirm', '--clean',
        '--name', $Tag, '--windowed',
        '--add-binary', "$Stage;vibikey_core",
        '--add-data', "$Locales;locales"
    )
    $args += $OneFile ? '--onefile' : '--onedir'
    $args += $Entry
    & $Python @args
    if ($LASTEXITCODE -ne 0) { throw "PyInstaller failed (exit $LASTEXITCODE)." }
    $built = $OneFile ? (Join-Path $Root "dist\$Tag.exe") : (Join-Path $Root "dist\$Tag\$Tag.exe")
    if (-not (Test-Path $built)) { throw "PyInstaller output missing: $built" }
    if ($built -ne $FinalExe) {
        Copy-Item -Force $built $FinalExe
    }
    $out = $FinalExe
}
else {
    & $Python -m pip install --quiet --upgrade 'Nuitka[app]' ordered-set zstandard
    $mode = $OneFile ? '--onefile' : '--standalone'
    $narch = if ($Arch -eq 'x64') { @() } else { "--target-arch=$Arch" }
    $outDir = Join-Path $Root "dist\$Tag-build"
    $args = @(
        '-m', 'nuitka', $mode,
        '--enable-plugin=pyside6',
        '--windows-console-mode=disable',
        '--assume-yes-for-downloads',
        '--remove-output',
        "--output-filename=$Tag.exe",
        "--output-dir=$outDir",
        "--include-data-dir=$Locales=locales"
    ) + $narch
    foreach ($lib in $libs) {
        $args += "--include-data-files=$($lib.FullName)=vibikey_core/$($lib.Name)"
    }
    $args += $Entry
    & $Python @args
    if ($LASTEXITCODE -ne 0) { throw "Nuitka failed (exit $LASTEXITCODE)." }
    $built = $OneFile ? (Join-Path $outDir "$Tag.exe") : (Join-Path $outDir "run_ui.dist\$Tag.exe")
    if (-not (Test-Path $built)) {
        $alt = Get-ChildItem -Path $outDir -Recurse -Filter '*.exe' -ErrorAction SilentlyContinue |
            Where-Object { $_.Name -like "$Name*" -or $_.Name -like "$Tag*" -or $_.Name -eq 'run_ui.exe' } |
            Select-Object -First 1
        if ($alt) { $built = $alt.FullName }
    }
    if (-not (Test-Path $built)) { throw "Nuitka output missing under $outDir" }
    Copy-Item -Force $built $FinalExe
    $out = $FinalExe
}

if (Test-Path $out) {
    $size = [math]::Round((Get-Item $out).Length / 1MB, 1)
    Write-Host "Build OK -> $out ($size MB)" -ForegroundColor Green
} else {
    Write-Host "Build finished but expected output not found: $out" -ForegroundColor Yellow
    Write-Host 'Check the dist/ folder.' -ForegroundColor Yellow
    exit 1
}
