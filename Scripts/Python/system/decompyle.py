#  Copyright (c) 1999 John Aycock
#  Copyright (c) 2000 by hartmut Goebel <hartmut@goebel.noris.de>
#
#  Permission is hereby granted, free of charge, to any person obtaining
#  a copy of this software and associated documentation files (the
#  "Software"), to deal in the Software without restriction, including
#  without limitation the rights to use, copy, modify, merge, publish,
#  distribute, sublicense, and/or sell copies of the Software, and to
#  permit persons to whom the Software is furnished to do so, subject to
#  the following conditions:
#  
#  The above copyright notice and this permission notice shall be
#  included in all copies or substantial portions of the Software.
#  
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
#  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
#  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
#  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
#  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
#  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
#  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#
# See 'CHANGES' for a list of changes
#
# NB. This is not a masterpiece of software, but became more like a hack.
#     Probably a complete write would be sensefull. hG/2000-12-27
#

import re, sys, os, types
import dis, imp, marshal
import string
import cStringIO


def _load_file(filename):
	"""
	load a Python source file and compile it to byte-code
	    _load_module(filename: string): code_object

	filename:    name of file containing Python source code
		     (normally a .py)
	code_object: code_object compiled from this source code

	This function does NOT write any file!
	"""
	fp = open(filename, 'rb')
	source = fp.read()+'\n'
	try:
		co = compile(source, filename, 'exec')
	except SyntaxError:
		sys.stderr.writelines( ['>>Syntax error in ', filename, '\n'] )
		raise
	fp.close()
	return co

def _load_module(filename):
	"""
	load a module without importing it
	    _load_module(filename: string): code_object

	filename:    name of file containing Python byte-code object
		     (normally a .pyc)
	code_object: code_object from this file
	"""
	fp = open(filename, 'rb')
	if fp.read(4) != imp.get_magic():
		raise ImportError, "Bad magic number in %s" % filename
	fp.read(4)
	co = marshal.load(fp)
	fp.close()
	return co


#-- start of (de-)compiler

#
#  Scanning
#

class Code:
	"""Class for representing code-objects.

	This is similar to the original code object, but additionally
	the diassembled code is stored in the attribute '_tokens'.
	"""
	def __init__(self, co):
		for i in dir(co):
			exec 'self.%s = co.%s' % (i, i)
		self._tokens, self._customize = disassemble(co)

class Token:
	"""Class representing a byte-code token.
	A byte-code token is equivalent to the contents of one line
        as output by dis.dis().
	"""
	def __init__(self, type, attr=None, pattr=None, offset=-1):
		self.type = intern(type)
		self.attr = attr
		self.pattr = pattr
		self.offset = offset

	def __cmp__(self, o):
		if isinstance(o, Token):
			# both are tokens: compare type and pattr 
			return cmp(self.type, o.type) \
			       or cmp(self.pattr, o.pattr)
		else:
			return cmp(self.type, o)
	
	def __repr__(self):		return str(self.type)
	def __str__(self):
		if self.pattr: pattr = self.pattr
		else: pattr = ''
		return '%s\t%-17s %s' % (self.offset, self.type, pattr)
	def __hash__(self):		return hash(self.type)
	def __getitem__(self, i):	raise IndexError

_JUMP_OPS_ = map(lambda op: dis.opname[op], dis.hasjrel + dis.hasjabs)

def disassemble(co):
	"""Disassemble a code object, returning a list of Token.

	The main part of this procedure is modelled after
	dis.diaassemble().
	"""
	rv = []
	customize = {}

	code = co.co_code
	cf = find_jump_targets(code)
	n = len(code)
	i = 0
	while i < n:
		offset = i
		if cf.has_key(offset):
			for j in range(cf[offset]):
				rv.append(Token('COME_FROM',
						offset="%s_%d" % (offset, j) ))

		c = code[i]
		op = ord(c)
		opname = dis.opname[op]
		i = i+1
		oparg = None; pattr = None
		if op >= dis.HAVE_ARGUMENT:
			oparg = ord(code[i]) + ord(code[i+1])*256
			i = i+2
			if op in dis.hasconst:
				const = co.co_consts[oparg]
				if type(const) == types.CodeType:
					oparg = const
					if const.co_name == '<lambda>':
						assert opname == 'LOAD_CONST'
						opname = 'LOAD_LAMBDA'
					# verify uses 'pattr' for
					# comparism, since 'attr' now
					# hold Code(const) and thus
					# can not be used for
					# comparism (todo: thinkg
					# about changing this)
					#pattr = 'code_object @ 0x%x %s->%s' %\
					#	(id(const), const.co_filename, const.co_name)
					pattr = 'code_object ' + const.co_name
				else:
					pattr = `const`
			elif op in dis.hasname:
				pattr = co.co_names[oparg]
			elif op in dis.hasjrel:
				pattr = `i + oparg`
			elif op in dis.haslocal:
				pattr = co.co_varnames[oparg]
			elif op in dis.hascompare:
				pattr = dis.cmp_op[oparg]

		if opname == 'SET_LINENO':
			continue
		elif opname in ('BUILD_LIST', 'BUILD_TUPLE', 'BUILD_SLICE',
				'UNPACK_LIST', 'UNPACK_TUPLE',
				'UNPACK_SEQUENCE',
				'MAKE_FUNCTION', 'CALL_FUNCTION',
				'CALL_FUNCTION_VAR', 'CALL_FUNCTION_KW',
				'CALL_FUNCTION_VAR_KW', 'DUP_TOPX',
				):
			opname = '%s_%d' % (opname, oparg)
			customize[opname] = oparg

		rv.append(Token(opname, oparg, pattr, offset))

	return rv, customize


def find_jump_targets(code):
	"""Detect all offsets in a byte code which are jump targets.

	Return the list of offsets.

	This procedure is modelled after dis.findlables(), but here
	for each target the number of jumps are counted.
	"""
	targets = {}
	n = len(code)
	i = 0
	while i < n:
		c = code[i]
		op = ord(c)
		i = i+1
		if op >= dis.HAVE_ARGUMENT:
			oparg = ord(code[i]) + ord(code[i+1])*256
			i = i+2
			label = -1
			if op in dis.hasjrel:
				label = i+oparg
			# todo: absolut jumps
			#elif op in dis.hasjabs:
			#	label = oparg
			if label >= 0:
				targets[label] = targets.get(label, 0) + 1
	return targets


#
#  Parsing
#

class AST:
	def __init__(self, type, kids=None):
		self.type = intern(type)
		if kids == None: kids = []
		self._kids = kids

	def append(self, o):			self._kids.append(o)
	def pop(self):				return self._kids.pop()
	def __getitem__(self, i):		return self._kids[i]
	def __setitem__(self, i, val):		self._kids[i] = val
	def __delitem__(self, i):		del self._kids[i]
	def __len__(self):			return len(self._kids)
	def __getslice__(self, low, high):	return self._kids[low:high]
	def __setslice__(self, low, high, seq):	self._kids[low:high] = seq
	def __delslice__(self, low, high):	del self._kids[low:high]
	def __cmp__(self, o):
		if isinstance(o, AST):
			return cmp(self.type, o.type) \
			       or cmp(self._kids, o._kids)
		else:
			return cmp(self.type, o)

	def __hash__(self):			return hash(self.type)

	def __repr__(self):
		rv = str(self.type)
		for k in self._kids:
			rv = rv + '\n' + string.replace(str(k), '\n', '\n   ')
		return rv

