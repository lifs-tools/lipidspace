<#
.SYNOPSIS
    Verifies that every symbol a packaged binary imports from a DLL shipped
    beside it is actually exported by that DLL.

.DESCRIPTION
    Windows resolves imports at process start. If a shipped DLL is older than
    the compiler that produced the binaries, the application dies before main()
    with "The procedure entry point <symbol> could not be located in the
    dynamic link library <path>" - a modal dialog, so on CI it looks like a
    hang rather than a failure.

    That is not hypothetical: windeployqt --compiler-runtime copies
    libstdc++-6.dll out of the Qt installation, which carries whichever MinGW
    built Qt. With Qt 6.5.3 (MinGW 11.2) and an application compiled by GCC
    15.2, the shipped libstdc++ was missing every symbol involving mbstate_t,
    because mingw-w64 changed it from int to the _Mbstatet struct in between.

    This checks statically, so it can neither hang nor pop a dialog. Only DLLs
    present in the bundle are checked; system DLLs are the loader's problem.

.EXAMPLE
    powershell -File tools\check-windows-bundle.ps1 -BundleDir Build\LipidSpace
#>
[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string] $BundleDir,

    # Defaults to the objdump next to the g++ on PATH, i.e. the toolchain that
    # compiled the application.
    [string] $Objdump
)

$ErrorActionPreference = 'Stop'

if (-not (Test-Path -LiteralPath $BundleDir)) {
    Write-Error "Bundle directory not found: $BundleDir"
    exit 1
}

if (-not $Objdump) {
    $gxx = Get-Command g++ -ErrorAction SilentlyContinue
    if (-not $gxx) {
        Write-Error "g++ is not on PATH and -Objdump was not given."
        exit 1
    }
    $Objdump = Join-Path (Split-Path $gxx.Source -Parent) 'objdump.exe'
}
if (-not (Test-Path -LiteralPath $Objdump)) {
    Write-Error "objdump not found: $Objdump"
    exit 1
}

# DLL name (lower case) -> set of exported symbols, for the DLLs in the bundle.
$exportsByDll = @{}
# Full path -> map of imported DLL name -> list of symbols.
$importsByFile = @{}

$binaries = Get-ChildItem -LiteralPath $BundleDir -Recurse -File |
    Where-Object { $_.Extension -in '.exe', '.dll' }

foreach ($binary in $binaries) {
    # objdump writes to stderr for anything it cannot parse; with
    # $ErrorActionPreference = 'Stop' that would abort the script, so keep only
    # the stdout strings.
    $dump = & $Objdump -p $binary.FullName 2>&1 |
        Where-Object { $_ -is [string] }

    # Imports: a "DLL Name:" header, then one indented line per symbol,
    #   <tab><vma><tab>  <hint>  <symbol>
    $imports = @{}
    $currentDll = $null
    foreach ($line in $dump) {
        if ($line -match '^\s+DLL Name:\s+(\S+)') {
            $currentDll = $Matches[1].ToLowerInvariant()
            if (-not $imports.ContainsKey($currentDll)) { $imports[$currentDll] = [System.Collections.Generic.List[string]]::new() }
        }
        elseif ($currentDll -and $line -match '^\s+[0-9a-f]+\s+\d+\s+(\S+)\s*$') {
            $imports[$currentDll].Add($Matches[1])
        }
        elseif ($line -match '^\s*$') {
            $currentDll = $null
        }
    }
    $importsByFile[$binary.FullName] = $imports

    # Exports: "[Ordinal/Name Pointer] Table" entries, <tab>[   N] <symbol>
    $exports = [System.Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    foreach ($line in $dump) {
        if ($line -match '^\s+\[\s*\d+\]\s+(\S+)\s*$') { [void]$exports.Add($Matches[1]) }
    }
    $exportsByDll[$binary.Name.ToLowerInvariant()] = $exports
}

$problems = [System.Collections.Generic.List[string]]::new()
$affected = 0
$bundleRoot = (Resolve-Path $BundleDir).Path

foreach ($file in ($importsByFile.Keys | Sort-Object)) {
    foreach ($dll in ($importsByFile[$file].Keys | Sort-Object)) {
        # Only DLLs shipped in the bundle are ours to verify.
        if (-not $exportsByDll.ContainsKey($dll)) { continue }

        $exported = $exportsByDll[$dll]
        $missing = @($importsByFile[$file][$dll] | Where-Object { -not $exported.Contains($_) })
        if ($missing.Count -eq 0) { continue }

        $affected++
        # One mismatched runtime breaks every module at once, so a few examples
        # are enough to diagnose it - keep the rest out of the log.
        if ($affected -le 5) {
            $rel = $file.Substring($bundleRoot.Length).TrimStart('\', '/')
            $problems.Add("$rel imports $($missing.Count) symbol(s) that $dll does not export:")
            foreach ($sym in ($missing | Select-Object -First 5)) { $problems.Add("    $sym") }
            if ($missing.Count -gt 5) { $problems.Add("    ... and $($missing.Count - 5) more") }
        }
    }
}

if ($affected -gt 0) {
    Write-Host ""
    Write-Host "ERROR: the packaged application would fail to start." -ForegroundColor Red
    foreach ($p in $problems) { Write-Host "  $p" }
    if ($affected -gt 5) { Write-Host "  ... and $($affected - 5) further module(s) with unresolved imports" }
    Write-Host ""
    Write-Host "  A shipped DLL is older than the toolchain that built the binaries."
    Write-Host "  Check that the MinGW runtime came from the same compiler as the exe."
    exit 1
}

Write-Host "Bundle check OK: $($binaries.Count) binaries, all imports from bundled DLLs resolve."
exit 0
