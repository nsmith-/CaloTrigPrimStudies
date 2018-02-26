// -*- C++ -*-
//
// Package:    L1Trigger/CaloTrigPrimStudies
// Class:      FantasyTrigPrimProducer
// 
/**\class FantasyTrigPrimProducer FantasyTrigPrimProducer.cc L1Trigger/CaloTrigPrimStudies/plugins/FantasyTrigPrimProducer.cc

 Description: 

 Implementation:
     
*/
//
// Original Author:  Nicholas Charles Smith
//         Created:  Thu, 09 Jun 2016 10:27:00 GMT
//
//


// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/ESHandle.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "DataFormats/CaloTowers/interface/CaloTowerDetId.h"
#include "DataFormats/DetId/interface/DetId.h"
#include "DataFormats/EcalDetId/interface/EBDetId.h"
#include "DataFormats/EcalDetId/interface/EEDetId.h"
#include "DataFormats/EcalDetId/interface/EcalTrigTowerDetId.h"
#include "DataFormats/EcalDetId/interface/EcalSubdetector.h"
#include "DataFormats/HcalDetId/interface/HcalDetId.h"
#include "DataFormats/HcalDetId/interface/HcalTrigTowerDetId.h"
#include "DataFormats/HcalDetId/interface/HcalSubdetector.h"

#include "Geometry/CaloEventSetup/interface/CaloTopologyRecord.h"
#include "Geometry/CaloGeometry/interface/CaloCellGeometry.h"
#include "Geometry/CaloGeometry/interface/CaloGeometry.h"
#include "Geometry/CaloGeometry/interface/CaloSubdetectorGeometry.h"
#include "Geometry/CaloTopology/interface/CaloSubdetectorTopology.h"
#include "Geometry/CaloTopology/interface/CaloTopology.h"
#include "Geometry/CaloTopology/interface/CaloTowerConstituentsMap.h"
#include "Geometry/CaloTopology/interface/EcalTrigTowerConstituentsMap.h"
#include "Geometry/HcalTowerAlgo/interface/HcalTrigTowerGeometry.h"
#include "Geometry/Records/interface/CaloGeometryRecord.h"
#include "Geometry/Records/interface/IdealGeometryRecord.h"

#include "DataFormats/EcalRecHit/interface/EcalRecHit.h"
#include "DataFormats/EcalRecHit/interface/EcalRecHitCollections.h"
#include "DataFormats/HcalRecHit/interface/HBHERecHit.h"
#include "DataFormats/HcalRecHit/interface/HFRecHit.h"
#include "DataFormats/HcalRecHit/interface/HcalRecHitCollections.h"

#include "DataFormats/EcalDetId/interface/EcalTrigTowerDetId.h"
#include "DataFormats/EcalDigi/interface/EcalDigiCollections.h"
#include "DataFormats/EcalDigi/interface/EcalTriggerPrimitiveDigi.h"
#include "DataFormats/EcalDigi/interface/EcalTriggerPrimitiveSample.h"

#include "DataFormats/HcalDetId/interface/HcalTrigTowerDetId.h"
#include "DataFormats/HcalDigi/interface/HcalDigiCollections.h"
#include "DataFormats/HcalDigi/interface/HcalTriggerPrimitiveDigi.h"
#include "DataFormats/HcalDigi/interface/HcalTriggerPrimitiveSample.h"

#include "CalibFormats/CaloTPG/interface/CaloTPGTranscoder.h"
#include "CalibFormats/CaloTPG/interface/CaloTPGRecord.h"
#include "CalibFormats/CaloTPG/interface/HcalTPGCompressor.h"

class FantasyTrigPrimProducer : public edm::stream::EDProducer<>  {
  public:
    explicit FantasyTrigPrimProducer(const edm::ParameterSet&);
    ~FantasyTrigPrimProducer();

