#!/bin/bash
#SBATCH --job-name=EPCalib_MC
#SBATCH --mem=16G
#SBATCH --cpus-per-task=2
#SBATCH --account=physics
#SBATCH --partition=cpu
#SBATCH --qos=standby
#SBATCH --time=4:00:00
#SBATCH --output=/scratch/negishi/saha115/EP_calib/EP2025_MC/logs/EPCalib_MC_%j.log
#SBATCH --error=/scratch/negishi/saha115/EP_calib/EP2025_MC/logs/EPCalib_MC_%j.err

# --- SUBMIT MODE: run as ./submit_epcalib_MC.sh on the login node ---
# Compute nodes cannot see the login node's /tmp, so copy the voms-proxy-init
# proxy (/tmp/x509up_u<uid>) to $HOME, then submit this same script with sbatch.
if [ -z "$SLURM_JOB_ID" ]; then
    TMP_PROXY=/tmp/x509up_u$(id -u)
    HOME_PROXY=/home/saha115/myproxy
    TMP_LEFT=$(voms-proxy-info -file $TMP_PROXY -timeleft 2>/dev/null || echo 0)
    HOME_LEFT=$(voms-proxy-info -file $HOME_PROXY -timeleft 2>/dev/null || echo 0)
    if [ "${TMP_LEFT:-0}" -gt "${HOME_LEFT:-0}" ]; then
        cp $TMP_PROXY $HOME_PROXY && chmod 600 $HOME_PROXY
        echo "Copied proxy $TMP_PROXY -> $HOME_PROXY (~$(( TMP_LEFT / 3600 )) hours left)"
    else
        echo "Keeping $HOME_PROXY (~$(( HOME_LEFT / 3600 )) hours left)"
    fi
    mkdir -p /scratch/negishi/saha115/EP_calib/EP2025_MC/logs
    sbatch "$0"
    exit $?
fi
# ---------------------------------------------------------------------

cd $SLURM_SUBMIT_DIR
export X509_USER_PROXY=/home/saha115/myproxy


echo "Checking grid proxy validity..."
if [ ! -f "$X509_USER_PROXY" ]; then
    echo "FATAL ERROR: Proxy file not found at $X509_USER_PROXY"
    echo "Run: voms-proxy-init -voms cms -valid 168:00 -out $X509_USER_PROXY"
    exit 1
fi
TIME_LEFT=$(voms-proxy-info -file $X509_USER_PROXY -timeleft 2>/dev/null)

if [ -z "$TIME_LEFT" ] || [ "$TIME_LEFT" -lt 36000 ]; then
    echo "FATAL ERROR: Proxy at $X509_USER_PROXY has less than 10 hours left! (Time left: ${TIME_LEFT} seconds)"
    echo "Please generate a fresh proxy before submitting jobs."
    echo "Run: voms-proxy-init -voms cms -valid 168:00 -out $X509_USER_PROXY"
    exit 1
else
    # Calculate hours for a cleaner log output
    HOURS_LEFT=$(( TIME_LEFT / 3600 ))
    echo "SUCCESS: Proxy is valid. Time left: ~${HOURS_LEFT} hours (${TIME_LEFT} seconds)."
fi

# ROOT comes from the submitting shell's environment (cmsenv)
if ! command -v root > /dev/null; then
    echo "FATAL ERROR: root not found. Run cmsenv before sbatch."
    exit 1
fi

echo "============================================="
echo "Starting calibration for MC (all runs)"
echo "============================================="

SCRATCH_BASE=/scratch/negishi/saha115/EP_calib/EP2025_MC
OUT_DIR=$SCRATCH_BASE/output
LOG_DIR=$SCRATCH_BASE/logs
RESCOR_DIR=$SCRATCH_BASE/Rescor_MC

mkdir -p $OUT_DIR
mkdir -p $LOG_DIR
mkdir -p $RESCOR_DIR

# MC has no run-wise calibration: use the full run range (macro defaults 0 - 500000)
root -b -q "EPCalib.C(0, 500000, \"inputFiles_PbPb2025_MC.lis\", \"${OUT_DIR}/tmp_MC.root\", \"${OUT_DIR}/ep_MC.root\", \"${OUT_DIR}/offset_MC.root\", \"${RESCOR_DIR}\")"

echo "============================================="
echo "Finished calibration for MC"
echo "============================================="
