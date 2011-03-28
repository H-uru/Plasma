""" *==LICENSE==*

CyanWorlds.com Engine - MMOG client, server and tools
Copyright (C) 2011  Cyan Worlds, Inc.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

 *==LICENSE==* """
"""Module: kdshPillarRoom.py
Age: Kadish Tolesa
Author: Doug McBride
Date: May 2003
Operates the Vault puzzle.
"""

from Plasma import *
from PlasmaTypes import *
import PlasmaControlKeys
import string

# VCP = Vault Control Panel

# define the attributes that will be entered in 3dsMAX
actButton1 = ptAttribActivator(1, "Act: Button 01") # Buttons are numbered from left to right in the Max file
actButton2 = ptAttribActivator(2, "Act: Button 02") 
actButton3 = ptAttribActivator(3, "Act: Button 03") 
actButton4 = ptAttribActivator(4, "Act: Button 04") 
actButton5 = ptAttribActivator(5, "Act: Button 05") 
actButton6 = ptAttribActivator(6, "Act: Button 06") 

respButton1 = ptAttribResponder(7, "Resp: Button 01 Down")
respButton2 = ptAttribResponder(8, "Resp: Button 02 Down")
respButton3 = ptAttribResponder(9, "Resp: Button 03 Down")
respButton4 = ptAttribResponder(10, "Resp: Button 04 Down")
respButton5 = ptAttribResponder(11, "Resp: Button 05 Down")
respButton6 = ptAttribResponder(12, "Resp: Button 06 Down")

Activate = ptAttribActivator(13, "Act: VCP Clickable") 
VCPCamera = ptAttribSceneobject(14,"VCP camera")
Behavior = ptAttribBehavior(15, "VCP idle behavior")
RaiseVCPClickable = ptAttribResponder(16, "Raise VCP Clickable")
LowerVCPClickable = ptAttribResponder(17, "Lower VCP Clickable")

RgnDisengage = ptAttribActivator(18, "Act: Disengage Rgn")
VaultRoomCamera = ptAttribSceneobject(19,"Release camera")
respResetButtons = ptAttribResponder(20, "Reset All Buttons")
respOpenVault = ptAttribResponder(21, "Open Vault Door")
respCloseVault = ptAttribResponder(22, "Close Vault Door")

actResetBtn = ptAttribActivator(23, "act:Reset Button")
respResetBtn = ptAttribResponder(24, "resp:Reset Button")

OnlyOneOwner = ptAttribSceneobject(25,"OnlyOneOwner") #ensures that after a oneshots, only one client toggles the SDL values

# globals
LocalAvatar = None

VCPboolOperated = false
VCPVCPOperatorID = -1

ButtonsPushed=0
VaultClosed=1
VaultDoorMoving=0




