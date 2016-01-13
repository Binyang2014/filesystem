write_speed_test="CFLAG=-DWRITE_SPEED_TEST name=write_speed_test"
make ${write_speed_test}
block_size=($((1<<10)) $((1<<11)) $((1<<12)) $((1<<13)) $((1<<14)) $((1<<15)) $((1<<16)) $((1<<17)) $((1<<18)) $((1<<19)) $((1<<20)) $((1<<21)) $((1<<22)))
for size in ${block_size[@]}
do
	mul_same_test="CFLAG=-DMULTI_SAME_TEST_BLOCK=${size} name=multi_same_${size}"
	make ${mul_same_test}
	mul_diff_test="CFLAG=-DMULTI_DIFF_TEST_BLOCK=${size} name=multi_diff_${size}"
	make ${mul_diff_test}
done
