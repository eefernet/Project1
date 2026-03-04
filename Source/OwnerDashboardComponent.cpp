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
    // Price label
    priceLabel.setText("Price:", juce::dontSendNotification);
    priceLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(priceLabel);

    // Price editor
    priceEditor.setText("0.00");
    priceEditor.setInputRestrictions(0, "0123456789."); // only allow numbers and dot
    addAndMakeVisible(priceEditor);

    // Set price button
    setPriceButton.setColour(juce::TextButton::buttonColourId, juce::Colours::lightblue);
    setPriceButton.setColour(juce::TextButton::textColourOffId, juce::Colours::black);

    setPriceButton.onClick = [this]
        {
            if (selectedSound == nullptr)
            {
                DBG("No sound selected to set price");
                return;
            }

            auto text = priceEditor.getText().trim();
            double value = text.getDoubleValue();

            // enforce non-negative and format to 2 decimals
            if (value < 0.0) value = 0.0;

            selectedSound->setSoundPrice(value);
            priceEditor.setText(juce::String(value, 2), juce::dontSendNotification);

            // refresh list so the new price shows immediately
            if (soundList != nullptr)
                soundList->repaint();
        };

    addAndMakeVisible(setPriceButton);
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
                    soundList->onSoundSelected = [this](Sound* s)
                        {
                            selectedSound = s;

                            if (selectedSound != nullptr)
                            {
                                DBG("Selected sound: " + selectedSound->getName());
                                priceEditor.setText(selectedSound->getSoundPrice(), juce::dontSendNotification);
                            }
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

    // Price controls row
    auto priceRow = area.removeFromTop(35);
    priceLabel.setBounds(priceRow.removeFromLeft(60));
    priceEditor.setBounds(priceRow.removeFromLeft(120));
    priceRow.removeFromLeft(10);
    setPriceButton.setBounds(priceRow.removeFromLeft(120));
    area.removeFromTop(10);

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
