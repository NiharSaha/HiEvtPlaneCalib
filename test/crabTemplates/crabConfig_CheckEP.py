import os
import CRABClient
from WMCore.Configuration import Configuration
config = Configuration()

dataset = '/Hydjet_MinBias_TuneCELLO_5p36TeV_pythia8/HINPbPbWinter24MiniAOD-NoPU_141X_mcRun3_2024_realistic_HI_v14-v2/MINIAODSIM'
user = 'ssanders'
store = 'MiniAOD_check'
work = 'crab_projects'
storagesite = 'T2_US_Vanderbilt'
dbfile='../HeavyIonRPRcd_offline.db'

config.section_('General')
config.General.transferOutputs = True
config.General.transferLogs = True
config.section_('JobType')
config.JobType.outputFiles = ['check.root']
config.JobType.pluginName = 'Analysis'
config.JobType.psetName = '../checkep_cfg.py'
config.section_('Data')
config.Data.allowNonValidInputDataset = True
config.Data.publication = False
config.Data.splitting = 'Automatic'
config.section_('User')
config.section_('Site')
config.Site.storageSite = storagesite
config.JobType.inputFiles = [ dbfile ]
config.JobType.pyCfgParams = ['noprint','aodType=MiniAOD','dbfile='+dbfile]
config.Data.inputDataset = dataset
config.Data.inputDBS = 'global'
