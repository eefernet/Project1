## Please read everything below before calling ```git clone```

# Project1 — Sound App 

A desktop audio application built with the [JUCE](https://juce.com/) framework in C++. The app features role-based access control with Owner and Guest user types, each with different levels of access to audio features like recording, playback, filtering, sound management, and 2D cluster visualization.

> **Status:** Sprint 2 — Audio features, dashboards, and 2D cluster map visualization. Playback sound, view a list of sounds, apply filters, create a new sound, and visualize sound clusters by length or similarity.

---

[Trello Task Board](https://trello.com/invite/b/69964e2da051502ab1943a96/ATTIbed5b57d17a4399478854f212afd3f91946F3DE5/project1)

## Features

### Sprint 1 (Finished)
- **Login screen** with Owner authentication (username + password) and Guest quick-access
- **Account creation** with form validation (empty fields, password confirmation, role selection)
- **Role-based user system** using an inheritance hierarchy (`User` → `Owner` / `Guest`)
- **View management** — seamless switching between Login and Account Setup screens
- **First-user enforcement** — the first account created is automatically assigned the `Owner` role (very seecure trust me....)

### Sprint 2–3 (Current)
- Owner dashboard: record sound, view waveform, play, filter, save, list saved sounds, cluster map
- Guest dashboard: browse sound list, play & filter, download, view cluster map (read-only)
- **2D Cluster Map** — visualize loaded sounds as a scatter plot, clustered by length or by similarity (length + RMS loudness). Both Owner and Guest can access the cluster view from their dashboards.
- Persistent user storage
- Audio recording and playback engine

## Architecture

```
Project1/
├── Project1.jucer                      # Projucer project file
├── README.md
└── Source/
    ├── Main.cpp                        # App entry point & window setup
    ├── MainComponent.h/cpp             # Root component — screen routing & user management
    │
    ├── Views/
    │   ├── LoginComponent.h/cpp        # Login screen UI & input handling
    │   ├── AccountSetupComponent.h/cpp # Account creation form & validation
    │   ├── OwnerDashboardComponent.h/cpp # Owner post-login dashboard (load button, sound list, cluster access)
    │   ├── GuestDashboardComponent.h/cpp # Guest post-login dashboard (cluster access)
    │   ├── SoundListComponent.h/cpp    # Scrollable ListBox UI for displaying loaded sounds
    │   ├── ClusterPageComponent.h/cpp  # Cluster view page with mode selector, run button, and back navigation
    │   └── ClusterView.h/cpp           # Custom JUCE component that renders the 2D cluster scatter plot
    │
    ├── Models/
    │   ├── User.h/cpp                  # Abstract base class for all users
    │   ├── Owner.h/cpp                 # Owner subclass (full access)
    │   ├── Guest.h/cpp                 # Guest subclass (limited access)
    │   ├── Sound.h/cpp                 # Sound object (audio buffer, waveform image, metadata)
    │   ├── SoundLibrary.h/cpp          # Loads & manages a collection of Sound objects from disk
    │   └── ClusterEngine.h/cpp         # Clustering algorithms — groups sounds by length or by similarity (length + RMS)
    │
    └── Utils/
        ├── UserRole.h                  # Enum for Owner / Guest roles
        ├── Filter.h                    # Filter enum (Pitch, Length, Effect)
        └── LoginTest.h                 # Unit tests for login system
```

### Cluster Map

The cluster feature lets users visualize their loaded sounds in a 2D space. `ClusterEngine` operates on the `SoundLibrary` and supports two modes:

- **By Length** — sorts sounds by duration and assigns them to 5 fixed groups, each positioned around a predefined center point with jitter for visual spread.
- **By Similarity** — plots sounds on a 2D plane where the X axis represents normalized duration and the Y axis represents normalized RMS loudness, giving a natural similarity grouping.

Both Owner and Guest dashboards have a "View 2D Cluster" button that navigates to `ClusterPageComponent`, which wraps `ClusterView` (the renderer) and provides a combo box to switch modes, a "Run Clustering" button, and a back button to return to the dashboard.

### Component Communication

Communication between UI components uses `std::function` callbacks — child components expose public callback members that the parent (`MainComponent`) wires up to its own handler methods. This
should make it easier for us to add multiple views that will be a child of the main view controller. Its as if we have a stack of cards, and the `MainComponent` contains the stack of cards. We only
flip to the card by making it `visable` to the user one at a time, while the other cards (`views`) stay hidden until needed. The child components do not have to reference or know about the parent `MainComponent`
but we use a lambda function in the `MainComponent` constructor to call each specific function regarding that request.

## Prerequisites

- **JUCE Framework** — download from [juce.com](https://juce.com/get-juce/)
- **Projucer** — included with JUCE, used to generate IDE project files from `Project1.jucer`
- **C++17** compatible compiler
- **Windows:** Visual Studio 2019+ | **macOS:** Xcode 12+ | **Linux:** GCC 9+ with standard JUCE dependencies

## Building

1. Clone the repository:
   ```bash
   git clone https://github.com/eefernet/Project1.git
   ```
2. Open `Project1.jucer` in **Projucer**.
3. Set the JUCE module path in Projucer. Default sometimes works, but you might need to point your **Projucer** application to the right folders that contain your modules and Jucer main folder.
4. Click **Save and Open in IDE**.
5. Build and run from your IDE (tested with Visual Studio — Debug x64, CLion with Linux Makefile).
6. Any new features must be created in a new branch labeled `feature/ShinyNewFeature`.
7. If your feature is ready for prod release, do a pull request to `master`

## Adding Files (IMPORTANT PLEASE READ)

Make sure when you are adding new .h and .cpp files, you DO NOT create files within your IDE. Make sure you create those files in Projucer then open project in your IDE.

## Usage

1. **First launch** — you'll be prompted to create an Owner account (role is locked to Owner for the first user).
2. **Subsequent launches** — log in as Owner with your credentials, continue as Guest, or create additional accounts.
3. **Owner dashboard** — load sounds from disk, view them in a list, and open the 2D cluster map to visualize groupings.
4. **Guest dashboard** — browse sounds and view the cluster map (read-only).
