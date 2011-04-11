#
# (C) Copyright 2000 by hartmut Goebel <hartmut@goebel.noris.de>
#
# byte-code verifier for decompyle
#

import types
import decompyle

#--- exceptions ---

class VerifyCmpError(Exception):
	pass

class CmpErrorConsts(VerifyCmpError):
	"""Exception to be raised when consts differ."""
	def __init__(self, name, index):
		self.name = name
		self.index = index

	def __str__(self):
		return 'Compare Error within Consts of %s at index %i' % \
		       (repr(self.name), self.index)
					
class CmpErrorConstsLen(VerifyCmpError):
	"""Exception to be raised when length of co_consts differs."""
	def __init__(self, name, consts1, consts2):
		self.name = name
		self.consts = (consts1, consts2)

	def __str__(self):
		return 'Consts length differs in %s:\n\n%i:\t%s\n\n%i:\t%s\n\n' % \
		       (repr(self.name),
			len(self.consts[0]), `self.consts[0]`,
			len(self.consts[1]), `self.consts[1]`)
					
class CmpErrorCode(VerifyCmpError):
	"""Exception to be raised when code differs."""
	def __init__(self, name, index, token1, token2):
		self.name = name
		self.index = index
		self.token1 = token1
		self.token2 = token2
		
	def __str__(self):
		return 'Code differs in %s at offset %i [%s] != [%s]' % \
		       (repr(self.name), self.index,
			repr(self.token1), repr(self.token2)) #\
			# + ('%s %s') % (self.token1.pattr, self.token2.pattr)

class CmpErrorCodeLen(VerifyCmpError):
	"""Exception to be raised when code length differs."""
	def __init__(self, name, tokens1, tokens2):
		self.name = name
		self.tokens = [tokens1, tokens2]

	def __str__(self):
		return reduce(lambda s,t: "%s%-37s\t%-37s\n" % (s, t[0], t[1]),
			      map(lambda a,b: (a,b),
				  self.tokens[0],
				  self.tokens[1]),
			      'Code len differs in %s\n' % str(self.name))

class CmpErrorMember(VerifyCmpError):
	"""Exception to be raised when other members differ."""
	def __init__(self, name, member, data1, data2):
		self.name = name
		self.member = member
		self.data = (data1, data2)

	def __str__(self):
		return 'Member %s differs in %s:\n\t%s\n\t%s\n' % \
		       (repr(self.member), repr(self.name),
			repr(self.data[0]), repr(self.data[1]))

#--- compare ---
					
# these members are ignored
__IGNORE_CODE_MEMBERS__ = ['co_filename', 'co_firstlineno', 'co_lnotab']

def cmp_code_objects(code_obj1, code_obj2, name=''):
	"""
	Compare two code-objects.

	This is the main part of this module.
	"""
	assert type(code_obj1) == types.CodeType
	assert type(code_obj2) == types.CodeType
	assert dir(code_obj1) == code_obj1.__members__
	assert dir(code_obj2) == code_obj2.__members__
	assert code_obj1.__members__ == code_obj2.__members__
	if name == '__main__':
		name = code_obj1.co_name
	else:
		name = '%s.%s' % (name, code_obj1.co_name)
		if name == '.?': name = '__main__'
		
	members = code_obj1.__members__; members.sort(); #members.reverse()
	tokens1 = None
	for member in members:
		if member in __IGNORE_CODE_MEMBERS__:
			pass
		elif member == 'co_code':
			# use changed Token class
			__Token = decompyle.Token
			decompyle.Token = Token
			# tokenize both code-objects
			tokens1, customize = decompyle._tokenize(None, code_obj1)
			tokens2, customize = decompyle._tokenize(None, code_obj2)
			del customize
			decompyle.Token = __Token # restore Token class

			# compare length
			if len(tokens1) != len(tokens2):
				raise CmpErrorCodeLen(name, tokens1, tokens2)
			# compare contents
			#print len(tokens1), type(tokens1), type(tokens2)
			for i in xrange(len(tokens1)):
				if tokens1[i] != tokens2[i]:
					#print '-->', i, type(tokens1[i]), type(tokens2[i])
					raise CmpErrorCode(name, i, tokens1[i],
							   tokens2[i])
		elif member == 'co_consts':
			# compare length
			if len(code_obj1.co_consts) != len(code_obj2.co_consts):
				raise CmpErrorConstsLen(name, code_obj1.co_consts ,code_obj2.co_consts)
			# compare contents
			for idx in xrange(len(code_obj1.co_consts)):
				const1 = code_obj1.co_consts[idx]
				const2 = code_obj2.co_consts[idx]
				# same type?
				if type(const1) != type(const2):
					raise CmpErrorContType(name, idx)
				if type(const1) == types.CodeType:
					# code object -> recursive compare
					cmp_code_objects(const1, const2,
							 name)
				elif cmp(const1, const2) != 0:
					# content differs
					raise CmpErrorConsts(name, idx)
		else:
			# all other members must be equal
			if eval('code_obj1.%s != code_obj2.%s' % (member, member)):
				data1 = eval('code_obj1.%s' % member)
				data2 = eval('code_obj2.%s' % member)
				raise CmpErrorMember(name, member, data1,data2)


class Token(decompyle.Token):
	"""Token class with changed semantics for 'cmp()'."""

	def __cmp__(self, o):
		if self.type in decompyle._JUMP_OPS_:
			# ignore offset
			return cmp(self.type, o.type)
		else:
			return cmp(self.type, o.type) \
			       or cmp(self.pattr, o.pattr)

	def __repr__(self):
		return '%s %s (%s)' % (str(self.type), str(self.attr),
				       str(self.pattr))


def compare_code_with_srcfile(pyc_filename, src_filename):
	"""Compare a .pyc with a source code file."""
	code_obj1 = decompyle._load_module(pyc_filename)
	code_obj2 = decompyle._load_file(src_filename)
	cmp_code_objects(code_obj1, code_obj2)

def compare_files(pyc_filename1, pyc_filename2):
	"""Compare two .pyc files."""
	code_obj1 = decompyle._load_module(pyc_filename1)
	code_obj2 = decompyle._load_module(pyc_filename2)
	cmp_code_objects(code_obj1, code_obj2)

if __name__ == '__main__':
	t1 = Token('LOAD_CONST', None, 'code_object _expandLang', 52)
	t2 = Token('LOAD_CONST', -421, 'code_object _expandLang', 55)
	print `t1`
	print `t2`
	print cmp(t1, t2), cmp(t1.type, t2.type), cmp(t1.attr, t2.attr)
