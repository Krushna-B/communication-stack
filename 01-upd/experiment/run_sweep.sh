#!/usr/bin/env bash
# Sweep send rate and collect a CSV per rate, then analyze.

set -euo pipefail
cd "$(dirname "$0")"

SENDER=../build/sender_exp
COUNT=${COUNT:-200000}
PAYLOAD=${PAYLOAD:-20}
RCVBUF=${RCVBUF:-0}          # 0 = kernel default

if [[ ! -x "$SENDER" ]]; then
  echo "build sender_exp first:  cmake --build ../build" >&2
  exit 1
fi

# 0 = unlimited blast (the overload case)
RATES=(10000 50000 100000 200000 500000 0)

for R in "${RATES[@]}"; do
  if [[ "$R" == "0" ]]; then label="unlimited"; else label="$R"; fi
  csv="results_${label}.csv"
  echo ">>> rate=$label  count=$COUNT  payload=$PAYLOAD  rcvbuf=$RCVBUF"
  python3 receiver_exp.py 9000 "$RCVBUF" "$csv" &
  RPID=$!
  sleep 0.4                    # let the receiver bind before we blast
  "$SENDER" "$COUNT" "$R" "$PAYLOAD"
  wait "$RPID"
  echo
done

echo "=== analysis ==="
python3 analyze.py results_*.csv
