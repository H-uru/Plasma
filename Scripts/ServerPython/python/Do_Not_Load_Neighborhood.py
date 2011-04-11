from PlasmaServer import *

heekPlayerList = []
curtime = 0
lastupdate = 0
countdownStarted = 0
winners = []
pot = 0

agePlayers = [] # a list of all people in the age, tuple where 0 is ID and 1 is Key
queueCleanup = 0 # do we need to send a cleanup message to the first person to link in?

kNone = 0
kRock = 1
kPaper = 2
kScissors = 3

class heekPlayer:
    def __init__(self):
        self.ID = -1
        self.key = None
        self.pos = -1
        self.choice = kNone
        self.rocksWon = 0
        self.papersWon = 0
        self.scissorsWon = 0
        self.roundsWon = 0
        self.curRoundScore = 0
        self.points = -1
        self.isPlaying = 0
        self.name = ""
        self.scoreDict = { (kRock, kRock): 0,
                            (kRock, kPaper): -1,
                            (kRock, kScissors): 1,
                            (kRock, kNone): 0,
                            
                            (kPaper, kRock): 1,
                            (kPaper, kPaper): 0,
                            (kPaper, kScissors): -1,
                            (kPaper, kNone): 0,
                            
                            (kScissors, kRock): -1,
                            (kScissors, kPaper): 1,
                            (kScissors, kScissors): 0,
                            (kScissors, kNone): 0,
                            
                            (kNone, kNone): 0,
                            (kNone, kRock): 0,
                            (kNone, kPaper): 0,
                            (kNone, kScissors): 0
                            }
    
    def ScoreAgainst(self,other):
        return self.scoreDict[(self.choice,other.choice)]
    
    def ResetGame(self):
        self.choice = kNone
        self.rocksWon = 0
        self.papersWon = 0
        self.scissorsWon = 0
        self.curRoundScore = 0
        self.isPlaying = 0

    def ResetRound(self):
        self.curRoundScore = 0
        self.choice = kNone
    
    def SendMessage(self,contents):
        msg = ptPythonMsg()
        msg.setKey(self.key)
        msg.setContents(str(contents))
        msg.send(self.ID)

def SendToAll(message):
    global heekPlayerList
    for player in heekPlayerList:
        player.SendMessage(message)


def CalculateRank(points):
    if points == 0:
        return 0
    gamesWinToRank = 100 # number of 1v1 games against same rank to level
    rank = 1
    curPoints = gamesWinToRank
    while 1:
        curPoints += rank*gamesWinToRank
        if points <= curPoints:
            return rank
        rank += 1

def CalculateAnte(points):
    return CalculateRank(points)
    
def SendWelcomeMsg(name,points):
    global heekPlayerList
    SendToAll("PlrWelcome "+str(points)+' '+str(CalculateAnte(points))+' '+name)   

def SendStatusMsg(player):
    player.SendMessage("Points "+str(player.points)+' '+str(CalculateAnte(player.points)))

def RewardRoundWinners():
    global winners
    global heekPlayerList
    for winner in winners:
        winner.roundsWon += 1
        lightNum = 0
        if winner.choice == kRock:
            winner.rocksWon += 1
            lightNum = winner.rocksWon
        elif winner.choice == kPaper:
            winner.papersWon += 1
            lightNum = winner.papersWon
        elif winner.choice == kScissors:
            winner.scissorsWon += 1
            lightNum = winner.scissorsWon
        if lightNum == 0: # shouldn't happen, but just in case
            print "lightNum is 0 for some reason...len(winners)="+str(len(winners))+", winner.choice="+str(winner.choice)
            print "len(heekPlayerList)="+str(len(heekPlayerList))+", winner.ID="+str(winner.ID)+", winner.pos="+str(winner.pos)
            return
        winner.SendMessage("Win "+str(winner.choice))
        if lightNum < 3:
            light = (winner.choice-1)*2+(lightNum-1)
            winner.SendMessage("LightOn "+str(light))
        elif lightNum == 3:
            light1 = (winner.choice-1)*2
            light2 = light1+1
            winner.SendMessage("Flash "+str(light1))
            winner.SendMessage("Flash "+str(light2))
    losers = [player for player in heekPlayerList if not player in winners]
    for loser in losers:
        loser.SendMessage("Lose "+str(loser.choice))

