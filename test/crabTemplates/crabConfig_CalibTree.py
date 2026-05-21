import os
import CRABClient
from WMCore.Configuration import Configuration
config = Configuration()

# ============================================================
#  USER SETTINGS — edit these before submitting
# ============================================================
# Set inputType to 'MC' or 'Data'
inputType = 'MC'   # <-- change to 'Data' for real 2024 PbPb calibration

user        = 'nsaha'             # your CMS username
storagesite = 'T2_US_Vanderbilt'  # a T2 you have write access to
store       = 'MiniAOD_calib'
work        = 'crab_projects'

# Dataset and run-range settings — chosen automatically by inputType
if inputType == 'Data':
    # 2024 PbPb HIRun2024A certified run range: 387853-388784
    dataset   = '/HIPhysicsRawPrime0/HIRun2024A-PromptReco-v1/MINIAOD'
    runRange  = '387853-388784'
    # Golden JSON for 2024 PbPb HIRun2024A (runs 387853-388784)
    lumiMask  = '/eos/user/c/cmsdqm/www/CAF/certification/Collisions24HI/Cert_Collisions2024_HI_387853_388784_Golden.json'
else:
    # 2024 PbPb MC: Hydjet MinBias MiniAODSIM
    dataset   = '/Hydjet_MinBias_TuneCELLO_5p36TeV_pythia8/HINPbPbWinter24MiniAOD-NoPU_141X_mcRun3_2024_realistic_HI_v14-v2/MINIAODSIM'
    runRange  = None
    lumiMask  = None
# ============================================================

config.section_('General')
config.General.transferOutputs = True
config.General.transferLogs = True
config.section_('JobType')
config.JobType.outputFiles = ['calib.root']
config.JobType.pluginName = 'Analysis'
config.JobType.psetName = '../calibtree_cfg.py'
config.section_('Data')
config.Data.allowNonValidInputDataset = True
config.Data.publication = False
config.Data.splitting = 'Automatic'
config.section_('User')
config.section_('Site')
config.Site.storageSite = storagesite

config.JobType.pyCfgParams = ['noprint', 'aodType=MiniAOD', 'inputType=' + inputType]
config.Data.inputDataset = dataset
config.Data.inputDBS = 'global'

# Apply run range and lumi mask when set (Data only)
if runRange is not None:
    config.Data.runRange = runRange
if lumiMask is not None:
    config.Data.lumiMask = lumiMask


