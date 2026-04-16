/*
  ==============================================================================

    GuestDashboardComponent.cpp
    Created: 11 Feb 2026 12:11:31pm
    Author:  ethan

  ==============================================================================
*/


#include "GuestDashboardComponent.h"
#include "UIController.h"

GuestDashboardComponent::GuestDashboardComponent(SoundLibrary& lib) : soundList(lib, true)
{
    // Title
    titleLabel.setText("Guest Dashboard", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 22.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    titleLabel.setColour(juce::Label::textColourId, juce::Colour(UIController::titleText));
    addAndMakeVisible(titleLabel);

    // Welcome message
    welcomeLabel.setText("Logged in as Guest", juce::dontSendNotification);
    welcomeLabel.setFont(juce::Font(13.0f));
    welcomeLabel.setJustificationType(juce::Justification::centred);
    welcomeLabel.setColour(juce::Label::textColourId, juce::Colour(UIController::accent));
    addAndMakeVisible(welcomeLabel);

    // Logout button — power symbol (⏻)
    logoutButton.setButtonText(juce::CharPointer_UTF8("\xe2\x8f\xbb Logout"));
    logoutButton.setColour(juce::TextButton::buttonColourId, juce::Colour(UIController::logoutBg));
    logoutButton.setColour(juce::TextButton::textColourOffId, juce::Colour(UIController::danger));
    logoutButton.onClick = [this] {
        if (onLogout)
            onLogout();
        };
    addAndMakeVisible(logoutButton);

    clustButton.setButtonText(juce::CharPointer_UTF8("\xf0\x9f\x93\x8a View 2D Cluster"));
    clustButton.setColour(juce::TextButton::buttonColourId, juce::Colour(UIController::warning));
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
    g.fillAll(juce::Colour(UIController::bg));
}

void GuestDashboardComponent::resized()
{
    auto area = getLocalBounds().reduced(10);

    const int btnH    = 42;
    const int logoutW = 150;
    const int guestW  = 200;
    const int itemGap = 10;

    // Title row at the top
    auto titleRow = area.removeFromTop(34);
    titleLabel.setBounds(titleRow.reduced(4, 0));
    area.removeFromTop(8);

    // Bottom row: [Guest: xxx] [Logout], centered
    auto bottomBar = area.removeFromBottom(btnH);
    const int bottomTotal = guestW + itemGap + logoutW;
    const int bottomX     = bottomBar.getX() + (bottomBar.getWidth() - bottomTotal) / 2;
    const int bottomY     = bottomBar.getY();
    welcomeLabel.setBounds(bottomX,                          bottomY, guestW,  btnH);
    logoutButton.setBounds(bottomX + guestW + itemGap,       bottomY, logoutW, btnH);

    area.removeFromBottom(10);

    // Left column: action buttons; right: sound list
    const int mainBtnW = 210;
    auto leftCol = area.removeFromLeft(mainBtnW);
    clustButton.setBounds(leftCol.removeFromTop(btnH));

    area.removeFromLeft(12);
    soundList.setBounds(area);
}
