.PHONY: all debug vcs clean

all:
	@echo "Usage: make <target>"
	@echo "target:"
	@echo "    debug - compile using g++"
	@echo "    vcs - compile using vcs"

debug:
	g++ -D_DEBUG -O0 -g3 -Wall -Wextra text2d.cc -o program

vcs:
	vcs +v2k -sverilog text2d_sample.sv text2d.cc

clean:
	${RM} ./sample.txt
	${RM} ./program
	${RM} ./simv
	${RM} -r ./csrc
	${RM} -r ./simv.daidir
	${RM} ./ucli.key
	${RM} ./vc_hdrs.h

