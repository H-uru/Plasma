from PlasmaServer import *

heekTables = [] # an array of heekGames for the tables we have running

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
        self.gamePlaying = None # the heekGame we are currently playing
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

def CompareRoundScores(player1, player2):
    if player1.curRoundScore < player2.curRoundScore:
        return -1
    if player1.curRoundScore > player2.curRoundScore:
        return 1
    return 0

class heekGame:
    def __init__(self):
        self.ID = -1
        self.playerList = []
        self.countdownStarted = 0
        self.winners = []
        self.pot = 0
        self.agePlayers = [] # a list of all people in the age, tuple where 0 is ID and 1 is Key
        self.queueCleanup = [] # do we need to send a cleanup message to the first person to link in?
    
    def PlayerIDInGame(self,ID):
        for player in self.playerList:
            if player.ID == ID:
                return 1
        return 0
    
    def PlayerKeyInGame(self,key):
        for player in self.playerList:
            if player.key == key:
                return 1
        return 0
    
    def SendToAll(self,message):
        for player in self.playerList:
            player.SendMessage(message)
    
    def SendWelcomeMsg(self,name,points):
        self.SendToAll("PlrWelcome "+str(points)+' '+str(self.CalculateAnte(points))+' '+name)
    
    def SendStatusMsg(self,player):
        player.SendMessage("Points "+str(player.points)+' '+str(self.CalculateAnte(player.points)))
    
    def CalculateRank(self,points):
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

    def CalculateAnte(self,points):
        return self.CalculateRank(points)
    
    def RewardRoundWinners(self):
        for winner in self.winners:
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
                print "ID: "+str(self.ID)+" lightNum is 0 for some reason...len(winners)="+str(len(self.winners))+", winner.choice="+str(winner.choice)
                print "ID: "+str(self.ID)+" len(playerList)="+str(len(self.playerList))+", winner.ID="+str(winner.ID)+", winner.pos="+str(winner.pos)
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
        losers = [player for player in self.playerList if not player in self.winners]
        for loser in losers:
            loser.SendMessage("Lose "+str(loser.choice))
    
    def ComputeRoundScores(self):
        cnt = len(self.playerList)
        
        #if cnt == 1: # for single-player only
            #self.playerList[0].curRoundScore = 1
        
        for i in range(cnt-1):
            player1 = self.playerList[i]
            for j in range(i+1,cnt):
                player2 = self.playerList[j]
                player1.curRoundScore += player1.ScoreAgainst(player2)
                player2.curRoundScore += player2.ScoreAgainst(player1)

    def PickRoundWinners(self):
        rwCopy = self.playerList
        rwCopy.sort(CompareRoundScores)
        big=rwCopy[len(rwCopy)-1].curRoundScore
        if big>0:
            self.winners = [winner for winner in rwCopy if winner.curRoundScore == big]

    def PickGameWinners(self):
        gameWinners = [winner for winner in self.winners if winner.rocksWon == 3 or winner.papersWon == 3 or winner.scissorsWon == 3]
        if (len(gameWinners) <= 1):
            return gameWinners
        gwCopy = gameWinners
        gwCopy.sort(CompareRoundsWon)
        big=gwCopy[len(gwCopy)-1].roundsWon
        if big>0:
            gameWinners = [winner for winner in gwCopy if winner.roundsWon==big]
        return gameWinners

    def HandlePlayerReady(self):
        if not self.countdownStarted:
            print "ID: "+str(self.ID)+" Starting countdown"
            self.SendToAll("StartCountdown")
            self.countdownStarted = 1
            return
        
        playersReady = [player for player in self.playerList if player.choice != kNone]
        allReady = (len(playersReady) == len(self.playerList))
        
        if allReady and len(playersReady)>1:
            self.SendToAll("StopCountdown")

    def ResetGame(self):
        self.winners = []
        for player in self.playerList:
            player.ResetGame()

    def ResetRound(self):
        self.winners = []
        for player in self.playerList:
            player.ResetRound()

    def PlayGame(self):
        if not self.countdownStarted:
            print "ID: "+str(self.ID)+" Extra countdown end message received, ignoring"
            return # extra countdown end message, ignore
        self.countdownStarted = 0
        playersReady = [player for player in self.playerList if player.choice != kNone]
        if len(playersReady) <= 1:
            print "ID: "+str(self.ID)+" Only one player was ready, aborting game"
            self.ResetRound()
            self.SendToAll("ShowIdleAnimation")
            return
        print "ID: "+str(self.ID)+" Disabling player's interfaces"
        self.SendToAll("Disable")
        self.ComputeRoundScores()
        self.PickRoundWinners()
        self.RewardRoundWinners()

    def HandleGameWinner(self):
        gameWinners = self.PickGameWinners()
        if len(gameWinners) > 0:
            print "ID: "+str(self.ID)+" A player has won, letting admin know about it"
            x = "init"
            if (gameWinners[0].choice == kRock): # it might be possible for multiple types to win, will need to check
                x = "rock"
            elif (gameWinners[0].choice == kPaper):
                x = "paper"
            elif (gameWinners[0].choice == kScissors):
                x = "scissors"
            self.SendToAll("GameWin "+x)
            pointsToGive = int(self.pot/len(gameWinners))
            self.pot -= pointsToGive*len(gameWinners)
            for winner in gameWinners:
                if winner.points != -1:
                    winner.SendMessage("SetPoints "+str(winner.points+pointsToGive))
                    for player in self.playerList:
                        if player.ID == winner.ID and player.key == winner.key:
                            player.points += pointsToGive
                            break
            self.ResetGame()
            for player in self.playerList:
                self.SendStatusMsg(player)
            return 1
        return 0

    def SendTableState(self,ID,key):
        for player in self.playerList:
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
    
    def ClientLeft(self,ID):
        position = -1
        for player in self.playerList[:]:
            if player.ID == ID:
                position = player.pos
                self.playerList.remove(player)
        for player in self.agePlayers[:]:
            if player[0] == ID:
                self.agePlayers.remove(player)
        if position == -1: # client wasn't playing
            return
        print "ID: "+str(self.ID)+" Client ID# "+str(ID)+" has left, cleaning up after him"
        if len(self.playerList) == 0: # no players left to cleanup the position
            if len(self.agePlayers) == 0: # no players in age left to cleanup the table
                self.queueCleanup.append(position)
            else:
                cleanup = ptPythonMsg()
                cleanup.setContents("Cleanup "+str(position))
                cleanup.setKey(self.agePlayers[0][1])
                cleanup.send(self.agePlayers[0][0])
                cleanup.setContents("ShowIdleAnimation")
                cleanup.send(self.agePlayers[0][0])
        for num in range(len(self.playerList)): # let everyone know of their new player numbers
            numberMsg = ptPythonMsg()
            numberMsg.setKey(self.playerList[num].key)
            numberMsg.setContents("Number "+str(num+1))
            numberMsg.send(self.playerList[num].ID)
        if position != -1:
            self.SendToAll("Drop "+str(position))
    
    def HandleAdd(self,senderID,key,pos):
        for player in self.playerList:
            if player.pos == pos:
                print "ID: "+str(self.ID)+" Ignoring client ID# "+str(senderID)+" request to play since someone is already sitting in their requested position: "+str(pos)
                return
        print "ID: "+str(self.ID)+" Adding client ID# "+str(senderID)+" to player list. They are playing position "+str(pos)
        player = heekPlayer()
        player.ID = senderID
        player.key = key
        player.pos = pos
        self.playerList.append(player)
        reply = ptPythonMsg()
        reply.setContents("Player "+str(len(self.playerList)))
        reply.setKey(key)
        reply.send(senderID)
        reply.setContents("GetPoints")
        reply.send(senderID)
        reply.setContents("GetName")
        reply.send(senderID)
    
    def HandleRemove(self,senderID,key):
        print "ID: "+str(self.ID)+" Removing client ID# "+str(senderID)+" from player list."
        for player in self.playerList:
            if player.ID == senderID and player.key == key:
                self.playerList.remove(player)
                if len(self.playerList) == 0: # last player left
                    print "ID: "+str(self.ID)+" Last player is requesting a leave, telling him to clean up"
                    if self.countdownStarted:
                        player.SendMessage("StopCountdown")
                        self.countdownStarted = 0
                    player.SendMessage("ShowIdleAnimation")
                    if player.points != -1:
                        print "ID: "+str(self.ID)+" Giving last player the remaining points in the pot: "+str(self.pot)
                        player.SendMessage("SetPoints "+str(player.points+self.pot))
                        self.pot = 0
                player.SendMessage("Goodbye")
                #self.ResetGame()
                break
        for num in range(len(self.playerList)): # let everyone know of their new player numbers
            numberMsg = ptPythonMsg()
            numberMsg.setKey(self.playerList[num].key)
            numberMsg.setContents("Number "+str(num+1))
            numberMsg.send(self.playerList[num].ID)
    
    def HandleChoice(self,senderID,key,choice):
        print "ID: "+str(self.ID)+" Player #"+str(senderID)+" has chosen "+str(choice)
        for player in self.playerList:
            if player.ID == senderID and player.key == key:
                if player.points != -1 and not player.isPlaying:
                    ante = self.CalculateAnte(player.points)
                    player.points -= ante
                    self.pot += ante
                    player.isPlaying = 1
                    player.SendMessage("SetPoints "+str(player.points))
                player.choice = choice
                self.HandlePlayerReady()
    
    def HandleCountdownFinished(self):
        print "ID: "+str(self.ID)+" Countdown has finished"
        self.PlayGame()
    
    def HandleChoiceAnimFinished(self):
        print "ID: "+str(self.ID)+" Choice animation has finished"
        if not self.HandleGameWinner():
            self.ResetRound()
            self.SendToAll("ShowIdleAnimation")
            self.SendToAll("Enable")
    
    def HandleWinAnimFinished(self):
        self.SendToAll("ShowIdleAnimation")
        self.SendToAll("Enable")
    
    def HandleLinkIn(self,senderID,key):
        self.agePlayers.append((senderID,key))
        if len(self.queueCleanup) != 0:
            for item in self.queueCleanup:
                cleanup = ptPythonMsg()
                cleanup.setContents("Cleanup "+str(item))
                cleanup.setKey(self.agePlayers[0][1])
                cleanup.send(self.agePlayers[0][0])
            self.queueCleanup = []
            idle = ptPythonMsg()
            idle.setContents("ShowIdleAnimation")
            idle.setKey(self.agePlayers[0][1])
            idle.send(self.agePlayers[0][0])
        self.SendTableState(senderID,key)
    
    def HandlePoints(self,senderID,key,points):
        print "ID: "+str(self.ID)+" Player ID# "+str(senderID)+" has "+str(points)+" points"
        for player in self.playerList:
            if player.ID == senderID and player.key == key:
                player.points = points
                if player.name != "":
                    self.SendWelcomeMsg(player.name,player.points)
    
    def HandleName(self,senderID,key,name):
        for player in self.playerList:
            if player.ID == senderID and player.key == key:
                player.name = name
                if player.points != -1:
                    self.SendWelcomeMsg(player.name,player.points)
                return

