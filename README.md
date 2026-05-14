# Event Plane Calibration for 2024 PbPb — Step-by-Step Guide

**CMSSW:** 14_1_0  
**Package:** `HeavyIonsAnalysis/HiEvtPlaneCalib`  
**Run range (2024 PbPb data):** 387853 – 388784  

---

## Table of Contents

1. [Prerequisites](#prerequisites)
2. [Initial CMSSW Setup](#initial-cmssw-setup)
3. [Check Out HeavyIon Packages](#check-out-heavyion-packages)
4. [Build the Release](#build-the-release)
5. [Working Directory Layout](#working-directory-layout)
6. [Step 1 — Produce `calib.root`](#step-1--produce-calibroot)
   - [1a. Local run (quick test)](#1a-local-run-quick-test)
   - [1b. Full statistics via CRAB](#1b-full-statistics-via-crab)
7. [Step 2 — Generate Flattening Parameters](#step-2--generate-flattening-parameters)
8. [Step 3 — Write Calibration to SQLite DB](#step-3--write-calibration-to-sqlite-db)
9. [Step 4 — Validate the Calibration](#step-4--validate-the-calibration)
10. [Running the Full Script Interactively](#running-the-full-script-interactively)
11. [Key Files Reference](#key-files-reference)
12. [When a New Calibration Is Required](#when-a-new-calibration-is-required)

---

## Prerequisites

- Access to CERN computing infrastructure (lxplus or equivalent)
- CMSSW 14_1_0 environment (scram arch `el9_amd64_gcc12` or the equivalent for your platform)
- Valid CMS VOMS proxy (`voms-proxy-init -voms cms`) — needed for any step that reads from CMS DAS / xrootd
- CRAB3 client (for large-scale Steps 1 and 4)
- `conddb_import` available in the CMSSW environment

---

## Initial CMSSW Setup

```bash
# Source the CMS environment (once per login)
source /cvmfs/cms.cern.ch/cmsset_default.sh

# Set the SCRAM architecture for Run3 (SL9/el9 64-bit GCC 12)
export SCRAM_ARCH=el9_amd64_gcc12

# Create and initialise a fresh CMSSW release
cmsrel CMSSW_14_1_0
cd CMSSW_14_1_0/src
cmsenv
```

> **Note:** If you are using an existing release (as in this repository), simply `cd` to
> `CMSSW_14_1_0/src` and run `cmsenv` — do **not** re-run `cmsrel`.

---

## Check Out HeavyIon Packages

Two packages are required:

| Package | Purpose |
|---|---|
| `HeavyIonsAnalysis/HiEvtPlaneCalib` | Calibration tree producer, DB writer, check module |
| `RecoHI/HiEvtPlaneAlgos` | Core EP reconstruction algorithms and `HiEvtPlaneList.h` |

> **Important:** `HeavyIonsAnalysis/HiEvtPlaneCalib` is **not** part of the official CMSSW
> repository and is **not publicly hosted on GitHub**.
> Running `git cms-addpkg HeavyIonsAnalysis/HiEvtPlaneCalib` will fail.
> The package is distributed directly by the EP calibration group (S. Sanders, U. Kansas).
> Copy it from an existing checkout as shown below.

```bash
# 1. Copy HeavyIonsAnalysis/HiEvtPlaneCalib from an existing working area
#    (replace the source path with wherever you received the package)
cp -r /path/to/existing/CMSSW_14_1_0/src/HeavyIonsAnalysis $CMSSW_BASE/src/

# 2. Check out RecoHI/HiEvtPlaneAlgos from the official CMSSW repo
#    (this package IS in the official repo — git cms-addpkg works here)

# Configure git identity (required by git cms-init; only needed once per machine)
git config --global user.name 'Firstname Lastname'
git config --global user.email 'your.email@cern.ch'
git config --global user.github your_github_username

cd $CMSSW_BASE/src
git cms-init
git cms-addpkg RecoHI/HiEvtPlaneAlgos
```

---

## Build the Release

```bash
cd $CMSSW_BASE/src
scram b -j8      # use -j<N> matching your available CPU cores
```

A clean build should produce no errors. Warnings from third-party packages are acceptable.

---

## Working Directory Layout

All interactive steps are run from:

```
$CMSSW_BASE/src/HeavyIonsAnalysis/HiEvtPlaneCalib/test/
```

The required files that must be present there:

```
test/
├── calibtree_cfg.py          # Step 1: produce calibration tree
├── checkep_cfg.py            # Step 4: validation replay
├── moveflatparamstodb_cfg.py # Step 3: write parameters to DB
├── compare.C                 # Step 4: produce comparison plots
├── CreateDB.sh               # Master interactive script (runs all 4 steps)
├── EPCalib/
│   ├── EPCalib.C             # Step 2: ROOT macro — computes flattening params
│   ├── HiEvtPlaneList.h      # Symlink or copy from RecoHI/HiEvtPlaneAlgos
│   └── HiEvtPlaneFlatten.h   # Modified copy (FWCore includes commented out)
└── crabTemplates/
    ├── crabConfig_CalibTree.py   # CRAB config for Step 1
    └── crabConfig_CheckEP.py     # CRAB config for Step 4
```

### Prepare the EPCalib headers

`EPCalib.C` is a standalone ROOT macro and cannot use CMSSW headers directly.
Two header files must be placed inside `EPCalib/` before running Step 2:

```bash
cd $CMSSW_BASE/src/HeavyIonsAnalysis/HiEvtPlaneCalib/test/EPCalib

# 1. HiEvtPlaneList.h — direct copy (no modification needed)
cp $CMSSW_BASE/src/RecoHI/HiEvtPlaneAlgos/interface/HiEvtPlaneList.h .

# 2. HiEvtPlaneFlatten.h — copy then comment out the CMSSW-specific includes
cp $CMSSW_BASE/src/RecoHI/HiEvtPlaneAlgos/interface/HiEvtPlaneFlatten.h .

# Comment out the two framework include blocks so ROOT can parse it standalone:
#   // #include "FWCore/..."
#   // #include "DataFormats/..."
# Edit the file manually or use sed (adjust the patterns to match the actual lines):
sed -i 's|^#include "FWCore/|// #include "FWCore/|' HiEvtPlaneFlatten.h
sed -i 's|^#include "DataFormats/|// #include "DataFormats/|' HiEvtPlaneFlatten.h

cd ..
```

---

## Step 1 — Produce `calib.root`

`calib.root` is the input tree for Step 2.  It is produced by replaying MiniAOD events
through `calibtree_cfg.py`.

### Global tags used

| Input type | Global tag |
|---|---|
| **Data** (2024 PbPb) | `141X_dataRun3_Prompt_forHI_NominalCentrality` |
| **MC** (Hydjet HINPbPbWinter24) | `141X_mcRun3_2024_realistic_HI_v16` |

### 1a. Local run (quick test)

```bash
cd $CMSSW_BASE/src/HeavyIonsAnalysis/HiEvtPlaneCalib/test

# Initialise a VOMS proxy (required for xrootd access)
voms-proxy-init -voms cms

# --- MC (default, 10 000 events) ---
cmsRun calibtree_cfg.py outfile=calib.root aodType=MiniAOD inputType=MC

# --- Real data ---
cmsRun calibtree_cfg.py outfile=calib.root aodType=MiniAOD inputType=Data

# --- Single local file test (set MINITEST first) ---
export MINITEST=/path/to/your/local/file.root
cmsRun calibtree_cfg.py outfile=calib.root aodType=testMiniAOD repFile=$MINITEST inputType=MC
```

### 1b. Full statistics via CRAB

For a real calibration you need many more events (O(100k–1M) minimum bias).
Submit via CRAB:

```bash
cd $CMSSW_BASE/src/HeavyIonsAnalysis/HiEvtPlaneCalib/test/crabTemplates

# Edit crabConfig_CalibTree.py:
#   - Set `user` to your CMS username
#   - Set `storagesite` to a T2 you have write access to
#   - For Data: change `dataset` to the HIRun2024A MiniAOD dataset
#     e.g. /HIPhysicsRawPrime0/HIRun2024A-PromptReco-v1/MINIAOD
#   - For MC: the default Hydjet dataset is already set

source /cvmfs/cms.cern.ch/crab3/crab.sh   # source CRAB environment
voms-proxy-init -voms cms

crab submit -c crabConfig_CalibTree.py
```

After jobs finish, **merge** all output files into a single `calib.root`:

```bash
hadd calib.root /path/to/crab/output/*.root
```

---

## Step 2 — Generate Flattening Parameters

This step runs the standalone ROOT macro `EPCalib/EPCalib.C` which reads `calib.root`
and produces the flattening tables.

```bash
cd $CMSSW_BASE/src/HeavyIonsAnalysis/HiEvtPlaneCalib/test

# Variables (adjust paths as needed)
DEFLOC="."
CALIBRUNMIN=387853   # Data; use 0 / 999999 for MC
CALIBRUNMAX=388784
MINRUN=387853
MAXRUN=388784
RANGE="${MINRUN}_${MAXRUN}"

# Create output directories / files
mkdir -p ${DEFLOC}/RescorTables/Rescor_${RANGE}

ls -1 ${DEFLOC}/calib.root > tmphi.lis

TMPFILE="${DEFLOC}/tmp_${RANGE}.root"
EPFILE="${DEFLOC}/ep_${RANGE}.root"
OFFFILE="${DEFLOC}/offset_${RANGE}.root"
RESDIR="${DEFLOC}/RescorTables/Rescor_${RANGE}"

ARG="EPCalib/EPCalib.C+(${CALIBRUNMIN},${CALIBRUNMAX},\"tmphi.lis\",\"${TMPFILE}\",\"${EPFILE}\",\"${OFFFILE}\",\"${RESDIR}\")"
echo "Running: root -l -b -q '${ARG}'"
root -l -b -q "${ARG}"
```

**Outputs produced:**

| File / Directory | Contents |
|---|---|
| `RescorTables/Rescor_<range>/` | Event plane resolution correction tables |
| `ep_<range>.root` | Flattening coefficients (sine/cosine offsets per centrality, vtx, EP) |
| `offset_<range>.root` | Track-by-track offset corrections |

---

## Step 3 — Write Calibration to SQLite DB

Two sub-steps: write a per-IOV SQLite file, then import it into the combined offline DB.

```bash
MINRUN=387853
MAXRUN=388784
RANGE="${MINRUN}_${MAXRUN}"
DEFLOC="."
EPFILE="${DEFLOC}/ep_${RANGE}.root"
RESDIR="${DEFLOC}/RescorTables/Rescor_${RANGE}"

# 3a. Create single-IOV SQLite file
ln -sf ${EPFILE} rpflat_combined.root

cmsRun moveflatparamstodb_cfg.py print \
    outputFile=HeavyIonRPRcd_${RANGE}.db \
    outtag=HeavyIonRPRcd \
    begin=${MINRUN} \
    end=${MAXRUN} \
    rescorloc=${RESDIR} \
    infile=${EPFILE}

rm -f rpflat_combined.root

# 3b. Merge into the combined offline DB (one IOV per run range)
conddb_import \
    -f sqlite_file:HeavyIonRPRcd_${RANGE}.db \
    -c sqlite_file:HeavyIonRPRcd_offline.db \
    -i HeavyIonRPRcd \
    -t HeavyIonRPRcd \
    -b ${MINRUN} \
    -e ${MAXRUN}

rm -f HeavyIonRPRcd_${RANGE}.db
```

After all IOVs are processed, `HeavyIonRPRcd_offline.db` is the final calibration file
ready for upload to the CMS conditions DB.

### MC validation DB (IOV starting at run 1)

```bash
cmsRun moveflatparamstodb_cfg.py print \
    outputFile=HeavyIonRPRcd_check.db \
    outtag=HeavyIonRPRcd \
    begin=1 \
    end=999999 \
    rescorloc=${RESDIR} \
    infile=${EPFILE}
```

---

## Step 4 — Validate the Calibration

Re-replay the same events using the new DB and compare with the uncalibrated distributions.

```bash
DEFLOC="."
CHECKAODTYPE="MiniAOD"
INFILE=""    # empty → uses files defined in checkep_cfg.py

# 4a. Re-replay with the new calibration applied
cmsRun checkep_cfg.py \
    checkfile=${DEFLOC}/checkep.root \
    repFile="${INFILE}" \
    aodType=${CHECKAODTYPE} \
    dbfile=HeavyIonRPRcd_check.db \
    inputType=MC    # or Data

rm -f HeavyIonRPRcd_check.db

# 4b. Produce comparison plots
CALIBFILE="${DEFLOC}/calib.root"
MOVEFILE="${DEFLOC}/ep_387853_388784.root"
CHECKFILE="${DEFLOC}/checkep.root"

root -l -b -q "compare.C(\"${CALIBFILE}\",\"${MOVEFILE}\",\"${CHECKFILE}\")"
```

**Interpreting the output plots (`checkPlot.pdf`):**

- **Black histograms** = raw (uncalibrated) event-plane distributions
- **Red histograms** = after flattening

The red distributions should completely cover the black ones.  Any visible black
indicates a problem with the calibration.  If this occurs:
- Verify that `Step 1` used the exact same events as `Step 4`
- Check that the correct global tag was used
- Confirm the EPCalib headers match the `HiEvtPlaneList.h` in the release

---

## Running the Full Script Interactively

The master script `CreateDB.sh` orchestrates all four steps interactively:

```bash
cd $CMSSW_BASE/src/HeavyIonsAnalysis/HiEvtPlaneCalib/test
voms-proxy-init -voms cms
bash CreateDB.sh
```

You will be prompted for:

| Prompt | Recommended value |
|---|---|
| Directory to store generated files | `./` (or a large-capacity disk) |
| Input type | `MC` (for testing) or `Data` (for real calibration) |
| Replay type | `MiniAOD` (or `testMiniAOD` for a single-file test) |

The script runs all four steps without further interaction.

---

## Key Files Reference

| File | Description |
|---|---|
| `calibtree_cfg.py` | CMSSW config: produces `calib.root` from MiniAOD |
| `moveflatparamstodb_cfg.py` | CMSSW config: packs flattening params into SQLite |
| `checkep_cfg.py` | CMSSW config: replays with calibration for validation |
| `EPCalib/EPCalib.C` | Standalone ROOT macro: computes all EP flat parameters |
| `EPCalib/HiEvtPlaneList.h` | Copy from `RecoHI/HiEvtPlaneAlgos/interface/` |
| `EPCalib/HiEvtPlaneFlatten.h` | Modified copy (CMSSW includes commented out) |
| `compare.C` | ROOT macro: generates `checkPlot.pdf` comparison histograms |
| `CreateDB.sh` | Master shell script wrapping all 4 steps |
| `crabTemplates/crabConfig_CalibTree.py` | CRAB3 config for Step 1 at scale |
| `crabTemplates/crabConfig_CheckEP.py` | CRAB3 config for Step 4 at scale |
| `HeavyIonRPRcd_offline.db` | **Final output**: combined SQLite DB for upload |

---

## When a New Calibration Is Required

A new calibration must be produced from scratch whenever:

1. `RecoHI/HiEvtPlaneAlgos/interface/HiEvtPlaneList.h` is modified (EP name/index changes)
2. Any default parameter in `RecoHI/HiEvtPlaneAlgos/python/HiEvtPlane_cfi.py` changes
   (η range, $p_T$ range, weights, etc.)
3. Any default parameter in `RecoHI/HiEvtPlaneAlgos/python/hiEvtPlaneFlat_cfi.py` changes
   (these two files **must** share the same default values)
4. A new data-taking period is added

After making such a change, always copy the updated `HiEvtPlaneList.h` and
`HiEvtPlaneFlatten.h` into `EPCalib/` before re-running Step 2.

---

## Dataset Reference

| Type | DAS dataset path |
|---|---|
| **2024 PbPb Data** | `/HIPhysicsRawPrime0/HIRun2024A-PromptReco-v1/MINIAOD` |
| **2024 PbPb MC** | `/Hydjet_MinBias_TuneCELLO_5p36TeV_pythia8/HINPbPbWinter24MiniAOD-NoPU_141X_mcRun3_2024_realistic_HI_v14-v2/MINIAODSIM` |
