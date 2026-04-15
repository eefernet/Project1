/*
  ==============================================================================

    SoundListComponent.cpp
    Created: 23 Feb 2026 9:35:04pm
    Author:  eefernet

  ==============================================================================
*/

#include "SoundListComponent.h"
#include "UIController.h"

// ============================================================================
// SoundRowComponent — custom row with a play/stop button
// ============================================================================
SoundRowComponent::SoundRowComponent(SoundListComponent& o) : owner(o)
{
    playButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff3498db));
    playButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    playButton.onClick = [this] { owner.togglePlayback(row); };
    addAndMakeVisible(playButton);

    // Buy button — only visible in guest mode
    buyButton.setButtonText(juce::CharPointer_UTF8("\xf0\x9f\x92\xb2 Buy"));
    buyButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff2ecc71));
    buyButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    buyButton.onClick = [this] {
        if (owner.onBuySound)
            owner.onBuySound(owner.getLibrary().getSound(row));
        else
            DBG("Buy clicked for row " << row << " — not wired up yet");
    };
    addAndMakeVisible(buyButton);

    // Edit button — only visible in owner mode
    editButton.setButtonText(juce::CharPointer_UTF8("\xe2\x9c\x8f Edit"));
    editButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff9b59b6));
    editButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    editButton.onClick = [this] {
        if (owner.onEditSound)
            owner.onEditSound(owner.getLibrary().getSound(row));
        else
            DBG("Edit clicked for row " << row << " — not wired up yet");
    };
    addAndMakeVisible(editButton);
}

void SoundRowComponent::setRow(int newRow)
{
    row = newRow;
    bool guest = owner.isGuestMode();

    // Update button text based on play state and mode
    if (owner.isPlaying(row))
        playButton.setButtonText(juce::CharPointer_UTF8("\xe2\x9c\x96 Stop"));
    else if (guest)
        playButton.setButtonText(juce::CharPointer_UTF8("\xe2\x96\xb6 Sample"));
    else
        playButton.setButtonText(juce::CharPointer_UTF8("\xe2\x96\xb6 Play"));

    buyButton.setVisible(guest);
    editButton.setVisible(!guest);
}

void SoundRowComponent::resized()
{
    auto area = getLocalBounds();
    int centreY = (area.getHeight() - 28) / 2;

    if (owner.isGuestMode())
    {
        // Two buttons: Sample + Buy, before the price area
        playButton.setBounds(area.getWidth() - 270, centreY, 80, 28);
        buyButton.setBounds(area.getWidth() - 185, centreY, 60, 28);
    }
    else
    {
        // Two buttons: Play + Edit, before the price area
        playButton.setBounds(area.getWidth() - 270, centreY, 70, 28);
        editButton.setBounds(area.getWidth() - 195, centreY, 65, 28);
    }
}

void SoundRowComponent::paint(juce::Graphics&)
{
    // Row painting is handled by paintListBoxItem — this component is transparent
}

bool SoundRowComponent::hitTest(int x, int y)
{
    // Only consume clicks that land on a visible button, let everything else pass through
    if (playButton.isVisible() && playButton.getBounds().contains(x, y))
        return true;
    if (buyButton.isVisible() && buyButton.getBounds().contains(x, y))
        return true;
    if (editButton.isVisible() && editButton.getBounds().contains(x, y))
        return true;
    return false;
}

// ============================================================================
// SoundListComponent
// ============================================================================
SoundListComponent::SoundListComponent(SoundLibrary& lib, bool guest) : library(lib), guestMode(guest){
    listBox.setModel(this);
    listBox.setRowHeight(70);
    listBox.setColour(juce::ListBox::backgroundColourId, juce::Colour(UIController::bg));
    addAndMakeVisible(listBox);

    // Initialize audio device for playback
    auto result = deviceManager.initialiseWithDefaultDevices(0, 2);
    if (result.isNotEmpty())
        DBG("Audio device init error: " << result);
    deviceManager.addAudioCallback(&sourcePlayer);
}

SoundListComponent::~SoundListComponent()
{
    stopPlayback();
    deviceManager.removeAudioCallback(&sourcePlayer);
}

int SoundListComponent::getNumRows(){
    return library.getNumSounds();
}