def onInit():
    global heekTables
    print "RestorationGuild.py is initializing"
    heekTables = []

def onShutdown():
    print "RestorationGuild.py is shutting down"

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
    global heekTables
    for table in heekTables:
        table.ClientLeft(ID)

def onMsgReceived(msg):
    global heekTables
    msgContents = msg.getContents()
    senderID = msg.getSenderID()
    key = msg.getKey()
    print "Message received from client. ID#: "+str(msg.getSenderID())+" Contents: "+msg.getContents()
    if len(msgContents) > 3 and msgContents[:3] == "Add": # they want to be added to our player list
        options = msgContents[4:].split(' ')
        pos = options[0]
        tableID = options[1]
        for table in heekTables:
            if table.ID == tableID:
                table.HandleAdd(senderID,key,pos)
    elif msgContents == "Remove": # they want to be removed from our player list
        for table in heekTables:
            if table.PlayerIDInGame(senderID):
                table.HandleRemove(senderID,key)
    elif msgContents == "Rock":
        for table in heekTables:
            if table.PlayerIDInGame(senderID):
                table.HandleChoice(senderID,key,kRock)
    elif msgContents == "Paper":
        for table in heekTables:
            if table.PlayerIDInGame(senderID):
                table.HandleChoice(senderID,key,kPaper)
    elif msgContents == "Scissors":
        for table in heekTables:
            if table.PlayerIDInGame(senderID):
                table.HandleChoice(senderID,key,kScissors)
    elif msgContents == "CountdownFinished":
        for table in heekTables:
            if table.PlayerIDInGame(senderID):
                table.HandleCountdownFinished()
    elif msgContents == "ChoiceAnimFinished":
        for table in heekTables:
            if table.PlayerIDInGame(senderID):
                table.HandleChoiceAnimFinished()
    elif msgContents == "GameWinAnimFinished":
        for table in heekTables:
            if table.PlayerIDInGame(senderID):
                table.HandleWinAnimFinished()
    elif len(msgContents) > 6 and msgContents[:6] == "LinkIn": # client telling us it's in the age
        tableID = msgContents[7:]
        for table in heekTables:
            if table.ID == tableID:
                table.HandleLinkIn(senderID,key)
                return
        # must be a new table, add it to our list
        print("Table ID # " + str(tableID) + " isn't in our list of games, adding it")
        newTable = heekGame()
        newTable.ID = tableID
        newTable.HandleLinkIn(senderID,key)
        heekTables.append(newTable)
    elif len(msgContents) > 6 and msgContents[:6] == "Points":
        points = int(msgContents[7:])
        for table in heekTables:
            if table.PlayerIDInGame(senderID):
                table.HandlePoints(senderID,key,points)
    elif len(msgContents) > 4 and msgContents[:4] == "Name":
        name = msgContents[5:]
        for table in heekTables:
            if table.PlayerIDInGame(senderID):
                table.HandleName(senderID,key,name)
    else:
        print "Unknown message, not handling"
