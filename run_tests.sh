#!/usr/bin/env bash
set -euo pipefail

# Optional: turn on tracing by running: TRACE=1 ./run_tests.sh
if [[ "${TRACE:-0}" == "1" ]]; then set -x; fi

# === Settings ===
QUANTUM=3

# Exact tests from the assignment (plus RR tie-break at the end)
tests=(
  "3 8 7 3 6 3 2"
  "3 8 7 3 6 7 2"
  "4 8 7 3 6 1 2"
  "3 3 3 4 2 1 2"
  "3 3 2 3 2 1 2"
  "3 5 2 5 4 1 1"
  "3 2 3 4 1 1 5"
  "3 2 3 4 1 4 1"
  "3 2 3 4 5 6 2"
  "3 1 1 4 1 3 1" # RR tie-break check (P1 should go first on ties)
)

# Normalize line endings if the file was edited on Windows
# (no-op on Linux/Unix; prevents weird script exits)
if command -v sed >/dev/null 2>&1; then
  sed -i 's/\r$//' "$0" || true
fi

# === Build ===
cd "$(dirname "$0")"
echo ">>> Building assign2 and runall2"
if grep -q '^assign2:' makefile 2>/dev/null; then
  make assign2
else
  echo "No 'assign2' target found; attempting direct compile..."
  ${CC:-gcc} ${COPS:--D_GNU_SOURCE -g} -o assign2 assign2.c pslibrary.c || {
    echo "Failed to compile assign2. Ensure assign2.c and pslibrary.c exist."; exit 1; }
fi

if grep -q '^runall2:' makefile 2>/dev/null; then
  make runall2
else
  echo "No 'runall2' target found; attempting direct compile..."
  ${CC:-gcc} ${COPS:--D_GNU_SOURCE -g} -o runall2 runall-2.c pslibrary.c || {
    echo "Failed to compile runall2. Ensure runall-2.c and pslibrary.c exist."; exit 1; }
fi

# === Run individual tests (assign2) ===
outdir="outputs"
mkdir -p "$outdir"

echo ">>> Running assign2 on ${#tests[@]} tests"
i=1
for line in "${tests[@]}"; do
  # shellcheck disable=SC2086
  set -- $line
  if [[ $# -ne 7 ]]; then
    echo "Skipping malformed test line: '$line'" >&2
    continue
  fi
  q="$1"; x1="$2"; y1="$3"; z1="$4"; x2="$5"; y2="$6"; z2="$7"
  fname="${outdir}/assign2_q${q}_P1-${x1}-${y1}-${z1}_P2-${x2}-${y2}-${z2}.out"

  {
    echo "------------------------------------------------------------"
    echo "Test #$i: ./assign2 $q $x1 $y1 $z1 $x2 $y2 $z2"
    ./assign2 "$q" "$x1" "$y1" "$z1" "$x2" "$y2" "$z2"
  } | tee "$fname"
  i=$((i+1))
done

# === Run comprehensive harness (runall-2) ===
echo ">>> Running runall-2 harness (this will create *.all)"
./runall2

echo ">>> Capturing file stats for *.all"
ls -l *.all | tee "${outdir}/all_ls.txt"
wc *.all      | tee "${outdir}/all_wc.txt"
md5sum *.all  | tee "${outdir}/all_md5sum.txt"

report="${outdir}/REPORT_snippet.txt"
{
  echo "===== Comprehensive Files (ls -l) ====="
  cat "${outdir}/all_ls.txt"
  echo
  echo "===== Line/Word Counts (wc) ====="
  cat "${outdir}/all_wc.txt"
  echo
  echo "===== MD5 Sums (md5sum) ====="
  cat "${outdir}/all_md5sum.txt"
  echo
  echo "Paste the assign2 outputs for required specific tests above into your REPORT.txt as needed."
} > "$report"

echo ">>> Done."
echo "Outputs saved under: $outdir/"
echo "Tip: run with TRACE=1 to see each command."