parser grammar SpecParser;
options {
	tokenVocab = SpecLexer;
}

file: nonSpec (spec nonSpec)*;

spec:
	specDeclarationBlock
	| specContractBlock
	| specStatementBlock
	| specLoopInvariantBlock
	| wrapperLocationMarker
	| predicateDefBlock
	| ghostDefBlock
	| givenBindingBlock
	| yieldsBindingBlock;

specDeclarationBlock: SPEC_START specDeclaration+ SPEC_END;

specContractBlock:
	SPEC_START specMappingClause? specPureFlag? givenDefClause* yieldsDefClause* specContractClause+
		SPEC_END;

specStatementBlock: SPEC_START specStatement+ SPEC_END;

specLoopInvariantBlock: SPEC_START specLoopInvariant+ SPEC_END;

wrapperLocationMarker:
	SPEC_START 'wrapper location' specExpression? SPEC_EXPR_END SPEC_END;

predicateDefBlock: SPEC_START predicateDef+ SPEC_END;

ghostDefBlock: SPEC_START ghostDef SPEC_END;

specContractClause: ('requires' specExpression SPEC_EXPR_END)	# requiresClause
	| ('ensures' specExpression SPEC_EXPR_END)					# ensuresClause;

specMappingClause:
	'assumed'? 'contract' 'for' ('external' | 'generated') (
		'ir'
		| 'src'
	)? specExpression SPEC_EXPR_END wrapperSigClause?;

givenDefClause: 'given' specExpression SPEC_EXPR_END;

yieldsDefClause: 'yields' specExpression SPEC_EXPR_END;

givenBindingBlock: SPEC_START 'given' givenArgBinding+ SPEC_END;
givenArgBinding:
	gArg = ghostArgBindingLeft '=' expr = specExpression SPEC_EXPR_END;

yieldsBindingBlock:
	SPEC_START 'yields' yieldsArgBinding+ SPEC_END;
yieldsArgBinding:
	expr = ghostArgBindingLeft '=' yArg = specExpression SPEC_EXPR_END;

ghostArgBindingLeft: (~('=' | SPEC_END | SPEC_EXPR_END))+;

/* Specification of signature for external contracts */
wrapperSigClause: wrapperArgClause wrapperRetClause;
wrapperArgClause: 'args' wrapperArgList? SPEC_EXPR_END;
wrapperArgList:
	argExpr
	| (argExpr WRAPPER_ARG_SEP wrapperArgList);
wrapperRetClause: 'returns' specExpression SPEC_EXPR_END;

specPureFlag: 'pure' SPEC_EXPR_END # pureFlag;

specLoopInvariant: (
		'loop_invariant' specExpression SPEC_EXPR_END
	);

specStatement: ('assert' specExpression SPEC_EXPR_END)	# assertStatement
	| ('assume' specExpression SPEC_EXPR_END)			# assumeStatement
	| ('fold' specExpression SPEC_EXPR_END)				# foldStatement
	| ('unfold' specExpression SPEC_EXPR_END)			# unfoldStatement
	| ('ghost' 'assign' gVar = ghostArgBindingLeft '=' specExpression SPEC_EXPR_END) #ghostAssignment
    ;

specDeclaration: ('declare' specExpression SPEC_EXPR_END) # declarationClause;

predicateDef:
	INLINE? 'predicate' predName PRED_SIG_START predicateSig PRED_SIG_END ':=' specExpression
		SPEC_EXPR_END;

ghostDef:
	'ghost' type = ('method' | 'function') contr = ghostContract impl = ghostImpl;

ghostContract: specContractClause*;

predName: (~PRED_SIG_START)+;
predicateSig: .*?;// (~PRED_SIG_END)+;
argExpr: (~WRAPPER_ARG_SEP | SPEC_EXPR_END)+;
specExpression: (~(SPEC_EXPR_END | SPEC_END))+;
ghostImpl: (~SPEC_END)+;

nonSpec: (~(SPEC_START | SPEC_END))*;