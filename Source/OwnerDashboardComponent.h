/*
  ==============================================================================

    OwnerDashboardComponent.h
    Created: 11 Feb 2026 12:10:55pm
    Author:  ethan

  ==============================================================================
*/
#pragma once
#include <JuceHeader.h>
#include "Soundlibrary.h"
#include "SoundListComponent.h"

class OwnerDashboardComponent : public juce::Component
{
public:
    OwnerDashboardComponent();
    ~OwnerDashboardComponent() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    // Set the username to display on the dashboard
    void setUsername(const juce::String& name);

    // Callback for when the user wants to log out
    std::function<void()> onLogout;
    std::function<void()> viewCluster;
    std::function<void()> viewRecorder;

    SoundLibrary& getSoundLibrary();
    juce::File getSoundsFolder() const { return loadedSoundsFolder; }
private:
    juce::Label titleLabel;
    juce::Label welcomeLabel;
    juce::TextButton logoutButton;

    juce::String username;

    //TODO: REMOVE LATER just using to test UI intigration or change idk
    SoundLibrary soundlibrary;
    std::unique_ptr<SoundListComponent> soundList;
    juce::TextButton loadButton;
    juce::TextButton clustButton;
    juce::TextButton recorderButton;
    juce::File loadedSoundsFolder;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OwnerDashboardComponent)
};
