cflags=`pkg-config --cflags pidgin`
ldflags=`pkg-config --libs pidgin`

all:
	gcc -shared -o pidgin_gentoo.so pidgin_gentoo.c ${cflags} ${ldflags}

install:
	mkdir -p ~/.purple/plugins
	cp pidgin_gentoo.so ~/.purple/plugins/

uninstall:
	rm ~/.purple/plugins/pidgin_gentoo.so

clean:
	rm pidgin_gentoo.so
