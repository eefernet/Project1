/*
  ==============================================================================

    SoundListComponent.cpp
    Created: 23 Feb 2026 9:35:04pm
    Author:  eefernet

  ==============================================================================
*/

#include "SoundListComponent.h"




//Constructor — sets up the ListBox with some basic styling and registers
SoundListComponent::SoundListComponent(SoundLibrary& lib) : library(lib){
    //Make this class handle on logic
    listBox.setModel(this);

    //Row setting to 70
    listBox.setRowHeight(70);

    //Dark background color
    listBox.setColour(juce::ListBox::backgroundColourId, juce::Colour(0xff1e1e1e));

    //Make the ListBox visible as a child of this component
    addAndMakeVisible(listBox);
}


//Returns the number of rows the ListBox should display.
int SoundListComponent::getNumRows(){
    return library.getNumSounds();
}

/*
* This is where we actually draw each row in the list.
* The ListBox calls this for every visible row, passing us the Graphics context
* to draw into.
*/
void SoundListComponent::paintListBoxItem(int row, juce::Graphics& g,int width, int height, bool selected){
    //Get the Sound object for this row from the library
    auto* sound = library.getSound(row);

    //Make sure we dont have an out of range index
    if (sound == nullptr)
        return;

    //Use a lighter shade when the row is selected so the user can see which
    if (selected)
        //lighter blue for selection
        g.fillAll(juce::Colour(0xff3a506b));
    else
        //default dark blue-greyish color idk
        g.fillAll(juce::Colour(0xff2c3e50));

    //Draw the pre-generated waveform image on the left side of the row.
    auto wf = sound->getWaveForm();
    if (wf.isValid()){
        //Draw the waveform image in a 120px wide area with some padding
        //Rectangle<float>(x, y, width, height) — positioned at x=8 with 5px top padding
        g.drawImage(wf, juce::Rectangle<float>(8, 5, 120, (float)height - 10));
    }

    //ddraw the display name of the sound (e.g. "kick_drum", "ambient_pad")
    g.setColour(juce::Colours::white);
    g.setFont(16.0f);
    g.drawText(sound->getName(),140, 5, width - 200, 25, juce::Justification::centredLeft);

    //Show the duration below the name in a smaller grey font
    g.setColour(juce::Colours::grey);
    g.setFont(12.0f);
    g.drawText("Duration: " + sound->getDurationString(),140, 30, width - 200, 20,juce::Justification::centredLeft);

    // Show sample rate and format at the bottom of the row (e.g. "44100 Hz | WAV")
    auto meta = sound->getMetaData();
    g.drawText(juce::String(meta["sampleRate"]) + " Hz  |  " + meta["format"],
               140, 48, width - 200, 16,
               juce::Justification::centredLeft);
}

/*
* Called when the user clicks on a row in the list.
* We fire the onSoundSelected callback with the corresponding Sound pointer
* so the parent component can respond (e.g. start playback, show details).
*/
void SoundListComponent::listBoxItemClicked(int row, const juce::MouseEvent&){
    //Fire the callback if one has been set by the parent component
    if (onSoundSelected)
        onSoundSelected(library.getSound(row));
}

//Called whenever this component is resized (including the initial layout).
void SoundListComponent::resized(){
    listBox.setBounds(getLocalBounds());
}
