# 15 minutes of Biometric Fame display

[x] init:
    [x] prepare face template database in memory
    [x] prepare a list of image files to go with the face template database

[x] watch incoming directory for new face
[x] when got - place it in the middle of the screen
[x] biometric thread: start matching, display thread - show random images from the database scroll in the smalls

[x] better match found handler:
    [x] add the image to database (in memory and on disk)
    [x] also add it to the slots of the display
    [x] display "neighbours" (including the new one)
    [x] show decoded slot name
    [x] delete the original incoming picture

[x] document the rationale behind FaceTemplatePtr
    to keep multiple collections of pointers to face templates and delete templates when all pointers are removed
    - at the moment this is broken and FaceTemplate::Ptr is defined as FacePointer*

[x] naming scheme
    [x] the new images should be named as:
        slot_parent-id_gen_my-id
    [x] originals are slot_id and gen is assumed 0
    [x] id's are only unique within a slot
    [x] Verilook::saveToSlot() should accept parent template and derive gen from it (as well as access its slot name)
        May be even template should have its "derive child" or something API

[x] display
    [x] should show the 'inheritance' chain ('ancestry')
    [x] for that to work 'matchFound' signal should wrap and send the chain
    [ ] this means display doesn't have to know about slots after all
        - should receive slot name via signal as well

[ ] matching order
    [ ] first - new images in reverse chronological order
    [ ] next - originals in no particular order

[ ] correct filename pattern:
    tag-lang-date-number
    each component may contain underscores except the last one
    [ ] on display convert - AMD _ to spaces

[ ] progress report when loading database...
[ ] if can't make template for DB image - rename it to xxx.jpg.broken
[ ] check if other image types are present
