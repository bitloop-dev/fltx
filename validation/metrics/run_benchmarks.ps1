#Requires -Version 7.0
<#
.SYNOPSIS
Run selected benchmarks on configured hosts and print one comparison table.
.EXAMPLE
./validation/metrics/run_benchmarks.ps1 -Operation fmod -Precision dd
.EXAMPLE
./validation/metrics/run_benchmarks.ps1 -Operation sin,cos -Precision all -ConsumerMode strict
#>
[CmdletBinding()]
param(
    [Parameter(Mandatory, Position = 0)][string[]]$Operation,
    [ValidateSet('dd', 'qd', 'all')][string]$Precision = 'dd',
    [ValidateSet('quick', 'standard', 'full')][string]$Profile = 'standard',
    [ValidateSet('strict', 'fastmath', 'all')][string]$ConsumerMode = 'all',
    [string]$Config = (Join-Path $PSScriptRoot 'benchmark_hosts.local.json')
)

$ErrorActionPreference = 'Stop'
$timer = [Diagnostics.Stopwatch]::StartNew()
$repository = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
if (!(Test-Path -LiteralPath $Config)) {
    throw "Missing host configuration: $Config. Copy benchmark_hosts.example.json and configure your hosts."
}
$definitions = @(Get-Content -Raw -LiteralPath $Config | ConvertFrom-Json -AsHashtable)
$operations = @($Operation | Sort-Object -Unique -CaseSensitive)
if (!$operations.Count -or @($operations | Where-Object { !$_ -or $_ -match '[,\r\n]' }).Count) {
    throw 'Specify one or more exact operation names (without commas or newlines inside a name).'
}
$precisions = if ($Precision -eq 'all') { @('dd', 'qd') } else { @($Precision) }
$modes = if ($ConsumerMode -eq 'all') { @('strict', 'fastmath') } else { @($ConsumerMode) }
$sampleMode = if ($Profile -eq 'quick') { 'small' } else { $Profile }
$names = @{}
$addresses = @{}
$presets = @{}
if (!$definitions.Count) { throw 'No benchmark hosts are configured.' }
foreach ($definition in $definitions) {
    if ($definition.name -notmatch '^[a-zA-Z0-9_-]+$' -or $names.ContainsKey($definition.name)) {
        throw 'Host names must be unique and contain only letters, digits, underscores or hyphens.'
    }
    $names[$definition.name] = $true
    $address = if ($definition.address) { $definition.address } else { 'local' }
    if ($addresses.ContainsKey($address)) { throw "Configure $address only once so its benchmarks stay sequential." }
    $addresses[$address] = $true
    if (!$definition.root -or !$definition.python -or !$definition.targets.Count) {
        throw "Host $($definition.name) needs root, python and targets."
    }
    if ($definition.address) {
        if ($definition.address -notmatch '^[a-zA-Z0-9_][a-zA-Z0-9_.@:-]*$' -or !$definition.root.StartsWith('/')) {
            throw "Host $($definition.name) needs an SSH destination and an absolute POSIX checkout path."
        }
    } else {
        $definition.root = [IO.Path]::GetFullPath([string]$definition.root, $repository)
    }
    foreach ($target in $definition.targets) {
        if ($target.preset -notmatch '^[a-zA-Z0-9_-]+$' -or $presets.ContainsKey($target.preset)) {
            throw 'Configured preset names must be nonempty and unique.'
        }
        $presets[$target.preset] = $true
        foreach ($mode in $modes) {
            if (!$target[$mode]) { throw "$($target.preset) has no $mode executable configured." }
        }
    }
}

