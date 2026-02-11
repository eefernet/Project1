/*
  ==============================================================================

    Guest.cpp
    Created: 10 Feb 2026 6:03:53pm
    Author:  ethan

  ==============================================================================
*/
#include "Guest.h"

Guest::Guest() : User()
{
    role = UserRole::Guest;
}

Guest::Guest(int userId, juce::String userName, juce::String password)
    : User(userId, userName, password, UserRole::Guest)
{
}

Guest::~Guest()
{
}

//This will be removed once pushed to prod, we would segue here to the main application window
//TODO: Implement a proper welcome screen with options to navigate to different features based on user role
void Guest::displayWelcome()
{
    DBG("=================================");
    DBG("Welcome Guest: " << userName);
    DBG("Limited access - Browse and play sounds");
    DBG("=================================");
}

void Guest::viewSoundList()
{
    DBG("Guest::viewSoundList() - TODO: Display available sounds");
    //Spint 2: Implement sound list view
}

void Guest::viewClusterMap()
{
    DBG("Guest::viewClusterMap() - TODO: Display cluster map (read-only)");
    //Sprint 2: Implement cluster map view
}

void Guest::playAndFilter()
{
    DBG("Guest::playAndFilter() - TODO: Play sound with or withjout filters");
    //Sprint 3: Implement playback with filters
}

void Guest::downloadSound()
{
    DBG("Guest::downloadSound() - TODO: Download sound file");
    //Sprint 3: Implement download functionality
}