def CompareRoundScores(player1, player2):
    if player1.curRoundScore < player2.curRoundScore:
        return -1
    if player1.curRoundScore > player2.curRoundScore:
        return 1
    return 0
    
def ComputeRoundScores():
    global heekPlayerList
    cnt = len(heekPlayerList)
    
    #if cnt == 1: # for single-player only
        #heekPlayerList[0].curRoundScore = 1
    
    for i in range(cnt-1):
        player1 = heekPlayerList[i]
        for j in range(i+1,cnt):
            player2 = heekPlayerList[j]
            player1.curRoundScore += player1.ScoreAgainst(player2)
            player2.curRoundScore += player2.ScoreAgainst(player1)

def PickRoundWinners():
    global heekPlayerList
    global winners
    rwCopy = heekPlayerList
    rwCopy.sort(CompareRoundScores)
    big=rwCopy[len(rwCopy)-1].curRoundScore
    if big>0:
        winners = [winner for winner in rwCopy if winner.curRoundScore == big]

def CompareRoundsWon(player1, player2):
    if player1.roundsWon < player2.roundsWon:
        return -1
    if player1.roundsWon > player2.roundsWon:
        return 1
    return 0

def PickGameWinners():
    global winners
    gameWinners = [winner for winner in winners if winner.rocksWon == 3 or winner.papersWon == 3 or winner.scissorsWon == 3]
    if (len(gameWinners) <= 1):
        return gameWinners
    gwCopy = gameWinners
    gwCopy.sort(CompareRoundsWon)
    big=gwCopy[len(gwCopy)-1].roundsWon
    if big>0:
        gameWinners = [winner for winner in gwCopy if winner.roundsWon==big]
    return gameWinners

def HandlePlayerReady():
    global countdownStarted
    global heekPlayerList
    if not countdownStarted:
        print "Starting countdown"
        SendToAll("StartCountdown")
        countdownStarted = 1
        return
    
    playersReady = [player for player in heekPlayerList if player.choice != kNone]
    allReady = (len(playersReady) == len(heekPlayerList))
    
    if allReady and len(playersReady)>1:
        SendToAll("StopCountdown")

def ResetGame():
    global heekPlayerList
    global winners
    winners = []
    for player in heekPlayerList:
        player.ResetGame()

def ResetRound():
    global heekPlayerList
    global winners
    winners = []
    for player in heekPlayerList:
        player.ResetRound()

def PlayGame():
    global countdownStarted
    global heekPlayerList
    global pot
    if not countdownStarted:
        print "Extra countdown end message received, ignoring"
        return # extra countdown end message, ignore
    countdownStarted = 0
    playersReady = [player for player in heekPlayerList if player.choice != kNone]
    if len(playersReady) <= 1:
        print "Only one player was ready, aborting game"
        ResetRound()
        SendToAll("ShowIdleAnimation")
        return
    print "Disabling player's interfaces"
    SendToAll("Disable")
    ComputeRoundScores()
    PickRoundWinners()
    RewardRoundWinners()

def HandleGameWinner():
    global heekPlayerList
    global pot
    gameWinners = PickGameWinners()
    if len(gameWinners) > 0:
        print "A player has won, letting admin know about it"
        x = "init"
        if (gameWinners[0].choice == kRock): # it might be possible for multiple types to win, will need to check
            x = "rock"
        elif (gameWinners[0].choice == kPaper):
            x = "paper"
        elif (gameWinners[0].choice == kScissors):
            x = "scissors"
        SendToAll("GameWin "+x)
        pointsToGive = int(pot/len(gameWinners))
        pot -= pointsToGive*len(gameWinners)
        for winner in gameWinners:
            if winner.points != -1:
                winner.SendMessage("SetPoints "+str(winner.points+pointsToGive))
                for player in heekPlayerList:
                    if player.ID == winner.ID and player.key == winner.key:
                        player.points += pointsToGive
                        break
        ResetGame()
        for player in heekPlayerList:
            SendStatusMsg(player)
        return 1
    return 0

