/*
  ==============================================================================

    Guest.cpp
    Created: 10 Feb 2026 6:03:53pm
    Author:  ethan

  ==============================================================================
*/
#include "Guest.h"

//Constructor and destructor
//defualt constructor
Guest::Guest() : User()
{
    role = UserRole::Guest;
}

//Param based constructor
Guest::Guest(int userId, juce::String userName, juce::String password)
    : User(userId, userName, password, UserRole::Guest)
{
}
//We probably dont need this? At least atm?
Guest::~Guest()
{
}

//This will be removed once pushed to prod, we would segue here to the main application window
//TODO: Implement a proper welcome screen with options to navigate to different features based on user role
void Guest::displayWelcome()
{
    DBG("=================================");
    DBG("Welcome Guest: " << userName);
    DBG("Limited access - Browse and play sounds");
    DBG("=================================");
}

/*
* These methods have not been implemented yet, sprint 2 and 3 will focus on implementing these features, 
* but for now they are just placeholders to show the intended functionality for a guest user.
*/

//TODO: REMOVING THESE FUNCTIONS BECAUSE I WENT A DIFFERENT ROUTE ON HOW WE PLAYBACK AND VIEW EACH VIEW
//OLD CODE PLEASE DO NOT USE THESE FUNCTIONS, THEY ARE DEPRECATED