# Some ASTs used for comparing code fragments (like 'return None' at
# the end of functions).

RETURN_LOCALS = AST('stmt',
		    [ AST('return_stmt',
			  [ AST('expr', [ Token('LOAD_LOCALS') ]),
			    Token('RETURN_VALUE')]) ])

RETURN_NONE = AST('stmt',
		  [ AST('return_stmt',
			[ AST('expr', [ Token('LOAD_CONST', pattr='None') ]),
			  Token('RETURN_VALUE')]) ])

ASSIGN_DOC_STRING = lambda doc_string: \
	AST('stmt',
	    [ AST('assign',
		  [ AST('expr', [ Token('LOAD_CONST', pattr=`doc_string`) ]),
		    AST('designator', [ Token('STORE_NAME', pattr='__doc__')])
		    ])])

BUILD_TUPLE_0 = AST('expr',
		    [ Token('BUILD_TUPLE_0') ] )

from spark import GenericASTBuilder, GenericASTMatcher

class Parser(GenericASTBuilder):
	def __init__(self):
		GenericASTBuilder.__init__(self, AST, 'code')
		self.customized = {}

	def cleanup(self):
		"""
		Remove recursive references to allow garbage
		collector to collect this object.
		"""
		for dict in (self.rule2func, self.rules, self.rule2name, self.first):
			for i in dict.keys():
				dict[i] = None
		for i in dir(self):
			setattr(self, i, None)

	def error(self, token):
		# output offset, too
		print "Syntax error at or near `%s' token at offset %s" % \
		      (`token`, token.offset)
		raise SystemExit

	def typestring(self, token):
		return token.type
	
	def p_funcdef(self, args):
		'''
		stmt ::= funcdef
		funcdef ::= mkfunc STORE_FAST
		funcdef ::= mkfunc STORE_NAME
		'''

