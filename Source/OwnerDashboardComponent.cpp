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
    logoutButton.setButtonText(juce::CharPointer_UTF8("\xf0\x9f\x9a\xaa Logout"));
    logoutButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xffe74c3c));
    logoutButton.onClick = [this] {
        if (onLogout)
            onLogout();
        };
    addAndMakeVisible(logoutButton);

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

            if (value < 0.0) value = 0.0;

            selectedSound->setSoundPrice(value);
            priceEditor.setText(juce::String(value, 2), juce::dontSendNotification);

            if (soundList != nullptr)
                soundList->repaint();
        };
    addAndMakeVisible(setPriceButton);

    // Load Sounds button
    loadButton.setButtonText(juce::CharPointer_UTF8("\xf0\x9f\x8e\xb5\xf0\x9f\x93\x82 Load Sounds"));
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

    // Cluster button
    clustButton.setButtonText(juce::CharPointer_UTF8("\xf0\x9f\x94\xac View 2D Cluster"));
    clustButton.setColour(juce::TextButton::buttonColourId, juce::Colours::orange);
    clustButton.setColour(juce::TextButton::textColourOffId, juce::Colours::black);
    clustButton.onClick = [this] {
        DBG(">>> CLUSTER BUTTON CLICKED <<<");
        if (viewCluster)
            viewCluster();
        };
    addAndMakeVisible(clustButton);

    // Recorder button
    recorderButton.setButtonText(juce::CharPointer_UTF8("\xf0\x9f\x8e\x99 Record Sound"));
    recorderButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xffff5c5c));
    recorderButton.setColour(juce::TextButton::textColourOffId, juce::Colours::black);
    recorderButton.onClick = [this] {
        DBG(">>> RECORDER BUTTON CLICKED <<<");
        if (viewRecorder)
            viewRecorder();
        };
    addAndMakeVisible(recorderButton);

    // Create Guest Account button
    createGuestButton.setButtonText(juce::CharPointer_UTF8("\xf0\x9f\x91\xa4 Create Guest Account"));
    createGuestButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff2ecc71));
    createGuestButton.setColour(juce::TextButton::textColourOffId, juce::Colours::black);
    createGuestButton.onClick = [this] {
        DBG(">>> CREATE GUEST ACCOUNT CLICKED <<<");
        if (createGuestAccount)
            createGuestAccount();
        };
    addAndMakeVisible(createGuestButton);

    DBG("=== OwnerDashboard constructor done ===");
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

    // Top bar: title + welcome label centered
    auto topBar = area.removeFromTop(24);
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setBounds(topBar);
    // Welcome label overlaid on right side of top bar
    welcomeLabel.setBounds(topBar.removeFromRight(200));

    area.removeFromTop(8);

    // All buttons in a single row: 4 action buttons + logout
    auto btnRow = area.removeFromTop(34);
    int btnWidth = 160;
    int gap = 8;
    int totalWidth = btnWidth * 4 + 100 + gap * 4;  // 4 action buttons + logout + gaps
    int startX = btnRow.getX() + (btnRow.getWidth() - totalWidth) / 2;
    int y = btnRow.getY();
    int h = btnRow.getHeight();
    loadButton.setBounds(startX, y, btnWidth, h);
    clustButton.setBounds(startX + (btnWidth + gap), y, btnWidth, h);
    recorderButton.setBounds(startX + (btnWidth + gap) * 2, y, btnWidth, h);
    createGuestButton.setBounds(startX + (btnWidth + gap) * 3, y, btnWidth, h);
    logoutButton.setBounds(startX + (btnWidth + gap) * 3 + btnWidth + gap, y, 100, h);

    area.removeFromTop(10);

    // Price controls row — centered below buttons
    auto priceRow = area.removeFromTop(30);
    int priceW = 50 + 4 + 100 + 6 + 100;  // label + gap + editor + gap + button
    int priceX = priceRow.getX() + (priceRow.getWidth() - priceW) / 2;
    int priceY = priceRow.getY();
    priceLabel.setBounds(priceX, priceY, 50, 30);
    priceEditor.setBounds(priceX + 54, priceY, 100, 30);
    setPriceButton.setBounds(priceX + 160, priceY, 100, 30);

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
