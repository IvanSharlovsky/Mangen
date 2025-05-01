#!/bin/bash
# test.sh — end-to-end automated tests for mangen utility

set -e

TESTDIR="test_env"
OUTFILE="manifest.txt"

# Clean up and prepare test environment
rm -rf "$TESTDIR" "$OUTFILE"
mkdir -p "$TESTDIR/dir1"
touch "$TESTDIR/file1.txt"
touch "$TESTDIR/file2.c"
touch "$TESTDIR/dir1/skipme.sh"

# ======= Test 1: No filters =======
echo "[TEST 1] Generation without exclusions..."
./mangen "$TESTDIR" > "$OUTFILE"

if grep -q "^file1.txt :" "$OUTFILE" && \
   grep -q "^file2.c :" "$OUTFILE" && \
   grep -q "^dir1/skipme.sh :" "$OUTFILE"; then
  echo "[TEST 1] OK"
else
  echo "[TEST 1] FAIL"
  exit 1
fi

# ======= Test 2: Exclude by name (-e) =======
echo "[TEST 2] Exclusion by name (file1.txt)..."
./mangen "$TESTDIR" -e file1.txt > "$OUTFILE"

if grep -q "^file1.txt :" "$OUTFILE"; then
  echo "[TEST 2] FAIL (file1.txt should not be included)"
  exit 1
else
  echo "[TEST 2] OK"
fi

# ======= Test 3: Exclude by pattern (-E *.sh) =======
echo "[TEST 3] Exclusion by pattern (*.sh)..."
./mangen "$TESTDIR" -E "*.sh" > "$OUTFILE"

if grep -q "skipme.sh" "$OUTFILE"; then
  echo "[TEST 3] FAIL (skipme.sh should not be included)"
  exit 1
else
  echo "[TEST 3] OK"
fi

# ======= Test 4: Version output (-v) =======
echo "[TEST 4] Version check..."
VERSION_OUTPUT=$(./mangen -v)

if [[ "$VERSION_OUTPUT" =~ ^commit:\ [a-f0-9]{7,}$ ]]; then
  echo "[TEST 4] OK"
else
  echo "[TEST 4] FAIL (invalid commit format: $VERSION_OUTPUT)"
  exit 1
fi

# ======= Test 5: Manifest verification (--verify) =======
echo "[TEST 5] Manifest verification..."
./mangen "$TESTDIR" > "$OUTFILE"
if ./mangen --verify "$OUTFILE" | grep -q "Valid"; then
  echo "[TEST 5] OK"
else
  echo "[TEST 5] FAIL (verification failed)"
  exit 1
fi

# ======= Test 6: Corrupted manifest detection =======
echo "[TEST 6] Corrupted manifest should be rejected..."
echo "tampered_line" >> "$OUTFILE"
if ./mangen --verify "$OUTFILE" | grep -q "Corrupted"; then
  echo "[TEST 6] OK"
else
  echo "[TEST 6] FAIL (corruption not detected)"
  exit 1
fi

# Success
rm -rf "$TESTDIR" "$OUTFILE"
echo -e "\n✅ All tests passed."
