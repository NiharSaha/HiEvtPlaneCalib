import os
import CRABClient
from WMCore.Configuration import Configuration
config = Configuration()

# ============================================================
#  USER SETTINGS — edit these before submitting
# ============================================================
# Set inputType to 'MC' or 'Data'
inputType = 'Data'   # <-- 'Data' for real 2025 PbPb calibration, 'MC' for Hydjet
outfile = 'calibData.root' if inputType == 'Data' else 'calibMC.root'

username    = 'nsaha'             # your CMS username
storagesite = 'T2_US_Purdue'  # a T2 you have write access to
workArea    = 'crab_EPcalib_PbPb2025'
data        = 'HIPhysicsRawPrime0'   # 2025 has HIPhysicsRawPrime0-59
tag         = 'Oct15'

# Dataset and run-range settings — chosen automatically by inputType
if inputType == 'Data':
    # 2025 PbPb HIRun2025A: dataset has runs 398954-400426, golden JSON certifies 399465-400426
    dataset   = f'/{data}/HIRun2025A-PromptReco-v1/MINIAOD'
    runRange  = '399465-400426'
    # Golden JSON for 2025 PbPb (81 runs, 399465-400426), from
    # /cvmfs/cms-griddata.cern.ch/cat/metadata/DC/Collisions25HI/latest/
    lumiMask  = 'Cert_Collisions2025_HI_399465_400426_Golden.json'
else:
    # 2025 PbPb MC: Hydjet Quenched MinBias MiniAODSIM (HINPbPbWinter25)
    dataset   = '/Hydjet_Quenched_MinBias_TuneCELLO_5p36TeV_pythia8/HINPbPbWinter25MiniAOD-NoPU_151X_mcRun3_2025_realistic_HI_v5-v2/MINIAODSIM'
    runRange  = None
    lumiMask  = None
# ============================================================

config.section_('General')
config.General.transferOutputs = True
config.General.transferLogs = True
config.General.workArea = workArea
config.General.requestName = f'{data}_{tag}'
config.section_('JobType')
config.JobType.outputFiles = [outfile]
config.JobType.pluginName = 'Analysis'
config.JobType.psetName = '../calibtree_cfg.py'
config.section_('Data')
config.Data.allowNonValidInputDataset = True
config.Data.publication = False
config.Data.splitting = 'LumiBased'
config.Data.unitsPerJob = 10
config.Data.totalUnits = -1
config.Data.outputDatasetTag = f'{data}_{tag}'
config.Data.outLFNDirBase = f'/store/user/{username}/{workArea}/'
config.section_('User')
config.section_('Site')
config.Site.storageSite = storagesite

config.JobType.pyCfgParams = ['noprint', 'aodType=MiniAOD', 'inputType=' + inputType, 'outfile=' + outfile]
config.Data.inputDataset = dataset
config.Data.inputDBS = 'global'

# Apply run range and lumi mask when set (Data only)
if runRange is not None:
    config.Data.runRange = runRange
if lumiMask is not None:
    config.Data.lumiMask = lumiMask


