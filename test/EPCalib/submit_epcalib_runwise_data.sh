#!/bin/bash
#SBATCH --job-name=EPCalib_Array
#SBATCH --array=0-17
#SBATCH --mem=16G
#SBATCH --cpus-per-task=2
#SBATCH --account=physics
#SBATCH --partition=cpu
#SBATCH --qos=standby
#SBATCH --time=4:00:00
#SBATCH --output=/scratch/negishi/saha115/EP_calib/EP2023_Runwise_WOEra_Aug7/logs/EPCalib_%A_%a.log
#SBATCH --error=/scratch/negishi/saha115/EP_calib/EP2023_Runwise_WOEra_Aug7/logs/EPCalib_%A_%a.err

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




# Define the array of 18 run numbers
RUNS=(374681 374719 374730 374778 374803 374810 374833 374970 375007 375013 375055 375064 375252 375391 375413 375754 375790 375823)

# Extract the specific run number for this array task (0 through 17)
RUN=${RUNS[$SLURM_ARRAY_TASK_ID]}

echo "============================================="
echo "Starting calibration for Run: ${RUN}"
echo "Slurm Array Task ID: ${SLURM_ARRAY_TASK_ID}"
echo "============================================="

SCRATCH_BASE=/scratch/negishi/saha115/EP_calib/EP2023_Runwise_WOEra_Aug7/
OUT_DIR=$SCRATCH_BASE/output
LOG_DIR=$SCRATCH_BASE/logs

# Make a run-specific Rescor directory so .dat files don't clash
RESCOR_DIR=$SCRATCH_BASE/Rescor_${RUN}

mkdir -p $OUT_DIR
mkdir -p $LOG_DIR
mkdir -p $RESCOR_DIR

# Execute ROOT, passing $RUN as both minRun and maxRun, and appending $RUN to filenames
root -b -q "EPCalib.C(${RUN}, ${RUN}, \"inputFiles_PbPb2023_MB0_woEra.lis\", \"${OUT_DIR}/tmp_HIMB0_${RUN}.root\", \"${OUT_DIR}/ep_HIMB0_${RUN}.root\", \"${OUT_DIR}/offset_HIMB0_${RUN}.root\", \"${RESCOR_DIR}\")"

echo "============================================="
echo "Finished calibration for Run: ${RUN}"
echo "============================================="
