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
    //TODO: Remove later: Using this to find the load button
    DBG("OwnerDashboard::resized() bounds = " + getLocalBounds().toString());
    auto area = getLocalBounds().reduced(20);

    titleLabel.setBounds(area.removeFromTop(50));
    area.removeFromTop(10);

    welcomeLabel.setBounds(area.removeFromTop(40));
    area.removeFromTop(15);

    //Load button at the top of the remaining space
    loadButton.setBounds(area.removeFromTop(40).withSizeKeepingCentre(250, 40));
    //TODO: Remove later, just trying to see if the button is redering to thee screen somewhere
    DBG("  loadButton bounds = " + loadButton.getBounds().toString());
    area.removeFromTop(15);

    //Logout button at the very bottom
    auto buttonArea = area.removeFromBottom(40);
    logoutButton.setBounds(buttonArea.withSizeKeepingCentre(150, 35));

    //Sound list fills the rest of the space (if it exists)
    if (soundList != nullptr)
        soundList->setBounds(area.removeFromTop(area.getHeight() - 50));
}

void OwnerDashboardComponent::setUsername(const juce::String& name){
    username = name;
    welcomeLabel.setText("Logged in as Owner: " + username, juce::dontSendNotification);
}
