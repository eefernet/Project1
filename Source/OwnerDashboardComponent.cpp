/*
  ==============================================================================

    OwnerDashboardComponent.cpp
    Created: 11 Feb 2026 12:10:55pm
    Author:  ethan

  ==============================================================================
*/

#include "OwnerDashboardComponent.h"
#include "SoundListComponent.h"
#include "Soundlibrary.h"


OwnerDashboardComponent::OwnerDashboardComponent()
{
    // Title
    titleLabel.setText("Owner Dashboard", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(15.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(titleLabel);

    // Welcome message
    welcomeLabel.setText("Logged in as Owner", juce::dontSendNotification);
    welcomeLabel.setFont(juce::Font(13.0f));
    welcomeLabel.setJustificationType(juce::Justification::centredLeft);
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
    //TODO: Remove later testing to see if the load button is even loading
    //DBG("loadButton bounds: " + loadButton.getBounds().toString());
    //DBG("loadButton visible: " + juce::String(loadButton.isVisible()));
    //DBG("loadButton parent: " + juce::String(loadButton.getParentComponent() != nullptr));
/*
 *TODO: Will change later just a load button to see if UI works
 */
    //Setup the "Load Sounds" button that lets the user pick a folder
    loadButton.setButtonText("Load Sounds Folder");
    loadButton.setColour(juce::TextButton::buttonColourId, juce::Colours::orange);
    loadButton.setColour(juce::TextButton::textColourOffId, juce::Colours::black);
    loadButton.onClick = [this]{
        DBG(">>> LOAD BUTTON CLICKED <<<");
        auto chooser = std::make_shared<juce::FileChooser>(
            "Select a folder containing your sound files", juce::File(), "");

        chooser->launchAsync(
            juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectDirectories,
            [this, chooser](const juce::FileChooser& fc){
                auto dir = fc.getResult();
                if (dir.isDirectory()){
                    loadedSoundsFolder = dir;
                    soundlibrary.loadFromDirectory(dir);
                    soundList = std::make_unique<SoundListComponent>(soundlibrary);
                    soundList->onSoundSelected = [this](Sound* s) {
                        DBG("Selected sound: " + s->getName());
                    };
                    addAndMakeVisible(*soundList);
                    resized();
                }
            });
    };
    addAndMakeVisible(loadButton);

    clustButton.setButtonText("View 2D Cluster");
    clustButton.setColour(juce::TextButton::buttonColourId, juce::Colours::orange);
    clustButton.setColour(juce::TextButton::textColourOffId, juce::Colours::black);
    clustButton.onClick = [this] {
        DBG(">>> CLUSTER BUTTON CLICKED <<<");
       /* if (currentUserOwner != nullptr)
            currentUserOwner->viewClusterMap();*/  

        if (viewCluster)
            viewCluster();
        };
    addAndMakeVisible(clustButton);

    recorderButton.setButtonText("Record Sound");
    recorderButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xffff5c5c));
    recorderButton.setColour(juce::TextButton::textColourOffId, juce::Colours::black);
    recorderButton.onClick = [this] {
        DBG(">>> RECORDER BUTTON CLICKED <<<");
        if (viewRecorder)
            viewRecorder();
        };
    addAndMakeVisible(recorderButton);

    DBG("=== OwnerDashboard constructor done ===");
    //Must not be using this right so removing for now
    //DBG("  Children count: " + juce::String(getNumChildComponents()));
    //DBG("  loadButton visible: " + juce::String(loadButton.isVisible() ? "yes" : "no"));
    //DBG("  loadButton text: " + loadButton.getButtonText());
}

OwnerDashboardComponent::~OwnerDashboardComponent(){
}

void OwnerDashboardComponent::paint(juce::Graphics& g){
    g.fillAll(juce::Colour(0xff2c3e50));

    g.setColour(juce::Colours::white.withAlpha(0.2f));
    g.drawRect(getLocalBounds().reduced(20), 2);
}

void OwnerDashboardComponent::resized(){
    auto area = getLocalBounds().reduced(10);

    // Top bar: title | welcome label | logout button — all on one row
    auto topBar = area.removeFromTop(36);
    titleLabel.setBounds(topBar.removeFromLeft(160));
    logoutButton.setBounds(topBar.removeFromRight(80));
    topBar.removeFromRight(6);
    welcomeLabel.setBounds(topBar);

    area.removeFromTop(6);

    // Action buttons in a single row
    auto actionRow = area.removeFromTop(32);
    int btnWidth = actionRow.getWidth() / 3;
    loadButton.setBounds(actionRow.removeFromLeft(btnWidth).reduced(4, 0));
    clustButton.setBounds(actionRow.removeFromLeft(btnWidth).reduced(4, 0));
    recorderButton.setBounds(actionRow.reduced(4, 0));

    area.removeFromTop(6);

    // Sound list fills remaining space
    if (soundList != nullptr)
        soundList->setBounds(area);
}

void OwnerDashboardComponent::setUsername(const juce::String& name){
    username = name;
    welcomeLabel.setText("Owner: " + username, juce::dontSendNotification);
}

SoundLibrary& OwnerDashboardComponent::getSoundLibrary()
{
    return soundlibrary;
}
