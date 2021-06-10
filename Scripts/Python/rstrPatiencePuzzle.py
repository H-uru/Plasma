# Source generated with PlasmaShop r298-26-g3372862
# Powered by Decompyle++
# File: rstrPatiencePuzzle.py (Python 2.2)

from Plasma import *
from PlasmaTypes import *
import PlasmaControlKeys
import string
actSwitchA = ptAttribActivator(1, 'Act: Switch A')
actButtonB = ptAttribActivator(2, 'Act: Button B')
actButtonC = ptAttribActivator(3, 'Act: Button C')
actButtonD = ptAttribActivator(4, 'Act: Button D')
rgnPatienceZone = ptAttribActivator(5, 'Act: Patience Zone')
respTickSfxOn = ptAttribResponder(6, 'resp: Tick Sfx On')
respTickSfxOff = ptAttribResponder(7, 'resp: Tick Sfx Off')
respGizmoButtonOneshot = ptAttribResponder(8, 'resp: GizmoButton Oneshot')
respPlungeBall = ptAttribResponder(9, 'resp: Plunge Ball')
respRevealLadder = ptAttribResponder(10, 'resp: Reveal Ladder')
respConcealLadder = ptAttribResponder(11, 'resp: Conceal Ladder')
respExtendBridge = ptAttribResponder(12, 'resp: Extend Bridge')
respRetractBridge = ptAttribResponder(13, 'resp: Retract Bridge')
respButtonDEnable = ptAttribResponder(14, 'resp: Button D Enable')
respButtonDDisable = ptAttribResponder(15, 'resp: Button D Disable')
respDoorClose = ptAttribResponder(16, 'resp: Close Door')
respDoorOpen = ptAttribResponder(17, 'resp: Open Door')
SwitchAUpOneshot = ptAttribResponder(18, 'resp: SwitchAUp Oneshot')
SwitchADownOneshot = ptAttribResponder(19, 'resp: SwitchADown Oneshot')
respSwitchAUp = ptAttribResponder(20, 'resp: SwitchAUp Results')
respSwitchADown = ptAttribResponder(21, 'resp: SwitchADown Results')
respDoorButtonOneshot = ptAttribResponder(22, 'resp: Door Button Oneshot')
respEnableDoorButton = ptAttribResponder(23, 'resp: Enable Door Button')
respDisableDoorButton = ptAttribResponder(24, 'resp: Disable Door Button')
rgnLadderZone = ptAttribActivator(25, 'Act: Ladder Zone')
rgnBallZone = ptAttribActivator(26, 'Act: Ball Zone')
respOpenHerringDoor = ptAttribResponder(27, 'resp: Open Red Herring Door')
respCloseHerringDoor = ptAttribResponder(28, 'resp: Close Red Herring Door')
rgnHerringDoor = ptAttribActivator(29, 'Act: Herring Door')
respLockSound = ptAttribResponder(30,'resp: Door Button Lock Sound')
boolLadderRevealed = 0
boolInPatienceZone = 0
kPatienceTime = 870
kTreeLightTime = 4380
boolStillInPatienceZone = 0
intAvatarsInPatienceZone = 0
intPatienceIntervalCount = 0
intAvatarsInLadderZone = 0

