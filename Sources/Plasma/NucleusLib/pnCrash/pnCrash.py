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
import sys, os, re, poplib, smtplib, time


#*****************************************************************************
#*
#*  Private data
#*
#***

s_pop3Srv = 'catherine.cyan.com'
s_pop3User = 'crashhandler'
s_pop3Pass = 'crashhandler'

s_smtpSrv = 'catherine.cyan.com'
#s_smtpRecipients = ['eric@cyan.com']
s_smtpRecipients = ['crash2@cyan.com']

#s_mapFileBasePath = 'P:\\Plasma20\\Scripts\\Server\\Admin\\';
s_mapFileBasePath = '\\\\dirtcake\\MapFiles\\';

s_modules = {}
s_mapfiles = {}
s_sortedModuleKeys = []


#*****************************************************************************
#*
#*  Local functions
#*
#***

#=============================================================================
def UndecorateCppName (name):
	name = re.sub('^\?', '', name, 1)
	name = re.sub('^_', '', name, 1)
	name = re.sub('@', ' ', name)
	name = re.sub('\s+', ' ', name)
	return name

#=============================================================================
# returns index of the first entry for which 'eval' returns false
def bsearch (a, eval):
	low = 0
	high = len(a)

	if low != high:
		while True:
			mid = (low + high) // 2
			result = eval(a[mid])
			if result > 0:
				if mid == low:
					break
				low = mid
			else:
				high = mid
				if mid == low:
					break
	return high


#*****************************************************************************
#*
#*  Function class
#*
#***

class Function:
	
	#=========================================================================
	def __init__ (self, name, baseAddr, relocAddr, module):
		self.name = UndecorateCppName(name)
		self.baseAddr = baseAddr
		self.relocAddr = relocAddr
		self.module = module
		#print "Function: %08x,%s,%s" % (self.relocAddr, self.module, self.name)


#*****************************************************************************
#*
#*  Mapfile class
#*
#***

class Mapfile:
	#=========================================================================
	def __init__ (self, filename):
		self.filename = filename
		self.prefLoadAddr = 0
		self.functions = {}
		self.sortedFunctionKeys = []
		
		try:
			file = open(filename, 'rb')
#			print "mapfile opened " + filename
			lines = file.readlines()
			file.close()
			self.ParseMapfile(lines)
		except IOError:
#			print 'mapfile not found: ' + filename
			pass

	#=========================================================================
	def ParseMapfile (self, lines):
		# Preferred load address is xxxxxxxx
		pattern = re.compile('\s*Preferred.*([\dA-Fa-f]{8})')
		for line in lines:
			match = pattern.match(line)
			if not match:
				continue
			self.prefLoadAddr = int(match.group(1), 16)
			break
			
		# section:baseAddr functionName relocAddr flags module
		pattern = re.compile('^\s?([\dA-Fa-f]{4}:[\dA-Fa-f]{8})\s+(\S+)\s+([\dA-Fa-f]{8})\s(.)\s.\s(\S+)')
		for line in lines:
			match = pattern.match(line)
			if not match:
				continue
			baseAddr = match.group(1)
			name = match.group(2)
			relocAddr = int(match.group(3), 16)
			if match.group(4) != 'f':
				continue
			module = match.group(5)
			if self.functions.has_key(relocAddr):
#				print "  duplicate: %s %08x %s" % (baseAddr, relocAddr, name)
#				print "  with     : %s %08x %s" % (self.functions[relocAddr].baseAddr, self.functions[relocAddr].relocAddr, self.functions[relocAddr].name)
				pass
			else:
				self.functions[relocAddr] = Function(name, baseAddr, relocAddr, module)
			
		self.sortedFunctionKeys = self.functions.keys()
		self.sortedFunctionKeys.sort()
#		print "  function count: %u" % (len(self.sortedFunctionKeys))

	#=========================================================================
	def FindNearestFunction (self, relocAddr):
		
		#=====================================================================
		def eval (a, b=relocAddr):
			return b > a
			
		index = bsearch(self.sortedFunctionKeys, eval)
		if index > 0 and index < len(self.sortedFunctionKeys):
			return self.functions[self.sortedFunctionKeys[index-1]]
		return Function("<FunctionNotFound> relocAddr %x" % (relocAddr), 0, relocAddr, self.filename)

