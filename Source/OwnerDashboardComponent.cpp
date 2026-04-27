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
#include "UIController.h"


OwnerDashboardComponent::OwnerDashboardComponent()
{
    // Title
    titleLabel.setText("Owner Dashboard", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 22.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    titleLabel.setColour(juce::Label::textColourId, juce::Colour(UIController::titleText));
    addAndMakeVisible(titleLabel);

    // Welcome message
    welcomeLabel.setText("Logged in as Owner", juce::dontSendNotification);
    welcomeLabel.setFont(juce::Font(13.0f));
    welcomeLabel.setJustificationType(juce::Justification::centredLeft);
    welcomeLabel.setJustificationType(juce::Justification::centredRight);
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

    // Price editor — hidden until a sound is selected. Configure font BEFORE setText
    // so initial text is drawn in the correct size/face.
    priceEditor.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 20.0f, juce::Font::bold));
    priceEditor.setJustification(juce::Justification::centred);
    priceEditor.setIndents(6, 6);
    priceEditor.setInputRestrictions(0, "0123456789."); // only allow numbers and dot
    priceEditor.setTextToShowWhenEmpty("0.00", juce::Colour(UIController::textDim));
    priceEditor.setText("0.00");
    priceEditor.setVisible(false);
    addChildComponent(priceEditor);

    // Set price button — money bag emoji
    setPriceButton.setButtonText(juce::CharPointer_UTF8("\xf0\x9f\x92\xb0 Set Price"));
    setPriceButton.setColour(juce::TextButton::buttonColourId, juce::Colour(UIController::setPriceBg));
    setPriceButton.setVisible(false);
    // Note: addChildComponent used below instead of addAndMakeVisible so it stays hidden until a sound is selected.
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
    addChildComponent(setPriceButton);

    // Load Sounds button — inbox tray emoji (📥)
    loadButton.setButtonText(juce::CharPointer_UTF8("\xf0\x9f\x93\xa5 Load Sounds"));
    loadButton.setColour(juce::TextButton::buttonColourId, juce::Colour(UIController::warning));
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
                    soundList->onEditSound = [this](Sound* s) {
                        if (editSound) editSound(s);
                    };
                    soundList->onSoundSelected = [this](Sound* s)
                        {
                            selectedSound = s;

                            const bool has = selectedSound != nullptr;
                            priceEditor.setVisible(has);
                            setPriceButton.setVisible(has);

                            if (has)
                            {
                                DBG("Selected sound: " + selectedSound->getName());
                                priceEditor.setText(selectedSound->getSoundPrice(), juce::dontSendNotification);
                                priceEditor.applyFontToAllText(
                                    juce::Font(juce::Font::getDefaultMonospacedFontName(), 20.0f, juce::Font::bold));
                            }
                        };
                    addAndMakeVisible(*soundList);
                    resized();
                }
            });
    };
    addAndMakeVisible(loadButton);

    // Cluster button — bar chart emoji
    clustButton.setButtonText(juce::CharPointer_UTF8("\xf0\x9f\x93\x8a View 2D Cluster"));
    clustButton.setColour(juce::TextButton::buttonColourId, juce::Colour(UIController::warning));
    clustButton.onClick = [this] {
        DBG(">>> CLUSTER BUTTON CLICKED <<<");
        if (viewCluster)
            viewCluster();
        };
    addAndMakeVisible(clustButton);

    // Recorder button
    recorderButton.setButtonText(juce::CharPointer_UTF8("\xf0\x9f\x8e\x99 Record Sound"));
    recorderButton.setColour(juce::TextButton::buttonColourId, juce::Colour(UIController::recorderBg));
    recorderButton.onClick = [this] {
        DBG(">>> RECORDER BUTTON CLICKED <<<");
        if (viewRecorder)
            viewRecorder();
        };
    addAndMakeVisible(recorderButton);

    // Create Guest Account button — people emoji
    createGuestButton.setButtonText(juce::CharPointer_UTF8("\xf0\x9f\x91\xa5 Create Guest Account"));
    createGuestButton.setColour(juce::TextButton::buttonColourId, juce::Colour(UIController::success));
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
    g.fillAll(juce::Colour(UIController::bg));
}

void OwnerDashboardComponent::resized(){
    auto area = getLocalBounds().reduced(10);

    const int btnH     = 42;
    const int logoutW  = 150;
    const int guestW   = 230;
    const int ownerW   = 200;
    const int itemGap  = 10;

    // Title row at the top
    auto titleRow = area.removeFromTop(34);
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    titleLabel.setBounds(titleRow.reduced(4, 0));

    area.removeFromTop(8);

    // Bottom row: [Owner: xxx] [Logout] [Create Guest Account], in a single centered line
    auto bottomBar = area.removeFromBottom(btnH);
    const int bottomTotal = ownerW + itemGap + guestW + itemGap + logoutW;
    const int bottomX     = bottomBar.getX() + (bottomBar.getWidth() - bottomTotal) / 2;
    const int bottomY     = bottomBar.getY();
    welcomeLabel.setJustificationType(juce::Justification::centred);
    welcomeLabel.setBounds     (bottomX,                                          bottomY, ownerW,  btnH);
    createGuestButton.setBounds(bottomX + ownerW + itemGap,                       bottomY, guestW,  btnH);
    logoutButton.setBounds     (bottomX + ownerW + itemGap + guestW + itemGap,    bottomY, logoutW, btnH);

    area.removeFromBottom(10); // gap above the bottom bar

    // Main content: left column of stacked action buttons, right side for sound list
    const int mainBtnW   = 210;
    const int mainBtnGap = 10;
    auto leftCol = area.removeFromLeft(mainBtnW);

    loadButton.setBounds    (leftCol.removeFromTop(btnH));
    leftCol.removeFromTop(mainBtnGap);
    clustButton.setBounds   (leftCol.removeFromTop(btnH));
    leftCol.removeFromTop(mainBtnGap);
    recorderButton.setBounds(leftCol.removeFromTop(btnH));

    // Price controls stacked under the action buttons (only laid out; visibility
    // is toggled by selection). Editor on top, Set Price right below.
    leftCol.removeFromTop(mainBtnGap + 6);
    priceEditor.setBounds   (leftCol.removeFromTop(btnH));
    leftCol.removeFromTop(6);
    setPriceButton.setBounds(leftCol.removeFromTop(btnH));

    // Label is no longer displayed; keep it off-screen so layout isn't affected.
    priceLabel.setVisible(false);

    area.removeFromLeft(12); // gap between buttons and list

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

void OwnerDashboardComponent::addRecording(const juce::File& file)
{
    soundlibrary.loadFile(file);
    if (soundList != nullptr) soundList->refresh();
}
