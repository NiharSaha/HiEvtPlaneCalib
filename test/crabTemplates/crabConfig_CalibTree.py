import os
import CRABClient
from WMCore.Configuration import Configuration
config = Configuration()

# ============================================================
#  USER SETTINGS — edit these before submitting
# ============================================================
# Set inputType to 'MC' or 'Data'
inputType = 'Data'   # <-- change to 'Data' for real 2023 PbPb calibration
outfile = 'calibData.root' if inputType == 'Data' else 'calibMC.root'

username    = 'nsaha'             # your CMS username
storagesite = 'T2_US_Purdue'  # a T2 you have write access to
workArea    = 'crab_EPcalib_PbPb2023'
data        = 'HIPhysicsRawPrime0'
tag         = 'July15'

# Dataset and run-range settings — chosen automatically by inputType
if inputType == 'Data':
    # 2023 PbPb HIRun2023A certified run range: 379650-380400
    dataset   = f'/{data}/HIRun2023A-PromptReco-v2/MINIAOD'
    runRange  = '374288-375823'
    # Golden JSON for 2023 PbPb HIRun2023A (runs 374288-375823)
    lumiMask  = 'Cert_Collisions2023HI_374288_375823_Golden.json'
else:
    # 2023 PbPb MC: Hydjet MinBias MiniAODSIM
    dataset   = '/Hydjet_MinBias_TuneCELLO_5p36TeV_pythia8/HINPbPbWinter23MiniAOD-NoPU_141X_mcRun3_2023_realistic_HI_v14-v2/MINIAODSIM'
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

