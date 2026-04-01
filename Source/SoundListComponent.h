/*
  ==============================================================================

    SoundListComponent.h
    Created: 23 Feb 2026 9:35:04pm
    Author:  eefernet

  ==============================================================================
*/

/*
 *TODO: Add price label to the list view
 */

#pragma once
#include <JuceHeader.h>
#include "Soundlibrary.h"

class SoundListComponent;

// Custom row component that holds a play/stop button (and buy button in guest mode) for each row
class SoundRowComponent : public juce::Component{
public:
    SoundRowComponent(SoundListComponent& owner);
    void setRow(int newRow);
    void resized() override;
    void paint(juce::Graphics& g) override;
    // Let clicks pass through to the ListBox row unless they hit a button
    bool hitTest(int x, int y) override;
private:
    SoundListComponent& owner;
    int row = -1;
    juce::TextButton playButton;
    juce::TextButton buyButton;
    juce::TextButton editButton;
};

/*
* SoundListComponent is a UI component that displays a scrollable list of loaded sounds.
* It uses JUCE's ListBox widget with custom row components for play buttons.
*
* Usage:
*   SoundListComponent list(mySoundLibrary);
*   list.onSoundSelected = [](Sound* s) { DBG("Selected: " + s->getName()); };
*   addAndMakeVisible(list);
*/
class SoundListComponent : public juce::Component, public juce::ListBoxModel{
    public:
    SoundListComponent(SoundLibrary& library, bool guestMode = false);
    ~SoundListComponent() override;

    int getNumRows() override;
    void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;
    juce::Component* refreshComponentForRow(int rowNumber, bool isRowSelected, juce::Component* existingComponentToUpdate) override;
    void listBoxItemClicked(int row, const juce::MouseEvent& event) override;
    void resized() override;

    //Callback that fires when the user clicks a sound in the list.
    std::function<void(Sound*)> onSoundSelected;

    // Call this when the library contents change so the list refreshes.
    void refresh() { listBox.updateContent(); listBox.repaint(); }

    // Play/stop a sound by row index — called from row component
    void togglePlayback(int row);

    // Check if a given row is currently playing
    bool isPlaying(int row) const;

    // Get the library reference (used by row components)
    SoundLibrary& getLibrary() { return library; }

    // Whether this list is in guest mode (sample playback + buy button)
    bool isGuestMode() const { return guestMode; }

    // Callback for buy button — wire this up from parent
    std::function<void(Sound*)> onBuySound;
    // Callback for edit button — wire this up from parent
    std::function<void(Sound*)> onEditSound;

private:
    SoundLibrary& library;
    bool guestMode;
    juce::ListBox listBox;

    // Audio playback
    juce::AudioDeviceManager deviceManager;
    juce::AudioSourcePlayer sourcePlayer;
    std::unique_ptr<juce::MemoryAudioSource> memorySource;
    juce::AudioBuffer<float> sampleBuffer;  // Holds the 20% sample for guest mode
    int currentlyPlayingRow = -1;

    void stopPlayback();
};