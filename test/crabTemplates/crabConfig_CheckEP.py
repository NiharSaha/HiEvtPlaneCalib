import os
import CRABClient
from WMCore.Configuration import Configuration
config = Configuration()

# ============================================================
#  USER SETTINGS — edit these before submitting
# ============================================================
# Set inputType to 'MC' or 'Data'
inputType = 'Data'

username    = 'nsaha'                 # your CMS username
storagesite = 'T2_US_Purdue'          # a T2 you have write access to
workArea    = 'crab_EPcheck_PbPb2024'
data        = 'HIPhysicsRawPrime0'    # one PD is enough for validation
tag         = 'Oct15'

# Run-wise DB produced by create_db_runwise.sh (shipped with the job)
dbfile = '../HeavyIonRPRcd_PbPb2024_Runwise.db'

# Dataset and run-range settings — chosen automatically by inputType
if inputType == 'Data':
    dataset  = f'/{data}/HIRun2024B-PromptReco-v2/MINIAOD'
    # Runs used for the run-wise comparison plots (subset of the 20 calibrated runs)
    runRange = '388468,388622,388713,388784'
    lumiMask = 'Cert_Collisions2024_HI_387853_388784_Golden.json'
else:
    # 2024 PbPb MC: Hydjet MinBias MiniAODSIM
    dataset  = '/Hydjet_MinBias_TuneCELLO_5p36TeV_pythia8/HINPbPbWinter24MiniAOD-NoPU_141X_mcRun3_2024_realistic_HI_v14-v2/MINIAODSIM'
    runRange = None
    lumiMask = None
# ============================================================

config.section_('General')
config.General.transferOutputs = True
config.General.transferLogs = True
config.General.workArea = workArea
config.General.requestName = f'{data}_check_{tag}'
config.section_('JobType')
config.JobType.outputFiles = ['check.root']
config.JobType.pluginName = 'Analysis'
config.JobType.psetName = '../checkep_cfg.py'
# The DB is copied into the job's working directory, so the job uses its basename
config.JobType.inputFiles = [ dbfile ]
config.JobType.pyCfgParams = ['noprint', 'aodType=MiniAOD', 'inputType=' + inputType,
                              'dbfile=' + os.path.basename(dbfile)]
config.section_('Data')
config.Data.allowNonValidInputDataset = True
config.Data.publication = False
config.Data.splitting = 'Automatic'
config.Data.outputDatasetTag = f'{data}_check_{tag}'
config.Data.outLFNDirBase = f'/store/user/{username}/{workArea}/'
config.Data.inputDataset = dataset
config.Data.inputDBS = 'global'
config.section_('User')
config.section_('Site')
config.Site.storageSite = storagesite

# Apply run range and lumi mask when set (Data only)
if runRange is not None:
    config.Data.runRange = runRange
if lumiMask is not None:
    config.Data.lumiMask = lumiMask
