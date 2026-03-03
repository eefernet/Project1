/*
  ==============================================================================

    User.cpp
    Created: 10 Feb 2026 6:03:26pm
    Author:  ethan

  ==============================================================================
*/

#include "User.h"   

User::User(): userID(0), userName(""), userPassword(""), role(UserRole::Guest), isLoggedIn(false){}

User::User(int userID, juce::String userName, juce::String userPassword, UserRole role)
    : userID(userID), userName(userName), userPassword(userPassword), role(role), isLoggedIn(false)
{
}

User::~User()
{
}

bool User::login(juce::String inputPassword)
{
    // Simple password check (in production, use proper hashing!)
    if (inputPassword == userPassword)
    {
        isLoggedIn = true;
        DBG("User " << userName << " logged in successfully");
        return true;
    }

    DBG("Login failed for user: " << userName);
    return false;
}

bool User::logout()
{
    isLoggedIn = false;
    DBG("User " << userName << " logged out");
    return true;
}
