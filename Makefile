default::
	cd src;\
	make || exit 1;\
	
install::

clean::
	rm -rf filesystem;\
	rm -rf ./lib/*.a;\
	cd src;\
	make clean || exit 1;\
