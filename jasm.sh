#!/bin/bash

KEYWORDS=(
		public private protected static abstract final class interface enum
		void byte short char int long float double boolean
		synchronized volatile transient native strictfp
		throws default
)

OPCODES=(
		nop
		iconst_m1 iconst_0 iconst_1 iconst_2 iconst_3 iconst_4 iconst_5 lconst_0 lconst_1 fconst_0 fconst_1 fconst_2 dconst_0 dconst_1
		bipush sipush
		aconst_null
		ldc ldc_w ldc2_w

		iload lload fload dload aload iload_0 iload_1 iload_2 iload_3 lload_0 lload_1 lload_2 lload_3
		fload_0 fload_1 fload_2 fload_3 dload_0 dload_1 dload_2 dload_3 aload_0 aload_1 aload_2 aload_3

		istore lstore fstore dstore astore istore_0 istore_1 istore_2 istore_3 lstore_0 lstore_1 lstore_2 lstore_3
		fstore_0 fstore_1 fstore_2 fstore_3 dstore_0 dstore_1 dstore_2 dstore_3 astore_0 astore_1 astore_2 astore_3

		iaload laload faload daload aaload baload caload saload
		iastore lastore fastore dastore aastore bastore castore sastore

		pop pop2 dup dup_x1 dup_x2 dup2 dup2_x1 dup2_x2 swap

		iadd ladd fadd dadd isub lsub fsub dsub imul lmul fmul dmul idiv ldiv fdiv ddiv irem lrem
		frem drem ineg lneg fneg dneg ishl lshl ishr lshr iushr lushr iand land ior lor ixor lxor
		iinc

		i2l i2f i2d l2i l2f l2d f2i f2l f2d d2i d2l d2f i2b i2c i2s

		lcmp fcmpl fcmpg dcmpl dcmpg
		ifeq ifne iflt ifge ifgt ifle if_icmpeq if_icmpne if_icmplt if_icmpge if_icmpgt if_icmple if_acmpeq if_acmpne ifnull ifnonnull
		goto goto_w
		tableswitch lookupswitch

		ireturn lreturn freturn dreturn areturn return athrow

		getstatic putstatic getfield putfield
		invokevirtual invokespecial invokestatic invokeinterface invokedynamic
		new newarray anewarray multianewarray

		arraylength checkcast instanceof monitorenter monitorexit
		wide jsr jsr_w ret
		breakpoint impdep1 impdep2
)

FLAGS=(
		ACC_PUBLIC ACC_PRIVATE ACC_PROTECTED ACC_STATIC ACC_ABSTRACT ACC_FINAL ACC_INTERFACE ACC_ANNOTATION ACC_ENUM
		ACC_SUPER ACC_SYNCHRONIZED ACC_VOLATILE ACC_TRANSIENT ACC_BRIDGE ACC_VARARGS ACC_NATIVE ACC_STRICT ACC_SYNTHETIC
)

CP_REFERENCE_COLOR='\033[1;4;34m'        # blue underline
CP_REFERENCE_BASE_COLOR='\033[1;34m'     # blue
KEYWORD_COLOR='\033[1;32m'               # light green
OPCODE_COLOR='\033[1;1m'                 # white
STRING_COLOR='\033[0;33m'                # yellow
NUMBER_COLOR='\033[1;34m'                # blue
COMMENT_COLOR='\033[1;30m'               # gray
OPCODE_COMMENT_NAME_COLOR='\033[0;36m'   # cyan
OPCODE_COMMENT_COLOR='\033[0;32m'        # green
OPCODE_STRING_COMMENT_COLOR='\033[0;40m' # dark background
ATTRIBUTE_COLOR='\033[1;35m'             # purple
FLAG_COLOR='\033[1;1m'                   # white
DEFAULT_COLOR='\033[0m'

CP_REFERENCE_SEDCOLOR="\\$CP_REFERENCE_COLOR"
CP_REFERENCE_BASE_SEDCOLOR="\\$CP_REFERENCE_BASE_COLOR"
KEYWORD_SEDCOLOR="\\$KEYWORD_COLOR"
OPCODE_SEDCOLOR="\\$OPCODE_COLOR"
STRING_SEDCOLOR="\\$STRING_COLOR"
NUMBER_SEDCOLOR="\\$NUMBER_COLOR"
COMMENT_SEDCOLOR="\\$COMMENT_COLOR"
OPCODE_COMMENT_NAME_SEDCOLOR="\\$OPCODE_COMMENT_NAME_COLOR"
OPCODE_COMMENT_SEDCOLOR="\\$OPCODE_COMMENT_COLOR"
OPCODE_STRING_COMMENT_SEDCOLOR="\\$OPCODE_STRING_COMMENT_COLOR"
ATTRIBUTE_SEDCOLOR="\\$ATTRIBUTE_COLOR"
FLAG_SEDCOLOR="\\$FLAG_COLOR"
DEFAULT_SEDCOLOR="\\$DEFAULT_COLOR"

COLOR_REGEX='\\033\[\([0-9]\+\;\)*[0-9]\+m'

