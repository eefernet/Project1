/*
  ==============================================================================

    Owner.cpp
    Created: 10 Feb 2026 6:03:43pm
    Author:  ethan

  ==============================================================================
*/

#include "Owner.h"

Owner::Owner() : User()
{
    role = UserRole::Owner;
}

Owner::Owner(int userId, juce::String userName, juce::String password)
    : User(userId, userName, password, UserRole::Owner)
{
}

Owner::~Owner()
{
}

void Owner::displayWelcome()
{
    //Right now just displaying this to console for testing
    DBG("=================================");
    DBG("Welcome Owner: " << userName);
    DBG("You have full access to all features");
    DBG("=================================");
}

//TODO: REMOVING THESE FUNCTIONS BECAUSE I WENT A DIFFERENT ROUTE ON HOW WE PLAYBACK AND VIEW EACH VIEW
//OLD CODE PLEASE DO NOT USE THESE FUNCTIONS, THEY ARE DEPRECATED