/*
* This is where we actually draw each row in the list.
*/
void SoundListComponent::paintListBoxItem(int row, juce::Graphics& g,int width, int height, bool selected){
    auto* sound = library.getSound(row);
    if (sound == nullptr)
        return;

    // Alternating row bands so lines feel like a terminal list
    auto base = (row % 2 == 0) ? juce::Colour(UIController::bg)
                               : juce::Colour(UIController::bgRaised);
    g.fillAll(base);

    if (selected)
    {
        g.setColour(juce::Colour(UIController::accent).withAlpha(0.18f));
        g.fillAll();
    }

    if (row == currentlyPlayingRow)
    {
        g.setColour(juce::Colour(UIController::accent).withAlpha(0.12f));
        g.fillAll();
    }

    // Thin left accent stripe for the active row
    if (selected || row == currentlyPlayingRow)
    {
        g.setColour(juce::Colour(UIController::accent));
        g.fillRect(0, 0, 2, height);
    }

    // Subtle bottom divider
    g.setColour(juce::Colour(UIController::border));
    g.fillRect(0, height - 1, width, 1);

    auto wf = sound->getWaveForm();
    if (wf.isValid()){
        g.drawImage(wf, juce::Rectangle<float>(8, 5, 120, (float)height - 10));
    }

    g.setColour(juce::Colour(UIController::text));
    g.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 16.0f, juce::Font::bold));
    g.drawText(sound->getName(),140, 5, width - 340, 25, juce::Justification::centredLeft);

    g.setColour(juce::Colour(UIController::textDim));
    g.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 14.0f, juce::Font::plain));
    g.drawText("Duration: " + sound->getDurationString(), 140, 32, width - 340, 20, juce::Justification::centredLeft);

    auto meta = sound->getMetaData();
    g.drawText(juce::String(meta["sampleRate"]) + " Hz  |  " + meta["format"],
               140, 52, width - 340, 18,
               juce::Justification::centredLeft);

    // Price on the right — vertically centered in the row
    g.setColour(juce::Colour(UIController::accent));
    g.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 16.0f, juce::Font::bold));
    auto priceText = "$" + sound->getSoundPrice();
    g.drawText(priceText,
        width - 120, 0, 110, height,
        juce::Justification::centredRight);
}

juce::Component* SoundListComponent::refreshComponentForRow(int rowNumber, bool /*isRowSelected*/, juce::Component* existingComponentToUpdate)
{
    if (rowNumber >= library.getNumSounds())
    {
        delete existingComponentToUpdate;
        return nullptr;
    }

    auto* rowComp = dynamic_cast<SoundRowComponent*>(existingComponentToUpdate);
    if (rowComp == nullptr)
        rowComp = new SoundRowComponent(*this);

    rowComp->setRow(rowNumber);
    return rowComp;
}

void SoundListComponent::listBoxItemClicked(int row, const juce::MouseEvent&){
    if (onSoundSelected)
        onSoundSelected(library.getSound(row));
}

void SoundListComponent::resized(){
    listBox.setBounds(getLocalBounds());
}

void SoundListComponent::togglePlayback(int row)
{
    // If already playing this row, stop
    if (row == currentlyPlayingRow)
    {
        stopPlayback();
        listBox.repaintRow(row);
        return;
    }

    // Stop any existing playback
    int prevRow = currentlyPlayingRow;
    stopPlayback();
    if (prevRow >= 0)
        listBox.repaintRow(prevRow);

    // Get the sound
    auto* sound = library.getSound(row);
    if (sound == nullptr || !sound->isValid())
        return;

    // In guest mode, create a trimmed buffer with only 20% of the samples
    auto& fullBuffer = sound->getAudioBuffer();
    if (guestMode)
    {
        int sampleCount = fullBuffer.getNumSamples() / 5;  // 20%
        sampleBuffer.setSize(fullBuffer.getNumChannels(), sampleCount, false, true, false);
        for (int ch = 0; ch < fullBuffer.getNumChannels(); ++ch)
            sampleBuffer.copyFrom(ch, 0, fullBuffer, ch, 0, sampleCount);
        // Make own copy so the source owns the exact trimmed data
        memorySource = std::make_unique<juce::MemoryAudioSource>(sampleBuffer, true, false);
        DBG("Guest sample: " << sampleCount << " of " << fullBuffer.getNumSamples() << " samples (20%)");
    }
    else
    {
        memorySource = std::make_unique<juce::MemoryAudioSource>(fullBuffer, false, false);
    }

    sourcePlayer.setSource(memorySource.get());
    currentlyPlayingRow = row;

    // Update the row to show stop button
    listBox.repaintRow(row);
    listBox.updateContent();

    DBG((guestMode ? "Playing 20% sample: " : "Playing sound: ") << sound->getName());
}

bool SoundListComponent::isPlaying(int row) const
{
    return row == currentlyPlayingRow;
}

void SoundListComponent::stopPlayback()
{
    sourcePlayer.setSource(nullptr);
    memorySource.reset();
    int prevRow = currentlyPlayingRow;
    currentlyPlayingRow = -1;
    if (prevRow >= 0)
        listBox.updateContent();
    DBG("Playback stopped");
}
