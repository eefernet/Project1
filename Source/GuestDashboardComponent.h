/*
  ==============================================================================

    GuestDashboardComponent.h
    Created: 11 Feb 2026 12:11:31pm
    Author:  ethan

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "Soundlibrary.h"
#include "SoundListComponent.h"

class GuestDashboardComponent : public juce::Component
{
public:
    GuestDashboardComponent(SoundLibrary& lib);
    ~GuestDashboardComponent() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void visibilityChanged() override;

    // Callback for when the user wants to log out
    std::function<void()> onLogout;
    std::function<void()> viewCluster;

private:
    juce::Label titleLabel;
    juce::Label welcomeLabel;
    juce::TextButton logoutButton;
    juce::TextButton clustButton;
    SoundListComponent soundList;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GuestDashboardComponent)
};
