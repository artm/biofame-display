# 15 minutes of Biometric Fame display

[ ] init:
    [ ] prepare face template database in memory
    [ ] prepare a list of image files to go with the face template database

[ ] database self update:
    [ ] maintain cropped faces directory...

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

## Short term...

[ ] start robust verilook wrapper: an object that can:
    2. make a match template
    3. run match loop and report best match with a signal

[ ] If match found - move the file into the corresponding slot and save its matching template.

should go differently:

1. watcher sends a signal to the biometric thread (BT).
2. the BT detects face and crops, posts the cropped image to the main thread.
3. the BT then continues with doing things to the picture.

