#!/bin/bash

# Ensure we are in the CMSSW environment
if [ -z "$CMSSW_BASE" ]; then
    echo "ERROR: CMSSW environment not set. Please run cmsenv first."
    exit 1
fi

# Define your run array
# TODO 2025: fill with the same runs as EPCalib/submit_epcalib_runwise_data.sh
RUNS=()
if [ ${#RUNS[@]} -eq 0 ]; then
    echo "FATAL ERROR: RUNS is empty. Fill in the 2025 run list first."
    exit 1
fi
FIRST_RUN=${RUNS[0]}

# Define paths and names
SCRATCH_BASE="/scratch/negishi/saha115/EP_calib/EP2025_Runwise"
DB_NAME="HeavyIonRPRcd_PbPb2025_Runwise.db"
DB_TAG="HeavyIonRPRcd"

# Remove the old database if it exists so we start completely fresh
rm -f ${DB_NAME}

echo "================================================="
echo "Building SQLite Database: ${DB_NAME}"
echo "================================================="

# --- Catch-all IOV from Run 1 using the first calibrated run ---
echo "Creating Catch-All IOV for Run: 1 (Using ${FIRST_RUN} data) ..."
INFILE_FIRST="${SCRATCH_BASE}/output/ep_HIMB01_${FIRST_RUN}.root"
RESCOR_FIRST="${SCRATCH_BASE}/Rescor_${FIRST_RUN}"

if [ -f "$INFILE_FIRST" ]; then
    cmsRun moveflatparamstodb_cfg.py \
        outputFile=${DB_NAME} \
        outtag=${DB_TAG} \
        begin=1 \
        end=$((FIRST_RUN - 1)) \
        infile=${INFILE_FIRST} \
        rescorloc=${RESCOR_FIRST}
else
    echo "FATAL ERROR: First run file not found. Cannot create Run 1 IOV."
    exit 1
fi
# ------------------------------------------------------

for RUN in "${RUNS[@]}"; do
    echo "Appending IOV for Run: ${RUN} ..."
    
    INFILE="${SCRATCH_BASE}/output/ep_HIMB01_${RUN}.root"
    RESCOR="${SCRATCH_BASE}/Rescor_${RUN}"
    
    # Check if the jobs actually produced the files
    if [ ! -f "$INFILE" ]; then
        echo "WARNING: $INFILE not found! Skipping run $RUN."
        continue
    fi

    
    # Run the CMSSW configuration to append the payload
    cmsRun moveflatparamstodb_cfg.py \
        outputFile=${DB_NAME} \
        outtag=${DB_TAG} \
        begin=${RUN} \
        end=${RUN} \
        infile=${INFILE} \
        rescorloc=${RESCOR}
done

echo "================================================="
echo "Database creation complete!"
echo "================================================="