# new for Python2.0
#
# UNPACK_SEQUENCE # number of tuple items
# EXTENDED_ARG

	def p_list_comprehension(self, args):
		'''
		expr ::= list_compr
		list_compr ::= lc_prep lc_for lc_cleanup
		lc_prep ::= BUILD_LIST_0 DUP_TOP LOAD_ATTR STORE_NAME
		lc_prep ::= BUILD_LIST_0 DUP_TOP LOAD_ATTR STORE_FAST

		lc_for ::= expr LOAD_CONST
				FOR_LOOP designator
				lc_for JUMP_ABSOLUTE
				COME_FROM
		lc_for ::= expr LOAD_CONST
				FOR_LOOP designator
				lc_if JUMP_ABSOLUTE
				COME_FROM
		lc_for ::= expr LOAD_CONST
				FOR_LOOP designator
				lc_body JUMP_ABSOLUTE
				COME_FROM
		lc_if ::= expr condjmp lc_body
				JUMP_FORWARD COME_FROM POP_TOP
				COME_FROM
		lc_body ::= LOAD_NAME expr CALL_FUNCTION_1 POP_TOP
		lc_body ::= LOAD_FAST expr CALL_FUNCTION_1 POP_TOP
		lc_cleanup ::= DELETE_NAME
		lc_cleanup ::= DELETE_FAST
		'''
		
	def p_augmented_assign(self, args):
		'''
		stmt ::= augassign1
		stmt ::= augassign2
		augassign1 ::= expr expr inplace_op designator
		augassign1 ::= expr expr inplace_op ROT_THREE STORE_SUBSCR
		augassign1 ::= expr expr inplace_op ROT_TWO   STORE_SLICE+0
		augassign1 ::= expr expr inplace_op ROT_THREE STORE_SLICE+1
		augassign1 ::= expr expr inplace_op ROT_THREE STORE_SLICE+2
		augassign1 ::= expr expr inplace_op ROT_FOUR  STORE_SLICE+3
		augassign2 ::= expr DUP_TOP LOAD_ATTR expr
				inplace_op ROT_TWO   STORE_ATTR

		inplace_op ::= INPLACE_ADD
		inplace_op ::= INPLACE_SUBTRACT
		inplace_op ::= INPLACE_MULTIPLY
		inplace_op ::= INPLACE_DIVIDE
		inplace_op ::= INPLACE_MODULO
		inplace_op ::= INPLACE_POWER
		inplace_op ::= INPLACE_LSHIFT
		inplace_op ::= INPLACE_RSHIFT
		inplace_op ::= INPLACE_AND
		inplace_op ::= INPLACE_XOR
		inplace_op ::= INPLACE_OR 
		'''

	def p_assign(self, args):
		'''
		stmt ::= assign
		assign ::= expr DUP_TOP designList
		assign ::= expr designator
		'''

	def p_print(self, args):
		'''
		stmt ::= print_stmt
		stmt ::= print_stmt_nl
		stmt ::= print_nl_stmt
		print_stmt ::= expr PRINT_ITEM
		print_nl_stmt ::= PRINT_NEWLINE
		print_stmt_nl ::= print_stmt print_nl_stmt
		'''

	def p_print_to(self, args):
		'''
		stmt ::= print_to
		stmt ::= print_to_nl
		stmt ::= print_nl_to
		print_to ::= expr print_to_items POP_TOP
		print_to_nl ::= expr print_to_items PRINT_NEWLINE_TO
		print_nl_to ::= expr PRINT_NEWLINE_TO
		print_to_items ::= print_to_items print_to_item
		print_to_items ::= print_to_item
		print_to_item ::= DUP_TOP expr ROT_TWO PRINT_ITEM_TO
		'''
		# expr   print_to*   POP_TOP
		# expr { print_to* } PRINT_NEWLINE_TO

	def p_import15(self, args):
		'''
		stmt ::= importstmt
		stmt ::= importfrom

		importstmt ::= IMPORT_NAME STORE_FAST
		importstmt ::= IMPORT_NAME STORE_NAME

		importfrom ::= IMPORT_NAME importlist POP_TOP
		importlist ::= importlist IMPORT_FROM
		importlist ::= IMPORT_FROM
		'''

	def p_import20(self, args):
		'''
		stmt ::= importstmt2
		stmt ::= importfrom2
		stmt ::= importstar2

		importstmt2 ::= LOAD_CONST import_as
		importstar2 ::= LOAD_CONST IMPORT_NAME IMPORT_STAR

		importfrom2 ::= LOAD_CONST IMPORT_NAME importlist2 POP_TOP
		importlist2 ::= importlist2 import_as
		importlist2 ::= import_as
		import_as ::= IMPORT_NAME STORE_FAST
		import_as ::= IMPORT_NAME STORE_NAME
		import_as ::= IMPORT_NAME LOAD_ATTR STORE_FAST
		import_as ::= IMPORT_NAME LOAD_ATTR STORE_NAME
		import_as ::= IMPORT_FROM STORE_FAST
		import_as ::= IMPORT_FROM STORE_NAME
		'''
		# 'import_as' can't use designator, since n_import_as()
		# needs to compare both kids' pattr
		
	def p_grammar(self, args):
		'''
		code ::= stmts
		code ::=
		
		stmts ::= stmts stmt
		stmts ::= stmt

		stmts_opt ::= stmts
		stmts_opt ::= passstmt
		passstmt ::= 

		designList ::= designator designator
		designList ::= designator DUP_TOP designList

		designator ::= STORE_FAST
		designator ::= STORE_NAME
		designator ::= STORE_GLOBAL
		designator ::= expr STORE_ATTR
		designator ::= expr STORE_SLICE+0
		designator ::= expr expr STORE_SLICE+1
		designator ::= expr expr STORE_SLICE+2
		designator ::= expr expr expr STORE_SLICE+3
		designator ::= store_subscr
		store_subscr ::= expr expr STORE_SUBSCR
		designator ::= unpack
		designator ::= unpack_list
		
		stmt ::= classdef
		stmt ::= call_stmt
		call_stmt ::= expr POP_TOP

		stmt ::= return_stmt
		return_stmt ::= expr RETURN_VALUE

		stmt ::= break_stmt
		break_stmt ::= BREAK_LOOP
		
		stmt ::= continue_stmt
		continue_stmt ::= JUMP_ABSOLUTE
		
		stmt ::= raise_stmt
		raise_stmt ::= exprlist RAISE_VARARGS
		raise_stmt ::= nullexprlist RAISE_VARARGS
		
		stmt ::= exec_stmt
		exec_stmt ::= expr exprlist DUP_TOP EXEC_STMT
		exec_stmt ::= expr exprlist EXEC_STMT
		
		stmt ::= assert
		stmt ::= assert2
		stmt ::= ifstmt
		stmt ::= ifelsestmt
		stmt ::= whilestmt
		stmt ::= whileelsestmt
		stmt ::= forstmt
		stmt ::= forelsestmt
		stmt ::= trystmt
		stmt ::= tryfinallystmt
		
		stmt ::= DELETE_FAST
		stmt ::= DELETE_NAME
		stmt ::= DELETE_GLOBAL
		stmt ::= expr DELETE_SLICE+0
		stmt ::= expr expr DELETE_SLICE+1
		stmt ::= expr expr DELETE_SLICE+2
		stmt ::= expr expr expr DELETE_SLICE+3
		stmt ::= delete_subscr
		delete_subscr ::= expr expr DELETE_SUBSCR
		stmt ::= expr DELETE_ATTR
		
		kwarg   ::= LOAD_CONST expr
		
		classdef ::= LOAD_CONST expr mkfunc
				CALL_FUNCTION_0 BUILD_CLASS STORE_NAME
		classdef ::= LOAD_CONST expr mkfunc
			        CALL_FUNCTION_0 BUILD_CLASS STORE_FAST

		condjmp    ::= JUMP_IF_FALSE POP_TOP
		condjmp    ::= JUMP_IF_TRUE  POP_TOP

		assert ::= expr JUMP_IF_FALSE POP_TOP
				expr JUMP_IF_TRUE POP_TOP
				LOAD_GLOBAL RAISE_VARARGS
				COME_FROM COME_FROM POP_TOP
		assert2 ::= expr JUMP_IF_FALSE POP_TOP
				expr JUMP_IF_TRUE POP_TOP
				LOAD_GLOBAL expr RAISE_VARARGS
				COME_FROM COME_FROM POP_TOP

		ifstmt ::= expr condjmp stmts_opt
				JUMP_FORWARD COME_FROM POP_TOP
				COME_FROM

		ifelsestmt ::= expr condjmp stmts_opt
				JUMP_FORWARD COME_FROM
				POP_TOP stmts COME_FROM

		trystmt ::= SETUP_EXCEPT stmts_opt
				POP_BLOCK JUMP_FORWARD
				COME_FROM except_stmt

		try_end  ::= END_FINALLY COME_FROM
		try_end  ::= except_else
		except_else ::= END_FINALLY COME_FROM stmts

		except_stmt ::= except_cond except_stmt COME_FROM
		except_stmt ::= except_conds try_end COME_FROM
		except_stmt ::= except try_end COME_FROM
		except_stmt ::= try_end

		except_conds ::= except_cond except_conds COME_FROM
		except_conds ::= 

		except_cond ::= except_cond1
		except_cond ::= except_cond2
		except_cond1 ::= DUP_TOP expr COMPARE_OP
				JUMP_IF_FALSE
				POP_TOP POP_TOP POP_TOP POP_TOP
				stmts_opt JUMP_FORWARD COME_FROM
				POP_TOP
		except_cond2 ::= DUP_TOP expr COMPARE_OP
				JUMP_IF_FALSE
				POP_TOP POP_TOP designator POP_TOP
				stmts_opt JUMP_FORWARD COME_FROM
				POP_TOP
		except  ::=  POP_TOP POP_TOP POP_TOP
				stmts_opt JUMP_FORWARD

		tryfinallystmt ::= SETUP_FINALLY stmts_opt
				POP_BLOCK LOAD_CONST
				COME_FROM stmts_opt END_FINALLY

		whilestmt ::= SETUP_LOOP
				expr JUMP_IF_FALSE POP_TOP
				stmts_opt JUMP_ABSOLUTE
				COME_FROM POP_TOP POP_BLOCK COME_FROM
		whileelsestmt ::= SETUP_LOOP
		              	expr JUMP_IF_FALSE POP_TOP
				stmts_opt JUMP_ABSOLUTE
				COME_FROM POP_TOP POP_BLOCK
				stmts COME_FROM

		forstmt ::= SETUP_LOOP expr LOAD_CONST
				FOR_LOOP designator
				stmts_opt JUMP_ABSOLUTE
				COME_FROM POP_BLOCK COME_FROM
		forelsestmt ::= SETUP_LOOP expr LOAD_CONST
				FOR_LOOP designator
				stmts_opt JUMP_ABSOLUTE
				COME_FROM POP_BLOCK stmts COME_FROM
		'''

	def p_expr(self, args):
		'''
			expr ::= mklambda
			expr ::= mkfunc
			expr ::= SET_LINENO
			expr ::= LOAD_FAST
			expr ::= LOAD_NAME
			expr ::= LOAD_CONST
			expr ::= LOAD_GLOBAL
			expr ::= LOAD_LOCALS
			expr ::= expr LOAD_ATTR
			expr ::= binary_expr

			binary_expr ::= expr expr binary_op
			binary_op ::= BINARY_ADD
			binary_op ::= BINARY_SUBTRACT
			binary_op ::= BINARY_MULTIPLY
			binary_op ::= BINARY_DIVIDE
			binary_op ::= BINARY_MODULO
			binary_op ::= BINARY_LSHIFT
			binary_op ::= BINARY_RSHIFT
			binary_op ::= BINARY_AND
			binary_op ::= BINARY_OR
			binary_op ::= BINARY_XOR
			binary_op ::= BINARY_POWER

			expr ::= binary_subscr
			binary_subscr ::= expr expr BINARY_SUBSCR
			expr ::= expr expr DUP_TOPX_2 BINARY_SUBSCR
			expr ::= cmp
			expr ::= expr UNARY_POSITIVE
			expr ::= expr UNARY_NEGATIVE
			expr ::= expr UNARY_CONVERT
			expr ::= expr UNARY_INVERT
			expr ::= expr UNARY_NOT
			expr ::= mapexpr
			expr ::= expr SLICE+0
			expr ::= expr expr SLICE+1
			expr ::= expr expr SLICE+2
			expr ::= expr expr expr SLICE+3
			expr ::= expr DUP_TOP SLICE+0
			expr ::= expr expr DUP_TOPX_2 SLICE+1
			expr ::= expr expr DUP_TOPX_2 SLICE+2
			expr ::= expr expr expr DUP_TOPX_3 SLICE+3
			expr ::= and
			expr ::= or
			or   ::= expr JUMP_IF_TRUE  POP_TOP expr COME_FROM
			and  ::= expr JUMP_IF_FALSE POP_TOP expr COME_FROM

			cmp ::= cmp_list
			cmp ::= compare
			compare ::= expr expr COMPARE_OP
			cmp_list ::= expr cmp_list1 ROT_TWO POP_TOP
					COME_FROM
			cmp_list1 ::= expr DUP_TOP ROT_THREE
					COMPARE_OP JUMP_IF_FALSE POP_TOP
					cmp_list1 COME_FROM
			cmp_list1 ::= expr DUP_TOP ROT_THREE
					COMPARE_OP JUMP_IF_FALSE POP_TOP
					cmp_list2 COME_FROM
			cmp_list2 ::= expr COMPARE_OP JUMP_FORWARD

			mapexpr ::= BUILD_MAP kvlist

			kvlist ::= kvlist kv
			kvlist ::=

			kv ::= DUP_TOP expr ROT_TWO expr STORE_SUBSCR

			exprlist ::= exprlist expr
			exprlist ::= expr

			nullexprlist ::=
		'''

	def nonterminal(self, nt, args):
		collect = ('stmts', 'exprlist', 'kvlist')

		if nt in collect and len(args) > 1:
			#
			#  Collect iterated thingies together.
			#
			rv = args[0]
			rv.append(args[1])
		else:
			rv = GenericASTBuilder.nonterminal(self, nt, args)
		return rv

	def __ambiguity(self, children):
		# only for debugging! to be removed hG/2000-10-15
		print children
		return GenericASTBuilder.ambiguity(self, children)

	def resolve(self, list):
		if len(list) == 2 and 'funcdef' in list and 'assign' in list:
			return 'funcdef'
		#sys.stderr.writelines( ['resolve ', str(list), '\n'] )
		return GenericASTBuilder.resolve(self, list)

