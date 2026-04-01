/*
  ==============================================================================

    GuestDashboardComponent.cpp
    Created: 11 Feb 2026 12:11:31pm
    Author:  ethan

  ==============================================================================
*/


#include "GuestDashboardComponent.h"

GuestDashboardComponent::GuestDashboardComponent(SoundLibrary& lib) : soundList(lib)
{
    // Title
    titleLabel.setText("Guest Dashboard", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(28.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(titleLabel);

    // Welcome message
    welcomeLabel.setText("Logged in as Guest", juce::dontSendNotification);
    welcomeLabel.setFont(juce::Font(18.0f));
    welcomeLabel.setJustificationType(juce::Justification::centred);
    welcomeLabel.setColour(juce::Label::textColourId, juce::Colours::lightyellow);
    addAndMakeVisible(welcomeLabel);

    // Logout button
    logoutButton.setButtonText(juce::CharPointer_UTF8("\xf0\x9f\x9a\xaa Logout"));
    logoutButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xffe74c3c));
    logoutButton.onClick = [this] {
        if (onLogout)
            onLogout();
        };
    addAndMakeVisible(logoutButton);

    clustButton.setButtonText(juce::CharPointer_UTF8("\xf0\x9f\x94\xac View 2D Cluster"));
    clustButton.setColour(juce::TextButton::buttonColourId, juce::Colours::orange);
    clustButton.setColour(juce::TextButton::textColourOffId, juce::Colours::black);
    clustButton.onClick = [this] {
        DBG(">>> CLUSTER BUTTON CLICKED <<<");
 

        if (viewCluster)
            viewCluster();
        };
    addAndMakeVisible(clustButton);

    addAndMakeVisible(soundList);
}

GuestDashboardComponent::~GuestDashboardComponent()
{
}

void GuestDashboardComponent::setUsername(const juce::String& name)
{
    welcomeLabel.setText("Guest: " + name, juce::dontSendNotification);
}

void GuestDashboardComponent::visibilityChanged()
{
    if (isVisible())
        soundList.refresh();
}

void GuestDashboardComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff2c3e50));

    g.setColour(juce::Colours::white.withAlpha(0.2f));
    g.drawRect(getLocalBounds().reduced(20), 2);
}

void GuestDashboardComponent::resized()
{
    auto area = getLocalBounds().reduced(20);

    titleLabel.setBounds(area.removeFromTop(40));
    area.removeFromTop(8);

    welcomeLabel.setBounds(area.removeFromTop(28));
    area.removeFromTop(8);

    auto buttonRow = area.removeFromTop(36);
    clustButton.setBounds(buttonRow.removeFromLeft(180));
    buttonRow.removeFromLeft(10);
    logoutButton.setBounds(buttonRow.removeFromLeft(120));
    area.removeFromTop(10);

    soundList.setBounds(area);
}
