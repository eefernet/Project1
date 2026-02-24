/*
  ==============================================================================

    SoundListComponent.h
    Created: 23 Feb 2026 9:35:04pm
    Author:  eefernet

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "Soundlibrary.h"

/*
* SoundListComponent is a UI component that displays a scrollable list of loaded sounds.
* It uses JUCE's ListBox widget.
*
* Usage:
*   SoundListComponent list(mySoundLibrary);
*   list.onSoundSelected = [](Sound* s) { DBG("Selected: " + s->getName()); };
*   addAndMakeVisible(list);
*/
class SoundListComponent : public juce::Component,public juce::ListBoxModel{
    public:
    /*
    * Constructor — takes a reference to the SoundLibrary that contains our sounds.
    * The SoundListComponent just reads from the soundlibrary. It does not have the
    * ability to modify the soundLibrary. TODO: might need a graphical way to manage the library?
    *
    * @param library - Reference to the SoundLibrary to display
    */
    SoundListComponent(SoundLibrary& library);
    //These methods bbelow are called by the ListBox to get data and render rows.
    //JUCE calls these automatically — we just need to implement them.

    /*
    * Called by the ListBox to find out how many rows to display.
    * We just return the number of sounds in our library.
    */
    int getNumRows() override;


    //Called by the ListBox to paint a single row.
    void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;

    //Called by the ListBox when the user clicks on a row.
    void listBoxItemClicked(int row, const juce::MouseEvent& event) override;
    //Called when this component is resized — we use it to make the ListBox fill our bounds
    void resized() override;

    //Callback that fires when the user clicks a sound in the list.
    std::function<void(Sound*)> onSoundSelected;

private:
    //Reference to the SoundLibrary, just read from it
    SoundLibrary& library;

    //The actual JUCE ListBox widget that handles scrolling, selection, and row rendering.
    juce::ListBox listBox;
};