nop = lambda self, args: None

def parse(tokens, customize):
	p = Parser()
	#
	#  Special handling for opcodes that take a variable number
	#  of arguments -- we add a new rule for each:
	#
	#	expr ::= {expr}^n BUILD_LIST_n
	#	expr ::= {expr}^n BUILD_TUPLE_n
	#	expr ::= {expr}^n BUILD_SLICE_n
	#	unpack_list ::= UNPACK_LIST {expr}^n
	#	unpack ::= UNPACK_TUPLE {expr}^n
	#	unpack ::= UNPACK_SEQEUENE {expr}^n
	#	mkfunc ::= {expr}^n LOAD_CONST MAKE_FUNCTION_n
	#	expr ::= expr {expr}^n CALL_FUNCTION_n
	#	expr ::= expr {expr}^n CALL_FUNCTION_VAR_n POP_TOP
	#	expr ::= expr {expr}^n CALL_FUNCTION_VAR_KW_n POP_TOP
	#	expr ::= expr {expr}^n CALL_FUNCTION_KW_n POP_TOP
	#
	for k, v in customize.items():
		## avoid adding the same rule twice to this parser
		#if p.customized.has_key(k):
		#	continue
		#p.customized[k] = None

		#nop = lambda self, args: None
		op = k[:string.rfind(k, '_')]
		if op in ('BUILD_LIST', 'BUILD_TUPLE', 'BUILD_SLICE'):
			rule = 'expr ::= ' + 'expr '*v + k
		elif op in ('UNPACK_TUPLE', 'UNPACK_SEQUENCE'):
			rule = 'unpack ::= ' + k + ' designator'*v
		elif op == 'UNPACK_LIST':
			rule = 'unpack_list ::= ' + k + ' designator'*v
		elif op == 'DUP_TOPX':
			# no need to add a rule
			pass
			#rule = 'dup_topx ::= ' + 'expr '*v + k
		elif op == 'MAKE_FUNCTION':
			p.addRule('mklambda ::= %s LOAD_LAMBDA %s' %
				  ('expr '*v, k), nop)
			rule = 'mkfunc ::= %s LOAD_CONST %s' % ('expr '*v, k)
		elif op in ('CALL_FUNCTION', 'CALL_FUNCTION_VAR',
			    'CALL_FUNCTION_VAR_KW', 'CALL_FUNCTION_KW'):
			na = (v & 0xff)           # positional parameters
			nk = (v >> 8) & 0xff      # keyword parameters
			# number of apply equiv arguments:
			nak = ( len(op)-len('CALL_FUNCTION') ) / 3
			rule = 'expr ::= expr ' + 'expr '*na + 'kwarg '*nk \
			       + 'expr ' * nak + k
		else:
			raise 'unknown customize token %s' % k
		p.addRule(rule, nop)
	ast = p.parse(tokens)
	p.cleanup()
	return ast

#
#  Decompilation (walking AST)
#
#  All table-driven.  Step 1 determines a table (T) and a path to a
#  table key (K) from the node type (N) (other nodes are shown as O):
#
#         N                  N               N&K
#     / | ... \          / | ... \        / | ... \
#    O  O      O        O  O      K      O  O      O
#              |
#              K
#
#  MAP_R0 (TABLE_R0)  MAP_R (TABLE_R)  MAP_DIRECT (TABLE_DIRECT)
#
#  The default is a direct mapping.  The key K is then extracted from the
#  subtree and used to find a table entry T[K], if any.  The result is a
#  format string and arguments (a la printf()) for the formatting engine.
#  Escapes in the format string are:
#
#	%c	evaluate N[A] recursively*
#	%C	evaluate N[A[0]]..N[A[1]] recursively, separate by A[2]*
#	%,	print ',' if last %C only printed one item (for tuples)
#	%|	tab to current indentation level
#	%+	increase current indentation level
#	%-	decrease current indentation level
#	%{...}	evaluate ... in context of N
#	%%	literal '%'
#
#  * indicates an argument (A) required.
#
#  The '%' may optionally be followed by a number (C) in square brackets, which
#  makes the engine walk down to N[C] before evaluating the escape code.
#

from spark import GenericASTTraversal

#TAB = '\t'			# as God intended
TAB = ' ' *4   # is less spacy than "\t"

