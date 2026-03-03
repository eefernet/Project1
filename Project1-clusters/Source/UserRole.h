/*
  ==============================================================================

    UserRole.h
    Created: 10 Feb 2026 6:36:20pm
    Author:  ethan

  ==============================================================================
*/

#pragma once

/*
* Technically we dont need an enum to dictate user roles, but this format makes it easier to scale code in the future
* It also makes the code mroe readable and easier to understand, as opposed to using a boolean value to determine if the user is an owner or a guest
*/
enum class UserRole
{
    Owner,
    Guest
};
