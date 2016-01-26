make clean
write_speed_test="CFLAG=-DWRITE_SPEED_TEST name=write_speed_test"
make params=${write_speed_test}
mv ./src/fileapp_* ../
make clean
block_size=($((1<<10)) )
#block_size=($((1<<10)) $((1<<11)) $((1<<12)) $((1<<13)) $((1<<14)) $((1<<15)) $((1<<16)) $((1<<17)) $((1<<18)) $((1<<19)) $((1<<20)) $((1<<21)) $((1<<22)))
for size in ${block_size[@]}
do
	mul_same_test="CFLAG=-DMULTI_SAME_TEST_BLOCK=${size} name=multi_same_${size}"
	make params=${mul_same_test}
	mv ./src/fileapp_* ../
	make clean
	mul_diff_test="CFLAG=-DMULTI_DIFF_TEST_BLOCK=${size} name=multi_diff_${size}"
	make params=${mul_diff_test}
	mv ./src/fileapp_* ../
	make clean
done
