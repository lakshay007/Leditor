all: qmake_makefile leditor-gui

qmake_makefile:
	qmake leditor-gui.pro -o Makefile.qt

leditor-gui: qmake_makefile
	make -f Makefile.qt

clean:
	make -f Makefile.qt clean 2>/dev/null || true
	rm -f Makefile.qt

run: leditor-gui
	./leditor-gui

.PHONY: all clean run qmake_makefile 