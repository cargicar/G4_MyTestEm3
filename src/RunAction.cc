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
//....oooOO0OOooo......CaloGan
#include "RunData.hh"
#include "Analysis.hh"
#include "G4Run.hh"
#include "G4RunManager.hh"
#include "G4UnitsTable.hh"
#include "G4SystemOfUnits.hh"
#include <sstream>
//....oooOO0OOooo......

#include "DetectorConstruction.hh"
#include "HistoManager.hh"
#include "PrimaryGeneratorAction.hh"
#include "Run.hh"
#include "RunActionMessenger.hh"

#include "G4RunManager.hh"
#include "G4Timer.hh"
#include "Randomize.hh"

// CaloGan
#include "RunAction.hh"
#include "RunData.hh"
#include "Analysis.hh"
#include "G4Run.hh"
//#include "G4RunManager.hh"
#include "G4UnitsTable.hh"
#include "G4SystemOfUnits.hh"
#include <sstream>

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......



//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

RunAction::RunAction(DetectorConstruction* det, PrimaryGeneratorAction* prim)
  : fDetector(det),
    fPrimary(prim),
    G4UserRunAction()
{
  fRunMessenger = new RunActionMessenger(this);
  fHistoManager = new HistoManager();

   //....oooOO0OOooo........oooOO0OOooo.CaloGan BEGIN oooOO0OOooo........oooOO0OOooo......
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
  

  analysisManager->FinishNtuple();
  //....oooOO0OOooo........oooOO0OOooo.CaloGan END oooOO0OOooo........oooOO0OOooo......
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

RunAction::~RunAction()
{
  delete fRunMessenger;
  delete G4AnalysisManager::Instance();  // CaloGan
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4Run* RunAction::GenerateRun()
{
  fRun = new Run(fDetector);
  fMyRunData = new RunData(); // CaloGan
  return fRun;
}

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

  // CaloGan
  if (fMyRunData){
  
  char const* val = getenv("GAN_FNAME"); 
  //std::string fname = (val == NULL ? std::string("plz_work_kthxbai") : std::string(val));
  std::string fname = (val == NULL ? std::string("calogan_bining.root") : std::string(val));
  G4String fileName = fname.c_str();
  //analysisManager->OpenFile(fileName);// CaloGan
  if (analysis->IsActive()) {
  analysis->OpenFile(fileName);
    }
  }
  //
  //if (analysis->IsActive()) {
  // analysis->OpenFile();
   
  // }
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
  if (analysis->IsActive()) {
    analysis->Write();
    analysis->CloseFile();
  }

  // show Rndm status
  //  if (isMaster)  G4Random::showEngineStatus();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void RunAction::SetEdepAndRMS(G4int i, G4double edep, G4double rms, G4double lim)
{
  if (fRun) fRun->SetEdepAndRMS(i, edep, rms, lim);
}


//....oooOO0OOooo........oooOO0OOooo.Merge with CaloGan RunAction. Begin oooOO0OOooo........oooOO0OOooo......

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

// G4Run* RunAction::GenerateRun()
// {
//   // Both scripts have a GenerateRun method.
//   // Script 1 creates a Run object with a pointer to the detector.
//   // Script 2 creates a RunData object.
//   // We'll prioritize creating a RunData object and potentially set the detector within it
//   // if RunData has a setter for it. If not, you might need to adjust based on your RunData class.

//   // Assuming RunData can handle or doesn't need fDetector:
//   return new RunData();

//   // If RunData needs fDetector, and has a SetDetector method:
//   // RunData* runData = new RunData();
//   // runData->SetDetector(fDetector);
//   // return runData;

//   // If you strictly need the functionality of the first script's Run class:
//   // return new Run(fDetector);
// }

// //....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

// void RunAction::BeginOfRunAction(const G4Run* run)
// {
//   G4cout << "### Run " << run->GetRunID() << " start." << G4endl;

//   // Keep run condition (from script 1)
//   if (fPrimary) {
//     G4ParticleDefinition* particle = fPrimary->GetParticleGun()->GetParticleDefinition();
//     G4double energy = fPrimary->GetParticleGun()->GetParticleEnergy();
//     if (fRun) { // Ensure fRun is valid if you still use it
//       fRun->SetPrimary(particle, energy);
//     }
//   }

//   // Get analysis manager (from both scripts)
//   G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();
//   if (analysisManager->IsActive()) analysisManager->OpenFile();

//   // Open an output file with a name from environment variable (from script 2)
//   char const* val = getenv("GAN_FNAME");
//   std::string fileName = (val == NULL ? std::string("p+_ener_1_to_100GeV_nparticles_1000.root") : std::string(val));
//   analysisManager->OpenFile(fileName); // This will overwrite the previous OpenFile if called immediately

//   // save Rndm status and open the timer (from script 1)
//   if (isMaster) {
//     fTimer = new G4Timer();
//     fTimer->Start();
//   }

//   //inform the runManager to save random number seed (from script 2 - commented out)
//   //G4RunManager::GetRunManager()->SetRandomNumberStore(true);
// }

// //....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

// void RunAction::EndOfRunAction(const G4Run* aRun)
// {
//   // Compute and print statistic (from script 1)
//   if (isMaster) {
//     fTimer->Stop();
//     if (!((G4RunManager::GetRunManager()->GetRunManagerType() == G4RunManager::sequentialRM))) {
//       G4cout << "\n"
//              << "Total number of events:  " << aRun->GetNumberOfEvent() << G4endl;
//       G4cout << "Master thread time:  " << *fTimer << G4endl;
//     }
//     delete fTimer;
//     if (fRun) { // Ensure fRun is valid if you still use it
//       fRun->EndOfRun();
//     }
//   }

//   // Print histogram statistics (from script 2 - commented out)
//   // G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();
//   // if ( analysisManager->GetH1(1) ) { ... }

//   // Save histograms & ntuple (from both scripts)
//   G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();
//   if (analysisManager->IsActive()) {
//     analysisManager->Write();
//     analysisManager->CloseFile();
//   }

//   // Show Rndm status (from script 1 - commented out)
//   // if (isMaster)  G4Random::showEngineStatus();
// }

//....oooOO0OOooo........oooOO0OOooo.END .oooOO0OOooo........oooOO0OOooo......
void RunAction::SetApplyLimit(G4bool val)
{
  if (fRun) fRun->SetApplyLimit(val);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