TABLE_R = {
	'build_tuple2':	( '%C', (0,-1,', ') ),
	'POP_TOP':	( '%|%c\n', 0 ),
	'STORE_ATTR':	( '%c.%[1]{pattr}', 0),
#	'STORE_SUBSCR':	( '%c[%c]', 0, 1 ),
	'STORE_SLICE+0':( '%c[:]', 0 ),
	'STORE_SLICE+1':( '%c[%c:]', 0, 1 ),
	'STORE_SLICE+2':( '%c[:%c]', 0, 1 ),
	'STORE_SLICE+3':( '%c[%c:%c]', 0, 1, 2 ),
	'JUMP_ABSOLUTE':( '%|continue\n', ),
	'DELETE_SLICE+0':( '%|del %c[:]\n', 0 ),
	'DELETE_SLICE+1':( '%|del %c[%c:]\n', 0, 1 ),
	'DELETE_SLICE+2':( '%|del %c[:%c]\n', 0, 1 ),
	'DELETE_SLICE+3':( '%|del %c[%c:%c]\n', 0, 1, 2 ),
	'DELETE_ATTR':	( '%|del %c.%[-1]{pattr}\n', 0 ),
	#'EXEC_STMT':	( '%|exec %c in %[1]C\n', 0, (0,sys.maxint,', ') ),
	'BINARY_SUBSCR':( '%c[%c]', 0, 1), # required for augmented assign
	'UNARY_POSITIVE':( '+%c', 0 ),
	'UNARY_NEGATIVE':( '-%c', 0 ),
	'UNARY_CONVERT':( '`%c`', 0 ),
	'UNARY_INVERT':	( '~%c', 0 ),
	'UNARY_NOT':	( '(not %c)', 0 ),
	'SLICE+0':	( '%c[:]', 0 ),
	'SLICE+1':	( '%c[%c:]', 0, 1 ),
	'SLICE+2':	( '%c[:%c]', 0, 1 ),
	'SLICE+3':	( '%c[%c:%c]', 0, 1, 2 ),
}
TABLE_R0 = {
#	'BUILD_LIST':	( '[%C]', (0,-1,', ') ),
#	'BUILD_TUPLE':	( '(%C)', (0,-1,', ') ),
#	'CALL_FUNCTION':( '%c(%C)', 0, (1,-1,', ') ),
}
TABLE_DIRECT = {
	'BINARY_ADD':		( '+' ,),
	'BINARY_SUBTRACT':	( '-' ,),
	'BINARY_MULTIPLY':	( '*' ,),
	'BINARY_DIVIDE':	( '/' ,),
	'BINARY_MODULO':	( '%%',),
	'BINARY_POWER':		( '**',),
	'BINARY_LSHIFT':	( '<<',),
	'BINARY_RSHIFT':	( '>>',),
	'BINARY_AND':		( '&' ,),
	'BINARY_OR':		( '|' ,),
	'BINARY_XOR':		( '^' ,),
	'INPLACE_ADD':		( '+=' ,),
	'INPLACE_SUBTRACT':	( '-=' ,),
	'INPLACE_MULTIPLY':	( '*=' ,),
	'INPLACE_DIVIDE':	( '/=' ,),
	'INPLACE_MODULO':	( '%%=',),
	'INPLACE_POWER':	( '**=',),
	'INPLACE_LSHIFT':	( '<<=',),
	'INPLACE_RSHIFT':	( '>>=',),
	'INPLACE_AND':		( '&=' ,),
	'INPLACE_OR':		( '|=' ,),
	'INPLACE_XOR':		( '^=' ,),
	'binary_expr':	( '(%c %c %c)', 0, -1, 1 ),

	'IMPORT_FROM':	( '%{pattr}', ),
	'LOAD_ATTR':	( '.%{pattr}', ),
	'LOAD_FAST':	( '%{pattr}', ),
	'LOAD_NAME':	( '%{pattr}', ),
	'LOAD_GLOBAL':	( '%{pattr}', ),
	'LOAD_LOCALS':	( 'locals()', ),
	#'LOAD_CONST':	( '%{pattr}', ), handled below
	'DELETE_FAST':	( '%|del %{pattr}\n', ),
	'DELETE_NAME':	( '%|del %{pattr}\n', ),
	'DELETE_GLOBAL':( '%|del %{pattr}\n', ),
	'delete_subscr':( '%|del %c[%c]\n', 0, 1,),
	'binary_subscr':( '%c[%c]', 0, 1),
	'store_subscr':	( '%c[%c]', 0, 1),
	'STORE_FAST':	( '%{pattr}', ),
	'STORE_NAME':	( '%{pattr}', ),
	'STORE_GLOBAL':	( '%{pattr}', ),
	'unpack':	( '(%C,)', (1, sys.maxint, ', ') ),
	'unpack_list':	( '[%C]', (1, sys.maxint, ', ') ),

	'list_compr':	( '[ %c ]', 1),
#	'lc_for':	( ' for %c in %c', 3, 0 ),
	'lc_for_nest':	( ' for %c in %c%c', 3, 0, 4 ),
	'lc_if':	( ' if %c', 0 ),
	'lc_body':	( '%c', 1),
	'lc_body__':	( '', ),
	
	'assign':	( '%|%c = %c\n', -1, 0 ),
	'augassign1':	( '%|%c %c %c\n', 0, 2, 1),
	'augassign2':	( '%|%c%c %c %c\n', 0, 2, -3, -4),
	#'dup_topx':	('%c', 0),
	'designList':	( '%c = %c', 0, -1 ),
	'and':          ( '(%c and %c)', 0, 3 ),
	'or':           ( '(%c or %c)', 0, 3 ),
	'compare':	( '(%c %[-1]{pattr} %c)', 0, 1 ),
	'cmp_list':	('%c %c', 0, 1),
	'cmp_list1':	('%[3]{pattr} %c %c', 0, -2),
	'cmp_list2':	('%[1]{pattr} %c', 0),
	'classdef':     ( '\n%|class %[0]{pattr[1:-1]}%c:\n%+%{build_class}%-', 1 ),
	'funcdef':      ( '\n%|def %c\n', 0),
	'kwarg':        ( '%[0]{pattr[1:-1]}=%c', 1),
	'importstmt':	( '%|import %[0]{pattr}\n', ),
	'importfrom':	( '%|from %[0]{pattr} import %c\n', 1 ),
	'importlist':	( '%C', (0, sys.maxint, ', ') ),
	'importstmt2':	( '%|import %c\n', 1),
	'importstar2':	( '%|from %[1]{pattr} import *\n', ),
	'importfrom2':	( '%|from %[1]{pattr} import %c\n', 2 ),
	'importlist2':	( '%C', (0, sys.maxint, ', ') ),
	'assert':	( '%|assert %c\n' , 3 ),
	'assert2':	( '%|assert %c, %c\n' , 3, -5 ),

	'print_stmt':		( '%|print %c,\n', 0 ),
	'print_stmt_nl':	( '%|print %[0]C\n', (0,1, None) ),
	'print_nl_stmt':	( '%|print\n', ),
	'print_to':		( '%|print >> %c, %c,\n', 0, 1 ),
	'print_to_nl':		( '%|print >> %c, %c\n', 0, 1 ),
	'print_nl_to':		( '%|print >> %c\n', 0 ),
	'print_to_items':	( '%C', (0, 2, ', ') ),

	'call_stmt':	( '%|%c\n', 0),
	'break_stmt':	( '%|break\n', ),
	'continue_stmt':( '%|continue\n', ),
	'raise_stmt':	( '%|raise %[0]C\n', (0,sys.maxint,', ') ),
	'return_stmt':	( '%|return %c\n', 0),
	'return_lambda':	( '%c', 0),

	'ifstmt':	( '%|if %c:\n%+%c%-', 0, 2 ),
	'ifelsestmt':	( '%|if %c:\n%+%c%-%|else:\n%+%c%-', 0, 2, -2 ),
	'ifelifstmt':	( '%|if %c:\n%+%c%-%c', 0, 2, -2 ),
	'elifelifstmt':	( '%|elif %c:\n%+%c%-%c', 0, 2, -2 ),
	'elifstmt':	( '%|elif %c:\n%+%c%-', 0, 2 ),
	'elifelsestmt':	( '%|elif %c:\n%+%c%-%|else:\n%+%c%-', 0, 2, -2 ),

	'whilestmt':	( '%|while %c:\n%+%c%-\n', 1, 4 ),
	'whileelsestmt':( '%|while %c:\n%+%c%-\n%|else:\n%+%c%-\n', 1, 4, 9 ),
	'forstmt':	( '%|for %c in %c:\n%+%c%-\n', 4, 1, 5 ),
	'forelsestmt':	(
		'%|for %c in %c:\n%+%c%-\n%|else:\n%+%c%-\n', 4, 1, 5, 9
	 ),
	'trystmt':	( '%|try:\n%+%c%-%c', 1, 5 ),
	'except':	( '%|except:\n%+%c%-', 3 ),
	'except_cond1':	( '%|except %c:\n%+%c%-', 1, 8 ),
	'except_cond2':	( '%|except %c, %c:\n%+%c%-', 1, 6, 8 ),
	'except_else':	( '%|else:\n%+%c%-', 2 ),
	'tryfinallystmt':( '%|try:\n%+%c%-\n%|finally:\n%+%c%-\n', 1, 5 ),
	'passstmt':	( '%|pass\n', ),
	'STORE_FAST':	( '%{pattr}', ),
	'kv':		( '%c: %c', 3, 1 ),
	'mapexpr':	( '{%[1]C}', (0,sys.maxint,', ') ),
}


