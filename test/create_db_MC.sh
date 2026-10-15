#!/bin/bash

# Ensure we are in the CMSSW environment
if [ -z "$CMSSW_BASE" ]; then
    echo "ERROR: CMSSW environment not set. Please run cmsenv first."
    exit 1
fi

# Define paths and names (output of EPCalib/submit_epcalib_MC.sh)
SCRATCH_BASE="/scratch/negishi/saha115/EP_calib/EP2023_D0MC_wEra_Oct4"
DB_NAME="HeavyIonRPRcd_PbPb2023_D0MC.db"
DB_TAG="HeavyIonRPRcd"

INFILE="${SCRATCH_BASE}/output/ep_MC.root"
RESCOR="${SCRATCH_BASE}/Rescor_MC"

# Check if the MC job actually produced the files
if [ ! -f "$INFILE" ] || [ ! -d "$RESCOR" ]; then
    echo "FATAL ERROR: $INFILE or $RESCOR not found. Cannot create MC IOV."
    exit 1
fi

# Remove the old database if it exists so we start completely fresh
rm -f ${DB_NAME}

echo "================================================="
echo "Building SQLite Database: ${DB_NAME}"
echo "================================================="

# MC has no run-wise calibration: one IOV from Run 1, valid for all runs
echo "Creating single IOV for MC (Run 1 onwards) ..."
cmsRun moveflatparamstodb_cfg.py \
    outputFile=${DB_NAME} \
    outtag=${DB_TAG} \
    begin=1 \
    end=1 \
    infile=${INFILE} \
    rescorloc=${RESCOR}

echo "================================================="
echo "Database creation complete!"
echo "================================================="
