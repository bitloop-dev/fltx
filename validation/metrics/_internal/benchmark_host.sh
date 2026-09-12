#!/usr/bin/env bash
# Sent over SSH stdin by benchmark_host.ps1. Requires only bash and the existing Python runner.
set -euo pipefail
root=$1
python=$2
path_prepend=$3
invocation=$4
profile=$5
IFS=, read -r -a precisions <<< "$6"
IFS=, read -r -a modes <<< "$7"
IFS=, read -r -a operations <<< "$8"
shift 8
cd "$root"
if [ -n "$path_prepend" ]; then export PATH="$path_prepend:$PATH"; fi
directory="build/metrics/benchmarks/$invocation"
mkdir -p "$directory"
handoffs=()
while [ "$#" -gt 0 ]; do
    preset=$1
    strict=$2
    fastmath=$3
    shift 3
    for mode in "${modes[@]}"; do
        executable=$strict
        if [ "$mode" = fastmath ]; then executable=$fastmath; fi
        handoff="$directory/$preset-$mode.json"
        log="$directory/$preset-$mode.log"
        arguments=(validation/metrics/_internal/run_metrics.py --benchmark "$executable"
            --sample-mode "$profile" --consumer-mode "$mode" --result-file "$handoff")
        for precision in "${precisions[@]}"; do arguments+=(--precision "$precision"); done
        for operation in "${operations[@]}"; do arguments+=(--operation "$operation"); done
        if ! "$python" "${arguments[@]}" > "$log" 2>&1; then
            tail -20 "$log" >&2
            exit 1
        fi
        handoffs+=("$handoff")
    done
done
# Stream the runner's JSON unchanged; parsing and collection belong to the coordinator.
printf '['
separator=
for handoff in "${handoffs[@]}"; do
    printf '%s' "$separator"
    cat "$handoff"
    separator=,
done
printf ']\n'
