#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"

CLIENT=../build/client_exp
COUNT=${COUNT:-100000}
PAYLOAD=${PAYLOAD:-20}

if [[ ! -x "$CLIENT" ]]; then
  echo "build client_exp first:  cmake --build ../build" >&2
  exit 1
fi

DELAYS=(0 5 10 20 50 100)   # microseconds of receiver work per message

for D in "${DELAYS[@]}"; do
  csv="results_d${D}.csv"
  echo ">>> delay=${D}us  count=$COUNT  payload=$PAYLOAD"
  python3 server_exp.py 9100 "$D" "$csv" &
  SPID=$!
  sleep 0.4
  "$CLIENT" "$COUNT" "$PAYLOAD"
  wait "$SPID"
  echo
done

echo "=== analysis ==="
python3 analyze.py results_d*.csv