#=============================================================================
def LoadMapfile (mapfilename):
	global s_mapfiles
	if not s_mapfiles.has_key(mapfilename):
		s_mapfiles[mapfilename] = Mapfile(mapfilename)
	return s_mapfiles[mapfilename]


#*****************************************************************************
#*
#*  Module class
#*
#***

class Module:
	#=========================================================================
	def __init__ (self, loadAddr, name, buildId, buildMark):
#		print 'module: %s:%u, loadaddr: %08x' % (name, buildId, loadAddr)
		self.loadAddr = loadAddr
		self.name = name
		self.buildMark = buildMark
		self.buildId = buildId
		mapfilename = s_mapFileBasePath + buildMark + '\\' + name.split('.')[0].split('_')[0] + '.map'
		self.mapfile = LoadMapfile(mapfilename)

	#=========================================================================
	def FindNearestFunction (self, trueAddr):
		relocAddr = trueAddr + self.mapfile.prefLoadAddr - self.loadAddr
#		print 'searching for function near Pc:%08x, Ra:%08x in %s' % (trueAddr, relocAddr, self.mapfile.filename)
		function = self.mapfile.FindNearestFunction(relocAddr)
		if function:
#			print "  found function %s, Ra:%08x, %s" % (function.baseAddr, function.relocAddr, function.name) 
			return function
		print "Function not found for Pc:%08x, Ra:%08x in %s" % (trueAddr, relocAddr, self.mapfile.filename)
		return None


#=============================================================================
def FindNearestModule (loadAddr):
	
	#=========================================================================
	def eval (a, b=loadAddr):
		return b > a

	global s_modules
	global s_sortedModuleKeys
	
	if len(s_sortedModuleKeys) == 0:
		return None
			
	index = bsearch(s_sortedModuleKeys, eval)
	if index > 0 and index <= len(s_sortedModuleKeys):
		return s_modules[s_sortedModuleKeys[index-1]]
	
	print "Module not found for loadAddr %08x" % (loadAddr)
	return Module(loadAddr, "<ModuleNotFound> loadAddr %x" %(loadAddr), 0, "<NoBuildMark>")
	


#*****************************************************************************
#*
#*  pnCrash class
#*
#***

class pnCrash:
	#=========================================================================
	def SendCrash (self, msgLines, smtp):
		pass
	
	#=========================================================================
	def GetModules (self, msgLines):
		global s_modules
		global s_sortedModuleKeys
		# Parse modules and open mapfiles
		pattern = re.compile('^([\dA-Fa-f]{8})\s(.*)\((\d+)\.\d+\.(.*)\)')
		for line in msgLines:
			match = pattern.match(line)
			if not match:
				continue
			loadAddr = int(match.group(1), 16)
			filename = os.path.basename(match.group(2)).strip()
			buildId = int(match.group(3))
			buildMark = match.group(4)
			#assert not s_modules.has_key(loadAddr)
			s_modules[loadAddr] = Module(loadAddr, filename, buildId, buildMark)
		s_sortedModuleKeys = s_modules.keys()
		s_sortedModuleKeys.sort()
		
		# if no modules found, use the reporting application's info
		if len(s_sortedModuleKeys) == 0:
			buildId = 0
			loadAddr = int('00400000', 16)	# temp hack
			patternApp = re.compile('^App\s+:\s+(.*)')
			patternBuildMark = re.compile('^BuildMark\s+:\s+(.*)')
			for line in msgLines:
				if patternApp:
					match = patternApp.match(line)
					if match:
						patternApp = None
						filename = os.path.basename(match.group(1)).strip().strip('\r\n')
						continue
				else:
					match = patternBuildMark.match(line)
					if match:
						buildMark = match.group(1).strip().strip('\r\n')
