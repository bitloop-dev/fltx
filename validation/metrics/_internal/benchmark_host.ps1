# Internal worker for run_benchmarks.ps1; one worker per physical host.
param(
    [hashtable]$Definition, [string[]]$Operations, [string[]]$Precisions,
    [string[]]$Modes, [string]$SampleMode, [string]$Invocation, [string]$OutputRoot
)
$ErrorActionPreference = 'Stop'
$timer = [Diagnostics.Stopwatch]::StartNew()
$hostDirectory = Join-Path $OutputRoot $Definition.name
$null = New-Item -ItemType Directory -Path $hostDirectory -Force
$log = Join-Path $hostDirectory 'worker.log'

function Invoke-Program([string]$File, [string[]]$Arguments, [string]$InputText = '', [string]$BinaryOutput = '') {
    $info = [Diagnostics.ProcessStartInfo]::new()
    $info.FileName = $File
    $info.UseShellExecute = $false
    $info.CreateNoWindow = $true
    $info.RedirectStandardInput = $true
    $info.RedirectStandardOutput = $true
    $info.RedirectStandardError = $true
    if (!$Definition.address) {
        $info.WorkingDirectory = $Definition.root
        if ($Definition.pathPrepend) {
            $info.Environment['PATH'] = ($Definition.pathPrepend -join [IO.Path]::PathSeparator) + [IO.Path]::PathSeparator + $env:PATH
        }
    }
    foreach ($argument in $Arguments) { $info.ArgumentList.Add($argument) }
    $process = [Diagnostics.Process]::new()
    $process.StartInfo = $info
    try {
        $null = $process.Start()
        $errors = $process.StandardError.ReadToEndAsync()
        if (!$BinaryOutput) { $stdout = $process.StandardOutput.ReadToEndAsync() }
        if ($InputText) { $process.StandardInput.Write($InputText) }
        $process.StandardInput.Close()
        if ($BinaryOutput) {
            $stream = [IO.File]::Create($BinaryOutput)
            try { $process.StandardOutput.BaseStream.CopyTo($stream) } finally { $stream.Dispose() }
            $output = ''
        } else {
            $output = $stdout.GetAwaiter().GetResult()
        }
        $process.WaitForExit()
        $errorText = $errors.GetAwaiter().GetResult()
        Add-Content -LiteralPath $log -Value ($output + $errorText)
        if ($process.ExitCode -ne 0) {
            $tail = (($output + $errorText) -split '\r?\n' | Select-Object -Last 14) -join "`n"
            throw "$File exited with $($process.ExitCode): $tail"
        }
        return $output
    } finally { $process.Dispose() }
}

function Quote-Shell([string]$Value) {
    return "'" + $Value.Replace("'", ("'" + '"' + "'" + '"' + "'")) + "'"
}

