
all: ausarbeitung.pdf

ausarbeitung.pdf: ausarbeitung.ps
	ps2pdf ausarbeitung.ps

ausarbeitung.ps: ausarbeitung.dvi
	dvips -t a4 -Ppdf ausarbeitung.dvi

ausarbeitung.dvi: ai-evolver_call.tex apdx_server_bugs.tex ausarbeitung.tex bedienung.tex class_backgammon.tex gui.tex ki.tex netzwerk.tex projektorganisation.tex schlusswort.tex src-files.aux
	latex ausarbeitung.tex
	latex ausarbeitung.tex
	latex ausarbeitung.tex

src-files.aux: src-files.tex
	latex src-files.tex
	latex src-files.tex
