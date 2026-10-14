#!/bin/bash

# Ensure we are in the CMSSW environment
if [ -z "$CMSSW_BASE" ]; then
    echo "ERROR: CMSSW environment not set. Please run cmsenv first."
    exit 1
fi

# Define your run array
RUNS=(388468 388475 388476 388574 388575 388577 388583 388613 388620 388622 388710 388711 388713 388733 388735 388737 388741 388750 388769 388784)

# Define paths and names
SCRATCH_BASE="/scratch/negishi/saha115/EP_calib/EP2024_Runwise_wEra_Sept29"
DB_NAME="HeavyIonRPRcd_PbPb2024_Runwise.db"
DB_TAG="HeavyIonRPRcd"

# Remove the old database if it exists so we start completely fresh
rm -f ${DB_NAME}

echo "================================================="
echo "Building SQLite Database: ${DB_NAME}"
echo "================================================="

# --- NEW: Explicitly insert Run 1 using 388468 data ---
echo "Creating Catch-All IOV for Run: 1 (Using 388468 data) ..."
INFILE_FIRST="${SCRATCH_BASE}/output/ep_HIMB01_388468.root"
RESCOR_FIRST="${SCRATCH_BASE}/Rescor_388468"

if [ -f "$INFILE_FIRST" ]; then
    cmsRun moveflatparamstodb_cfg.py \
        outputFile=${DB_NAME} \
        outtag=${DB_TAG} \
        begin=1 \
        end=388467 \
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