class kdshVault(ptResponder):

    def __init__(self):
        # run parent class init
        ptResponder.__init__(self)
        self.id = 5234
        
        version = 6
        self.version = version
        print "__init__kdshVault v. ", version,".2"

    def OnServerInitComplete(self):
        global ButtonsPushed
        ageSDL = PtGetAgeSDL()
        
        ageSDL.sendToClients("ButtonsPushed")
        ageSDL.sendToClients("VaultClosed")
        ageSDL.sendToClients("VCPboolOperated")
        ageSDL.sendToClients("VCPVCPOperatorID")

        ageSDL.setFlags("ButtonsPushed",1,1)
        ageSDL.setFlags("VaultClosed",1,1)
        ageSDL.setFlags("VCPboolOperated",1,1)
        ageSDL.setFlags("VCPVCPOperatorID",1,1)

        # register for notification of age SDL var changes
        ageSDL.setNotify(self.key,"ButtonsPushed",0.0)
        ageSDL.setNotify(self.key,"VaultClosed",0.0)
        
        ButtonsPushed = ageSDL["ButtonsPushed"][0]

        print "kdshVault: When I got here:"
        print "\t ButtonsPushed = ", ButtonsPushed
        
        ButtonsPushed = str(ButtonsPushed)
        
        if len(ButtonsPushed) >= 6:
            print "All 6 buttons were already pushed. Resetting."
            respResetButtons.run(self.key)
            ageSDL["ButtonsPushed"] = (0,)
            
            return
        
        if "1" in ButtonsPushed:
            print "fast forwarding button 1"
            respButton1.run(self.key, fastforward=1)
            actButton1.disable()
        if "2" in ButtonsPushed:
            print "fast forwarding button 2"
            respButton2.run(self.key, fastforward=1)
            actButton2.disable()          
        if "3" in ButtonsPushed:
            print "fast forwarding button 3"
            respButton3.run(self.key, fastforward=1)
            actButton3.disable()
        if "4" in ButtonsPushed:
            print "fast forwarding button 4"
            respButton4.run(self.key, fastforward=1)
            actButton4.disable()            
        if "5" in ButtonsPushed:
            print "fast forwarding button 5"
            respButton5.run(self.key, fastforward=1)
            actButton5.disable()            
        if "6" in ButtonsPushed:
            print "fast forwarding button 6"
            respButton6.run(self.key, fastforward=1)
            actButton6.disable()
        if "0" in ButtonsPushed:
            print "No buttons have been pushed."
            #~ string.join(string.split(ButtonsPushed, "0"), "")
            ageSDL["ButtonsPushed"] = (0,)
            


    def Load(self):
        global VCPboolOperated
        global ButtonsPushed
        ageSDL = PtGetAgeSDL()     
        
        solo = true
        if len(PtGetPlayerList()):
            solo = false

        VCPboolOperated = ageSDL["VCPboolOperated"][0]
        if VCPboolOperated:
            if solo:
                print "kdshVault.Load():\tVCPboolOperated=%d but no one else here...correcting" % VCPboolOperated
                VCPboolOperated = 0
                ageSDL["VCPboolOperated"] = (0,)
                ageSDL["VCPOperatorID"] = (-1,)
                Activate.enable()
            else:
                Activate.disable()
                print "kdshVault.Load():\tVCPboolOperated=%d, disabling Vault Control Panel clickable" % VCPboolOperated

    def AvatarPage(self, avObj, pageIn, lastOut):
        "reset scope accessibility if scope user quits or crashes"
        ageSDL = PtGetAgeSDL()     
        
        global VCPboolScopeOperated
        
        if pageIn:
            return
            
        avID = PtGetClientIDFromAvatarKey(avObj.getKey())
        if avID == ageSDL["VCPOperatorID"][0]:
            Activate.enable()
            ageSDL["VCPOperatorID"] = (-1,)
            ageSDL["VCPboolOperated"] = (0,)
            LowerVCPClickable.run(self.key)
            print "kdshVault.AvatarPage(): Vault Control Panel operator paged out, reenabled VCP clickable."
        else:
            return
            


    def OnNotify(self,state,id,events):
        "Activated... start Vault Control Panel"
        global LocalAvatar
        global VCPboolOperated
        global ButtonsPushed
        global VaultDoorMoving
        
        ageSDL = PtGetAgeSDL()             
        
        #~ print "kdshVault:OnNotify  state=%f id=%d events=" % (state,id),events

        #Hacked this in to work with the UruLive changes
        avatar = PtFindAvatar(events)
        
        if state and id == Activate.id and avatar == PtGetLocalAvatar():
            print "kdshVault: I'm engaging VCP."

            # Disable First Person Camera
            cam = ptCamera()
            cam.undoFirstPerson()
            cam.disableFirstPersonOverride()

            LocalAvatar = avatar
            Behavior.run(LocalAvatar)
            RaiseVCPClickable.run(self.key)
            
            Activate.disable()
            
            ageSDL = PtGetAgeSDL()
            ageSDL["VCPboolOperated"] = (1,)
            avID = PtGetClientIDFromAvatarKey(LocalAvatar.getKey())
            ageSDL["VCPOperatorID"] = (avID,)


        elif id == Behavior.id and avatar == PtGetLocalAvatar(): # Smart seek done
            #~ print "Done with smart seek"
            LocalAvatar = avatar
            Behavior.gotoStage(LocalAvatar, -1)
            
            # Disable forward movement
            PtDisableForwardMovement()
            
            # set camera to Shelf Camera
            virtCam = ptCamera()
            virtCam.save(VCPCamera.sceneobject.getKey())            

            PtGetControlEvents(true,self.key)
            #~ PtDisableMovementKeys()

            #~ PtFadeLocalAvatar(1)
            # Wait .1 seconds before fading avatar out
            PtAtTimeCallback(self.key,.2,2) 


        elif state and id in [1,2,3,4,5,6] and avatar == PtGetLocalAvatar():
            if VaultDoorMoving:
                print "Button has no effect. The Vault Door is already moving."
                return            
            
            print "\tkdshVault.OnNotify: Button #%d pushed" % (id)
            

            #append the pushed button to the list of those already clicked            
            ButtonsPushed = ageSDL["ButtonsPushed"][0]
            ButtonsPushed = str(ButtonsPushed)
            print "kdshVault.OnNotify: Before, ButtonsPushed was ", ButtonsPushed

            
            ButtonsPushed = string.atoi(ButtonsPushed + (str(id)))
            print "kdshVault.OnNotify: Now, ButtonsPushed = ", ButtonsPushed
            
            #update the ageSDL value for that button            
            ageSDL["ButtonsPushed"] = (ButtonsPushed,)
            
            if len(str(ButtonsPushed)) >= 6:
                PtAtTimeCallback(self.key,1,1) 
                #~ self.CheckSolution()

        elif state and id == actResetBtn.id:
            #~ print "kdshVault.OnNotify: Reset Button clicked."
            LocalAvatar = PtFindAvatar(events)
            respResetBtn.run(self.key,events=events)
            
        elif id == respResetBtn.id and OnlyOneOwner.sceneobject.isLocallyOwned():
            if VaultDoorMoving:
                print "Button has no effect. The Vault Door is already moving."
                return
                
            print "kdshVault.OnNotify: Reset Button Pushed. Toggling Vault Door state."
            
            vaultclosed = ageSDL["VaultClosed"][0]
            if vaultclosed == 1:
                #Open the door
                print "\t trying to open the Vault."
                
                ageSDL.setTagString("VaultClosed","fromOutside")                
                ageSDL["VaultClosed"] = (0,)                
                
            elif vaultclosed == 0:
                #Close the door
                print "\t trying to close the Vault."
                ageSDL.setTagString("VaultClosed","fromInside")                
                ageSDL["VaultClosed"] = (1,)
            
            #Make sure nobody can fool with the vault door until it stops moving 18 seconds later.
            VaultDoorMoving=1
            PtAtTimeCallback(self.key,18,3) 


    def IDisengageVCP(self):         

        LowerVCPClickable.run(self.key)
        Activate.enable()
        
        PtFadeLocalAvatar(0)
        #reeneable first person
        cam = ptCamera()
        cam.enableFirstPersonOverride()
        # go back to the Vault Room Camera
        virtCam = ptCamera()
        virtCam.save(VaultRoomCamera.sceneobject.getKey())

        # Enable forward movement
        PtEnableForwardMovement()

        PtGetControlEvents(false,self.key)
        
    def OnSDLNotify(self,VARname,SDLname,playerID,tag):
        ageSDL = PtGetAgeSDL()

        print "kdshVault.OnSDLNotify:\tVARname=",VARname," value=",ageSDL[VARname][0]
            
        
        if VARname == "ButtonsPushed":
            
            ButtonsPushed = ageSDL["ButtonsPushed"][0]
            
            if ButtonsPushed == 0:
                return
            
            ButtonsPushed = str(ButtonsPushed)
            lastbuttonpushed = ButtonsPushed[-1:]
            print "kdshVault.OnSDLNotify: new ButtonsPushed = ", ButtonsPushed
            #~ print "kdshVault.OnSDLNotify: lastbuttonpushed = ", lastbuttonpushed
            
            #run the animation on the button itself
            code = "respButton" + str(lastbuttonpushed) + ".run(self.key)"
            #~ print "code = ", code
            exec code
        
            #disable the clickable for that button
            code = "actButton" + str(lastbuttonpushed) + ".disable()"
            #~ print "code = ", code
            exec code
        
    def OnTimer(self,id):
        global VaultDoorMoving

        ageSDL = PtGetAgeSDL()     
        if id==1:
            ButtonsPushed = ageSDL["ButtonsPushed"][0]
            print "kdshVault: Check solution. ButtonsPushed = ", ButtonsPushed
            if ButtonsPushed == 152346:
                print "kdshVault: Puzzle solved. Opening door."

                ageSDL.setTagString("VaultClosed","fromOutside")                
                ageSDL["VaultClosed"] = (0,)

                #Make sure nobody can fool with the vault door until it stops moving 18 seconds later.
                VaultDoorMoving=1
                PtAtTimeCallback(self.key,18,3) 
                
            respResetButtons.run(self.key)
            ageSDL["ButtonsPushed"] = (0,)

        elif id==2:
            PtFadeLocalAvatar(1)
        
        elif id == 3:
            print "kdshVault: The Vault door has stopped moving."
            VaultDoorMoving=0

            
    def OnControlKeyEvent(self,controlKey,activeFlag):
        if controlKey == PlasmaControlKeys.kKeyExitMode:
            self.IDisengageVCP()            
        elif controlKey == PlasmaControlKeys.kKeyMoveBackward or controlKey == PlasmaControlKeys.kKeyRotateLeft or controlKey == PlasmaControlKeys.kKeyRotateRight:
            self.IDisengageVCP()            