MAP_DIRECT = (TABLE_DIRECT, )
MAP_R0 = (TABLE_R0, -1, 0)
MAP_R = (TABLE_R, -1)

MAP = {
	'stmt':		MAP_R,
	'designator':	MAP_R,
	'expr':		MAP_R,
	'exprlist':	MAP_R0,
}


ASSIGN_TUPLE_PARAM = lambda param_name: \
	  AST('expr', [ Token('LOAD_FAST', pattr=param_name) ])


def get_tuple_parameter(ast, name):
	"""
	If the name of the formal parameter starts with dot,
	it's a tuple parameter, like this:
		def MyFunc(xx, (a,b,c), yy):
			print a, b*2, c*42
	In byte-code, the whole tuple is assigned to parameter '.1' and
	then the tuple gets unpacked to 'a', 'b' and 'c'.

	Since identifiers starting with a dot are illegal in Python,
	we can search for the byte-code equivalent to '(a,b,c) = .1'
	"""
	assert ast == 'code' and ast[0] == 'stmts'
	for i in xrange(len(ast[0])):
		# search for an assign-statement
		assert ast[0][i] == 'stmt'
		node = ast[0][i][0]
		if node == 'assign' \
		   and node[0] == ASSIGN_TUPLE_PARAM(name):
			# okay, this assigns '.n' to something
			del ast[0][i]
			# walk lhs; this
			# returns a tuple of identifiers as used
			# within the function definition
			assert node[1] == 'designator'
			# if lhs is not a UNPACK_TUPLE (or equiv.),
			# add parenteses to make this a tuple
			if node[1][0] not in ('unpack', 'unpack_list'):
				return '(' + walk(node[1]) + ')'
			return walk(node[1])
	raise "Can't find tuple parameter" % name
	
def make_function(self, code, defparams, isLambda, nested=1):
	"""Dump function defintion, doc string, and function body."""
 
	def build_param(ast, name, default):
		"""build parameters:
			- handle defaults
			- handle format tuple parameters
		"""
		# if formal parameter is a tuple, the paramater name
		# starts with a dot (eg. '.1', '.2')
		if name[0] == '.':
			# replace the name with the tuple-string
			name = get_tuple_parameter(ast, name)

		if default:
			if Showast:
				print '--', name
				print default
				print '--'
			result = '%s = %s' % ( name, walk(default, indent=0) )
			##w = Walk(default, 0)
			##result = '%s = %s' % ( name, w.traverse() )
			##del w	# hg/2000-09-03
			if result[-2:] == '= ':	# default was 'LOAD_CONST None'
				result = result + 'None'
			return result
		else:
			return name

	def writeParams(self, params):
		for i in range(len(params)):
			if i > 0: self.f.write(', ')
			self.f.write(params[i])

	assert type(code) == types.CodeType
	code = Code(code)
	#assert isinstance(code, Code)

	ast = _build_ast(self.f, code._tokens, code._customize)
	code._tokens = None # save memory
	assert ast == 'code' and ast[0] == 'stmts'
	if isLambda:
		# convert 'return' statement to expression
		#assert len(ast[0]) == 1  wrong, see 'lambda (r,b): r,b,g'
		assert ast[-1][-1] == 'stmt'
		assert len(ast[-1][-1]) == 1
		assert ast[-1][-1][0] == 'return_stmt'
		ast[-1][-1][0].type = 'return_lambda'
	else:
		if ast[0][-1] == RETURN_NONE:
			# Python adds a 'return None' to the
			# end of any function; remove it
			ast[0].pop() # remove last node

	# add defaults values to parameter names
	argc = code.co_argcount
	paramnames = list(code.co_varnames[:argc])

	# defaults are for last n parameters, thus reverse
	paramnames.reverse(); defparams.reverse()

	# build parameters
	#
	##This would be a nicer piece of code, but I can't get this to work
	## now, have to find a usable lambda constuct  hG/2000-09-05
	##params = map(lambda name, default: build_param(ast, name, default),
	##	     paramnames, defparams)
	params = []
	for name, default in map(lambda a,b: (a,b), paramnames, defparams):
		params.append( build_param(ast, name, default) )

	params.reverse() # back to correct order

	if 4 & code.co_flags:	# flag 2 -> variable number of args
		params.append('*%s' % code.co_varnames[argc])
		argc = argc +1
	if 8 & code.co_flags:	# flag 3 -> keyword args
		params.append('**%s' % code.co_varnames[argc])
		argc = argc +1

	# dump parameter list (with default values)
	indent = TAB * self.indent
	if isLambda:
		self.f.write('lambda ')
		writeParams(self, params)
		self.f.write(': ')
	else:
		self.f.write('(')
		writeParams(self, params)
		self.f.write('):\n')
	        #self.f.write('%s#flags:\t%i\n' % (indent, code.co_flags))

	if code.co_consts[0] != None: # docstring exists, dump it
		self.f.writelines([indent, `code.co_consts[0]`, '\n'])

	_gen_source(self.f, ast, code._customize, self.indent,
		    isLambda=isLambda)
	code._tokens = None; code._customize = None # save memory


