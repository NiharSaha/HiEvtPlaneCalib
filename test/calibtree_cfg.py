import FWCore.ParameterSet.Config as cms
import FWCore.ParameterSet.VarParsing as VarParsing
import os
import sys
ivars = VarParsing.VarParsing('analysis')

ivars.register('outfile',
               'calibMC.root',
		VarParsing.VarParsing.multiplicity.singleton,
		VarParsing.VarParsing.varType.string,
                "output file name")

ivars.register ('aodType',
                'MiniAOD',
                mult=ivars.multiplicity.singleton,
                mytype=ivars.varType.string,
                info="AOD/testAOD/MiniAOD/testMiniAOD")

ivars.register ('repFile',
                'root://cmsxrootd.fnal.gov//store',
                mult=ivars.multiplicity.singleton,
                mytype=ivars.varType.string,
                info="Data file to be replayed. Null if crab submission.")

ivars.register ('inputType',
                'MC',
                mult=ivars.multiplicity.singleton,
                mytype=ivars.varType.string,
                info="MC or Data: selects which input files to use for MiniAOD mode")


ivars.parseArguments()

process = cms.Process("FlatCalib")

process.load('Configuration.StandardSequences.Services_cff')
process.load("CondCore.CondDB.CondDB_cfi")
process.load('FWCore.MessageService.MessageLogger_cfi')
process.load('Configuration.StandardSequences.GeometryDB_cff')
process.load('Configuration.StandardSequences.MagneticField_38T_cff')
process.load('Configuration.StandardSequences.EndOfProcess_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
#process.load("HeavyIonsAnalysis.Configuration.hfCoincFilter_cff")
#process.load("HeavyIonsAnalysis.Configuration.analysisFilters_cff")
#process.load("HeavyIonsAnalysis.Configuration.collisionEventSelection_cff")
#process.load("HeavyIonsAnalysis.HiEvtPlaneCalib.checkflattening_cfi")
process.load("RecoHI.HiEvtPlaneAlgos.HiEvtPlane_cfi")
process.load("RecoHI.HiEvtPlaneAlgos.EvtPlaneFilter_cfi")
process.load("RecoHI.HiEvtPlaneAlgos.hiEvtPlaneFlat_cfi")
process.load("HeavyIonsAnalysis.HiEvtPlaneCalib.evtplanecalibtree_cfi")

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
#	process.primaryVertexFilter
#    )

process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(10000))
process.MessageLogger.cerr.FwkReport.reportEvery = 100


if ivars.aodType == 'testMiniAOD':
    process.source = cms.Source("PoolSource", fileNames = cms.untracked.vstring(
            'file:'+ivars.repFile
        ),
        inputCommands=cms.untracked.vstring(
            'keep *',
            'drop *_hiEvtPlane_*_*'
        )
    )

if ivars.aodType == 'MiniAOD':
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
        fileName = cms.string(ivars.outfile)
    )
if ivars.aodType == 'MiniAOD':
    process.hiEvtPlane.trackTag = cms.InputTag("packedPFCandidates")
    process.hiEvtPlane.vertexTag = cms.InputTag("offlineSlimmedPrimaryVertices")
 
process.dump = cms.EDAnalyzer("EventContentAnalyzer")
process.evtPlaneCalibTree.centralityVariable = process.hiEvtPlane.centralityVariable
process.evtPlaneCalibTree.centralityBinTag = process.hiEvtPlane.centralityBinTag
process.evtPlaneCalibTree.vertexTag = process.hiEvtPlane.vertexTag
process.evtPlaneCalibTree.caloTag = process.hiEvtPlane.caloTag
process.evtPlaneCalibTree.castorTag = process.hiEvtPlane.castorTag
process.evtPlaneCalibTree.trackTag = process.hiEvtPlane.trackTag
process.evtPlaneCalibTree.lostTag = process.hiEvtPlane.lostTag
process.evtPlaneCalibTree.chi2MapTag = process.hiEvtPlane.chi2MapTag
process.evtPlaneCalibTree.chi2MapLostTag = process.hiEvtPlane.chi2MapLostTag
process.evtPlaneCalibTree.nonDefaultGlauberModel = process.hiEvtPlane.nonDefaultGlauberModel
process.evtPlaneCalibTree.loadDB = process.hiEvtPlane.loadDB
process.evtPlaneCalibTree.minet = process.hiEvtPlane.minet
process.evtPlaneCalibTree.maxet = process.hiEvtPlane.maxet
process.evtPlaneCalibTree.minpt = process.hiEvtPlane.minpt
process.evtPlaneCalibTree.flatnvtxbins = process.hiEvtPlane.flatnvtxbins
process.evtPlaneCalibTree.flatminvtx = process.hiEvtPlane.flatminvtx
process.evtPlaneCalibTree.flatdelvtx = process.hiEvtPlane.flatdelvtx
process.evtPlaneCalibTree.dzdzerror = process.hiEvtPlane.dzdzerror
process.evtPlaneCalibTree.d0d0error = process.hiEvtPlane.d0d0error
process.evtPlaneCalibTree.pterror = process.hiEvtPlane.pterror
process.evtPlaneCalibTree.chi2perlayer = process.hiEvtPlane.chi2perlayer
process.evtPlaneCalibTree.dzdzerror_pix = process.hiEvtPlane.dzdzerror_pix
process.evtPlaneCalibTree.chi2 = process.hiEvtPlane.chi2
process.evtPlaneCalibTree.nhitsValid = process.hiEvtPlane.nhitsValid
process.evtPlaneCalibTree.FlatOrder = process.hiEvtPlane.FlatOrder
process.evtPlaneCalibTree.NumFlatBins = process.hiEvtPlane.NumFlatBins
process.evtPlaneCalibTree.caloCentRef = process.hiEvtPlane.caloCentRef
process.evtPlaneCalibTree.caloCentRefWidth = process.hiEvtPlane.caloCentRefWidth
process.evtPlaneCalibTree.CentBinCompression = process.hiEvtPlane.CentBinCompression
process.evtPlaneCalibTree.cutEra = process.hiEvtPlane.cutEra

process.hiEvtPlane.loadDB = cms.bool(False)

if ivars.aodType == 'testMiniAOD' or ivars.aodType == 'MiniAOD':
    process.p = cms.Path(process.centralityBin*process.hiEvtPlane * process.evtPlaneCalibTree  )


