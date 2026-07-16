#!/bin/bash
# crab_submit_all.sh
# Loops over a list of primary datasets, patches the CRAB config template
# (data / tag fields), and submits each job with crab submit.
# Usage:
#   ./crab_submit_all.sh

set -euo pipefail

# ------------------------------------------------------------------
# EDIT THESE
# ------------------------------------------------------------------
TEMPLATE="crabConfig_CalibTree.py"     # path to your crab config template shown above
TAG="July15"                      # shared tag for this submission round
VOMS_PROXY_MIN_HOURS=4           # sanity check threshold below
PD_PREFIX="HIPhysicsRawPrime"

# Range of trailing index numbers to submit (inclusive).
# e.g. START=0 END=3 -> HIPhysicsRawPrime0 .. HIPhysicsRawPrime3
PD_START=0
PD_END=1
# ------------------------------------------------------------------

if [ ! -f "$TEMPLATE" ]; then
    echo "ERROR: template config '$TEMPLATE' not found." >&2
    exit 1
fi

# Sanity-check the voms proxy before firing off a batch of submissions
if command -v voms-proxy-info >/dev/null 2>&1; then
    remaining=$(voms-proxy-info --timeleft 2>/dev/null || echo 0)
    if [ "$remaining" -lt $((VOMS_PROXY_MIN_HOURS * 3600)) ]; then
        echo "WARNING: VOMS proxy has less than ${VOMS_PROXY_MIN_HOURS}h left (${remaining}s)."
        echo "Run your voms_() helper before continuing."
        exit 1
    fi
fi

mkdir -p submitted_configs

for ((idx=PD_START; idx<=PD_END; idx++)); do
    data="${PD_PREFIX}${idx}"
    echo "=== Preparing CRAB config for dataset: ${data} (tag=${TAG}) ==="

    cfg="submitted_configs/crab_cfg_${data}_${TAG}.py"
    cp "$TEMPLATE" "$cfg"
    sed -i "s/^data\s*=.*/data        = '${data}'/" "$cfg"
    sed -i "s/^tag\s*=.*/tag         = '${TAG}'/" "$cfg"

    if ! grep -q "data        = '${data}'" "$cfg"; then
        echo "ERROR: sed substitution for 'data' failed on ${cfg}" >&2
        exit 1
    fi

    echo "--- Submitting ${cfg} ---"
    crab submit -c "$cfg"

    echo "=== Submitted: ${data} ==="
    sleep 5   
done

echo "All submissions complete. Check status with: crab status -d crab_projects/crab_<requestName>"
