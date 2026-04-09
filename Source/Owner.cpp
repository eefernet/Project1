/*
  ==============================================================================

    Owner.cpp
    Created: 10 Feb 2026 6:03:43pm
    Author:  ethan

  ==============================================================================
*/

#include "Owner.h"

Owner::Owner() : User()
{
    role = UserRole::Owner;
}

Owner::Owner(int userId, juce::String userName, juce::String password)
    : User(userId, userName, password, UserRole::Owner)
{
}

Owner::~Owner()
{
}

void Owner::displayWelcome()
{
    //Right now just displaying this to console for testing
    DBG("=================================");
    DBG("Welcome Owner: " << userName);
    DBG("You have full access to all features");
    DBG("=================================");
}

//TODO: REMOVING THESE FUNCTIONS BECAUSE I WENT A DIFFERENT ROUTE ON HOW WE PLAYBACK AND VIEW EACH VIEW
//OLD CODE PLEASE DO NOT USE THESE FUNCTIONS, THEY ARE DEPRECATED
void Owner::recordSound()
{
    DBG("Owner::recordSound() - TODO: Implement audio recording");
    //Sprint 2/3: Implement actual recording
}

void Owner::viewWaveForm()
{
    DBG("Owner::viewWaveForm() - TODO: Display waveform visualization");
    //Sprint 2: Implement waveform display
}

void Owner::playSound()
{
    DBG("Owner::playSound() - TODO: Play selected sound");
    //Sprint 3: Implement playback
}

void Owner::filterSound()
{
    DBG("Owner::filterSound() - TODO: Apply filter to sound");
    //Sprint 2: Implement filtering
}

void Owner::saveSound()
{
    DBG("Owner::saveSound() - TODO: Save sound to disk");
    //Sprint 3: Implement save functionality
}

void Owner::listSavedSounds()
{
    DBG("Owner::listSavedSounds() - TODO: Display list of saved sounds");
    //Sprint 2: Implement sound library view
}

void Owner::viewClusterMap()
{
    DBG("Owner::viewClusterMap() - TODO: Display cluster map view");
    //Sprint 2: Implement cluster map visualization
}

void Owner::createGuestAccount()
{
    DBG("Owner::createGuestAccount() - TODO: Create new guest user");
    //Sprint 1: Implement guest account creation
}
