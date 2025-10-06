#!/usr/bin/env bash
set -euo pipefail

# === Settings ===
# Adjust QUANTUM if your assignment uses a different default for RR
QUANTUM=3

# Edit the TESTS list to match the exact cases required by your PDF.
# Each line: q x1 y1 z1 x2 y2 z2
read -r -d '' TESTS <<'EOF'
3 1 1 4 1 3 1       # RR tie-break check: P1 should run first on ties
3 3 2 3 3 2 3       # symmetric bursts
3 4 0 2 5 1 0       # zero I/O in both
3 5 2 1 2 5 4       # different shapes
3 2 4 2 7 0 5       # long CPU2 on P2
3 8 1 1 2 2 2       # long CPU1 vs short bursts
EOF

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

echo ">>> Running assign2 on TESTS list"
i=1
while IFS= read -r line; do
  # skip comments/blank lines
  [[ -z "${line// }" ]] && continue
  [[ "$line" =~ ^# ]] && continue

  # normalize params
  set -- $line
  if [[ $# -ne 7 ]]; then
    echo "Skipping malformed test line: $line" >&2
    continue
  fi
  q="$1"; x1="$2"; y1="$3"; z1="$4"; x2="$5"; y2="$6"; z2="$7"
  fname="${outdir}/assign2_q${q}_P1-${x1}-${y1}-${z1}_P2-${x2}-${y2}-${z2}.out"

  echo "------------------------------------------------------------" | tee "$fname"
  echo "Test #$i: ./assign2 $q $x1 $y1 $z1 $x2 $y2 $z2" | tee -a "$fname"
  ./assign2 "$q" "$x1" "$y1" "$z1" "$x2" "$y2" "$z2" | tee -a "$fname"
  i=$((i+1))
done <<< "$TESTS"

# === Run comprehensive harness (runall-2) ===
echo ">>> Running runall-2 harness (this will create *.all)"
./runall2

echo ">>> Capturing file stats for *.all"
ls -l *.all | tee "${outdir}/all_ls.txt"
wc *.all      | tee "${outdir}/all_wc.txt"
md5sum *.all  | tee "${outdir}/all_md5sum.txt"

# === append a REPORT snippet ===
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
echo "Suggested to copy '${report}' blocks into your REPORT.txt per the assignment."
