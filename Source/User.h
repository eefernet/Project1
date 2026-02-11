/*
  ==============================================================================

    User.h
    Created: 10 Feb 2026 6:03:26pm
    Author:  ethan

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "UserRole.h"

/*
* The User class is a class that represents a user of the application. It contains information about the user, 
such as their name and role. The User class is used to determine what actions a user can perform within the 
application, based on their role.
*/

class User {

public:
    User();
	User(int userID, juce::String userName, juce::String userPassword, UserRole role);
	virtual ~User();

    //Getters
    int getUserID() const {return userID;}
	juce::String getUserName() const { return userName; }
	//obv this is not secure by any means 
    //TODO: implement a encryption/decryption method for storing user passwords?
	juce::String getUserPassword() const { return userPassword; }
    UserRole getUserRole() const { return role; }


    //Setters
	void setUserID(int newUserID) { userID = newUserID; }
	void setUserName(juce::String newUserName) { userName = newUserName; }

    //Authentication
	bool login(juce::String inputPassword);
	bool logout();

	/*
    * Vitrual methods so each user type can implement and override these methods
	* This method is used to perform an action based on the user's role. For example, 
    * an Owner can perform actions such as adding or removing users, while a Guest can only view certain information.
    */
	virtual void displayWelcome() = 0; 

   
//Protected members are used to store information about the user.
  
protected:
    int userID;
    juce::String userName;
    juce::String userPassword;
	UserRole role;
	bool isLoggedIn;
};