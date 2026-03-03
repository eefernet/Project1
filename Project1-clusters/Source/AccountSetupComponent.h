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
    AccountSetupComponent(bool isFirstUser = true);
    ~AccountSetupComponent() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    //Callback when account is created or we cancel account creation
    std::function<void(juce::String username, juce::String password, UserRole role)> onAccountCreated;
    std::function<void()> onCancel;

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

    //Callback functions for button clicks, so we can handle logic
    //These remain private
    void onCreateAccount();
	//Learned that this needs to be unique from the callback above, otherwise we get a "redefinition of onCancel" error, which is fun
    void onCancelClicked();
	void ResetFields();

	//Label to display error messages or success messages
    juce::Label messageLabel;

	//This is so that if its the first user being created, we can automatically make them an owner and skip the role selection step
    bool isFirstUser;
	//This is so we can check whos creating the account and only allow owners to create new accounts, guests should not have access to this component at all
    //honestly, this needs to be rethought maybe, because there should only beb one "Owner"
    bool isOwner;

	//Somethign that juice needs for memory management and debugging
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AccountSetupComponent)
};