def SendTableState(ID,key):
    global heekPlayerList
    for player in heekPlayerList:
        setupMsg = ptPythonMsg()
        setupMsg.setKey(key)
        setupStr = "SetupP"+str(player.pos)
        setupMsg.setContents(setupStr+"B") # enable buttons
        setupMsg.send(ID)
        if player.rocksWon >= 1:
            setupMsg.setContents(setupStr+"L0")
            setupMsg.send(ID)
        if player.rocksWon >= 2:
            setupMsg.setContents(setupStr+"L1")
            setupMsg.send(ID)
        if player.papersWon >= 1:
            setupMsg.setContents(setupStr+"L2")
            setupMsg.send(ID)
        if player.papersWon >= 2:
            setupMsg.setContents(setupStr+"L3")
            setupMsg.send(ID)
        if player.scissorsWon >= 1:
            setupMsg.setContents(setupStr+"L4")
            setupMsg.send(ID)
        if player.scissorsWon >= 2:
            setupMsg.setContents(setupStr+"L5")
            setupMsg.send(ID)

def onInit():
    global heekPlayerList
    global lastupdate
    global countdownStarted
    global winners
    global queueCleanup
    global pot
    print "Neighborhood.py is initializing"
    heekPlayerList = []
    winners = []
    lastupdate = 0
    countdownStarted = 0
    queueCleanup = []
    pot = 0

def onShutdown():
    print "Neighborhood.py is shutting down"

#def onUpdate(secs):
    #global curtime
    #global lastupdate
    #global heekPlayerList
    #if lastupdate == 0:
        #lastupdate = secs
    #curtime = secs
    #if curtime - lastupdate >= 1:
        # put any code here that wants to be run every second (nothing currently)
        #lastupdate += 1

def onClientLeft(ID):
    global heekPlayerList
    global agePlayers
    global queueCleanup
    position = -1
    for player in heekPlayerList[:]:
        if player.ID == ID:
            position = player.pos
            heekPlayerList.remove(player)
    for player in agePlayers[:]:
        if player[0] == ID:
            agePlayers.remove(player)
    if position == -1: # client wasn't playing
        return
    print "Client ID# "+str(ID)+" has left, cleaning up after him"
    if len(heekPlayerList) == 0: # no players left to cleanup the position
        if len(agePlayers) == 0: # no players in age left to cleanup the table
            queueCleanup.append(position)
        else:
            cleanup = ptPythonMsg()
            cleanup.setContents("Cleanup "+str(position))
            cleanup.setKey(agePlayers[0][1])
            cleanup.send(agePlayers[0][0])
            cleanup.setContents("ShowIdleAnimation")
            cleanup.send(agePlayers[0][0])
    for num in range(len(heekPlayerList)): # let everyone know of their new player numbers
        numberMsg = ptPythonMsg()
        numberMsg.setKey(heekPlayerList[num].key)
        numberMsg.setContents("Number "+str(num+1))
        numberMsg.send(heekPlayerList[num].ID)
    if position != -1:
        SendToAll("Drop "+str(position))

