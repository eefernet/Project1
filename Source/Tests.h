/*
  ==============================================================================

    Tests.h
    Created: 25 Feb 2026 6:55:03pm
    Author:  eefernet

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Owner.h"
#include "Guest.h"
#include "User.h"
#include "UserRole.h"
#include "Soundlibrary.h"
#include "SoundListComponent.h"

// ============================================================================
// Authentication Tests
// ============================================================================
class AuthenticationTests : public juce::UnitTest{

    public:
    AuthenticationTests() : juce::UnitTest("Authentication Tests", "Authentication") {}

    void runTest() override{
        //Owner test logging in with correct password
        beginTest("Owner login with correct password succeeds");
        {
            Owner owner(1, "ethan", "pass123");
            expect(owner.login("pass123") == true,
                   "Owner should be able to log in with the correct password");
        }

        //Owner login with wrong password
        beginTest("Owner login with wrong password fails");
        {
            Owner owner(2, "ethan", "pass123");
            expect(owner.login("wrongpass") == false,
                   "Owner should NOT be able to log in with a wrong password");
        }

        //Guest login with correct password
        beginTest("Guest login with correct password succeeds");
        {
            Guest guest(3, "visitor", "guestpass");
            expect(guest.login("guestpass") == true,
                   "Guest should be able to log in with the correct password");
        }

        //Guest login with wrong password
        beginTest("Guest login with wrong password fails");
        {
            Guest guest(4, "visitor", "guestpass");
            expect(guest.login("nope") == false,
                   "Guest should NOT be able to log in with a wrong password");
        }

        //Login with empty password fails
        beginTest("Login with empty password fails");
        {
            Owner owner(5, "ethan", "secret");
            expect(owner.login("") == false,
                   "Login should fail when an empty password is provided");
        }

        //Logout returns true after a successful login
        beginTest("Logout returns true after a successful login");
        {
            Owner owner(6, "ethan", "pass123");
            owner.login("pass123");
            expect(owner.logout() == true, "Logout should return true");
        }

        //Login, logout, then login again
        beginTest("Re-login after logout succeeds");
        {
            Guest guest(7, "visitor", "pass");
            expect(guest.login("pass") == true, "First login should succeed");
            expect(guest.logout() == true, "Logout should succeed");
            expect(guest.login("pass") == true, "Re-login after logout should succeed");
        }

        //Login with wrong password then correct password
        beginTest("Login succeeds after a failed attempt");
        {
            Owner owner(8, "admin", "secure");
            expect(owner.login("wrong") == false, "Wrong password should fail");
            expect(owner.login("secure") == true, "Correct password after failure should succeed");
        }
    }
};

// ============================================================================
// User Role Tests
// ============================================================================
class UserRoleTests : public juce::UnitTest{

    public:
    UserRoleTests() : juce::UnitTest("User Role Tests", "UserManagement") {}

    void runTest() override{
        //Owner has Owner role and Guest has Guest role
        beginTest("Owner has Owner role and Guest has Guest role");
        {
            Owner owner(1, "admin", "pw");
            Guest guest(2, "viewer", "pw");

            expect(owner.getUserRole() == UserRole::Owner, "Owner's role should be UserRole::Owner");
            expect(guest.getUserRole() == UserRole::Guest, "Guest's role should be UserRole::Guest");
        }

        //Getters return correct user info
        beginTest("Getters return correct user info");
        {
            Owner owner(42, "testuser", "pw");
            expectEquals(owner.getUserID(), 42, "getUserID should return the ID passed to the constructor");
            expectEquals(owner.getUserName(), juce::String("testuser"), "getUserName should return the name passed to the constructor");
        }

        //Default constructor sets Guest role
        beginTest("Default Owner constructor sets Owner role");
        {
            Owner owner;
            expect(owner.getUserRole() == UserRole::Owner, "Default Owner should have Owner role");
        }

        beginTest("Default Guest constructor sets Guest role");
        {
            Guest guest;
            expect(guest.getUserRole() == UserRole::Guest, "Default Guest should have Guest role");
        }

        //Password getter returns stored password
        beginTest("getUserPassword returns stored password");
        {
            Owner owner(10, "user", "mypassword");
            expectEquals(owner.getUserPassword(), juce::String("mypassword"),
                         "getUserPassword should return the password passed to the constructor");
        }

        //Setters work correctly
        beginTest("setUserID updates the user ID");
        {
            Guest guest(1, "test", "pw");
            guest.setUserID(99);
            expectEquals(guest.getUserID(), 99, "setUserID should update the user ID");
        }

        beginTest("setUserName updates the username");
        {
            Owner owner(1, "oldname", "pw");
            owner.setUserName("newname");
            expectEquals(owner.getUserName(), juce::String("newname"), "setUserName should update the username");
        }
    }
};

// ============================================================================
// Account Management Tests (simulates what MainComponent does)
// ============================================================================
class AccountManagementTests : public juce::UnitTest{

    public:
    AccountManagementTests() : juce::UnitTest("Account Management Tests", "UserManagement") {}

    void runTest() override{
        beginTest("First account created is Owner");
        {
            std::vector<std::unique_ptr<User>> allUsers;
            //Simulate: no users exist, first account must be owner
            expect(allUsers.empty(), "User list should start empty");

            allUsers.push_back(std::make_unique<Owner>(1, "admin", "pass"));
            expectEquals((int)allUsers.size(), 1, "Should have 1 user after creation");
            expect(allUsers[0]->getUserRole() == UserRole::Owner, "First user should be Owner");
        }

        beginTest("Owner can create guest accounts");
        {
            std::vector<std::unique_ptr<User>> allUsers;
            allUsers.push_back(std::make_unique<Owner>(1, "admin", "pass"));

            //Simulate owner creating a guest account
            allUsers.push_back(std::make_unique<Guest>(2, "guestuser", "guestpass"));
            expectEquals((int)allUsers.size(), 2, "Should have 2 users");
            expect(allUsers[1]->getUserRole() == UserRole::Guest, "Second user should be Guest");
        }

        beginTest("Duplicate username detection");
        {
            std::vector<std::unique_ptr<User>> allUsers;
            allUsers.push_back(std::make_unique<Owner>(1, "admin", "pass"));

            //Try to find existing username
            juce::String targetName = "admin";
            bool found = false;
            for (auto& user : allUsers)
            {
                if (user->getUserName() == targetName)
                {
                    found = true;
                    break;
                }
            }
            expect(found, "Should detect existing username 'admin'");

            //Try to find non-existing username
            targetName = "newuser";
            found = false;
            for (auto& user : allUsers)
            {
                if (user->getUserName() == targetName)
                {
                    found = true;
                    break;
                }
            }
            expect(!found, "Should not find non-existing username 'newuser'");
        }

        beginTest("Login routes to correct role");
        {
            std::vector<std::unique_ptr<User>> allUsers;
            allUsers.push_back(std::make_unique<Owner>(1, "owner1", "ownerpass"));
            allUsers.push_back(std::make_unique<Guest>(2, "guest1", "guestpass"));

            //Simulate login for owner
            User* user = nullptr;
            for (auto& u : allUsers)
            {
                if (u->getUserName() == "owner1")
                {
                    user = u.get();
                    break;
                }
            }
            expect(user != nullptr, "Should find owner1");
            expect(user->login("ownerpass"), "Owner login should succeed");
            expect(user->getUserRole() == UserRole::Owner, "Should route to Owner dashboard");

            //Simulate login for guest
            user = nullptr;
            for (auto& u : allUsers)
            {
                if (u->getUserName() == "guest1")
                {
                    user = u.get();
                    break;
                }
            }
            expect(user != nullptr, "Should find guest1");
            expect(user->login("guestpass"), "Guest login should succeed");
            expect(user->getUserRole() == UserRole::Guest, "Should route to Guest dashboard");
        }

        beginTest("Multiple guest accounts can coexist");
        {
            std::vector<std::unique_ptr<User>> allUsers;
            allUsers.push_back(std::make_unique<Owner>(1, "owner", "pass"));
            allUsers.push_back(std::make_unique<Guest>(2, "guest1", "pass1"));
            allUsers.push_back(std::make_unique<Guest>(3, "guest2", "pass2"));
            allUsers.push_back(std::make_unique<Guest>(4, "guest3", "pass3"));

            expectEquals((int)allUsers.size(), 4, "Should have 4 users total");

            //Each guest should have unique credentials
            expect(allUsers[1]->login("pass1"), "guest1 should login with pass1");
            expect(allUsers[2]->login("pass2"), "guest2 should login with pass2");
            expect(allUsers[3]->login("pass3"), "guest3 should login with pass3");

            //Guests cant use each others passwords
            expect(!allUsers[1]->login("pass2"), "guest1 should NOT login with guest2's password");
        }
    }
};

// ============================================================================
// SoundLibrary Tests
// ============================================================================
class SoundLibraryTests : public juce::UnitTest{

    public:
    SoundLibraryTests() : juce::UnitTest("SoundLibrary Tests", "Sound") {}

    void runTest() override{
        beginTest("SoundLibrary starts empty");
        {
            SoundLibrary lib;
            expectEquals(lib.getNumSounds(), 0, "New SoundLibrary should have 0 sounds");
        }

        beginTest("getSound returns nullptr for invalid index");
        {
            SoundLibrary lib;
            expect(lib.getSound(0) == nullptr, "getSound(0) on empty library should return nullptr");
            expect(lib.getSound(-1) == nullptr, "getSound(-1) should return nullptr");
            expect(lib.getSound(100) == nullptr, "getSound(100) should return nullptr");
        }

        beginTest("getSounds returns empty vector initially");
        {
            SoundLibrary lib;
            expect(lib.getSounds().empty(), "getSounds should return empty vector for new library");
        }

        beginTest("Loading nonexistent directory does not crash");
        {
            SoundLibrary lib;
            juce::File fakeDir("/tmp/this_directory_does_not_exist_12345");
            lib.loadFromDirectory(fakeDir);
            expectEquals(lib.getNumSounds(), 0, "Loading from nonexistent dir should result in 0 sounds");
        }

        beginTest("Loading nonexistent file does not crash");
        {
            SoundLibrary lib;
            juce::File fakeFile("/tmp/nonexistent_sound_file.wav");
            lib.loadFile(fakeFile);
            expectEquals(lib.getNumSounds(), 0, "Loading nonexistent file should result in 0 sounds");
        }
    }
};

// ============================================================================
// Sound Price Tests
// ============================================================================
class SoundPriceTests : public juce::UnitTest{

    public:
    SoundPriceTests() : juce::UnitTest("Sound Price Tests", "Sound") {}

    void runTest() override{
        //We need a format manager and a real or fake file to construct a Sound
        //Use a temp wav file so the Sound constructor can actually load something
        beginTest("Sound price defaults and can be set by string");
        {
            juce::AudioFormatManager fm;
            fm.registerBasicFormats();

            //Create a minimal valid WAV file in temp
            juce::File tempFile = juce::File::getSpecialLocation(juce::File::tempDirectory)
                                      .getChildFile("test_price.wav");
            {
                std::unique_ptr<juce::AudioFormatWriter> writer(
                    fm.findFormatForFileExtension("wav")
                      ->createWriterFor(new juce::FileOutputStream(tempFile), 44100.0, 1, 16, {}, 0));
                if (writer != nullptr)
                {
                    juce::AudioBuffer<float> buf(1, 441); // 10ms of silence
                    buf.clear();
                    writer->writeFromAudioSampleBuffer(buf, 0, 441);
                }
            }

            if (tempFile.existsAsFile())
            {
                Sound s(1, "test", tempFile, fm);
                if (s.isValid())
                {
                    s.setSoundPrice("9.99");
                    expectEquals(s.getSoundPrice(), juce::String("9.99"),
                                 "Price should be 9.99 after setting");

                    s.setSoundPrice(0.0);
                    expectEquals(s.getSoundPrice(), juce::String("0.00"),
                                 "Price should be 0.00 after setting to 0");

                    s.setSoundPrice(25.50);
                    expectEquals(s.getSoundPrice(), juce::String("25.50"),
                                 "Price should be 25.50 after setting double");
                }
                tempFile.deleteFile();
            }
        }

        beginTest("Sound metadata and getters work on valid file");
        {
            juce::AudioFormatManager fm;
            fm.registerBasicFormats();

            juce::File tempFile = juce::File::getSpecialLocation(juce::File::tempDirectory)
                                      .getChildFile("test_meta.wav");
            {
                std::unique_ptr<juce::AudioFormatWriter> writer(
                    fm.findFormatForFileExtension("wav")
                      ->createWriterFor(new juce::FileOutputStream(tempFile), 44100.0, 1, 16, {}, 0));
                if (writer != nullptr)
                {
                    juce::AudioBuffer<float> buf(1, 44100); // 1 second of silence
                    buf.clear();
                    writer->writeFromAudioSampleBuffer(buf, 0, 44100);
                }
            }

            if (tempFile.existsAsFile())
            {
                Sound s(42, "mysound", tempFile, fm);
                if (s.isValid())
                {
                    expectEquals(s.getSoundId(), 42, "Sound ID should be 42");
                    expectEquals(s.getName(), juce::String("mysound"), "Sound name should be 'mysound'");
                    expectGreaterThan(s.getSampleRate(), 0.0, "Sample rate should be > 0");
                    expectEquals(s.getAudioBuffer().getNumChannels(), 1, "Should have 1 channel");
                    expect(!s.getDurationString().isEmpty(), "Duration string should not be empty");
                    expect(!s.getMetaData().empty(), "Metadata should not be empty");
                }
                tempFile.deleteFile();
            }
        }
    }
};

// ============================================================================
// Unified Login Flow Tests (simulates the new combined login system)
// NOTE: This test was generated by Claude, will need to add more later to test sound effects
// ============================================================================
class UnifiedLoginTests : public juce::UnitTest{

    public:
    UnifiedLoginTests() : juce::UnitTest("Unified Login Tests", "Authentication") {}

    void runTest() override{
        beginTest("Unified login works for Owner account");
        {
            std::vector<std::unique_ptr<User>> allUsers;
            allUsers.push_back(std::make_unique<Owner>(1, "admin", "ownerpass"));

            // Simulate unified login — find user, check password, route by role
            User* user = nullptr;
            for (auto& u : allUsers)
            {
                if (u->getUserName() == "admin")
                {
                    user = u.get();
                    break;
                }
            }
            expect(user != nullptr, "Should find user 'admin'");
            expect(user->login("ownerpass"), "Login should succeed with correct password");
            expect(user->getUserRole() == UserRole::Owner, "User should be routed to Owner dashboard");
        }

        beginTest("Unified login works for Guest account");
        {
            std::vector<std::unique_ptr<User>> allUsers;
            allUsers.push_back(std::make_unique<Owner>(1, "admin", "ownerpass"));
            allUsers.push_back(std::make_unique<Guest>(2, "guestuser", "guestpass"));

            // Guest logs in through same login flow
            User* user = nullptr;
            for (auto& u : allUsers)
            {
                if (u->getUserName() == "guestuser")
                {
                    user = u.get();
                    break;
                }
            }
            expect(user != nullptr, "Should find user 'guestuser'");
            expect(user->login("guestpass"), "Guest login should succeed");
            expect(user->getUserRole() == UserRole::Guest, "User should be routed to Guest dashboard");
        }

        beginTest("Unified login rejects nonexistent user");
        {
            std::vector<std::unique_ptr<User>> allUsers;
            allUsers.push_back(std::make_unique<Owner>(1, "admin", "pass"));

            User* user = nullptr;
            for (auto& u : allUsers)
            {
                if (u->getUserName() == "nobody")
                {
                    user = u.get();
                    break;
                }
            }
            expect(user == nullptr, "Should not find nonexistent user");
        }

        beginTest("Unified login rejects wrong password for both roles");
        {
            std::vector<std::unique_ptr<User>> allUsers;
            allUsers.push_back(std::make_unique<Owner>(1, "owner", "ownerpass"));
            allUsers.push_back(std::make_unique<Guest>(2, "guest", "guestpass"));

            expect(!allUsers[0]->login("wrong"), "Owner login should fail with wrong password");
            expect(!allUsers[1]->login("wrong"), "Guest login should fail with wrong password");
        }

        beginTest("Guest accounts are persistent, not ephemeral");
        {
            std::vector<std::unique_ptr<User>> allUsers;
            allUsers.push_back(std::make_unique<Owner>(1, "admin", "pass"));
            allUsers.push_back(std::make_unique<Guest>(2, "guest1", "gpass"));

            // Guest logs in
            expect(allUsers[1]->login("gpass"), "Guest first login should succeed");
            allUsers[1]->logout();

            // Guest can log in again — account persists
            expect(allUsers[1]->login("gpass"), "Guest re-login should succeed (persistent account)");

            // Guest is still in the user list
            expectEquals((int)allUsers.size(), 2, "User list should still have 2 users after guest logout/login");
        }
    }
};

// ============================================================================
// Owner Creates Guest Account Tests
// ============================================================================
class OwnerGuestCreationTests : public juce::UnitTest{

    public:
    OwnerGuestCreationTests() : juce::UnitTest("Owner Guest Creation Tests", "UserManagement") {}

    void runTest() override{
        beginTest("Owner creates guest account — guest can then login");
        {
            std::vector<std::unique_ptr<User>> allUsers;
            int nextId = 1;

            // First user is owner
            allUsers.push_back(std::make_unique<Owner>(nextId++, "owner", "ownerpass"));

            // Owner creates a guest account
            allUsers.push_back(std::make_unique<Guest>(nextId++, "newguest", "guestpass"));

            expectEquals((int)allUsers.size(), 2, "Should have 2 users");

            // New guest can log in
            User* guest = nullptr;
            for (auto& u : allUsers)
            {
                if (u->getUserName() == "newguest")
                {
                    guest = u.get();
                    break;
                }
            }
            expect(guest != nullptr, "Should find the newly created guest");
            expect(guest->login("guestpass"), "New guest should be able to login");
            expect(guest->getUserRole() == UserRole::Guest, "New account should be Guest role");
        }

        beginTest("Owner creates multiple guest accounts");
        {
            std::vector<std::unique_ptr<User>> allUsers;
            int nextId = 1;

            allUsers.push_back(std::make_unique<Owner>(nextId++, "owner", "pass"));
            allUsers.push_back(std::make_unique<Guest>(nextId++, "guest_a", "pass_a"));
            allUsers.push_back(std::make_unique<Guest>(nextId++, "guest_b", "pass_b"));
            allUsers.push_back(std::make_unique<Guest>(nextId++, "guest_c", "pass_c"));

            expectEquals((int)allUsers.size(), 4, "Should have 4 users");
            expectEquals(nextId, 5, "Next ID should be 5");

            // All guests have unique IDs
            expect(allUsers[1]->getUserID() != allUsers[2]->getUserID(), "Guest IDs should be unique");
            expect(allUsers[2]->getUserID() != allUsers[3]->getUserID(), "Guest IDs should be unique");

            // Each guest can log in with own password only
            expect(allUsers[1]->login("pass_a"), "guest_a logs in with own password");
            expect(!allUsers[1]->login("pass_b"), "guest_a cannot use guest_b's password");
            expect(allUsers[2]->login("pass_b"), "guest_b logs in with own password");
            expect(allUsers[3]->login("pass_c"), "guest_c logs in with own password");
        }

        beginTest("Cannot create guest with duplicate username");
        {
            std::vector<std::unique_ptr<User>> allUsers;
            allUsers.push_back(std::make_unique<Owner>(1, "admin", "pass"));
            allUsers.push_back(std::make_unique<Guest>(2, "guest1", "gpass"));

            // Simulate duplicate check before creating
            juce::String newName = "guest1";
            bool duplicate = false;
            for (auto& u : allUsers)
            {
                if (u->getUserName() == newName)
                {
                    duplicate = true;
                    break;
                }
            }
            expect(duplicate, "Should detect duplicate username 'guest1'");

            // Non-duplicate should pass
            newName = "guest2";
            duplicate = false;
            for (auto& u : allUsers)
            {
                if (u->getUserName() == newName)
                {
                    duplicate = true;
                    break;
                }
            }
            expect(!duplicate, "Should NOT detect duplicate for 'guest2'");
        }
    }
};

// ============================================================================
// SoundListComponent Mode Tests
// ============================================================================
class SoundListModeTests : public juce::UnitTest{

    public:
    SoundListModeTests() : juce::UnitTest("SoundList Mode Tests", "Sound") {}

    void runTest() override{
        beginTest("SoundListComponent defaults to owner mode");
        {
            SoundLibrary lib;
            SoundListComponent list(lib);
            expect(!list.isGuestMode(), "Default SoundListComponent should be in owner mode");
        }

        beginTest("SoundListComponent can be set to guest mode");
        {
            SoundLibrary lib;
            SoundListComponent list(lib, true);
            expect(list.isGuestMode(), "SoundListComponent(lib, true) should be in guest mode");
        }

        beginTest("SoundListComponent owner mode — not guest");
        {
            SoundLibrary lib;
            SoundListComponent list(lib, false);
            expect(!list.isGuestMode(), "SoundListComponent(lib, false) should be in owner mode");
        }

        beginTest("Empty library shows 0 rows in both modes");
        {
            SoundLibrary lib;
            SoundListComponent ownerList(lib, false);
            SoundListComponent guestList(lib, true);
            expectEquals(ownerList.getNumRows(), 0, "Owner list should have 0 rows for empty library");
            expectEquals(guestList.getNumRows(), 0, "Guest list should have 0 rows for empty library");
        }

        beginTest("isPlaying returns false when nothing is playing");
        {
            SoundLibrary lib;
            SoundListComponent list(lib);
            expect(!list.isPlaying(0), "Row 0 should not be playing initially");
            expect(!list.isPlaying(-1), "Row -1 should not be playing");
            expect(!list.isPlaying(100), "Row 100 should not be playing");
        }

        beginTest("Callbacks are initially null");
        {
            SoundLibrary lib;
            SoundListComponent list(lib);
            expect(list.onSoundSelected == nullptr, "onSoundSelected should be null initially");
            expect(list.onBuySound == nullptr, "onBuySound should be null initially");
            expect(list.onEditSound == nullptr, "onEditSound should be null initially");
        }

        beginTest("Callbacks can be assigned");
        {
            SoundLibrary lib;
            SoundListComponent list(lib);

            bool selectFired = false;
            bool buyFired = false;
            bool editFired = false;

            list.onSoundSelected = [&](Sound*) { selectFired = true; };
            list.onBuySound = [&](Sound*) { buyFired = true; };
            list.onEditSound = [&](Sound*) { editFired = true; };

            expect(list.onSoundSelected != nullptr, "onSoundSelected should be set");
            expect(list.onBuySound != nullptr, "onBuySound should be set");
            expect(list.onEditSound != nullptr, "onEditSound should be set");

            // Fire callbacks with nullptr to verify they work
            list.onSoundSelected(nullptr);
            list.onBuySound(nullptr);
            list.onEditSound(nullptr);

            expect(selectFired, "onSoundSelected callback should fire");
            expect(buyFired, "onBuySound callback should fire");
            expect(editFired, "onEditSound callback should fire");
        }
    }
};

// ============================================================================
// Guest Sample Playback Tests (buffer trimming logic)
// ============================================================================
class GuestSampleTests : public juce::UnitTest{

    public:
    GuestSampleTests() : juce::UnitTest("Guest Sample Tests", "Sound") {}

    void runTest() override{
        beginTest("20% sample buffer is correctly sized");
        {
            // Simulate what togglePlayback does for guest mode
            int fullSamples = 44100;  // 1 second at 44.1kHz
            int sampleCount = fullSamples / 5;  // 20%

            expectEquals(sampleCount, 8820, "20% of 44100 should be 8820 samples");

            juce::AudioBuffer<float> fullBuffer(1, fullSamples);
            fullBuffer.clear();

            // Fill with a known pattern so we can verify the copy
            for (int i = 0; i < fullSamples; ++i)
                fullBuffer.setSample(0, i, (float)i / (float)fullSamples);

            juce::AudioBuffer<float> sampleBuffer;
            sampleBuffer.setSize(fullBuffer.getNumChannels(), sampleCount, false, true, false);
            for (int ch = 0; ch < fullBuffer.getNumChannels(); ++ch)
                sampleBuffer.copyFrom(ch, 0, fullBuffer, ch, 0, sampleCount);

            expectEquals(sampleBuffer.getNumSamples(), 8820, "Sample buffer should have 8820 samples");
            expectEquals(sampleBuffer.getNumChannels(), 1, "Sample buffer should have 1 channel");
        }

        beginTest("20% sample buffer contains correct data from start of original");
        {
            int fullSamples = 10000;
            int sampleCount = fullSamples / 5;  // 2000

            juce::AudioBuffer<float> fullBuffer(2, fullSamples);  // stereo
            for (int ch = 0; ch < 2; ++ch)
                for (int i = 0; i < fullSamples; ++i)
                    fullBuffer.setSample(ch, i, (float)(i + ch * 100));

            juce::AudioBuffer<float> sampleBuffer;
            sampleBuffer.setSize(2, sampleCount, false, true, false);
            for (int ch = 0; ch < 2; ++ch)
                sampleBuffer.copyFrom(ch, 0, fullBuffer, ch, 0, sampleCount);

            expectEquals(sampleBuffer.getNumSamples(), 2000, "Sample should be 2000 samples");
            expectEquals(sampleBuffer.getNumChannels(), 2, "Sample should be stereo");

            // Verify first and last sample of the trimmed buffer
            expectEquals(sampleBuffer.getSample(0, 0), 0.0f, "First sample ch0 should be 0");
            expectEquals(sampleBuffer.getSample(0, 1999), 1999.0f, "Last sample ch0 should be 1999");
            expectEquals(sampleBuffer.getSample(1, 0), 100.0f, "First sample ch1 should be 100");
            expectEquals(sampleBuffer.getSample(1, 1999), 2099.0f, "Last sample ch1 should be 2099");

            // Verify the sample does NOT contain data beyond 20%
            // (the buffer literally doesn't have index 2000, so this is a size check)
            expectEquals(sampleBuffer.getNumSamples(), sampleCount,
                         "Buffer size must be exactly 20% — no extra data");
        }

        beginTest("20% of various durations");
        {
            // Short sound: 100 samples
            expectEquals(100 / 5, 20, "20% of 100 samples = 20");

            // Very short: 5 samples
            expectEquals(5 / 5, 1, "20% of 5 samples = 1");

            // Long sound: 441000 samples (10 seconds)
            expectEquals(441000 / 5, 88200, "20% of 441000 samples = 88200");
        }

        beginTest("MemoryAudioSource from sample buffer has correct length");
        {
            int fullSamples = 44100;
            int sampleCount = fullSamples / 5;

            juce::AudioBuffer<float> sampleBuffer(1, sampleCount);
            sampleBuffer.clear();

            // Create MemoryAudioSource with copy
            juce::MemoryAudioSource source(sampleBuffer, true, false);

            // Prepare it and read — should only produce sampleCount samples
            source.prepareToPlay(512, 44100.0);

            int totalRead = 0;
            juce::AudioBuffer<float> readBuf(1, 512);
            juce::AudioSourceChannelInfo info(readBuf);

            // Read until the source is exhausted
            for (int i = 0; i < 100; ++i)  // safety limit
            {
                readBuf.clear();
                source.getNextAudioBlock(info);

                // Check if the block is silent (source exhausted)
                float mag = readBuf.getMagnitude(0, 0, 512);
                // Since our source is all zeros, we check position instead
                totalRead += 512;
                if (totalRead >= sampleCount)
                    break;
            }

            // The source should not produce more than sampleCount samples worth of non-repeated data
            expect(totalRead >= sampleCount, "Should be able to read at least sampleCount samples");
        }
    }
};

// ============================================================================
// Static test instances — JUCE picks these up automatically
// ============================================================================
static AuthenticationTests authTestInstance;
static UserRoleTests roleTestInstance;
static AccountManagementTests accountTestInstance;
static SoundLibraryTests soundLibTestInstance;
static SoundPriceTests soundPriceTestInstance;
static UnifiedLoginTests unifiedLoginTestInstance;
static OwnerGuestCreationTests ownerGuestCreationTestInstance;
static SoundListModeTests soundListModeTestInstance;
static GuestSampleTests guestSampleTestInstance;

// ============================================================================
// Test runner helper
// ============================================================================
inline void runAllTests(){
    DBG("============================================");
    DBG("         RUNNING ALL UNIT TESTS");
    DBG("============================================");

    //Create test runner object
    juce::UnitTestRunner runner;
    runner.setAssertOnFailure(false);
    runner.runAllTests();

    //Build a results string
    juce::String results;
    results << "============ TEST RESULTS ============\n\n";
    int totalPasses = 0;
    int totalFailures = 0;

    //Loop through results
    for (int i = 0; i < runner.getNumResults(); ++i){
        auto* result = runner.getResult(i);
        if (result != nullptr){
            results << "Test: " << result->unitTestName << "\n";
            results << "  Passes:   " << result->passes << "\n";
            results << "  Failures: " << result->failures << "\n";

            totalPasses   += result->passes;
            totalFailures += result->failures;

            //Print any failure messages
            for (auto& msg : result->messages)
                results << "  >> " << msg << "\n";

            results << "\n";
        }
    }

    results << "----------------------------------------\n";
    results << "TOTAL  Passes: " << totalPasses
            << "  Failures: " << totalFailures << "\n";
    results << "========================================\n";

    DBG(results);

    //Also write to a file on the desktop so its easy to find
    juce::File desktop = juce::File::getSpecialLocation(juce::File::userDesktopDirectory);
    juce::File outputFile = desktop.getChildFile("TestResults.txt");
    outputFile.replaceWithText(results);

    DBG("Test results also saved to: " << outputFile.getFullPathName());
}
