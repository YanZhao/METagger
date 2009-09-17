bin_PROGRAMS = me_dutch_mwu_release


# set the include path found by configure
INCLUDES= $(all_includes)

# the library search path.
me_dutch_mwu_release_LDFLAGS = $(all_libraries) 
me_dutch_mwu_release_SOURCES = CRF.cpp CRF.h dev_log.txt doublematrix.cpp \
	doublematrix.h doublevector.cpp doublevector.h Events.cpp Events.h fadd.h \
	HashFunction.cpp HashFunction.h JBaseProc.cpp JBaseProc.h JEvaluate.cpp JEvaluate.h \
	JTriggerMgr.cpp JTriggerMgr.h LogLinear.cpp LogLinear.h PosMaxEn.cpp PosMaxEn.h \
	SparseMatrix.cpp SparseMatrix.h StateNode.cpp StateNode.h StrHashTrait.h TagNode.cpp \
	TagNode.h TagsSequence.h Trellis.cpp Trellis.h Verify.h Viterbi.cpp Viterbi.h \
	transition.cpp transition.h lbfgs_c.c global.h log_linear_context.cpp mathlib.cpp \
	mathlib.h mwutool.cpp mwutool.h
me_dutch_mwu_release_LDADD = -lfadd