#						print 'No modules found, adding %s, %08x, %u, %s' % (filename, loadAddr, buildId, buildMark)
						s_modules[loadAddr] = Module(loadAddr, filename, buildId, buildMark)
						s_sortedModuleKeys = s_modules.keys()
						s_sortedModuleKeys.sort()
						break
						

	#=========================================================================
	def ConvertTraces (self, msgLines):
		lines = []
		callstack = []
		intrace = False
		
		pattern = re.compile('Pc:([\dA-Fa-f]{8})\s+Fr:[\dA-Fa-f]{8}\s+Rt:([\dA-Fa-f]{8})')
		for rawline in msgLines:
			line = rawline.strip('\r\n')
			if line.find('|') >= 0:	# a previous conversion, skip over it
				continue
			lines.append(line)
			match = pattern.match(line)
			if not match:
				if intrace:
					callstack.append('')
					lines += callstack
					callstack = []
					lines.append('')
					intrace = False
				continue
			if not intrace:
				callstack.append('|--------> Callstack <--------')
				intrace = True
			returnAddr = int(match.group(2), 16)
			if not returnAddr:
				continue
			programCounter = int(match.group(1), 16)
			module = FindNearestModule(programCounter)
			function = module.FindNearestFunction(programCounter)
			callstack.append('| %s, %s:%s' % (function.name, function.module, module.name))
			
		if intrace:
			callstack.append('')
			lines += callstack
			callstack = []
			intrace = False
			
		return lines

	
	#=========================================================================
	def ProcessClientCrash(self, msgLines):
		lines = []
		fd = open('untranslated.txt', 'w')
		for x in msgLines:
			fd.write(x)
			fd.write('\n')
		fd.close()
		os.spawnl(os.P_WAIT, 'plStackTrace.exe', 'plStackTrace.exe', 'untranslated.txt')
		fd = open('out.txt', 'r')
		lines = fd.readlines()
		return lines


	#=========================================================================
	def ProcessCrash (self, msgLines, smtp):
		global s_modules
		global s_sortedModuleKeys
		
		subject = '[Crash]'
		pattern = re.compile('^Subject: (.*)$')
		serverPattern = re.compile('App        : plServer.exe')
		serverCrash = 0
		
		for line in msgLines:
			match = serverPattern.match(line)
			if match:
				serverCrash = 1
				break
				
		if not serverCrash:
			subject = 'Client Crash Report'
		
		for line in msgLines:
			match = pattern.match(line)
			if match:
				subject = match.group(1)
				subject.strip('\r\n')
		
		from_ = 'crash2@cyan.com'
		pattern = re.compile('^From: (.*)$')
		for line in msgLines:
			match = pattern.match(line)
			if match:
				from_ = match.group(1)
				from_.strip('\r\n')
				break
				
		print "Rcvd: %s from %s" % (subject, from_)

		smtp.helo()
		smtp.mail(from_, [])
		for rcpt in s_smtpRecipients:
			smtp.rcpt(rcpt, [])
			
		lines = []
		lines.append('From: %s' % (from_))
		lines.append('Subject: %s' % (subject))
		lines.append('')
		
		if not serverCrash:
			lines += self.ProcessClientCrash(msgLines)	# process client callstack
			for x in range(len(lines)):
			    lines[x] = lines[x].strip('\r\n')
                       		
		if serverCrash:
			s_modules.clear()
			s_sortedModuleKeys = []
			self.GetModules(msgLines)
			lines += self.ConvertTraces(msgLines)
			s_sortedModuleKeys = []
			s_modules.clear()
		
		msg = '\n'.join([line for line in lines])
		smtp.data(msg)
		print "Sent: %s from %s" % (subject, from_)
	
	
	#=========================================================================
	def GetAndProcessCrashes (self):
		global s_mapfiles
		s_mapfiles.clear()
		
#		print "initializing pop3"
		pop3 = poplib.POP3(s_pop3Srv)
		pop3.user(s_pop3User)
		pop3.pass_(s_pop3Pass)
		pop3.list()
		(numMsgs, totalSize) = pop3.stat()
		
		if numMsgs > 0:
#			print "initializing smtp"
			smtp = smtplib.SMTP(s_smtpSrv)
		
#			print "retrieving emails"
			for i in range(1, numMsgs + 1):
				(header, msg, octets) = pop3.retr(i)
				self.ProcessCrash(msg, smtp)
				pop3.dele(i)
			
			smtp.quit()
			
		pop3.quit()

		s_mapfiles.clear()
		
	#=========================================================================
	def RunForever (self):
		global s_modules
		s_modules[0] = Module(0, "<NilModule>", 0, "<NilBuild>")
		while 1:
			self.GetAndProcessCrashes()
			# sleep for an interruptable minute
			for i in range(1, 20):
				time.sleep(1)



#*****************************************************************************
#*
#*  Program entry point
#*
#***

#=============================================================================
if __name__ == '__main__':
	pnCrash().RunForever()
