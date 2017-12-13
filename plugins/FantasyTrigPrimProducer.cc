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

#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "TTree.h"

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
  // analogous to EcalTrigTowerConstituentsMap::towerOf()
  auto hcalTowerOf = [this](const DetId& detId) -> HcalTrigTowerDetId {
    auto towerDetIds = hcalTriggerTowerMap_->towerIds(detId);
    if ( towerDetIds.size() == 0 ) {
      std::cout << "HCAL DetId that is not mapped to a trigger tower" << detId.rawId() << std::endl;
      return HcalTrigTowerDetId();
    }
    HcalTrigTowerDetId towerDetId = * std::min_element(begin(towerDetIds), end(towerDetIds),
      [](auto a, auto b) {
        // a < b, prefer highest version lower iphi
        return a.version() > b.version() || ( a.version() == b.version() && a.iphi() < b.iphi() );
      }
    );
    return towerDetId;
  };


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
    HcalTrigTowerDetId towerDetId = hcalTowerOf(hit.id());
    hbheTThits[towerDetId].emplace_back(&hit);
  }

  std::map<HcalTrigTowerDetId, std::vector<const HFRecHit*>> hfTThits;
  for ( const auto& hit : *hfRecHits ) {
    HcalTrigTowerDetId towerDetId = hcalTowerOf(hit.id());
    hfTThits[towerDetId].emplace_back(&hit);
  }

  (void) ecalTThits;
  (void) hbheTThits;
  (void) hfTThits;
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
