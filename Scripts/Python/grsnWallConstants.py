#Count Light Responder
kTeamLightsOn = 0
kTeamLightsOff = 1
kRedFlash = 2
kRedOn = 3
kRedOff = 4

#Go button Responders
kDim = 0
kBright = 1
kPulse = 2

#Blocker Responders
kBlockerOn = 0
kBlockerOff = 1
kBlockerBlink = 2

#Game States
kStandby = 0
kSit = 1
kSelectCount = 2
kSetBlocker = 3
kWait = 4
kEntry = 5
kGameInProgress = 6
kEnd = 7

#Teams
kYellow = "y"
kPurple = "p"

#Sounds
kEventInit = "HandleInit"
kEventEntry = "HandleEntry"
kEventStart = "HandleStart"
kEventYWin = "HandleYWin"
kEventPWin = "HandlePWin"
kEventYQuit = "HandleYQuit"
kEventPQuit = "HandlePQuit"

eventHandler = None
def InitEventHandler(instance):
    global eventHandler
    eventHandler = instance