def onMsgReceived(msg):
    global heekPlayerList
    global countdownStarted
    global agePlayers
    global queueCleanup
    global pot
    msgContents = msg.getContents()
    senderID = msg.getSenderID()
    key = msg.getKey()
    print "Message received from client. ID#: "+str(msg.getSenderID())+" Contents: "+msg.getContents()
    if len(msgContents) > 3 and msgContents[:3] == "Add": # they want to be added to our player list
        pos = msgContents[4:]
        for player in heekPlayerList:
            if player.pos == pos:
                print "Ignoring client ID# "+str(senderID)+" request to play since someone is already sitting in their requested position: "+str(pos)
                return
        print "Adding client ID# "+str(senderID)+" to player list. They are playing position "+str(pos)
        player = heekPlayer()
        player.ID = senderID
        player.key = key
        player.pos = pos
        heekPlayerList.append(player)
        reply = ptPythonMsg()
        reply.setContents("Player "+str(len(heekPlayerList)))
        reply.setKey(key)
        reply.send(senderID)
        reply.setContents("GetPoints")
        reply.send(senderID)
        reply.setContents("GetName")
        reply.send(senderID)
    elif msgContents == "Remove": # they want to be removed from our player list
        print "Removing client ID# "+str(senderID)+" from player list."
        for player in heekPlayerList:
            if player.ID == senderID and player.key == key:
                heekPlayerList.remove(player)
                if len(heekPlayerList) == 0: # last player left
                    print "Last player is requesting a leave, telling him to clean up"
                    if countdownStarted:
                        player.SendMessage("StopCountdown")
                        countdownStarted = 0
                    player.SendMessage("ShowIdleAnimation")
                    if player.points != -1:
                        print "Giving last player the remaining points in the pot: "+str(pot)
                        player.SendMessage("SetPoints "+str(player.points+pot))
                        pot = 0
                player.SendMessage("Goodbye")
                #ResetGame()
                break
        for num in range(len(heekPlayerList)): # let everyone know of their new player numbers
            numberMsg = ptPythonMsg()
            numberMsg.setKey(heekPlayerList[num].key)
            numberMsg.setContents("Number "+str(num+1))
            numberMsg.send(heekPlayerList[num].ID)
    elif msgContents == "Rock":
        print "Player #"+str(senderID)+" has chosen rock"
        for player in heekPlayerList:
            if player.ID == senderID and player.key == key:
                if player.points != -1 and not player.isPlaying:
                    ante = CalculateAnte(player.points)
                    player.points -= ante
                    pot += ante
                    player.isPlaying = 1
                    player.SendMessage("SetPoints "+str(player.points))
                player.choice = kRock
                HandlePlayerReady()
                return
    elif msgContents == "Paper":
        print "Player #"+str(senderID)+" has chosen paper"
        for player in heekPlayerList:
            if player.ID == senderID and player.key == key:
                if player.points != -1 and not player.isPlaying:
                    ante = CalculateAnte(player.points)
                    player.points -= ante
                    pot += ante
                    player.isPlaying = 1
                    player.SendMessage("SetPoints "+str(player.points))
                player.choice = kPaper
                HandlePlayerReady()
                return
    elif msgContents == "Scissors":
        print "Player #"+str(senderID)+" has chosen scissors"
        for player in heekPlayerList:
            if player.ID == senderID and player.key == key:
                if player.points != -1 and not player.isPlaying:
                    ante = CalculateAnte(player.points)
                    player.points -= ante
                    pot += ante
                    player.isPlaying = 1
                    player.SendMessage("SetPoints "+str(player.points))
                player.choice = kScissors
                HandlePlayerReady()
                return
    elif msgContents == "CountdownFinished":
        print "Countdown has finished"
        PlayGame()
    elif msgContents == "ChoiceAnimFinished":
        print "Choice animation has finished"
        if not HandleGameWinner():
            ResetRound()
            SendToAll("ShowIdleAnimation")
            SendToAll("Enable")
    elif msgContents == "GameWinAnimFinished":
        SendToAll("ShowIdleAnimation")
        SendToAll("Enable")
    elif msgContents == "LinkIn": # client telling us it's in the age
        agePlayers.append((senderID,key))
        if len(queueCleanup) != 0:
            for item in queueCleanup:
                cleanup = ptPythonMsg()
                cleanup.setContents("Cleanup "+str(item))
                cleanup.setKey(agePlayers[0][1])
                cleanup.send(agePlayers[0][0])
            queueCleanup = []
            idle = ptPythonMsg()
            idle.setContents("ShowIdleAnimation")
            idle.setKey(agePlayers[0][1])
            idle.send(agePlayers[0][0])
        SendTableState(senderID,key)
    elif len(msgContents) > 6 and msgContents[:6] == "Points":
        points = int(msgContents[7:])
        print "Player ID# "+str(senderID)+" has "+str(points)+" points"
        for player in heekPlayerList:
            if player.ID == senderID and player.key == key:
                player.points = points
                if player.name != "":
                    SendWelcomeMsg(player.name,player.points)
                return
    elif len(msgContents) > 4 and msgContents[:4] == "Name":
        name = msgContents[5:]
        for player in heekPlayerList:
            if player.ID == senderID and player.key == key:
                player.name = name
                if player.points != -1:
                    SendWelcomeMsg(player.name,player.points)
                return
    else:
        print "Unknown message, not handling"