$invocation = (Get-Date -Format 'yyyyMMdd-HHmmss') + '-' + [guid]::NewGuid().ToString('N').Substring(0, 8)
$outputRoot = Join-Path $repository "build/metrics/benchmarks/$invocation"
$null = New-Item -ItemType Directory -Path $outputRoot -Force
$worker = Join-Path $PSScriptRoot '_internal/benchmark_host.ps1'
$results = @($definitions | ForEach-Object -Parallel {
    & $using:worker -Definition $_ -Operations $using:operations -Precisions $using:precisions `
        -Modes $using:modes -SampleMode $using:sampleMode -Invocation $using:invocation -OutputRoot $using:outputRoot
} -ThrottleLimit $definitions.Count)
$failures = @($results | Where-Object { !$_.success })
if ($failures.Count) {
    foreach ($failure in $failures) { Write-Warning "$($failure.name): $($failure.error)" }
    throw "Benchmark collection failed. Logs: $outputRoot. Repair missing/stale runners with the existing preset build workflow."
}
$rows = @($results | ForEach-Object { $_.rows })
if (!$rows.Count) { throw "No benchmark rows were returned. Logs: $outputRoot" }
if (@($rows.source_fingerprint | Sort-Object -Unique).Count -ne 1) {
    throw "Hosts measured different source fingerprints; synchronize their sources and rebuild. Evidence: $outputRoot"
}
$rows | Export-Csv -LiteralPath (Join-Path $outputRoot 'results.csv') -NoTypeInformation

function Format-Timing($Rows, [string]$Implementation, [string]$Mode) {
    $matches = @($Rows | Where-Object { $_.implementation -eq $Implementation -and $_.consumer_mode -eq $Mode })
    if (!$matches.Count) { return '-' }
    if ($matches.Count -ne 1) { throw "Duplicate $Implementation/$Mode benchmark rows." }
    return ([double]::Parse($matches[0].ns_iter, [Globalization.CultureInfo]::InvariantCulture)).ToString('F2', [Globalization.CultureInfo]::InvariantCulture)
}

$markdown = [Collections.Generic.List[string]]::new()
$markdown.Add("Profile: $Profile. Timings: ns/op (lower is better). External libraries: strict mode.")
foreach ($operationName in $operations) {
    foreach ($precisionName in $precisions) {
        $markdown.Add('')
        $markdown.Add("$operationName / $precisionName")
        $markdown.Add('')
        $markdown.Add('| Target | FLTX strict | FLTX fast | qdpp | Boost | TLFloat |')
        $markdown.Add('|---|---:|---:|---:|---:|---:|')
        foreach ($definition in $definitions) {
            foreach ($target in $definition.targets) {
                $selection = @($rows | Where-Object {
                    $_.preset -eq $target.preset -and $_.operation -ceq $operationName -and $_.precision -eq $precisionName
                })
                if (!$selection.Count) { continue }
                $boost = if ($precisionName -eq 'dd') { 'cppdd' } else { 'mpfr64' }
                $values = @(
                    Format-Timing $selection 'fltx' 'strict'
                    Format-Timing $selection 'fltx' 'fastmath'
                    Format-Timing $selection 'qdpp' 'strict'
                    Format-Timing $selection $boost 'strict'
                    Format-Timing $selection 'tlfloat' 'strict'
                )
                $markdown.Add('| ' + $selection[0].target + ' | ' + ($values -join ' | ') + ' |')
            }
        }
    }
}
$timer.Stop()
$markdown.Add('')
$markdown.Add('Total command time: ' + $timer.Elapsed.TotalSeconds.ToString('F2', [Globalization.CultureInfo]::InvariantCulture) + ' s (including collection).')
$hostTimes = foreach ($definition in $definitions) {
    $result = $results | Where-Object { $_.name -eq $definition.name }
    "$($result.name): $($result.seconds) s"
}
$markdown.Add('Host execution and collection: ' + ($hostTimes -join '; ') + '.')
$markdown.Add("CSV: $(Join-Path $outputRoot 'results.csv')")
$text = $markdown -join "`n"
$text | Set-Content -LiteralPath (Join-Path $outputRoot 'results.md') -Encoding utf8
@{ directory = $outputRoot; operations = $operations; precisions = $precisions; profile = $Profile; consumer_mode = $ConsumerMode } |
    ConvertTo-Json | Set-Content -LiteralPath (Join-Path $repository 'build/metrics/benchmarks/latest.json') -Encoding utf8
$text
