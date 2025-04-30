//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
/// \file electromagnetic/TestEm3/src/SteppingAction.cc
/// \brief Implementation of the SteppingAction class
//
//
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#include "SteppingAction.hh"

#include "DetectorConstruction.hh"
#include "EventAction.hh"
#include "HistoManager.hh"
#include "Run.hh"
#include "RunData.hh"

#include "G4PhysicalConstants.hh"
#include "G4Positron.hh"
#include "G4RunManager.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SteppingAction::SteppingAction(DetectorConstruction* det, EventAction* evt)
  : fDetector(det), fEventAct(evt)
{}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SteppingAction::~SteppingAction()
{ 
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
// Bining
int SteppingAction::WhichZBin(double zpos){

  //zsegmentation = TH1F("","",3,np.array([-240.,-150.,197.,240.]))
  if (zpos < -150.) return 0;
  else if (zpos < 197.) return 1;
  else return 2;

}

int SteppingAction::WhichXYbin(double xpos, double ypos, int zbin){
  int xbin = -1;
  int ybin = -1;
  int nbins1x = 3;
  int nbins2x = 12;
  int nbins3x = 12;
  int nbins1y = 96;
  int nbins2y = 12;
  int nbins3y = 6;
  int nbinsx[]={nbins1x,nbins2x,nbins3x};
  int nbinsy[]={nbins1y,nbins2y,nbins3y};

  for (int i=1; i<=nbinsx[zbin]; i++){
    if ((xpos < -240 + i*480/nbinsx[zbin]) && (xpos > -240)){
      xbin = i - 1;
      //G4cout << "###### XBIN ###########" << xbin << G4endl; 
      break;
    }
  }
  for (int i=1; i<=nbinsy[zbin]; i++){
    if ((ypos < -240 +i*480/nbinsy[zbin]) && (ypos > -240)){
      ybin = i - 1;
      break;
    }
  }


  int lvl1 = nbins1x * nbins1y;
  int lvl2 = nbins2x * nbins2y;
  int lvl3 = nbins3x * nbins3y;



  if ((xbin == -1) || (ybin == -1)) {
    return lvl1 + lvl2 + lvl3 + zbin;
  }

  if (zbin == 0) {
    return xbin * nbins1y + ybin;
  } 
  else if (zbin == 1) {
    return lvl1 + (xbin * nbins2y + ybin);
  }
  else {
    return (lvl1 + lvl2) + (xbin * nbins3y + ybin);
  }



  // return zbin*1e4 + xbin*1e2 + ybin;
  //sampling1_eta = TH2F("","",3,-240.,240.,480/5,-240.,240.)
  //sampling2_eta = TH2F("","",480/40,-240.,240.,480/40,-240.,240.)
  //sampling3_eta = TH2F("","",480/40,-240.,240.,480/80,-240.,240.)
}
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......


void SteppingAction::UserSteppingAction(const G4Step* aStep)
{
  // track informations
  const G4StepPoint* prePoint1 = aStep->GetPreStepPoint();
  const G4StepPoint* prePoint2 = aStep->GetPreStepPoint();
  G4ThreeVector pos1 = prePoint1->GetPosition();
  G4ThreeVector pos2 = prePoint2->GetPosition();


  // if World, return
  //
  G4VPhysicalVolume* volume = prePoint1->GetTouchableHandle()->GetCopyNumber(0);
  G4int layerNum = prePoint1->GetTouchableHandle()->GetCopyNumber(1);
    // int mybin = 0;
  //G4cout << "zbin " << WhichZBin(pos1.z()) << " " << mybin << " " << mybin%100 << std::endl;

  
  // get Run
  Run* run = static_cast<Run*>(G4RunManager::GetRunManager()->GetNonConstCurrentRun());

  // get RunData, copied from CaloGan
  RunData* runData = static_cast<RunData*>
    (G4RunManager::GetRunManager()->GetNonConstCurrentRun());

  // collect energy deposit taking into account track weight
  G4double edep = aStep->GetTotalEnergyDeposit() * aStep->GetTrack()->GetWeight();

  // collect step length of charged particles
  G4double stepl = 0.;
  if (particle->GetPDGCharge() != 0.) {
    stepl = aStep->GetStepLength();
    run->AddChargedStep();
  }
  else {
    run->AddNeutralStep();
  }
  
  // runData->Add(mybin, edep, stepLength); 
  runData->Add(mybin, edep); 

  //  G4cout << "Nabs= " << absorNum << "   edep(keV)= " << edep << G4endl;

  // sum up per event
  fEventAct->SumEnergy(absorNum, edep, stepl);

  // longitudinal profile of edep per absorber
  if (edep > 0.) {
    G4AnalysisManager::Instance()->FillH1(kMaxAbsor + absorNum, G4double(layerNum + 1), edep);
  }
  // energy flow
  //
  //  unique identificator of layer+absorber
  G4int Idnow = (fDetector->GetNbOfAbsor()) * layerNum + absorNum;
  G4int plane;
  //
  // leaving the absorber ?
  if (endPoint1->GetStepStatus() == fGeomBoundary) {
    G4ThreeVector position = endPoint1->GetPosition();
    G4ThreeVector direction = endPoint1->GetMomentumDirection();
    G4double sizeYZ = 0.5 * fDetector->GetCalorSizeYZ();
    G4double Eflow = endPoint1->GetKineticEnergy();
    if (particle == G4Positron::Positron()) Eflow += 2 * electron_mass_c2;
    if ((std::abs(position.y()) >= sizeYZ) || (std::abs(position.z()) >= sizeYZ))
      run->SumLateralEleak(Idnow, Eflow);
    else if (direction.x() >= 0.)
      run->SumEnergyFlow(plane = Idnow + 1, Eflow);
    else
      run->SumEnergyFlow(plane = Idnow, -Eflow);
  }

  ////  example of Birk attenuation
  /// G4double destep   = aStep->GetTotalEnergyDeposit();
  /// G4double response = BirksAttenuation(aStep);
  /// G4cout << " Destep: " << destep/keV << " keV"
  ///       << " response after Birks: " << response/keV << " keV" << G4endl;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4double SteppingAction::BirksAttenuation(const G4Step* aStep)
{
  // Example of Birk attenuation law in organic scintillators.
  // adapted from Geant3 PHYS337. See MIN 80 (1970) 239-244
  //
  const G4Material* material = aStep->GetTrack()->GetMaterial();
  G4double birk1 = material->GetIonisation()->GetBirksConstant();
  G4double destep = aStep->GetTotalEnergyDeposit();
  G4double stepl = aStep->GetStepLength();
  G4double charge = aStep->GetTrack()->GetDefinition()->GetPDGCharge();
  //
  G4double response = destep;
  if (birk1 * destep * stepl * charge != 0.) {
    response = destep / (1. + birk1 * destep / stepl);
  }
  return response;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
