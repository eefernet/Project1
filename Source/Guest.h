/*
  ==============================================================================

    Guest.h
    Created: 10 Feb 2026 6:03:53pm
    Author:  ethan

  ==============================================================================
*/

#pragma once
#include "User.h"

/*
* The Guest class is a derived class of the User class. It represents a guest user of the application, 
* who has limited access to features compared to an Owner. The Guest class inherits all the properties and 
* methods of the User class, and can also have its own specific methods that are relevant to a guest user.
*/
class Guest : public User
{
public:
    Guest();
    Guest(int userId, juce::String userName, juce::String password);
    ~Guest() override;

    //Override and display the welcome message for a guest user
    void displayWelcome() override;
};
