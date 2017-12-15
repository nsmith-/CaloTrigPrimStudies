import FWCore.ParameterSet.Config as cms

fantasyTriggerPrimitives = cms.EDProducer(
    "FantasyTrigPrimProducer",
    ebRecHits = cms.InputTag("ecalRecHit", "EcalRecHitsEB"),
    eeRecHits = cms.InputTag("ecalRecHit", "EcalRecHitsEE"),
    hbheRecHits = cms.InputTag("hbhereco"),
    hfRecHits = cms.InputTag("hfreco"),
)
