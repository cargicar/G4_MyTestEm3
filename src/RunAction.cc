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
/// \file electromagnetic/TestEm3/src/RunAction.cc
/// \brief Implementation of the RunAction class
//
//
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#include "RunAction.hh"
#include "RunData.hh"
#include "Analysis.hh"

#include "G4Run.hh"
#include "G4UnitsTable.hh"
#include "G4SystemOfUnits.hh"
#include <sstream>

#include "DetectorConstruction.hh"
#include "HistoManager.hh"
#include "PrimaryGeneratorAction.hh"
#include "Run.hh"
#include "RunActionMessenger.hh"

#include "G4RunManager.hh"
#include "G4Timer.hh"
#include "Randomize.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

RunAction::RunAction(DetectorConstruction* det, PrimaryGeneratorAction* prim)
  : fDetector(det), fPrimary(prim),  G4UserRunAction()
{
  fRunMessenger = new RunActionMessenger(this);
  fHistoManager = new HistoManager();
  // set printing event number per each event
  G4RunManager::GetRunManager()->SetPrintProgress(1);     

  // Create analysis manager
  // The choice of analysis technology is done via selectin of a namespace
  // in Analysis.hh
  G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();
  G4cout << "Using " << analysisManager->GetType() << G4endl;

  // Create directories 
  //analysisManager->SetHistoDirectoryName("histograms");
  //analysisManager->SetNtupleDirectoryName("ntuple");
  analysisManager->SetVerboseLevel(1);
  analysisManager->SetFirstHistoId(1);

  // Book histograms, ntuple
  //
  
  // Creating histograms
  // analysisManager->CreateH1("1","Edep in absorber", 100, 0., 800*MeV);
  // analysisManager->CreateH1("2","Edep in gap", 100, 0., 100*MeV);
  // analysisManager->CreateH1("3","trackL in absorber", 100, 0., 1*m);
  // analysisManager->CreateH1("4","trackL in gap", 100, 0., 50*cm);

  // Creating ntuple
  //

  char const* val = getenv("GAN_TREENAME"); 
  std::string fname = (val == NULL ? std::string("fancy_tree") : std::string(val));


  analysisManager->CreateNtuple(fname.c_str(), "Edep and TrackL");

  int total_bins = 504 + 3;  // 3 overflow bins for the three calo layers

  for (int i = 0; i < total_bins; ++i) {

    std::stringstream out;
    out << i;
    analysisManager->CreateNtupleDColumn("cell_" + out.str());
  }
  analysisManager->CreateNtupleDColumn("TotalEnergy");
  
  // analysisManager->CreateNtupleDColumn("Eabs");
  // analysisManager->CreateNtupleDColumn("Egap");
  // analysisManager->CreateNtupleDColumn("Labs");
  // analysisManager->CreateNtupleDColumn("Lgap");



  analysisManager->FinishNtuple();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......


RunAction::~RunAction()
{
  delete fRunMessenger;
  delete G4AnalysisManager::Instance();  
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4Run* RunAction::GenerateRun()
{
  fRun = new Run(fDetector);
  // drun = new RunData;
  return fRun, new RunData;
}


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void RunAction::BeginOfRunAction(const G4Run*)
{
  // keep run condition
  if (fPrimary) {
    G4ParticleDefinition* particle = fPrimary->GetParticleGun()->GetParticleDefinition();
    G4double energy = fPrimary->GetParticleGun()->GetParticleEnergy();
    fRun->SetPrimary(particle, energy);
  }

  // histograms
  //
  G4AnalysisManager* analysis = G4AnalysisManager::Instance();
  G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();

  char const* val = getenv("GAN_FNAME"); 
  //std::string fname = (val == NULL ? std::string("plz_work_kthxbai") : std::string(val));
  std::string fname = (val == NULL ? std::string("calogan_interactive.root") : std::string(val));


  G4String fileName = fname.c_str();
  analysisManager->OpenFile(fileName);
  
  if (analysis->IsActive()) analysis->OpenFile();

  // save Rndm status and open the timer

  if (isMaster) {
    //    G4Random::showEngineStatus();
    fTimer = new G4Timer();
    fTimer->Start();
  }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void RunAction::EndOfRunAction(const G4Run*)
{
  // compute and print statistic
  if (isMaster) {
    fTimer->Stop();
    if (!((G4RunManager::GetRunManager()->GetRunManagerType() == G4RunManager::sequentialRM))) {
      G4cout << "\n"
             << "Total number of events:  " << fRun->GetNumberOfEvent() << G4endl;
      G4cout << "Master thread time:  " << *fTimer << G4endl;
    }
    delete fTimer;
    fRun->EndOfRun();
  }
  // save histograms
  G4AnalysisManager* analysis = G4AnalysisManager::Instance();
  G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();
  if (analysis->IsActive()) {
    analysis->Write();
    analysis->CloseFile();
  }
  analysisManager->Write();
  analysisManager->CloseFile();

  // show Rndm status
  //  if (isMaster)  G4Random::showEngineStatus();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void RunAction::SetEdepAndRMS(G4int i, G4double edep, G4double rms, G4double lim)
{
  if (fRun) fRun->SetEdepAndRMS(i, edep, rms, lim);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void RunAction::SetApplyLimit(G4bool val)
{
  if (fRun) fRun->SetApplyLimit(val);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
