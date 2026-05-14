# Run2 → Run3 Migration Changes
## CMSSW 14_1_0 / 2024 PbPb EP Calibration

---

## Overview
This calibration code was originally written for Run2 (2018 PbPb, CMSSW 10_3_X, AOD).
The following changes were made to make it work with Run3 (2024 PbPb, CMSSW 14_1_0, MiniAOD).

---

## 1. Global Tag (`calibtree_cfg.py`, `checkep_cfg.py`)

**Run2 (old):**
```python
process.GlobalTag = GlobalTag(process.GlobalTag, '103X_dataRun2_v6', '')
process.es_prefer_GlobalTag = cms.ESPrefer('GlobalTag','')
# Manual toGet for centrality table:
process.GlobalTag.toGet.extend([cms.PSet(
    record = cms.string("HeavyIonRcd"),
    tag = cms.string("CentralityTable_HFtowers200_DataPbPb_periHYDJETshape_run2v1031x02_offline"),
    connect = cms.string("frontier://FrontierProd/CMS_CONDITIONS"),
    label = cms.untracked.string("HFtowers")
)])
```

**Run3 (new):**
```python
process.GlobalTag = GlobalTag(process.GlobalTag, '141X_dataRun3_Prompt_forHI_NominalCentrality', '')
# No manual toGet needed — centrality table is embedded in the GT
```

**Reason:** Run3 2024 PbPb global tag embeds the centrality table directly; no manual `toGet` is required.

---

## 2. Input Data Format: AOD → MiniAOD

**Run2 (old):** Used full AOD format.

**Run3 (new):** Uses MiniAOD format. Default `aodType` changed to `MiniAOD` in `CreateDB.sh` and both cfg files.

**Impact on track/vertex tags** (both `calibtree_cfg.py` and `checkep_cfg.py`):

| Parameter | AOD (Run2) | MiniAOD (Run3) |
|---|---|---|
| `trackTag` | `generalTracks` | `packedPFCandidates` |
| `vertexTag` | `offlinePrimaryVertices` | `offlineSlimmedPrimaryVertices` |

---

## 3. Input Dataset

**Run2 (old):** `/HIRun2018A/HIMinimumBias1/AOD/04Apr2019-v1`

**Run3 (new):** `/Hydjet_MinBias_TuneCELLO_5p36TeV_pythia8/HINPbPbWinter24MiniAOD-NoPU_141X_mcRun3_2024_realistic_HI_v14-v2/MINIAODSIM`

File used:
```
/store/mc/HINPbPbWinter24MiniAOD/Hydjet_MinBias_TuneCELLO_5p36TeV_pythia8/MINIAODSIM/NoPU_141X_mcRun3_2024_realistic_HI_v14-v2/2810000/6b0edb76-0f4e-49e4-a8b4-7d614b72e92f.root
```

To refresh with a current file:
```bash
dasgoclient --query 'file dataset=/Hydjet_MinBias_TuneCELLO_5p36TeV_pythia8/HINPbPbWinter24MiniAOD-NoPU_141X_mcRun3_2024_realistic_HI_v14-v2/MINIAODSIM' | head -1
```

---

## 4. Centrality Sequence (`calibtree_cfg.py`)

**Run2 (old):**
```python
# Centrality was computed on-the-fly from AOD
process.load("RecoHI.HiCentralityAlgos.HiCentrality_cfi")
process.p = cms.Path(process.centralityBin * process.hiEvtPlane * ...)
```

**Run3 (new):**
```python
# MiniAOD already contains centralityBin; hiCentrality re-computation included but
# centralityBin is read from the stored collection
process.load('RecoHI.HiCentralityAlgos.HiCentrality_cfi')
process.load("RecoHI.HiCentralityAlgos.CentralityBin_cfi")
process.centralityBin.Centrality = cms.InputTag("hiCentrality")
process.centralityBin.centralityVariable = cms.string("HFtowers")
```

---

## 5. Run Range / IOV (`CreateDB.sh`)

**Run2 (old):**
```bash
declare -a list=(326381 327564)  # 2018 PbPb run range
calibrunmin=326381
calibrunmax=327563
```

**Run3 (new):**
```bash
declare -a list=(387853 388785)  # 2024 PbPb run range: 387853-388784
# MC simulation events have run=1 — set calibrunmin/max to accept all:
calibrunmin=0
calibrunmax=999999
```

**Key distinction:**
- `list[]` defines the DB IOV range (real data run numbers → goes into `HeavyIonRPRcd_offline.db`).
- `calibrunmin`/`calibrunmax` filter events in the `calib.root` tree. For MC input (run=1), set to `0` and `999999`.