function Relative-EvidencePath([string]$Path) {
    $root = $Definition.root.TrimEnd('/', '\').Replace('\', '/')
    $normalized = $Path.Replace('\', '/')
    if (!$normalized.StartsWith($root + '/', [StringComparison]::OrdinalIgnoreCase)) {
        throw "Evidence outside the configured checkout: $Path"
    }
    $relative = $normalized.Substring($root.Length + 1)
    if (!$relative.StartsWith('build/metrics/runs/') -or '..' -in ($relative -split '/')) {
        throw "Unexpected evidence path: $Path"
    }
    return $relative
}

try {
    $runs = @()
    $sshOptions = @('-o', 'BatchMode=yes', '-o', 'ConnectTimeout=10', '-o', 'ServerAliveInterval=15', '-o', 'ServerAliveCountMax=2')
    if ($Definition.address) {
        $arguments = @($Definition.root, $Definition.python, ($Definition.pathPrepend -join ':'), $Invocation, $SampleMode,
            ($Precisions -join ','), ($Modes -join ','), ($Operations -join ','))
        foreach ($target in $Definition.targets) { $arguments += @($target.preset, $target.strict, $target.fastmath) }
        $command = 'bash -s -- ' + (($arguments | ForEach-Object { Quote-Shell $_ }) -join ' ')
        $script = (Get-Content -Raw -LiteralPath (Join-Path $PSScriptRoot 'benchmark_host.sh')).Replace("`r`n", "`n")
        $response = Invoke-Program 'ssh' ($sshOptions + @($Definition.address, $command)) $script
        $runs = @($response | ConvertFrom-Json)
    } else {
        foreach ($target in $Definition.targets) {
            foreach ($mode in $Modes) {
                $handoff = Join-Path $hostDirectory "$($target.preset)-$mode.json"
                $arguments = @('validation/metrics/_internal/run_metrics.py', '--benchmark', $target[$mode],
                    '--sample-mode', $SampleMode, '--consumer-mode', $mode, '--result-file', $handoff)
                foreach ($precision in $Precisions) { $arguments += @('--precision', $precision) }
                foreach ($operation in $Operations) { $arguments += @('--operation', $operation) }
                $null = Invoke-Program $Definition.python $arguments
                $runs += Get-Content -Raw -LiteralPath $handoff | ConvertFrom-Json
            }
        }
    }

    $paths = [Collections.Generic.List[string]]::new()
    foreach ($run in $runs) {
        $relative = Relative-EvidencePath $run.metadata
        $paths.Add($relative)
        $directory = $relative.Substring(0, $relative.LastIndexOf('/'))
        $suffix = if ($run.consumer_mode -eq 'fastmath') { '_fastmath' } else { '' }
        foreach ($precision in $Precisions) { $paths.Add("$directory/$($run.compiler)_$precision${suffix}_benchmark.csv") }
    }
    if ($Definition.address) {
        $archive = Join-Path $hostDirectory 'evidence.tar'
        $command = 'tar -cf - -C ' + (Quote-Shell $Definition.root) + ' -- ' + (($paths | ForEach-Object { Quote-Shell $_ }) -join ' ')
        $null = Invoke-Program 'ssh' ($sshOptions + @($Definition.address, $command)) '' $archive
        $null = Invoke-Program 'tar' @('-xf', $archive, '-C', $hostDirectory)
    } else {
        foreach ($path in $paths) {
            $destination = Join-Path $hostDirectory $path
            $null = New-Item -ItemType Directory -Path (Split-Path -Parent $destination) -Force
            Copy-Item -LiteralPath (Join-Path $Definition.root $path) -Destination $destination
        }
    }
    $rows = @()
    for ($index = 0; $index -lt $runs.Count; $index++) {
        $run = $runs[$index]
        $preset = $Definition.targets[[int][Math]::Floor($index / $Modes.Count)].preset
        $metadataPath = Join-Path $hostDirectory (Relative-EvidencePath $run.metadata)
        $metadata = Get-Content -Raw -LiteralPath $metadataPath | ConvertFrom-Json
        if ($metadata.status -ne 'complete') { throw "Incomplete run: $metadataPath" }
        $suffix = if ($run.consumer_mode -eq 'fastmath') { '_fastmath' } else { '' }
        foreach ($precision in $Precisions) {
            $csv = Join-Path (Split-Path -Parent $metadataPath) "$($run.compiler)_$precision${suffix}_benchmark.csv"
            foreach ($row in Import-Csv -LiteralPath $csv) {
                $row | Add-Member -NotePropertyMembers @{ host = $Definition.name; preset = $preset; target = $run.target; consumer_mode = $run.consumer_mode }
                $rows += $row
            }
        }
    }
    $timer.Stop()
    [pscustomobject]@{ name = $Definition.name; success = $true; rows = $rows; seconds = [Math]::Round($timer.Elapsed.TotalSeconds, 2) }
} catch {
    [pscustomobject]@{ name = $Definition.name; success = $false; error = $_.Exception.Message; rows = @() }
}