regex="\
s/\b-\?[0-9]\+\(\.[0-9]\+\([eE][+\-]\?[0-9]\+\)\?\)\?[lfd]\?\b/$NUMBER_SEDCOLOR&$DEFAULT_SEDCOLOR/g;\
s/\b-\?\.[0-9]\+\([eE][+\-]\?[0-9]\+\)\?[fd]\?\b/$NUMBER_SEDCOLOR&$DEFAULT_SEDCOLOR/g;\
s/\b-\?0x[0-9a-fA-F]\+\b/$NUMBER_SEDCOLOR&$DEFAULT_SEDCOLOR/g;\
s/\b-\?0b[01]\+\b/$NUMBER_SEDCOLOR&$DEFAULT_SEDCOLOR/g;\
\
s/\(\#\?\)$COLOR_REGEX\([0-9]\+\)$COLOR_REGEX\([\.:]\)\#$COLOR_REGEX\([0-9]\+\)\b/\
$CP_REFERENCE_SEDCOLOR\1\3$DEFAULT_SEDCOLOR$CP_REFERENCE_BASE_SEDCOLOR\5$DEFAULT_SEDCOLOR$CP_REFERENCE_SEDCOLOR#\7$DEFAULT_SEDCOLOR/g;\
s/\#$COLOR_REGEX\([0-9]\+\)$COLOR_REGEX\b/$CP_REFERENCE_SEDCOLOR#\2$DEFAULT_SEDCOLOR/g;\
\
s/\"[^\"]*\"/$STRING_SEDCOLOR&$DEFAULT_SEDCOLOR/g;\
s/\/\/.*/$COMMENT_SEDCOLOR&$DEFAULT_SEDCOLOR/g;s/\/\*.*\*\//$COMMENT_SEDCOLOR&$DEFAULT_SEDCOLOR/g"


for kwd in "${KEYWORDS[@]}"; do
	regex+=";s/\b$kwd\b/$KEYWORD_SEDCOLOR$kwd$DEFAULT_SEDCOLOR/g"
done

opcodeRegex=";s/\/\/ \(Field\|Method\) \(.*\)/\/\/ $OPCODE_COMMENT_NAME_SEDCOLOR\1$DEFAULT_SEDCOLOR $OPCODE_COMMENT_SEDCOLOR\2$DEFAULT_SEDCOLOR/g;\
s/\/\/ \(String\) \(.*\)/\/\/ $OPCODE_COMMENT_NAME_SEDCOLOR\1$DEFAULT_SEDCOLOR $OPCODE_STRING_COMMENT_SEDCOLOR\2$DEFAULT_SEDCOLOR/g"

for opcode in "${OPCODES[@]}"; do
	opcodeRegex+=";s/\b$opcode\b/$OPCODE_SEDCOLOR$opcode$DEFAULT_SEDCOLOR/g"
done

for flag in "${FLAGS[@]}"; do
	regex+=";s/\b$flag\b/$FLAG_SEDCOLOR$flag$DEFAULT_SEDCOLOR/g"
done

regex+=";s/^\(\s*\)\([A-Za-z][A-Za-z0-9_ \-]*\):/\1$ATTRIBUTE_SEDCOLOR\2$DEFAULT_SEDCOLOR:/g" # attributes


lineNum=-1

padding=3

printLine() {
	echo -e "$(sed "$regex" <<< "$1")"
}

printCodeLine() {
	echo -ne "    $([ $1 -lt 10 ] && echo -n '  ' || { [ $1 -lt 100 ] && echo -n ' '; })$NUMBER_COLOR$1$DEFAULT_COLOR"
	echo -e "$(sed "$regex$opcodeRegex" <<< "${3:$2}")"
}

printCodeHeader() {
	echo -e "   ${ATTRIBUTE_COLOR}index  pos  instruction$DEFAULT_COLOR"
}

javap -c -p "$@" |
while IFS= read line; do
	if [ $lineNum -lt 0 ] && [ "$line" = "    Code:" ]; then
		echo -e "$ATTRIBUTE_COLOR$line$DEFAULT_COLOR"
		IFS= read line

		lineNum=0

		if [[ "$line" =~ ' '+stack=[0-9]+,\ locals=[0-9]+,\ args_size=[0-9]+ ]]; then
			padding=5
			printLine "$line"
			printCodeHeader
			continue
		fi

		printCodeHeader
		padding=3

	elif [ $lineNum -ge 0 ]; then
		if [[ "$line" =~ ^' '{4,}[0-9]+:' '(tableswitch|lookupswitch)' '+\{ ]]; then
			printCodeLine $lineNum $padding "$line"
			oldLineNum=$lineNum
			lineNum=-2
			continue
		elif [[ ! "$line" =~ ^' '{4,}[0-9]+: ]]; then
			lineNum=-1
		fi

	fi
	if [ $lineNum -ge 0 ]; then
		printCodeLine $lineNum $padding "$line"
		let lineNum++
	else
		if [ $lineNum -eq -2 ]; then
			echo -n '  ';
			if [[ "$line" =~ ^' '+\} ]]; then
				lineNum=$oldLineNum
				oldLineNum=-1
			fi
		fi
		printLine "$line"
	fi
done
