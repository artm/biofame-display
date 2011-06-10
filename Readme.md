# 15 minutes of Biometric Fame display

[ ] init:
    [ ] prepare face template database in memory
    [ ] prepare a list of image files to go with the face template database

[ ] watch incoming directory for new face
[ ] when got - place it in the middle of the screen
[ ] start matching thread, in the main thread - show random images from the database scroll in the smalls
[ ] match found: make sure it's the last of the small images


[ ] when looking for a match the random images appear in the second portrait.
[ ] when match is found thumbs line is filled with images form the corresponding slot: 0, [matched, [others ...]]
    (matched is optional because it could be 0 - don't duplicate then)

[ ] delay...
[ ] current image slides into the line after the matched
[ ] current image is added to the database under the modified slot name

