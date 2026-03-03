/*
  ==============================================================================

    LoginTest.h
    Created: 25 Feb 2026 6:55:03pm
    Author:  eefernet

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Owner.h"
#include "Guest.h"

class LoginTest : public juce::UnitTest{

    public:
    LoginTest() : juce::UnitTest("Login System Tests", "Authentication") {}

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

        //Owner has Owner role and Guest has Guest role
        beginTest("Owner has Owner role and Guest has Guest role");
        {
            Owner owner(7, "admin", "pw");
            Guest guest(8, "viewer", "pw");

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
    }
};

static LoginTest loginTestInstance;

//helper method for running login test, didnt seem to want to solo use the one above
inline void runLoginTests(){
    //Print to debug what we are doing
    DBG("============================================");
    DBG("       RUNNING LOGIN UNIT TESTS");
    DBG("============================================");

    //Create test runner object
    juce::UnitTestRunner runner;
    runner.setAssertOnFailure(false);
    runner.runAllTests();

    //Build a results string
    juce::String results;
    results << "========== LOGIN TEST RESULTS ==========\n\n";
    //vars for fails and passes
    int totalPasses = 0;
    int totalFailures = 0;

    //Loop through results
    for (int i = 0; i < runner.getNumResults(); ++i){
        //get results for each test then print
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

    /*Print to debug output (if on linux, right click and click run in terminal) for windows
     *just CD to the dir with shell and compile the program there. Results should print to
     *shell. If not compiled this will be visibble in IDE console
     */
    DBG(results);

    //Also write to a file on the to desktop so its easy to find
    //should work across all OS? juce should be handling this backend
    juce::File desktop = juce::File::getSpecialLocation(juce::File::userDesktopDirectory);
    juce::File outputFile = desktop.getChildFile("TestResults.txt");
    outputFile.replaceWithText(results);

    DBG("Test results also saved to: " << outputFile.getFullPathName());
}