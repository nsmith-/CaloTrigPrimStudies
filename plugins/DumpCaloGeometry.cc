// -*- C++ -*-
//
// Package:    Analysis/DumpCaloGeometry
// Class:      DumpCaloGeometry
// 
/**\class DumpCaloGeometry DumpCaloGeometry.cc Analysis/DumpCaloGeometry/plugins/DumpCaloGeometry.cc

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
#include "FWCore/Framework/interface/one/EDAnalyzer.h"

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

class DumpCaloGeometry : public edm::one::EDAnalyzer<edm::one::SharedResources, edm::one::WatchRuns>  {
  public:
    explicit DumpCaloGeometry(const edm::ParameterSet&);
    ~DumpCaloGeometry();

    static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);


  private:
    virtual void beginJob() override;
    virtual void beginRun(const edm::Run&, const edm::EventSetup&) override;
    virtual void endRun(edm::Run const&, edm::EventSetup const&) override {}
    virtual void analyze(const edm::Event&, const edm::EventSetup&) override;
    virtual void endJob() override;

    edm::ESHandle<CaloGeometry> caloGeometry_;
    edm::ESHandle<EcalTrigTowerConstituentsMap> ecalTriggerTowerMap_;
    edm::ESHandle<HcalTrigTowerGeometry> hcalTriggerTowerMap_;

    const CaloSubdetectorGeometry * ebGeometry;
    const CaloSubdetectorGeometry * eeGeometry;
    const CaloSubdetectorGeometry * hbGeometry;
    const CaloSubdetectorGeometry * heGeometry;
    const CaloSubdetectorGeometry * hfGeometry;
    const CaloSubdetectorGeometry * hcalTriggerGeometry;
};

DumpCaloGeometry::DumpCaloGeometry(const edm::ParameterSet& iConfig)

{
  usesResource("TFileService");
}


DumpCaloGeometry::~DumpCaloGeometry()
{
}


void 
DumpCaloGeometry::beginJob()
{
}


void
DumpCaloGeometry::beginRun(const edm::Run& iRun, const edm::EventSetup& iSetup)
{
  iSetup.get<CaloGeometryRecord>().get(caloGeometry_);
  iSetup.get<IdealGeometryRecord>().get(ecalTriggerTowerMap_);
  iSetup.get<CaloGeometryRecord>().get(hcalTriggerTowerMap_);

  std::cout << "detid,etaPos,phiPos,etaSpan,phiSpan,depth,ieta,iphi" << std::endl;

  ebGeometry = caloGeometry_->getSubdetectorGeometry(DetId::Ecal, EcalBarrel);
  for ( const auto& detId : ebGeometry->getValidDetIds() ) {
    const CaloCellGeometry * cell = ebGeometry->getGeometry(detId);
    float depth = cell->getBackPoint().mag() - cell->getPosition().mag();
    EcalTrigTowerDetId towerDetId = ecalTriggerTowerMap_->towerOf(detId);

    std::cout << detId.rawId()
      << "," << cell->etaPos() << "," << cell->phiPos()
      << "," << cell->etaSpan() << "," << cell->phiSpan() << "," << depth
      << "," << towerDetId.ieta() << "," << towerDetId.iphi()
      << std::endl;
  }

  eeGeometry = caloGeometry_->getSubdetectorGeometry(DetId::Ecal, EcalEndcap);
  for ( const auto& detId : eeGeometry->getValidDetIds() ) {
    const CaloCellGeometry * cell = eeGeometry->getGeometry(detId);
    float depth = cell->getBackPoint().mag() - cell->getPosition().mag();
    EcalTrigTowerDetId towerDetId = ecalTriggerTowerMap_->towerOf(detId);

    std::cout << detId.rawId()
      << "," << cell->etaPos() << "," << cell->phiPos()
      << "," << cell->etaSpan() << "," << cell->phiSpan() << "," << depth
      << "," << towerDetId.ieta() << "," << towerDetId.iphi()
      << std::endl;
  }

  hbGeometry = caloGeometry_->getSubdetectorGeometry(DetId::Hcal, HcalBarrel);
  for ( const auto& detId : hbGeometry->getValidDetIds() ) {
    const CaloCellGeometry * cell = hbGeometry->getGeometry(detId);
    float depth = cell->getBackPoint().mag() - cell->getPosition().mag();

    auto towerDetIds = hcalTriggerTowerMap_->towerIds(detId);
    if ( towerDetIds.size() == 0 ) {
      std::cout << "No-tower hcal DetId" << std::endl;
      continue;
    }
    HcalTrigTowerDetId towerDetId = * std::min_element(begin(towerDetIds), end(towerDetIds), [](auto a, auto b) {
        // a < b, prefer highest version lower iphi
        return a.version() > b.version() || ( a.version() == b.version() && a.iphi() < b.iphi() );
      });

    std::cout << detId.rawId()
      << "," << cell->etaPos() << "," << cell->phiPos()
      << "," << cell->etaSpan() << "," << cell->phiSpan() << "," << depth
      << "," << towerDetId.ieta() << "," << towerDetId.iphi()
      << std::endl;
  }

  std::cout << "reading he" << std::endl;
  heGeometry = caloGeometry_->getSubdetectorGeometry(DetId::Hcal, HcalEndcap);

  std::cout << "reading hf" << std::endl;
  hfGeometry = caloGeometry_->getSubdetectorGeometry(DetId::Hcal, HcalForward);

  std::cout << "reading hcal tt" << std::endl;
  hcalTriggerGeometry = caloGeometry_->getSubdetectorGeometry(DetId::Hcal, HcalTriggerTower);
}


void
DumpCaloGeometry::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  using namespace edm;

}


void 
DumpCaloGeometry::endJob() 
{
}


void
DumpCaloGeometry::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

DEFINE_FWK_MODULE(DumpCaloGeometry);
