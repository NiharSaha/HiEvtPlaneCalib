import FWCore.ParameterSet.Config as cms
import FWCore.ParameterSet.VarParsing as VarParsing
import os
import sys
ivars = VarParsing.VarParsing('standard')

ivars.register ('lumifile',
                '',
                mult=ivars.multiplicity.singleton,
                mytype=ivars.varType.string,
                info="lumi file")

ivars.register ('dbfile',
                'HeavyIonRPRcd_offline.db',
                mult=ivars.multiplicity.singleton,
                mytype=ivars.varType.string,
                info="dbfile file")

ivars.register ('checkfile',
                'check.root',
                mult=ivars.multiplicity.singleton,
                mytype=ivars.varType.string,
                info="check output file")

ivars.register ('aodType',
                'MiniAOD',
                mult=ivars.multiplicity.singleton,
                mytype=ivars.varType.string,
                info="AOD/TestAOD/MiniAOD/testMiniAOD")

ivars.register ('repFile',
                'infile.root',
                mult=ivars.multiplicity.singleton,
                mytype=ivars.varType.string,
                info="single file replay")

ivars.register ('inputType',
                'MC',
                mult=ivars.multiplicity.singleton,
                mytype=ivars.varType.string,
                info="MC or Data: selects which input files to use for MiniAOD mode")

ivars.parseArguments()

process = cms.Process("check")
process.load('Configuration.StandardSequences.Services_cff')
process.load("CondCore.CondDB.CondDB_cfi")
process.load('FWCore.MessageService.MessageLogger_cfi')
process.load('Configuration.StandardSequences.GeometryDB_cff')
process.load('Configuration.StandardSequences.MagneticField_38T_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
process.load('Configuration.StandardSequences.EndOfProcess_cff')
#if ivars.aodType == 'AOD':
#	process.load("HeavyIonsAnalysis.Configuration.hfCoincFilter_cff")
#	process.load("HeavyIonsAnalysis.Configuration.analysisFilters_cff")
#	process.load("HeavyIonsAnalysis.Configuration.collisionEventSelection_cff")

process.load("HeavyIonsAnalysis.HiEvtPlaneCalib.checkflattening_cfi")
process.load("RecoHI.HiEvtPlaneAlgos.HiEvtPlane_cfi")
process.load("RecoHI.HiEvtPlaneAlgos.hiEvtPlaneFlat_cfi")

from Configuration.AlCa.GlobalTag import GlobalTag
# Select global tag based on inputType:
#   Data: 141X_dataRun3_Prompt_forHI_NominalCentrality
#   MC:   141X_mcRun3_2024_realistic_HI_v16
if ivars.inputType == 'Data':
    process.GlobalTag = GlobalTag(process.GlobalTag, '141X_dataRun3_Prompt_forHI_NominalCentrality', '')
else:
    process.GlobalTag = GlobalTag(process.GlobalTag, '141X_mcRun3_2024_realistic_HI_v16', '')


process.load('RecoHI.HiCentralityAlgos.HiCentrality_cfi')
process.load("RecoHI.HiCentralityAlgos.CentralityBin_cfi")
process.centralityBin.Centrality = cms.InputTag("hiCentrality")
process.centralityBin.centralityVariable = cms.string("HFtowers")

#process.load('HeavyIonsAnalysis.Configuration.hfCoincFilter_cff')
#process.load('HeavyIonsAnalysis.Configuration.collisionEventSelection_cff')
process.load('RecoHI.HiCentralityAlgos.CentralityFilter_cfi')

#process.eventSelection = cms.Sequence(
#		process.primaryVertexFilter
#    )

process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(-1) )
process.MessageLogger.cerr.FwkReport.reportEvery=100

process.CondDB.connect = "sqlite_file:"+ivars.dbfile
process.PoolDBESSource = cms.ESSource("PoolDBESSource",
                                       process.CondDB,
                                       toGet = cms.VPSet(cms.PSet(record = cms.string('HeavyIonRPRcd'),
                                                                  tag = cms.string('HeavyIonRPRcd')
                                                                  )
                                                         )
                                      )
process.es_prefer_flatparms = cms.ESPrefer('PoolDBESSource','')

if not ivars.lumifile.isspace()  :
    import FWCore.PythonUtilities.LumiList as LumiList
    goodLumiSecs = LumiList.LumiList(filename = ivars.lumifile ).getCMSSWString().split(',')

if ivars.aodType == 'testMiniAOD':
    process.source = cms.Source("PoolSource", fileNames = cms.untracked.vstring(
            'file:'+ivars.repFile
        ),
        inputCommands=cms.untracked.vstring(
            'keep *',
            'drop *_hiEvtPlane_*_*'
        )
    )
