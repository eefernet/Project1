/*
  ==============================================================================

    Owner.h
    Created: 10 Feb 2026 6:03:43pm
    Author:  ethan

  ==============================================================================
*/

#pragma once
#include "User.h"

class Owner : public User
{
public:
    Owner();
    Owner(int userId, juce::String userName, juce::String password);
    ~Owner() override;

    // Owner-specific methods from your UML
    void recordSound();
    void viewWaveForm();
    void playSound();
    void filterSound();
    void saveSound();
    void listSavedSounds();
    void viewClusterMap();
    void createGuestAccount();

    void displayWelcome() override;
};