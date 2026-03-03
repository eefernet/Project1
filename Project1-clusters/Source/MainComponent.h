
#pragma once

#include <JuceHeader.h>
#include "LoginComponent.h"
#include "AccountSetupComponent.h"
#include "User.h"
#include "Owner.h"
#include "Guest.h"
#include "OwnerDashboardComponent.h"
#include "GuestDashboardComponent.h"
#include "ClusterPageComponent.h"
#include "ClusterEngine.h"
#include "Soundlibrary.h"



/*
* This is the main parent compontent that will manage the different scereens and switching between them.
* It will also handle user management and authentication logic for Sprint 1, and then we can move that 
* to a separate UserManager class in Sprint 2 when we implement persistent storage.
*/
class MainComponent : public juce::Component
{
public:
    MainComponent();
    ~MainComponent() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    //UI Components
    LoginComponent loginScreen;
    AccountSetupComponent accountSetup;
    OwnerDashboardComponent ownerDashboard;
    GuestDashboardComponent guestDashboard;
    ClusterEngine clusterEngine;
    ClusterPageComponent clusterPage;
    
    //User management
    std::unique_ptr<User> currentUser;
    std::vector<std::unique_ptr<User>> allUsers;  // Simple in-memory storage for Sprint 1
    int nextUserId;

	//View state to manage which screen is currently being displayed, we can add more later as we add more windows
    enum class ViewState
    {
        Login,
        AccountSetup,
        OwnerDashboard,
        GuestDashboard,
        ClusterView
    };
    ViewState currentView;
    ViewState lastDashboardView = ViewState::Login;
    //Event handlers
    void handleOwnerLogin(juce::String username, juce::String password);
    void handleGuestLogin();
    void handleCreateAccountRequest();
    void handleAccountCreated(juce::String username, juce::String password, UserRole role);
    void handleCancelAccountSetup();

    //Helper methods
    void showView(ViewState view);
    User* findUser(const juce::String& username);
    bool isFirstUser() const;

    //Placeholder for future dashboards
    void showOwnerDashboard();
    void showGuestDashboard();

    void showClusterView();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
