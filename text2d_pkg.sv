package text2d_pkg;
    import "DPI-C" pure function chandle text2d_open(input string filename, input string flags, input int unsigned bitdepth, input string delimiter, input int padding);
    import "DPI-C" pure function void text2d_close(input chandle handle);
    import "DPI-C" pure function void text2d_cache_clear(input chandle handle);
    import "DPI-C" pure function void text2d_cache_resize(input chandle handle, input int unsigned width, input int unsigned height);
    import "DPI-C" pure function void text2d_cache_dump(input chandle handle, input int dump_header);
    import "DPI-C" pure function int text2d_fread(input chandle handle, input int unsigned lines);
    import "DPI-C" pure function int text2d_fwrite(input chandle handle);
    import "DPI-C" context function int text2d_cache_read(input chandle handle, inout byte unsigned array[], input int cachex, input int cachey, input int unsigned read_width, input int unsigned read_height, input int unsigned stridex, input int unsigned stridey);
    import "DPI-C" pure function int text2d_cache_write(input chandle handle, input byte unsigned array[], input int cachex, input int cachey, input int unsigned write_width, input int unsigned write_height);
endpackage
