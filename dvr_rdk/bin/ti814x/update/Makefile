TARGET:u-boot.bin uImage version bellphone.db
DESTDIR=/home/be/bell/git/work_for_update/opt/
RDKDIR=$(DESTDIR)/dvr_rdk/ti814x/
BINDIR=$(DESTDIR)/bin/
LIBDIR=$(DESTDIR)/lib/
QTDIR=$(DESTDIR)/bin/linphone-qt/
CONFIGDIR=$(DESTDIR)/
BOOTDIR=$(DESTDIR)/boot/
VERSION=
all:clean $(TARGET)

install:
	mkdir -p $(BOOTDIR)
	cp uImage $(BOOTDIR)
	cp u-boot.bin $(BOOTDIR)
	mkdir -p $(BINDIR)
	cp opt/bin/bellphone.db $(BINDIR)

uninstall:
	rm -rf $(BINDIR)/bellphone.db

clean:
	rm -f *.o
	rm -f /var/cache/apt/archives/*.deb
.PHONY:all clean
