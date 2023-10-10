Here are all the scripts relating automation testing system done by magick image comparisons:

1. auto_image_compare_testing.ps1 is the main powershell script that does the testing part
- Run it from Windows Powershell command line (must allow powershell scripts to be executed)
- At the end of script there are bunch of DoImageTestingForGivenDirectories function calls, comment all those directories that won't be tested at the moment 
- Much faster to execute smaller portion of tests that are relevant then all at once every time...

2. Beta-product scripts that are used to generate both reference and current(ly tested) images
- They are originally stored in %DropboxPath%\Macros\BetaProducts\marko\automationtesting directory
- They are now stored here for versioning also in betaproducts directory

3. ViewMacro scripts that are used to generate both test images with Beta-products
- They are originally stored in %DropboxPath%\Macros\FMI\ViewMacros\FMI\Marko\automationtesting directory
- They are now stored here for versioning also in viewmacros directory

