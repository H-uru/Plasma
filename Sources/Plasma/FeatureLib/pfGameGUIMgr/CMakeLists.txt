include_directories(../../CoreLib)
include_directories(../../FeatureLib)
include_directories(../../NucleusLib)
include_directories(../../NucleusLib/inc)
include_directories(../../PubUtilLib)

set(pfGameGUIMgr_SOURCES
    pfGameGUIMgr.cpp
    pfGUIButtonMod.cpp
    pfGUICheckBoxCtrl.cpp
    pfGUIClickMapCtrl.cpp
    pfGUIControlHandlers.cpp
    pfGUIControlMod.cpp
    pfGUICtrlGenerator.cpp
    pfGUIDialogMod.cpp
    pfGUIDialogNotifyProc.cpp
    pfGUIDragBarCtrl.cpp
    pfGUIDraggableMod.cpp
    pfGUIDynDisplayCtrl.cpp
    pfGUIEditBoxMod.cpp
    pfGUIKnobCtrl.cpp
    pfGUIListBoxMod.cpp
    pfGUIListElement.cpp
    pfGUIMenuItem.cpp
    pfGUIMultiLineEditCtrl.cpp
    pfGUIPopUpMenu.cpp
    pfGUIProgressCtrl.cpp
    pfGUIRadioGroupCtrl.cpp
    pfGUITagDefs.cpp
    pfGUITextBoxMod.cpp
    pfGUIUpDownPairMod.cpp
    pfGUIValueCtrl.cpp
)

set(pfGameGUIMgr_HEADERS
    pfGameGUIMgr.h
    pfGameGUIMgrCreatable.h
    pfGUIButtonMod.h
    pfGUICheckBoxCtrl.h
    pfGUIClickMapCtrl.h
    pfGUIControlHandlers.h
    pfGUIControlMod.h
    pfGUICtrlGenerator.h
    pfGUIDialogHandlers.h
    pfGUIDialogMod.h
    pfGUIDialogNotifyProc.h
    pfGUIDragBarCtrl.h
    pfGUIDraggableMod.h
    pfGUIDynDisplayCtrl.h
    pfGUIEditBoxMod.h
    pfGUIKnobCtrl.h
    pfGUIListBoxMod.h
    pfGUIListElement.h
    pfGUIMenuItem.h
    pfGUIMultiLineEditCtrl.h
    pfGUIPopUpMenu.h
    pfGUIProgressCtrl.h
    pfGUIRadioGroupCtrl.h
    pfGUITagDefs.h
    pfGUITextBoxMod.h
    pfGUIUpDownPairMod.h
    pfGUIValueCtrl.h
)

add_library(pfGameGUIMgr STATIC ${pfGameGUIMgr_SOURCES} ${pfGameGUIMgr_HEADERS})

source_group("Source Files" FILES ${pfGameGUIMgr_SOURCES})
source_group("Header Files" FILES ${pfGameGUIMgr_HEADERS})
