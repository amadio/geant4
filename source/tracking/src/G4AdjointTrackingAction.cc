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
//
// $Id: G4AdjointTrackingAction.cc 75409 2013-11-01 00:31:00Z ldesorgh$
//
// ---------------------------------------------------------------
//
//
//
// ---------------------------------------------------------------

#include "G4AdjointTrackingAction.hh"
#include "G4ParticleTable.hh"
#include "G4Track.hh"
#include "G4AdjointSteppingAction.hh"



/////////////////////////////////////////////////////////
G4AdjointTrackingAction::G4AdjointTrackingAction(
                                         G4AdjointSteppingAction* anAction)
 :theAdjointSteppingAction(anAction),theUserFwdTrackingAction(0)
{;}
/////////////////////////////////////////////////////////
G4AdjointTrackingAction::~G4AdjointTrackingAction()
/////////////////////////////////////////////////////////
{;}
/////////////////////////////////////////////////////////
void G4AdjointTrackingAction::PreUserTrackingAction(const G4Track* aTrack)
{ G4String partType = aTrack->GetParticleDefinition()->GetParticleType();
  if (aTrack->GetDynamicParticle()->GetPrimaryParticle() &&
              partType.contains(G4String("adjoint"))){
   is_adjoint_tracking_mode =true;
   theAdjointSteppingAction->SetPrimWeight(aTrack->GetWeight());
  }
  else {
   is_adjoint_tracking_mode =false;
   if (theUserFwdTrackingAction)
        theUserFwdTrackingAction->PreUserTrackingAction(aTrack);
  }
  theAdjointSteppingAction->SetAdjointTrackingMode(is_adjoint_tracking_mode);
}
/////////////////////////////////////////////////////////
 void G4AdjointTrackingAction::PostUserTrackingAction(const G4Track* aTrack)
{ if(!is_adjoint_tracking_mode){
	if (theUserFwdTrackingAction)
		theUserFwdTrackingAction->PostUserTrackingAction(aTrack);
  }
  else if (theAdjointSteppingAction->GetDidAdjParticleReachTheExtSource()){
    last_pos = theAdjointSteppingAction->GetLastPosition();
    last_direction = theAdjointSteppingAction->GetLastMomentum();
    last_direction /=last_direction.mag();
    last_cos_th =  last_direction.z();
    G4ParticleDefinition* aPartDef= theAdjointSteppingAction->GetLastPartDef();
    last_fwd_part_name= aPartDef->GetParticleName();
    last_fwd_part_name.remove(0,4);
    last_fwd_part_PDGEncoding=G4ParticleTable::GetParticleTable()
                         ->FindParticle(last_fwd_part_name)->GetPDGEncoding();
    last_ekin = theAdjointSteppingAction->GetLastEkin();
    last_ekin_nuc = last_ekin;
    if (aPartDef->GetParticleType() == "adjoint_nucleus") {
      G4double  nb_nuc=double(aPartDef->GetBaryonNumber());
      last_ekin_nuc /=nb_nuc;
    }
    last_weight = theAdjointSteppingAction->GetLastWeight(); ;
  }
}

