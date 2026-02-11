/*
  ==============================================================================

    AccountSetupComponent.h
    Created: 10 Feb 2026 6:03:10pm
    Author:  ethan

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "UserRole.h"


/// This component will be used to create new user accounts. It will have input fields for username, password, and role selection (owner or guest).
class AccountSetupComponent : public juce::Component
{
public:
    //This constructor is going to be overriden in the implementation file
    AccountSetupComponent();
    ~AccountSetupComponent() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    //Callback when account is created
    std::function<void(juce::String username, juce::String password, UserRole role)> onAccountCreated;

private:
	//UI Elements for account setup form
    juce::Label titleLabel;
    juce::Label usernameLabel;
    juce::Label passwordLabel;
    juce::Label confirmPasswordLabel;
    juce::Label roleLabel;

    juce::TextEditor usernameInput;
    juce::TextEditor passwordInput;
    juce::TextEditor confirmPasswordInput;

    juce::ComboBox roleSelector;
    juce::TextButton createAccountButton;
    juce::TextButton cancelButton;

	//Label to display error messages or success messages
    juce::Label messageLabel;

	//Callback functions for button clicks, so we can handle logic
    void onCreateAccount();
    void onCancel();

	//This is so that if its the first user being created, we can automatically make them an owner and skip the role selection step
    bool isFirstUser;
	//This is so we can check whos creating the account and only allow owners to create new accounts, guests should not have access to this component at all
    //honestly, this needs to be rethought maybe, because there should only beb one "Owner"
    bool isOwner;

	//Somethign that juice needs for memory management and debugging
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AccountSetupComponent)
};
