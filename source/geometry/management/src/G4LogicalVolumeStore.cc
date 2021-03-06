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
// G4LogicalVolumeStore implementation for singleton container
//
// 10.07.95 - P.Kent, Initial version
// --------------------------------------------------------------------

#include "G4Types.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4GeometryManager.hh"

// ***************************************************************************
// Static class variables
// ***************************************************************************
//
G4LogicalVolumeStore* G4LogicalVolumeStore::fgInstance = nullptr;
G4ThreadLocal G4VStoreNotifier* G4LogicalVolumeStore::fgNotifier = nullptr;
G4ThreadLocal G4bool G4LogicalVolumeStore::locked = false;

// ***************************************************************************
// Protected constructor: Construct underlying container with
// initial size of 100 entries
// ***************************************************************************
//
G4LogicalVolumeStore::G4LogicalVolumeStore()
 : std::vector<G4LogicalVolume*>()
{
  reserve(100);
}

// ***************************************************************************
// Destructor
// ***************************************************************************
//
G4LogicalVolumeStore::~G4LogicalVolumeStore()
{
  Clean();  // Delete all volumes in the store
  G4LogicalVolume::Clean();  // Delete allocated sub-instance data
}

// ***************************************************************************
// Delete all elements from the store
// ***************************************************************************
//
void G4LogicalVolumeStore::Clean()
{
  // Do nothing if geometry is closed
  //
  if (G4GeometryManager::IsGeometryClosed())
  {
    G4cout << "WARNING - Attempt to delete the logical volume store"
           << " while geometry closed !" << G4endl;
    return;
  }

  // Locks store for deletion of volumes. De-registration will be
  // performed at this stage. G4LogicalVolumes will not de-register themselves.
  //
  locked = true;  

  size_t i = 0;
  G4LogicalVolumeStore* store = GetInstance();

#ifdef G4GEOMETRY_VOXELDEBUG
  G4cout << "Deleting Logical Volumes ... ";
#endif

  for(auto pos=store->cbegin(); pos!=store->cend(); ++pos)
  {
    if (fgNotifier != nullptr) { fgNotifier->NotifyDeRegistration(); }
    if (*pos != nullptr) { (*pos)->Lock(); delete *pos; }
    ++i;
  }

#ifdef G4GEOMETRY_VOXELDEBUG
  if (store->size() < i-1)
    { G4cout << "No volumes deleted. Already deleted by user ?" << G4endl; }
  else
    { G4cout << i-1 << " volumes deleted !" << G4endl; }
#endif

  locked = false;
  store->clear();
}

// ***************************************************************************
// Associate user notifier to the store
// ***************************************************************************
//
void G4LogicalVolumeStore::SetNotifier(G4VStoreNotifier* pNotifier)
{
  GetInstance();
  fgNotifier = pNotifier;
}

// ***************************************************************************
// Add volume to container
// ***************************************************************************
//
void G4LogicalVolumeStore::Register(G4LogicalVolume* pVolume)
{
  GetInstance()->push_back(pVolume);
  if (fgNotifier) { fgNotifier->NotifyRegistration(); }
}

// ***************************************************************************
// Remove volume from container
// ***************************************************************************
//
void G4LogicalVolumeStore::DeRegister(G4LogicalVolume* pVolume)
{
  if (!locked)    // Do not de-register if locked !
  {
    if (fgNotifier != nullptr) { fgNotifier->NotifyDeRegistration(); }
    for (auto i=GetInstance()->cbegin(); i!=GetInstance()->cend(); ++i)
    {
      if (**i==*pVolume)
      {
        GetInstance()->erase(i);
        break;
      }
    }
  }
}

// ***************************************************************************
// Retrieve the first volume pointer in the container having that name
// ***************************************************************************
//
G4LogicalVolume*
G4LogicalVolumeStore::GetVolume(const G4String& name, G4bool verbose) const
{
  for (auto i=GetInstance()->cbegin(); i!=GetInstance()->cend(); ++i)
  {
    if ((*i)->GetName() == name) { return *i; }
  }
  if (verbose)
  {
     std::ostringstream message;
     message << "Volume NOT found in store !" << G4endl
             << "        Volume " << name << " NOT found in store !" << G4endl
             << "        Returning NULL pointer.";
     G4Exception("G4LogicalVolumeStore::GetVolume()",
                 "GeomMgt1001", JustWarning, message);
  }
  return 0;
}

// ***************************************************************************
// Return ptr to Store, setting if necessary
// ***************************************************************************
//
G4LogicalVolumeStore* G4LogicalVolumeStore::GetInstance()
{
  static G4LogicalVolumeStore worldStore;
  if (fgInstance == nullptr)
  {
    fgInstance = &worldStore;
  }
  return fgInstance;
}
