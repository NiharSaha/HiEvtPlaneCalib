#!/bin/bash
#SBATCH --job-name=EPCalib_Array
#SBATCH --array=0-19
#SBATCH --mem=16G
#SBATCH --cpus-per-task=2
#SBATCH --account=physics
#SBATCH --partition=cpu
#SBATCH --qos=standby
#SBATCH --time=4:00:00
#SBATCH --output=/scratch/negishi/saha115/EP_calib/EP2025_Runwise/logs/EPCalib_%A_%a.log
#SBATCH --error=/scratch/negishi/saha115/EP_calib/EP2025_Runwise/logs/EPCalib_%A_%a.err

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
    exit 1
else
    HOURS_LEFT=$(( TIME_LEFT / 3600 ))
    echo "SUCCESS: Proxy is valid. Time left: ~${HOURS_LEFT} hours (${TIME_LEFT} seconds)."
fi

if [ -z "$SLURM_ARRAY_TASK_ID" ]; then
    echo "FATAL ERROR: This script must be submitted via Slurm using 'sbatch'."
    exit 1
fi

# TODO 2025: fill with the selected runs after scanning the 2025 calibData files,
# and set '#SBATCH --array=0-<N-1>' to match the number of runs
RUNS=()
if [ ${#RUNS[@]} -eq 0 ]; then
    echo "FATAL ERROR: RUNS is empty. Fill in the 2025 run list first."
    exit 1
fi
RUN=${RUNS[$SLURM_ARRAY_TASK_ID]}

echo "============================================="
echo "Slurm Array Task ID: ${SLURM_ARRAY_TASK_ID}"
echo "Run Target: ${RUN}"
echo "============================================="

# --- AUTOMATED SAFE COMPILATION ---
# The flock command ensures only one job enters this block at a time
(
    flock -x 9
    
    # Check if .so doesn't exist OR if EPCalib.C has been edited more recently than the .so file
    if [ ! -f "EPCalib_C.so" ] || [ "EPCalib.C" -nt "EPCalib_C.so" ]; then
        echo "Task ${SLURM_ARRAY_TASK_ID} acquired the lock and is compiling the macro..."
        root -b -l -q "EPCalib.C+"
    else
        echo "Task ${SLURM_ARRAY_TASK_ID} confirms macro is already compiled and up to date."
    fi
) 9>compile.lock
# ----------------------------------

SCRATCH_BASE=/scratch/negishi/saha115/EP_calib/EP2025_Runwise
OUT_DIR=$SCRATCH_BASE/output
LOG_DIR=$SCRATCH_BASE/logs
RESCOR_DIR=$SCRATCH_BASE/Rescor_${RUN}
 
mkdir -p $OUT_DIR
mkdir -p $LOG_DIR
mkdir -p $RESCOR_DIR

# Execute ROOT (No '+' needed here, ROOT automatically uses the EPCalib_C.so generated above)
root -b -q "EPCalib.C(${RUN}, ${RUN}, \"inputFiles_PbPb2025_MB.lis\", \"${OUT_DIR}/tmp_HIMB01_${RUN}.root\", \"${OUT_DIR}/ep_HIMB01_${RUN}.root\", \"${OUT_DIR}/offset_HIMB01_${RUN}.root\", \"${RESCOR_DIR}\")"

echo "============================================="
echo "Finished calibration for Run: ${RUN}"
echo "============================================="