def build_class(self, code):
	"""Dump class definition, duc string and class body."""
	
	assert type(code) == types.CodeType
	code = Code(code)
	#assert isinstance(code, Code)

	indent = TAB * self.indent
	#self.f.write('%s#flags:\t%i\n' % (indent, code.co_flags))
	ast = _build_ast(self.f, code._tokens, code._customize)
	code._tokens = None # save memory
	assert ast == 'code' and ast[0] == 'stmts'

	# if docstring exists, dump it
	if code.co_consts[0] != None \
	   and ast[0][0] == ASSIGN_DOC_STRING(code.co_consts[0]):
		#print '\n\n>>-->>doc string set\n\n'
		self.f.writelines( [indent,repr(code.co_consts[0]), '\n'] )
		del ast[0][0]

	# the function defining a class normally returns locals(); we
	# don't want this to show up in the source, thus remove the node
	if ast[0][-1] == RETURN_LOCALS:
		ast[0].pop() # remove last node

	_gen_source(self.f, ast, code._customize, self.indent)
	code._tokens = None; code._customize = None # save memory

__globals_tokens__ =  ('STORE_GLOBAL', 'DELETE_GLOBAL') # 'LOAD_GLOBAL'

def find_globals(node, globals):
	"""Find globals in this statement."""
	for n in node:
		if isinstance(n, AST):
			if n != 'stmt': # skip nested statements
				globals = find_globals(n, globals)
		elif n.type in __globals_tokens__:
			globals[n.pattr] = None
	return globals


class Walk(GenericASTTraversal):
	def __init__(self, ast, indent=0, isLambda=0):
		GenericASTTraversal.__init__(self, ast)
		self._globals = {}
		self.f = cStringIO.StringIO()
		self.f.seek(0)
		self.indent = indent
		self.isLambda = isLambda

	def __del__(self):
		self.f.close()

	def traverse(self, node=None):
		self.preorder(node)
		return self.f.getvalue()

	def n_LOAD_CONST(self, node):
		data = node.pattr
		if data == 'None':
			# LOAD_CONST 'None' only occurs, when None is
			# implicit eg. in 'return' w/o params
			pass
		elif data == 'Ellipsis':
			self.f.write('...')
		elif data[0] == '-': # assume negative integer constant
			# convert to hex, since decimal representation
			# would result in 'LOAD_CONST; UNARY_NEGATIVE'
			self.f.write('0x%x' % int(data))
		else:
			self.f.write(data)

	def n_delete_subscr(self, node):
		#print >>self.f, '>#', node
		#print >>self.f, '---'
		maybe_tuple = node[-2][-1]
		#print >>self.f, '##', maybe_tuple, maybe_tuple.type[:11]
		if maybe_tuple.type[:11] == 'BUILD_TUPLE':
			maybe_tuple.type = 'build_tuple2'
			#print >>self.f, '##', node
			#print >>self.f, '##', maybe_tuple.type
		self.default(node)

	n_store_subscr = n_binary_subscr = n_delete_subscr
		
	def __n_stmts(self, node):
		# optimize "print 1, ; print"
		last = None; i = 0
		while i < len(node):
			n = node[i]
			assert(n == 'stmt')
			if n[0] == 'print_nl_stmt' and \
			   last is not None and \
			   last[0] == 'print_stmt':
				last[0].type = 'print_stmt_nl'
				del node[i]
				last = None
			else:
				last = n
			i = i + 1
		self.default(node)

	def n_stmt(self, node):
		if not self.isLambda:
			indent = TAB * self.indent
			for g in find_globals(node, {}).keys():
				self.f.writelines( [indent,
						    'global ',
						    g, '\n'] )
			## nice output does not work since engine() 
			## creates a new Walk instance when recursing
			## TODO: reconsider this: engine() no longer
			##	 creates a new Walk instancew hG/2000-12-31
			##	if not self._globals.has_key(g):
			##		self._globals[g] = None
			##		self.f.writelines( [TAB * self.indent,
			##				    'global ',
			##				    g, '\n'] )
		self.default(node)

	def n_exec_stmt(self, node):
		"""
		exec_stmt ::= expr exprlist DUP_TOP EXEC_STMT
		exec_stmt ::= expr exprlist EXEC_STMT
		"""
		w = Walk(node, indent=self.indent)
		w.engine(( '%|exec %c in %[1]C', 0, (0,sys.maxint,', ') ),
			 node)
		s = w.f.getvalue()
		del w
		if s[-3:] == 'in ':
			s = s[:-3]
		self.f.writelines( [s, '\n'] )
		node[:] = [] # avoid print out when recursive descenting
			
	def n_ifelsestmt(self, node, preprocess=0):
		if len(node[-2]) == 1:
			ifnode = node[-2][0][0]
			if ifnode == 'ifelsestmt':
				node.type = 'ifelifstmt'
				self.n_ifelsestmt(ifnode, preprocess=1)
				if ifnode == 'ifelifstmt':
					ifnode.type = 'elifelifstmt'
				elif ifnode == 'ifelsestmt':
					ifnode.type = 'elifelsestmt'
			elif ifnode == 'ifstmt':
				node.type = 'ifelifstmt'
				ifnode.type = 'elifstmt'
		if not preprocess:
			self.default(node)

	def n_import_as(self, node):
		iname = node[0].pattr; sname = node[-1].pattr
		if iname == sname \
		   or iname[:len(sname)+1] == (sname+'.'):
			self.f.write(iname)
		else:
			self.f.writelines([iname, ' as ', sname])
		node[:] = [] # avoid print out when recursive descenting

	def n_mkfunc(self, node):
		defparams = node[0:-2]
		code = node[-2].attr
		node[:] = [] # avoid print out when recursive descenting
		self.indent = self.indent + 1
		self.f.write(code.co_name)
		make_function(self, code, defparams, isLambda=0)
		self.indent = self.indent - 1

	def n_mklambda(self, node):
		defparams = node[0:-2]
		code = node[-2].attr
		node[:] = [] # avoid print out when recursive descenting
		make_function(self, code, defparams, isLambda=1)

	def n_classdef(self, node):
		self.f.writelines(['\n', TAB * self.indent, 'class '])
		self.f.write(node[0].pattr[1:-1])
		node._code = node[-4][0].attr
		 # avoid print out when recursive descenting
		if node[1] == BUILD_TUPLE_0:
			node[:] = []
		else:
			node[:] = [ node[1] ]
			
	def n_classdef_exit(self, node):
		self.f.write(':\n')
		self.indent = self.indent +1
		# '\n%|class %[0]{pattr[1:-1]}%c:\n%+%{build_class}%-', 1 ),
		# -4 -> MAKE_FUNCTION; -2 -> LOAD_CONST (code)
		build_class(self,node._code)
		self.indent = self.indent -1
		node._code = None # save memory

	def n_lc_for(self, node):
		node.type = 'lc_for_nest'
		content = node[4]
		while content == 'lc_for':
			content.type = 'lc_for_nest'
			content = content[4]
		while content == 'lc_if':
			content = content[2]
		assert content == 'lc_body'
		self.preorder(content)
		content.type = 'lc_body__'
		self.default(node)
		
	def engine(self, entry, startnode):
		#self.f.write("-----\n")
		#self.f.write(str(startnode.__dict__)); self.f.write('\n')
		escape = re.compile(r'''
			% ( \[ (?P<child> -? \d+ ) \] )?
				((?P<type> [^{] ) |
				 ( [{] (?P<expr> [^}]* ) [}] ))
		''', re.VERBOSE)

		fmt = entry[0]
		n = len(fmt)
		lastC = 0
		arg = 1
		i = 0

		while i < n:
			m = escape.match(fmt, i)
			if m is None:
				self.f.write(fmt[i])
				i = i + 1
				continue

			i = m.end()
			typ = m.group('type') or '{'

			node = startnode
			try:
				if m.group('child'):
					node = node[string.atoi(m.group('child'))]
			except:
				print node.__dict__
				raise

			if typ == '%':
				self.f.write('%')
			elif typ == '+':
				self.indent = self.indent + 1
			elif typ == '-':
				self.indent = self.indent - 1
			elif typ == '|':
				self.f.write(TAB * self.indent)
			elif typ == ',':
				if lastC == 1:
					self.f.write(',')
			elif typ == 'c':
				self.traverse(node[entry[arg]])
				##w = Walk(node[entry[arg]], self.indent)
				##self.f.write(w.traverse())
				##del w	# hg/2000-09-03
				arg = arg + 1
			elif typ == 'C':
				low, high, sep = entry[arg]
				lastC = remaining = len(node[low:high])
				for subnode in node[low:high]:
					self.traverse(subnode)
					##w = Walk(subnode, self.indent)
					##self.f.write(w.traverse())
					##del w	# hg/2000-09-03
					remaining = remaining - 1
					if remaining > 0:
						self.f.write(sep)
				arg = arg + 1
			elif typ == '{':
				d = node.__dict__
				expr = m.group('expr')
				if  expr == 'build_class':
					# -4 -> MAKE_FUNCTION; -2 -> LOAD_CONST (code)
					build_class(self,node[-4][-2].attr)
				else:
					try:
						self.f.write(eval(expr, d, d))
					except:
						print node
						raise

	def default(self, node):
		mapping = MAP.get(node, MAP_DIRECT)
		table = mapping[0]
		key = node

		for i in mapping[1:]:
			key = key[i]

		if table.has_key(key):
			self.engine(table[key], node)
			self.prune()

