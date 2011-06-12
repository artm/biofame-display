# 15 minutes of Biometric Fame display

[x] init:
    [x] prepare face template database in memory
    [x] prepare a list of image files to go with the face template database

[x] watch incoming directory for new face
[x] when got - place it in the middle of the screen
[x] biometric thread: start matching, display thread - show random images from the database scroll in the smalls

[ ] better match found handler:
    [x] add the image to database (in memory and on disk)
    [ ] also add it to the slots of the display
    [ ] display "neighbours" (including the new one)
    [x] show decoded slot name
    [x] delete the original incoming picture

[ ] refactoring - separate db shared between other objects
