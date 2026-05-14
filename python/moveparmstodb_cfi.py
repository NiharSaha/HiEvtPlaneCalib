import FWCore.ParameterSet.Config as cms

moveparmstodb = cms.EDAnalyzer("MoveFlatParamsToDB",
                               rescorloc = cms.string("rescorloc"),
                               outtag = cms.string("outtag"),
                               infile = cms.string("infile")
                               )
