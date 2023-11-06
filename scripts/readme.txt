smartmet-workstation scripts directory information:

1. cmake directory
- Contains scripts dealing with cmake build.
- smartmet-workstation .gitignore ignores this directory.
- If you make changes to cmake scripts, you must copy the changes into \tools\cmake_build_script directory files.

2. git directory
- Contains scripts dealing with git workflows with different branches (main/dev-smartmet) and pull/push from/to github repository.
- smartmet-workstation .gitignore ignores this directory.
- If you make changes to cmake scripts, you must copy the changes into \tools\gitflow directory files.

3. dropbox
- Contains scripts that are used with SmartMet's Dropbox distribution system.
- Scripts are used to generate Windows shortcuts and Dropbox-safe launcher-batches.
- These scripts are used from the dropbox-path\SmartMet Dropbox directory and are copied here for versioning purposes.
- This directory is not ignored by .gitignore so all changes are stored in files here.
- Only usable script at the moment (after Dropbox made massive changes in it's directory structures) is smartmet_create_bat_launchers.ps1.
- smartmet_create_bat_launchers creates dos batch scripts that use relative paths and should be immune if dropbox should change directory structures in the futere again (it's not the first time that happened).
- All other scripts which create Windows shortcuts are now obsolite.

4. autotesting directory
- Contain scripts that are used with automatic testing environment for comparing images with imagemagick tools.
- These scripts are used from the automation-test directory and are copied here for versioning purposes.
- This directory is not ignored by .gitignore so all changes are stored in files here.
