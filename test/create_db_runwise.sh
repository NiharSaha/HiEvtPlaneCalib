#!/bin/bash

# Ensure we are in the CMSSW environment
if [ -z "$CMSSW_BASE" ]; then
    echo "ERROR: CMSSW environment not set. Please run cmsenv first."
    exit 1
fi

# Define your run array
RUNS=(374681 374719 374730 374778 374803 374810 374833 374970 375007 375013 375055 375064 375252 375391 375413 375754 375790 375823)

# Define paths and names
SCRATCH_BASE="/scratch/negishi/saha115/EP_calib/EP2023_Runwise_Aug5"
DB_NAME="HeavyIonRPRcd_PbPb2023_Runwise.db"
DB_TAG="HeavyIonRPRcd"

# Remove the old database if it exists so we start completely fresh
rm -f ${DB_NAME}

echo "================================================="
echo "Building SQLite Database: ${DB_NAME}"
echo "================================================="

# --- NEW: Explicitly insert Run 1 using 374681 data ---
echo "Creating Catch-All IOV for Run: 1 (Using 374681 data) ..."
INFILE_FIRST="${SCRATCH_BASE}/output/ep_HIMB0_374681.root"
RESCOR_FIRST="${SCRATCH_BASE}/Rescor_374681"

if [ -f "$INFILE_FIRST" ]; then
    cmsRun moveflatparamstodb_cfg.py \
        outputFile=${DB_NAME} \
        outtag=${DB_TAG} \
        begin=1 \
        end=374681 \
        infile=${INFILE_FIRST} \
        rescorloc=${RESCOR_FIRST}
else
    echo "FATAL ERROR: First run file not found. Cannot create Run 1 IOV."
    exit 1
fi
# ------------------------------------------------------

for RUN in "${RUNS[@]}"; do
    echo "Appending IOV for Run: ${RUN} ..."
    
    INFILE="${SCRATCH_BASE}/output/ep_HIMB0_${RUN}.root"
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