    static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);


  private:
    virtual void beginRun(const edm::Run&, const edm::EventSetup&) override;
    virtual void endRun(edm::Run const&, edm::EventSetup const&) override {}
    virtual void produce(edm::Event&, const edm::EventSetup&) override;

    edm::ESHandle<CaloGeometry> caloGeometry_;
    edm::ESHandle<EcalTrigTowerConstituentsMap> ecalTriggerTowerMap_;
    edm::ESHandle<HcalTrigTowerGeometry> hcalTriggerTowerMap_;

    edm::EDGetTokenT<EcalRecHitCollection> ebRecHitsToken_;
    edm::EDGetTokenT<EcalRecHitCollection> eeRecHitsToken_;
    edm::EDGetTokenT<HBHERecHitCollection> hbheRecHitsToken_;
    edm::EDGetTokenT<HFRecHitCollection> hfRecHitsToken_;

};

FantasyTrigPrimProducer::FantasyTrigPrimProducer(const edm::ParameterSet& iConfig) :
  ebRecHitsToken_(consumes<EcalRecHitCollection>(iConfig.getParameter<edm::InputTag>("ebRecHits"))),
  eeRecHitsToken_(consumes<EcalRecHitCollection>(iConfig.getParameter<edm::InputTag>("eeRecHits"))),
  hbheRecHitsToken_(consumes<HBHERecHitCollection>(iConfig.getParameter<edm::InputTag>("hbheRecHits"))),
  hfRecHitsToken_(consumes<HFRecHitCollection>(iConfig.getParameter<edm::InputTag>("hfRecHits")))
{
  produces<EcalTrigPrimDigiCollection>();
  produces<HcalTrigPrimDigiCollection>();
}


FantasyTrigPrimProducer::~FantasyTrigPrimProducer()
{
}


void
FantasyTrigPrimProducer::beginRun(const edm::Run& iRun, const edm::EventSetup& iSetup)
{
  iSetup.get<CaloGeometryRecord>().get(caloGeometry_);
  iSetup.get<IdealGeometryRecord>().get(ecalTriggerTowerMap_);
  iSetup.get<CaloGeometryRecord>().get(hcalTriggerTowerMap_);
}


