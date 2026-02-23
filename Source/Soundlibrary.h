/*
  ==============================================================================

    Soundlibrary.h
    Created: 23 Feb 2026 1:57:16pm
    Author:  eefernet

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "Sound.h"
#include <vector>
#include <memory>

/*
* The SoundLibrary class is responsible for loading and managing a collection of
* Sound objects. It owns the AudioFormatManager (which knows how to decode various
* audio file formats like WAV, MP3, AIFF, FLAC, OGG) and provides methods to load
* sounds either from an entire directory or one file at a time.
*
* Usage:
*   SoundLibrary library;
*   library.loadFromDirectory(juce::File("/path/to/sounds/"));
*   Sound* firstSound = library.getSound(0);  // get a non-owning pointer
*/
class SoundLibrary
{
public:
    SoundLibrary();

    /*
    * Load all supported audio files from a directory.
    * This scans the given folder for files with supported extensions
    * (wav, mp3, aiff, flac, ogg) and loads each one into a Sound object.
    * Non-recursive — only looks at files directly in the folder, not subfolders.
    *
    * @param directory - The folder to scan for audio files
    */
    void loadFromDirectory(const juce::File& directory);

    /*
    * Load a single audio file and add it to the library.
    * The file is loaded into a new Sound object with an auto-incremented ID.
    * If the file can't be loaded (bad format, missing, etc.), it is silently skipped.
    *
    * @param file - The audio file to load
    */
    void loadFile(const juce::File& file);

    /*
    * Returns the total number of successfully loaded sounds.
    * Sounds that failed to load are not counted.
    */
    int getNumSounds() const;

    /*
    * Get a non-owning pointer to a sound by its index in the list.
    * Returns nullptr if the index is out of range.
    * The SoundLibrary still owns the Sound — don't delete the returned pointer!
    *
    * @param index - Zero-based index into the loaded sounds list
    */
    Sound* getSound(int index);

    /*
    * Get a const reference to the entire list of sounds.
    * Useful if you need to iterate over all sounds, e.g. for searching or filtering.
    * The unique_ptrs in the vector own the Sound objects — don't try to move them out.
    */
    const std::vector<std::unique_ptr<Sound>>& getSounds() const;

private:
    /*
    * The AudioFormatManager is JUCE's central hub for audio file decoding.
    * After calling registerBasicFormats(), it knows how to read WAV, AIFF, FLAC,
    * OGG Vorbis, and (if the JUCE MP3 module is enabled) MP3 files.
    * We keep one instance here and pass it by reference to each Sound we create,
    * so we don't have to create a new one for every file.
    */
    juce::AudioFormatManager formatManager;

    /*
    * The actual collection of loaded sounds.
    * We use unique_ptr so the SoundLibrary owns the Sound objects and they get
    * cleaned up automatically. When we pass Sound* pointers to the UI, those are
    * non-owning "borrowed" pointers — the UI should never delete them.
    */
    std::vector<std::unique_ptr<Sound>> sounds;

    /*
    * Auto-incrementing ID counter for assigning unique IDs to each sound.
    * Every time we load a new file, we give it nextId and then increment.
    * Starts at 1 so that 0 can be used as an "invalid" or "no sound" sentinel.
    */
    int nextId = 1;
};