elif ivars.aodType == 'MiniAOD':
    if ivars.inputType == 'Data':
        # 2024 PbPb real data: HIRun2024A HIPhysicsRawPrime0 MINIAOD PromptReco-v1
        process.source = cms.Source("PoolSource", fileNames = cms.untracked.vstring(
            '/store/hidata/HIRun2024A/HIPhysicsRawPrime0/MINIAOD/PromptReco-v1/000/387/749/00000/f26d9cb3-f878-4136-b2d3-ae669e445d4a.root'
            ),
            inputCommands=cms.untracked.vstring(
                'keep *',
                'drop *_hiEvtPlane_*_*'
            )
        )
    else:  # MC
        # 2024 PbPb MC: Hydjet MinBias MiniAODSIM (HINPbPbWinter24, 5.36 TeV)
        process.source = cms.Source("PoolSource", fileNames = cms.untracked.vstring(
            '/store/mc/HINPbPbWinter24MiniAOD/Hydjet_MinBias_TuneCELLO_5p36TeV_pythia8/MINIAODSIM/NoPU_141X_mcRun3_2024_realistic_HI_v14-v2/2810000/6b0edb76-0f4e-49e4-a8b4-7d614b72e92f.root',
            '/store/mc/HINPbPbWinter24MiniAOD/Hydjet_MinBias_TuneCELLO_5p36TeV_pythia8/MINIAODSIM/NoPU_141X_mcRun3_2024_realistic_HI_v14-v2/2810000/00e1cc61-b738-47ce-8d89-e0e1a7290212.root',
            '/store/mc/HINPbPbWinter24MiniAOD/Hydjet_MinBias_TuneCELLO_5p36TeV_pythia8/MINIAODSIM/NoPU_141X_mcRun3_2024_realistic_HI_v14-v2/2810000/04840c4e-7732-4846-bbbd-4940bfdcb8cd.root',
            '/store/mc/HINPbPbWinter24MiniAOD/Hydjet_MinBias_TuneCELLO_5p36TeV_pythia8/MINIAODSIM/NoPU_141X_mcRun3_2024_realistic_HI_v14-v2/2810000/1230138a-d4f6-4828-a2f7-adf024d0d190.root'
            ),
            inputCommands=cms.untracked.vstring(
                'keep *',
                'drop *_hiEvtPlane_*_*'
            )
        )

process.TFileService = cms.Service("TFileService",
        fileName = cms.string(ivars.checkfile)
    )


process.dump = cms.EDAnalyzer("EventContentAnalyzer")

process.hiEvtPlane.trackTag = cms.InputTag("generalTracks")
process.hiEvtPlane.vertexTag = cms.InputTag("offlinePrimaryVertices")
process.hiEvtPlaneFlat.vertexTag = cms.InputTag("offlinePrimaryVertices")
if ivars.aodType == 'MiniAOD':
    process.hiEvtPlane.trackTag = cms.InputTag("packedPFCandidates")
    process.hiEvtPlane.vertexTag = cms.InputTag("offlineSlimmedPrimaryVertices")
    process.hiEvtPlaneFlat.vertexTag = cms.InputTag("offlineSlimmedPrimaryVertices")

process.hiEvtPlane.loadDB = cms.bool(True)
process.hiEvtPlaneFlat.centralityVariable=process.hiEvtPlane.centralityVariable
process.hiEvtPlaneFlat.vertexTag=process.hiEvtPlane.vertexTag
process.hiEvtPlaneFlat.flatminvtx=process.hiEvtPlane.flatminvtx
process.hiEvtPlaneFlat.flatnvtxbins=process.hiEvtPlane.flatnvtxbins
process.hiEvtPlaneFlat.flatdelvtx=process.hiEvtPlane.flatdelvtx
process.hiEvtPlaneFlat.FlatOrder=process.hiEvtPlane.FlatOrder
process.hiEvtPlaneFlat.CentBinCompression=process.hiEvtPlane.CentBinCompression
process.hiEvtPlaneFlat.caloCentRef=process.hiEvtPlane.caloCentRef
process.hiEvtPlaneFlat.caloCentRefWidth=process.hiEvtPlane.caloCentRefWidth

process.checkflattening.centralityVariable=process.hiEvtPlane.centralityVariable
process.checkflattening.vertexTag=process.hiEvtPlane.vertexTag
process.checkflattening.minvtx=process.hiEvtPlane.flatminvtx
process.checkflattening.flatnvtxbins=process.hiEvtPlane.flatnvtxbins
process.checkflattening.latminvtx=process.hiEvtPlane.flatminvtx
process.checkflattening.flatdelvtx=process.hiEvtPlane.flatdelvtx
process.checkflattening.FlatOrder=process.hiEvtPlane.FlatOrder
process.checkflattening.CentBinCompression=process.hiEvtPlane.CentBinCompression
process.checkflattening.caloCentRef=process.hiEvtPlane.caloCentRef
process.checkflattening.caloCentRefWidth=process.hiEvtPlane.caloCentRefWidth

process.dump = cms.EDAnalyzer("EventContentAnalyzer")

if ivars.aodType == 'testMiniAOD' or ivars.aodType == 'MiniAOD':
    process.p = cms.Path(process.hiEvtPlane*process.hiEvtPlaneFlat*process.checkflattening)

