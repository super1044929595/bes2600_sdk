TARGET_PRJ=stdf_project
make T=$TARGET_PRJ REL_BUILD=1 -j all lst
python3 ./tools/generate_py3_crc32_of_image.py ./out/$TARGET_PRJ/${TARGET_PRJ}.bin

./tools/build_compressed_ota.sh out/$TARGET_PRJ/${TARGET_PRJ}.bin out/$TARGET_PRJ/${TARGET_PRJ}_compressed_ota.bin
python3 ./tools/generate_py3_crc32_of_image.py out/$TARGET_PRJ/${TARGET_PRJ}_compressed_ota.bin

bin_path=out/${TARGET_PRJ}/${TARGET_PRJ}.bin
map_path=out/${TARGET_PRJ}/${TARGET_PRJ}.map
list_path=out/${TARGET_PRJ}/${TARGET_PRJ}.lst
elf_path=out/${TARGET_PRJ}/${TARGET_PRJ}.elf
if [ -f ${bin_path} ]; then
###############################################################################
echo -n -e "\n\033[32m--------------------------\033[0m"
echo -n -e "\n\033[34mapp bin size and ram left:\033[0m\n\n"
file_b=$(size ${bin_path})
echo -n -e "\033[33mAPP BIN(KB)     : \033[0m"
echo "scale=3; $file_b/1024" |bc
ramcpx_size=$((`grep "__free_ramcpx =" ${map_path} |awk '{print $1}'`))
echo -n -e "\033[33mFREE RAMCPX(KB) :\033[0m "
echo "scale=3; $ramcpx_size/1024" |bc
ramcp_size=$((`grep "__free_ramcp =" ${map_path} |awk '{print $1}'`))
echo -n -e "\033[33mFREE RAMCP(KB)  :\033[0m "
echo "scale=3; $ramcp_size/1024" |bc
ram_size=$((`grep "__free_ram =" ${map_path} |awk '{print $1}'`))
echo -n -e "\033[33mFREE RAM(KB)    :\033[0m "
echo "scale=3; $ram_size/1024" |bc
echo -n -e "\033[32m--------------------------\033[0m\n"
else
echo -e "\033[31m Bulid Error!!\033[0m \n"
################################################################################
fi

