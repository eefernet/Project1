/*
  ==============================================================================

    OwnerDashboardComponent.cpp
    Created: 11 Feb 2026 12:10:55pm
    Author:  ethan

  ==============================================================================
*/

#include "OwnerDashboardComponent.h"

OwnerDashboardComponent::OwnerDashboardComponent()
{
    // Title
    titleLabel.setText("Owner Dashboard", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(28.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(titleLabel);

    // Welcome message
    welcomeLabel.setText("Logged in as Owner", juce::dontSendNotification);
    welcomeLabel.setFont(juce::Font(18.0f));
    welcomeLabel.setJustificationType(juce::Justification::centred);
    welcomeLabel.setColour(juce::Label::textColourId, juce::Colours::lightgreen);
    addAndMakeVisible(welcomeLabel);

    // Logout button
    logoutButton.setButtonText("Logout");
    logoutButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xffe74c3c));
    logoutButton.onClick = [this] {
        if (onLogout)
            onLogout();
        };
    addAndMakeVisible(logoutButton);

    setSize(450, 450);
}

OwnerDashboardComponent::~OwnerDashboardComponent()
{
}

void OwnerDashboardComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff2c3e50));

    g.setColour(juce::Colours::white.withAlpha(0.2f));
    g.drawRect(getLocalBounds().reduced(20), 2);
}

void OwnerDashboardComponent::resized()
{
    auto area = getLocalBounds().reduced(40);

    titleLabel.setBounds(area.removeFromTop(50));
    area.removeFromTop(30);

    welcomeLabel.setBounds(area.removeFromTop(40));
    area.removeFromTop(30);

    // Center the logout button
    auto buttonArea = area.removeFromBottom(40);
    logoutButton.setBounds(buttonArea.withSizeKeepingCentre(150, 35));
}

void OwnerDashboardComponent::setUsername(const juce::String& name)
{
    username = name;
    welcomeLabel.setText("Logged in as Owner: " + username, juce::dontSendNotification);
}
