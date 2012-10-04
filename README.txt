$Id$

Description
===========
The D7Permissions module aims to port some of the useful functionality that has been included in the Drupal 7 roles and permissions system into Drupal 6.

After enabling the module, you can specify the order of roles on a revised permissions page via a drag-and-drop interface. Additionally, this page includes minor updates such as displaying all roles (aside from anonymous) as "checked" when a permission has "authenticated user" checked.

These pages can be reached under the User Management menu item, or by going directly to the admin page: admin/user/d7permissions

Benefits
========
The ability to order roles on the permissions page becomes quite helpful when your site has a lot of roles to deal with. For example, this list of roles is ordered alphabetically and can be difficult to manage:

anonymous
authenticated
administrator
editor
moderator
semi-authenticated (for use with the LoginTobogan module)

Using this module, you could order these roles like this:

anonymous
semi-authenticated
authenticated
moderator
editor
administrator

Additionally, the "dummy checkboxes" on the permissions page make a multitude of permissions easier to sort through. If a specific permission is given to "authenticated user", it inherently applies to *all* roles (except for anonymous) -- this is the default and intended behavior of Drupal. This functionality is described to the user through this use of this module.