void
FantasyTrigPrimProducer::produce(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  edm::ESHandle<CaloTPGTranscoder> decoder;
  iSetup.get<CaloTPGRecord>().get(decoder);

  std::unique_ptr<EcalTrigPrimDigiCollection> ecalTPs(new EcalTrigPrimDigiCollection);
  std::unique_ptr<HcalTrigPrimDigiCollection> hcalTPs(new HcalTrigPrimDigiCollection);

  edm::Handle<EcalRecHitCollection> ebRecHits;
  iEvent.getByToken(ebRecHitsToken_, ebRecHits);
  edm::Handle<EcalRecHitCollection> eeRecHits;
  iEvent.getByToken(eeRecHitsToken_, eeRecHits);
  edm::Handle<HBHERecHitCollection> hbheRecHits;
  iEvent.getByToken(hbheRecHitsToken_, hbheRecHits);
  edm::Handle<HFRecHitCollection> hfRecHits;
  iEvent.getByToken(hfRecHitsToken_, hfRecHits);


  std::map<EcalTrigTowerDetId, std::vector<const EcalRecHit*>> ecalTThits;
  for ( const auto& hit : *ebRecHits ) {
    EcalTrigTowerDetId towerDetId = ecalTriggerTowerMap_->towerOf(hit.id());
    ecalTThits[towerDetId].emplace_back(&hit);
  }
  for ( const auto& hit : *eeRecHits ) {
    EcalTrigTowerDetId towerDetId = ecalTriggerTowerMap_->towerOf(hit.id());
    ecalTThits[towerDetId].emplace_back(&hit);
  }

  std::map<HcalTrigTowerDetId, std::vector<const HBHERecHit*>> hbheTThits;
  for ( const auto& hit : *hbheRecHits ) {
    auto towerDetIds = hcalTriggerTowerMap_->towerIds(hit.id());
    for ( const auto& towerDetId : towerDetIds ) {
      hbheTThits[towerDetId].emplace_back(&hit);
    }
  }

  std::map<HcalTrigTowerDetId, std::vector<const HFRecHit*>> hfTThits;
  for ( const auto& hit : *hfRecHits ) {
    auto towerDetIds = hcalTriggerTowerMap_->towerIds(hit.id());
    for ( const auto& towerDetId : towerDetIds ) {
      hfTThits[towerDetId].emplace_back(&hit);
    }
  }

  auto ebGeometry = caloGeometry_->getSubdetectorGeometry(DetId::Ecal, EcalBarrel);
  auto eeGeometry = caloGeometry_->getSubdetectorGeometry(DetId::Ecal, EcalEndcap);
  auto hcalGeometry = caloGeometry_->getSubdetectorGeometry(DetId::Hcal, HcalBarrel);  // actually all of HCAL

  for ( const auto& dh : ecalTThits ) {
    int absIeta = std::abs(dh.first.ieta());
    double towerEt = 0.;
    for ( const auto* hit : dh.second ) {
      if ( hit->id().subdetId() == EcalBarrel ) {
        towerEt += hit->energy() / cosh(ebGeometry->getGeometry(hit->id())->etaPos());
      } else {
        towerEt += hit->energy() / cosh(eeGeometry->getGeometry(hit->id())->etaPos());
      }
    }

    // Reduced granularity
    if ( absIeta > 26 ) {
      towerEt *= 0.5;
    }
    // LSB 0.5 GeV
    uint32_t rawTowerEt = int(round(towerEt * 2.)) & 0xff;

    EcalTriggerPrimitiveDigi tp(dh.first);
    tp.setSize(1);
    EcalTriggerPrimitiveSample sample(rawTowerEt); 
    tp.setSample(0, sample);
    ecalTPs->push_back(tp);
    if ( absIeta > 26 ) {
      EcalTrigTowerDetId id2(dh.first.zside(), EcalSubdetector::EcalEndcap, std::abs(dh.first.ieta()), dh.first.iphi()+1*dh.first.zside());
      EcalTriggerPrimitiveDigi tp(id2);
      tp.setSize(1);
      EcalTriggerPrimitiveSample sample(rawTowerEt); 
      tp.setSample(0, sample);
      ecalTPs->push_back(tp);
    }
  }

  for ( const auto& dh : hbheTThits ) {
    int absIeta = std::abs(dh.first.ieta());
    double towerEt = 0.;
    for ( const auto* hit : dh.second ) {
      towerEt += hit->energy() / cosh(hcalGeometry->getGeometry(hit->id())->etaPos());
    }

    // Reduced granularity
    if ( absIeta > 20 ) {
      towerEt *= 0.5;
    }
    // Soon, LSB 0.5 GeV
    uint32_t rawTowerEt = int(round(towerEt * 2.)) & 0xff;

    HcalTriggerPrimitiveDigi tp(dh.first);
    tp.setSize(1);
    // Need to comress, easier to pull changes from https://github.com/cms-sw/cmssw/pull/21657/files
    // HcalTriggerPrimitiveSample sample = decoder->getHcalCompressor()->compress(dh.first, rawTowerEt, 0);
    // std::cout << rawTowerEt*0.5 << decoder->hcaletValue(dh.first, sample) << std::endl;
    HcalTriggerPrimitiveSample sample(rawTowerEt);
    tp.setSample(0, sample);
    hcalTPs->push_back(tp);
  }

  for ( const auto& dh : hfTThits ) {
    double towerEt = 0.;
    for ( const auto* hit : dh.second ) {
      towerEt += hit->energy() / cosh(hcalGeometry->getGeometry(hit->id())->etaPos());
    }

    // LSB 0.5 GeV
    uint32_t rawTowerEt = int(round(towerEt * 2.)) & 0xff;

    HcalTriggerPrimitiveDigi tp(dh.first);
    tp.setSize(1);
    HcalTriggerPrimitiveSample sample(rawTowerEt); 
    tp.setSample(0, sample);
    hcalTPs->push_back(tp);
  }

  iEvent.put(std::move(ecalTPs));
  iEvent.put(std::move(hcalTPs));
}


void
FantasyTrigPrimProducer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

DEFINE_FWK_MODULE(FantasyTrigPrimProducer);