class rstrPatiencePuzzle(ptResponder):
    
    def __init__(self):
        ptResponder.__init__(self)
        self.id = 5247
        version = 4
        self.version = version
        print('__init__rstrPatiencePuzzle v. ', version, '.0')

    
    def OnServerInitComplete(self):
        ageSDL = PtGetAgeSDL()
        ageSDL.sendToClients('boolSwitchAUp')
        ageSDL.sendToClients('boolLadderRevealed')
        ageSDL.sendToClients('boolBridgeExtended')
        ageSDL.sendToClients('boolFirstTimeHere')
        ageSDL.sendToClients('boolBallAtTop')
        ageSDL.setFlags('boolSwitchAUp', 1, 1)
        ageSDL.setFlags('boolLadderRevealed', 1, 1)
        ageSDL.setFlags('boolBridgeExtended', 1, 1)
        ageSDL.setFlags('boolFirstTimeHere', 1, 1)
        ageSDL.setFlags('boolBallAtTop', 1, 1)
        ageSDL.setNotify(self.key, 'boolSwitchAUp', 0.0)
        ageSDL.setNotify(self.key, 'boolLadderRevealed', 0.0)
        ageSDL.setNotify(self.key, 'boolBridgeExtended', 0.0)
        ageSDL.setNotify(self.key, 'boolFirstTimeHere', 0.0)
        ageSDL.setNotify(self.key, 'boolBallAtTop', 0.0)
        if not PtGetPlayerList():
            print('\tResetting puzzle completely, no other players around')
            ageSDL['boolSwitchAUp'] = (1,)
            ageSDL['boolFirstTimeHere'] = (1,)
            
        boolSwitchAUp = ageSDL['boolSwitchAUp'][0]
        
        print('rstrPatiencePuzzle: When I got here:')
        if boolSwitchAUp:
            print('\tSwitchA is up, so the Door should be DOWN.')
            respSwitchAUp.run(self.key, fastforward = 1)
            respDoorClose.run(self.key, fastforward = 1)
            respButtonDEnable.run(self.key, fastforward = 1)
            actButtonD.enable()
            TimeEnteredPatienceZone = ageSDL['TimeEnteredPatienceZone'][0]
            CurrentTime = PtGetDniTime()
            PtClearTimerCallbacks(self.key)
            if ageSDL['boolFirstTimeHere'][0] == 1:
                print("\tYou've never opened the ball room door, so we don't care about the timer.")
            elif ageSDL['boolLadderRevealed'][0] == 1:
                print("\tThe ladder WAS revealed when we got here, so we don't care about the timer.")
            elif CurrentTime - TimeEnteredPatienceZone < kPatienceTime:
                print('\tThe Patience Timer will expire in %d seconds.' % (kPatienceTime - CurrentTime + TimeEnteredPatienceZone))
                PtAtTimeCallback(self.key, kPatienceTime - CurrentTime + TimeEnteredPatienceZone, 1)
                respTickSfxOn.run(self.key)
            else:
                print('\tThe Patience Timer expired in the time nobody was here.')
                ageSDL['boolSwitchAUp'] = (0,)
                ageSDL['boolLadderRevealed'] = (0,)
        else:
            print('\tSwitchA is down, so the Door should be UP.')
            respButtonDDisable.run(self.key, fastforward = 1)
            respSwitchADown.run(self.key, fastforward = 1)
            respDoorOpen.run(self.key, fastforward = 1)
            actButtonD.disable()
        if ageSDL['boolLadderRevealed'][0] == 1:
            if not boolSwitchAUp:
                print("\tERROR: The ladder shouldn't be revealed if the door is open. I'll conceal it now.")
                ageSDL['boolLadderRevealed'] = (0,)
            else:
                print('\tThe Ladder is revealed.')
                respRevealLadder.run(self.key, fastforward = 1)
        else:
            print('\tThe ladder is not revealed.')
        if ageSDL['boolBridgeExtended'][0] == 1:
            print('\tThe Bridge was already extended.')
            respExtendBridge.run(self.key, fastforward = 1)
        elif ageSDL['boolBridgeExtended'][0] == 0:
            print('\tThe Bridge was not extended.')
        if ageSDL['boolBallAtTop'][0] == 1:
            print('\tOpening Red Herring Door')
            respOpenHerringDoor.run(self.key, fastforward = 1)
        if ageSDL['TreeDayLights'][0] + kTreeLightTime < CurrentTime:
            print('\tBecause 1 Dni hour has passed, turning off day.')
            ageSDL['TreeDayLights'] = (0,)
        else:
            print('\tBecause 1 Dni hour has not passed, Tree timer set to turn off in', ageSDL['TreeDayLights'][0] + kTreeLightTime - CurrentTime, 'seconds.')
            PtAtTimeCallback(self.key, ageSDL['TreeDayLights'][0] + kTreeLightTime - CurrentTime, 5)

    
    def OnNotify(self, state, id, events):
        global boolInPatienceZone, boolStillInPatienceZone, intAvatarsInPatienceZone, intAvatarsInLadderZone
        ageSDL = PtGetAgeSDL()
        if not state:
            return None
        
        if id == rgnPatienceZone.id and not boolLadderRevealed:
            for event in events:
                if event[1] == 0:
                    print('An avatar just walked out of the Patience Zone')
                    boolInPatienceZone = 0
                    intAvatarsInPatienceZone -= 1
                    if intAvatarsInPatienceZone <= 0:
                        boolStillInPatienceZone = 0
                    break
                elif event[1] == 1:
                    print('An avatar just walked into the Patience Zone')
                    boolInPatienceZone = 1
                    intAvatarsInPatienceZone += 1
                    break
                
            return None
        elif id == rgnLadderZone.id:
            for event in events:
                if event[1] == 0:
                    print('An avatar just walked out of the Ladder Zone')
                    intAvatarsInLadderZone -= 1
                    break
                elif event[1] == 1:
                    print('An avatar just walked into the Ladder Zone')
                    intAvatarsInLadderZone += 1
                    break
                
            return None
        elif id == rgnBallZone.id:
            for event in events:
                if event[1] == 0:
                    print('Ball just rolled out of the Ball Zone')
                    ageSDL['boolBallAtTop'] = (0,)
                    break
                elif event[1] == 1:
                    print('Ball just rolled into the Ball Zone')
                    ageSDL['boolBallAtTop'] = (1,)
                    break
                
            return None
        elif id == rgnHerringDoor.id:
            for event in events:
                if event[1] == 0:
                    print('Herring Door Exit Region')
                    if not ageSDL['boolBallAtTop'][0]:
                        respCloseHerringDoor.run(self.key)
                    break
                elif event[1] == 1:
                    print('Herring Door Enter Region')
                    if not ageSDL['boolBallAtTop'][0]:
                        respOpenHerringDoor.run(self.key)
                    break
                
            return None
        elif id == actSwitchA.id:
            boolSwitchAUp = ageSDL['boolSwitchAUp'][0]
            if boolSwitchAUp:
                SwitchADownOneshot.run(self.key, events = events)
            else:
                SwitchAUpOneshot.run(self.key, events = events)
        elif id == SwitchADownOneshot.id:
            print('Avatar finished pushing LeverA DOWN.')
            respSwitchADown.run(self.key)
            ageSDL.setTagString('boolSwitchAUp', 'foo')
            ageSDL['boolSwitchAUp'] = (0,)
        elif id == SwitchAUpOneshot.id:
            print('Avatar finished pushing LeverA UP.')
            boolStillInPatienceZone = 1
            respSwitchAUp.run(self.key)
            ageSDL.setTagString('boolSwitchAUp', 'foo')
            ageSDL['boolSwitchAUp'] = (1,)
        elif id == actButtonB.id:
            print('The Gizmo button was just clicked.')
            respGizmoButtonOneshot.run(self.key, events = events)
        elif id == respGizmoButtonOneshot.id:
            print('The Avatar just finished pushing the Gizmo button.')
            ageSDL['boolBridgeExtended'] = (1,)
            self.RecordTreeLightTime()
            return None
        elif id == actButtonC.id:
            print('The Button inside the great tree has just been manipulated.')
            return None
        elif id == actButtonD.id:
            TimeEnteredPatienceZone = ageSDL['TimeEnteredPatienceZone'][0]
            CurrentTime = PtGetDniTime()
            ElapsedTime = CurrentTime - TimeEnteredPatienceZone
            if ElapsedTime >= kPatienceTime:
                respDoorButtonOneshot.run(self.key, events = events)
            else:
                print('Button D Pressed,', kPatienceTime - ElapsedTime, 'seconds to go')
                respLockSound.run(self.key, events = events)
                actButtonD.enable()
        elif id == respDoorButtonOneshot.id:
            print('D touched by avatar. Waiting 10 seconds to open the door.')
            PtAtTimeCallback(self.key, 10, 2)
            if ageSDL['boolFirstTimeHere'][0] == 1:
                print("\tThat was the first time you've ever opened the ball room door.")
                ageSDL['boolFirstTimeHere'] = (0,)
            
            return None

    def RecordZoneEnterTime(self):
        ageSDL = PtGetAgeSDL()
        CurrentTime = PtGetDniTime()
        ageSDL['TimeEnteredPatienceZone'] = (CurrentTime,)
        
    def RecordTreeLightTime(self):
        ageSDL = PtGetAgeSDL()
        CurrentTime = PtGetDniTime()
        ageSDL['TreeDayLights'] = (CurrentTime,)
        PtAtTimeCallback(self.key, kTreeLightTime, 5)
    
    def OnTimer(self, id):
        global intAvatarsInPatienceZone, intPatienceIntervalCount, intAvatarsInLadderZone
        ageSDL = PtGetAgeSDL()
        if id == 1:
            intPatienceIntervalCount += 1
            TimeEnteredPatienceZone = ageSDL['TimeEnteredPatienceZone'][0]
            CurrentTime = PtGetDniTime()
            ElapsedTime = CurrentTime - TimeEnteredPatienceZone
            if intAvatarsInPatienceZone <= 1:
                NumberofPlayers = 1
            else:
                NumberofPlayers = intAvatarsInPatienceZone
                
            print('rstrPatiencePuzzle: You started the Patience Timer', ElapsedTime, 'seconds ago.')
            print('rstrPatiencePuzzle: You have ', kPatienceTime / NumberofPlayers - ElapsedTime, ' to go')
            print('boolStillInPatienceZone = ', boolStillInPatienceZone)
            print('NumberofPlayers = ', NumberofPlayers)
            if not boolStillInPatienceZone and ElapsedTime >= kPatienceTime:
                print('\tBut you are not currently in the zone.')
                ageSDL['boolSwitchAUp'] = (0,)
                intPatienceIntervalCount = 0
                return None
            elif ElapsedTime >= kPatienceTime / NumberofPlayers and boolStillInPatienceZone:
                print('\tAnd you HAVE been in the zone for', kPatienceTime / NumberofPlayers, 'consecutive seconds. Puzzle solved.')
                print('\tReveal Ladder, Door stays closed, Lights stay on, Switch A stays up, ticking stops, plunge ball')
                ageSDL['boolLadderRevealed'] = (1,)
                PtAtTimeCallback(self.key, kPatienceTime / NumberofPlayers, 3)
                respTickSfxOff.run(self.key)
                respPlungeBall.run(self.key)
                intPatienceIntervalCount = 0
            else:
                if kPatienceTime / NumberofPlayers - ElapsedTime < 10:
                    PtAtTimeCallback(self.key, kPatienceTime / NumberofPlayers - ElapsedTime, 1)
                else:
                    PtAtTimeCallback(self.key, 10, 1)
        elif id == 2:
            ageSDL.setTagString('boolSwitchAUp', 'ButtonDPushed')
            ageSDL['boolSwitchAUp'] = (0,)
            intPatienceIntervalCount = 0
        elif id == 3:
            ageSDL['boolLadderRevealed'] = (0,)
        elif id == 4:
            if intAvatarsInLadderZone <= 0:
                print('\tConcealing ladder.')
                respConcealLadder.run(self.key)
            else:
                PtAtTimeCallback(self.key, 1, 4)
        elif id == 5:
            print('\tBecause 1 Dni hour has passed, turning off day.')
            ageSDL['TreeDayLights'] = (0,)
        

    
    def OnSDLNotify(self, VARname, SDLname, playerID, tag):
        global intAvatarsInLadderZone
        ageSDL = PtGetAgeSDL()
        CurrentTime = PtGetDniTime()
        print('OnSDLNotify: VARname =', VARname, ' value =', ageSDL[VARname][0], ' tag =', tag)
        if VARname == 'boolSwitchAUp':
            boolSwitchAUp = ageSDL['boolSwitchAUp'][0]
            if ageSDL['boolFirstTimeHere'][0]:
                return
            if boolSwitchAUp:
                print('\tTimer Starts, Tick Sfx starts, Door closes, lights on, plunge ball, Button D enabled')
                boolInPatienceZone = 1
                PtAtTimeCallback(self.key, 10, 1)
                self.RecordZoneEnterTime()
                respTickSfxOn.run(self.key)
                respDoorClose.run(self.key)
                respPlungeBall.run(self.key)
                respButtonDEnable.run(self.key, fastforward = 1)
                actButtonD.enable()
            else:
                print('\tTimer Stops, Tick Sfx stops, Door opens, lights off, Switch A down')
                PtClearTimerCallbacks(self.key)
                respTickSfxOff.run(self.key)
                respDoorOpen.run(self.key)
                respSwitchADown.run(self.key)
                boolLadderRevealed = ageSDL['boolLadderRevealed'][0]
                if ageSDL['boolLadderRevealed'][0]:
                    print('\tAlso putting away ladder.')
                    ageSDL['boolLadderRevealed'] = (0,)
                
                if tag == 'ButtonDPushed':
                    TreeDayLights = ageSDL['TreeDayLights'][0]
                    if TreeDayLights + kTreeLightTime < CurrentTime:
                        print('\tBecause D was pushed, turning off day.')
                        ageSDL['TreeDayLights'] = (0,)
                    
                    boolBridgeExtended = ageSDL['boolBridgeExtended'][0]
                    if boolBridgeExtended:
                        print('\tBecause D was pushed, retracting bridge.')
                        ageSDL['boolBridgeExtended'] = (0,)
                    
                
        elif VARname == 'boolBridgeExtended':
            if ageSDL['boolBridgeExtended'][0] == 1:
                print('\tExtending bridge')
                respExtendBridge.run(self.key)
            elif ageSDL['boolBridgeExtended'][0] == 0:
                respRetractBridge.run(self.key)
                print('\tRetracting bridge')
            
        elif VARname == 'boolLadderRevealed':
            if ageSDL['boolLadderRevealed'][0] == 1:
                print('\tRevealing ladder.')
                respRevealLadder.run(self.key)
            else:
                if intAvatarsInLadderZone <= 0:
                    print('\tConcealing ladder.')
                    respConcealLadder.run(self.key)
                else:
                    PtAtTimeCallback(self.key, 1, 4)
        elif VARname == 'boolBallAtTop':
            if ageSDL['boolBallAtTop'][0] == 1:
                print('\tOpening Red Herring Door')
                respOpenHerringDoor.run(self.key)
            else:
                print('\tClosing Red Herring Door')
                respCloseHerringDoor.run(self.key)
                

    
    def AvatarPage(self, avObj, pageIn, lastOut):
        if not pageIn:
            print('rstrPatiencePuzzle: Turning off Patience Timer.')
            PtClearTimerCallbacks(self.key)