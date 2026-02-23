/*
  ==============================================================================

    SoundLibrary.cpp
    Created: 23 Feb 2026
    Author:  eefernet

  ==============================================================================
*/

#include "Soundlibrary.h"

/*
* Constructor — registers all the basic audio formats that JUCE supports out of the box.
* After this call, the formatManager can decode: WAV, AIFF, FLAC, OGG Vorbis, and MP3.
* This needs to happen BEFORE we try to load any files, otherwise createReaderFor()
* won't know how to handle any file type and will always return nullptr.
*/
Soundlibrary::Soundlibrary(){
    // registerBasicFormats() adds all the built-in format readers.
    // Without this call, the format manager is empty and can't read anything!
    formatManager.registerBasicFormats();
}

/*
* Loads every supported audio file from the given directory.
* Uses JUCE's File::findChildFiles() to search for files matching our
* supported extensions. The search is non-recursive (false parameter),
* meaning it only looks at files directly in the folder, not in subfolders.
*
* Each file that matches is passed to loadFile(), which handles the actual
* loading and error checking. Files that fail to load are silently skipped.
*/
void SoundLibrary::loadFromDirectory(const juce::File& directory){
    //Make sure the path actually exists
    if (!directory.isDirectory())
        return;

    //Find all files in the directory that match our supported audio extensions.
    //The wildcard pattern uses semicolons to separate multiple extensions.
    //findChildFiles returns a juce::Array<juce::File> with all matches.
    //parameters: (whatToLookFor, searchRecursively, wildcardPattern)
    auto files = directory.findChildFiles(
        juce::File::findFiles, false,
        "*.wav;*.mp3;*.aiff;*.flac;*.ogg");

    //Load each file we found — loadFile() handles the actual reading
    //and will skip files that can't be decoded
    for (auto& file : files)
        loadFile(file);

    //Log how many sounds we successfully loaded for debugging
    DBG("Loaded " + juce::String(sounds.size()) + " sounds from "
        + directory.getFullPathName());
}

/*
* Loads a single audio file and adds it to our sounds collection.
* Creates a new Sound object with an auto-incremented ID and the filename
* (without extension) as the display name. If the Sound fails to load
* (isValid() returns false), we just skip it and don't add it to the list.
*
* We use std::make_unique to create the Sound on the heap and std::move to
* transfer ownership into the vector — this is the same pattern used in
* MainComponent for managing User objects.
*/
void SoundLibrary::loadFile(const juce::File& file){
    // Create a new Sound object — this triggers the actual file loading
    // inside Sound's constructor. We pass our formatManager by reference
    // so the Sound can use it to create the appropriate format reader.
    auto sound = std::make_unique<Sound>(
        nextId++,                                //Give it a unique ID and increment
        file.getFileNameWithoutExtension(),      //Use filename as display name (e.g. "kick_drum")
        file,                                    //The actual file path on disk
        formatManager);                       //Our format manager that knows how to decode audio

    //Only add the sound to our collection if it loaded successfully.
    //If the file was corrupt, missing, or in an unsupported format,
    //isValid() will return false and we just skip it.
    if (sound->isValid())
        sounds.push_back(std::move(sound));  // Transfer ownership to the vector
}

//Getters
// Returns how many sounds were successfully loaded (not counting failed ones)
int SoundLibrary::getNumSounds() const{
    return (int)sounds.size();
}

/*
* Returns a raw pointer to a Sound by index. This is a non-owning pointer —
* the SoundLibrary still owns the Sound through the unique_ptr in the vector.
* Returns nullptr if the index is out of bounds, so always check the return value!
*/
Sound* SoundLibrary::getSound(int index){
    // Bounds check to avoid undefined behavior
    return (index >= 0 && index < (int)sounds.size()) ? sounds[index].get() : nullptr;
}

/*
* Returns a const reference to the entire vector of sounds.
* The caller can iterate over this but can't modify or take ownership of the sounds.
*/
const std::vector<std::unique_ptr<Sound>>& SoundLibrary::getSounds() const{
    return sounds;
}
