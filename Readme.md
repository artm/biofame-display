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

[ ] document the rationale behind FaceTemplatePtr

[x] naming scheme
    [x] the new images should be named as:
        slot_parent-id_gen_my-id
    [x] originals are slot_id and gen is assumed 0
    [x] id's are only unique within a slot
    [x] Verilook::saveToSlot() should accept parent template and derive gen from it (as well as access its slot name)
        May be even template should have its "derive child" or something API

[ ] display
    [ ] should show the 'inheritance' chain
    [ ] for that to work 'matchFound' signal should wrap and send the chain
    [ ] this means display doesn't have to know about slots after all

[ ] matching order
    [ ] first - new images in reverse chronological order
    [ ] next - originals in no particular order