def walk(ast, customize={}, indent=0, isLambda=0):
	w = Walk(ast, indent, isLambda=isLambda)
	#
	#  Special handling for opcodes that take a variable number
	#  of arguments -- we add a new entry for each in TABLE_R.
	#
	for k, v in customize.items():
		op = k[:string.rfind(k, '_')]
		if op == 'BUILD_LIST':
			TABLE_R[k] = ( '[%C]', (0,-1,', ') )
		elif op == 'BUILD_SLICE':
			TABLE_R[k] = ( '%C', (0,-1,':') )
		elif op == 'BUILD_TUPLE':
			TABLE_R[k] = ( '(%C%,)', (0,-1,', ') )
		elif op == 'CALL_FUNCTION':
			TABLE_R[k] = ( '%c(%C)', 0, (1,-1,', ') )
		elif op in ('CALL_FUNCTION_VAR',
			    'CALL_FUNCTION_VAR_KW', 'CALL_FUNCTION_KW'):
			if v == 0:
				str = '%c(%C' # '%C' is a dummy here ...
				p2 = (0, 0, None) # .. because of this
			else:
				str = '%c(%C, '
				p2 = (1,-2, ', ')
			if op == 'CALL_FUNCTION_VAR':
				str = str + '*%c)'
				entry = (str, 0, p2, -2)
			elif op == 'CALL_FUNCTION_KW':
				str = str + '**%c)'
				entry = (str, 0, p2, -2)
			else:
				str = str + '*%c, **%c)'
				if p2[2]: p2 = (1,-3, ', ')
				entry = (str, 0, p2, -3, -2)
			TABLE_R[k] = entry
	result = w.traverse()
	return result

#-- end of (de-)compiler ---

#-- start 

Showasm = 0
Showast = 0
__real_out = None

def _tokenize(out, co):
	"""Disassemble code object into a token list"""
	assert type(co) == types.CodeType
	
	tokens, customize = disassemble(co)
	#  See the disassembly..
	if Showasm and out is not None:
		for t in tokens:
			out.write('%s\n' % t)
		out.write('\n')
	return tokens, customize

def _build_ast(out, tokens, customize):
	assert type(tokens) == types.ListType
	assert isinstance(tokens[0], Token)

	#  Build AST from disassembly.
	try:
		ast = parse(tokens, customize)
	except:  # parser failed, dump disassembly
		#if not Showasm:
		__real_out.write('--- This code section failed: ---\n')
		for t in tokens:
			__real_out.write('%s\n' % t)
		__real_out.write('\n')
		raise
	return ast

def _gen_source(out, ast, customize, indent=0, isLambda=0):
	"""convert AST to source code"""
	if Showast:
		out.write(`ast`)

	# if code would be empty, append 'pass'
	if len(ast[0]) == 0:
		out.write(indent * TAB)
		out.write('pass\n')
	else:
		out.write(walk(ast, customize, indent, isLambda=isLambda))


def decompyle(co, out=None, indent=0, showasm=0, showast=0):
	"""
	diassembles a given code block 'co'
	"""
	assert type(co) == types.CodeType

	global Showasm, Showast
	Showasm = showasm
	Showast = showast
	
	if not out:
		out = sys.stdout
	global __real_out
	__real_out = out # store final output stream for case of error

	tokens, customize = _tokenize(out, co)
	ast = _build_ast(out, tokens, customize)
	tokens = None # save memory

	assert ast == 'code' and ast[0] == 'stmts'
	# convert leading '__doc__ = "..." into doc string
	if ast[0][0] == ASSIGN_DOC_STRING(co.co_consts[0]):
		out.writelines( [repr(co.co_consts[0]), '\n'] )
		del ast[0][0]
	if ast[0][-1] == RETURN_NONE:
		ast[0].pop() # remove last node
		#todo: if empty, add 'pass'

	_gen_source(out, ast, customize, indent)


def decompyle_file(filename, outstream=None, showasm=0, showast=0):
	"""
	decompile Python byte-code file (.pyc)
	"""
	co = _load_module(filename)
	decompyle(co, out=outstream, showasm=showasm, showast=showast)
	co = None