---

## 6. Check DB IOV for MC Validation (`CreateDB.sh`)

**Run2 (old):** Step 4 used the offline DB directly, which only covers real data runs.

**Run3 (new):** A separate check DB with IOV `1–999999` is created after the main loop to cover MC events (run=1):
```bash
cmsRun moveflatparamstodb_cfg.py print outputFile=HeavyIonRPRcd_check.db \
    outtag=HeavyIonRPRcd begin=1 end=999999 \
    rescorloc=$resdir infile=$epfile
```
This is used only for Step 4 validation and deleted afterward.

**Root cause of Step 4 failure:** If `HeavyIonRPRcd_offline.db` (IOV 387853–388784) is used for MC, `cmsRun` throws:
```
An exception of category 'ConditionDatabase': The Payload has not been loaded.
```
because MC events have `run=1`, which is outside the DB IOV.

---

## 7. Bug Fix: `CreateDB.sh` — `skip` option breaks Step 4

**Problem:** When the user enters `skip` at the "Replay type" prompt (to skip Step 1), `aodtype=skip` was passed directly to `checkep_cfg.py` in Step 4. Since `checkep_cfg.py` has no `process.source` block for `aodType=skip`, CMSSW crashes with:
```
An exception of category 'Configuration':
There must be exactly one source in the configuration.
```

**Fix:** Added a separate `checkaodtype` variable that defaults to `MiniAOD` when `aodtype=skip`:
```bash
if [ $aodtype = "skip" ]; then
    checkaodtype="MiniAOD"
    infile=''
else
    checkaodtype=$aodtype
fi
```
Step 4 now uses `aodType=$checkaodtype` instead of `aodType=$aodtype`.

---

## 8. Bug Fix: `checkep_cfg.py` TFileService output

**Run2 (old):**
```python
process.TFileService = cms.Service("TFileService",
    fileName = cms.string("checkep.root")  # hardcoded!
)
```

**Run3 (fixed):**
```python
process.TFileService = cms.Service("TFileService",
    fileName = cms.string(ivars.checkfile)  # uses command-line argument
)
```

**Impact:** Without this fix, the `checkfile` command-line argument passed by `CreateDB.sh` was silently ignored and output was always written to `checkep.root` in the current directory regardless of what was passed.

---

## 8. EPCalib Standalone Code (`EPCalib/EPCalib.C`, headers)

**Run2 (old):** `HiEvtPlaneList.h` and `HiEvtPlaneFlatten.h` were symlinked from `$CMSSW_BASE`.

**Run3 (new):** Copies of these files are kept directly in `EPCalib/`. The `HiEvtPlaneFlatten.h` copy has `FWCore` and `DataFormats` include statements commented out (required for standalone ROOT compilation outside CMSSW).

The symlink creation lines in `CreateDB.sh` are commented out accordingly:
```bash
# [ -h HiEvtPlaneList.h ] && rm -f HiEvtPlaneList.h
# ln -s $CMSSW_BASE/src/RecoHI/HiEvtPlaneAlgos/interface/HiEvtPlaneList.h
```

---

## 9. Event Plane Detectors (Run3 EP list)

The `HiEvtPlaneList.h` for Run3 2024 PbPb contains **12 EP names** (reduced from Run2):

| Index | Name | Harmonic |
|---|---|---|
| 0 | HFm2 | 2 |
| 1 | HFp2 | 2 |
| 2 | HF2 | 2 |
| 3 | trackmid2 | 2 |
| 4 | trackm2 | 2 |
| 5 | trackp2 | 2 |
| 6 | HFm3 | 3 |
| 7 | HFp3 | 3 |
| 8 | HF3 | 3 |
| 9 | trackmid3 | 3 |
| 10 | trackm3 | 3 |
| 11 | trackp3 | 3 |

Run2 had additional EP detectors (e.g., tracker full range, CASTOR). Castor was decommissioned for Run3.

---

## Summary of Files Modified

| File | Change |
|---|---|
| `calibtree_cfg.py` | Global tag, input dataset/format, centrality GT (no manual toGet) |
| `checkep_cfg.py` | Global tag, input dataset/format, **TFileService uses `ivars.checkfile`** |
| `CreateDB.sh` | Run range (2018→2024), default aodType=MiniAOD, calibrunmin/max for MC, added check DB step |
| `EPCalib/HiEvtPlaneList.h` | Updated to Run3 EP list (12 planes) |
| `EPCalib/HiEvtPlaneFlatten.h` | FWCore/DataFormats includes commented out for standalone use |
