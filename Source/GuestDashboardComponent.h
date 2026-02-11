/*
  ==============================================================================

    GuestDashboardComponent.h
    Created: 11 Feb 2026 12:11:31pm
    Author:  ethan

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class GuestDashboardComponent : public juce::Component
{
public:
    GuestDashboardComponent();
    ~GuestDashboardComponent() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    // Callback for when the user wants to log out
    std::function<void()> onLogout;

private:
    juce::Label titleLabel;
    juce::Label welcomeLabel;
    juce::TextButton logoutButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GuestDashboardComponent)
};
