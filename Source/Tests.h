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
// Static test instances — JUCE picks these up automatically
// ============================================================================
static AuthenticationTests authTestInstance;
static UserRoleTests roleTestInstance;
static AccountManagementTests accountTestInstance;
static SoundLibraryTests soundLibTestInstance;
static SoundPriceTests soundPriceTestInstance;

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
